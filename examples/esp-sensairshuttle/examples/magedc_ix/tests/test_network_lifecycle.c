/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* Host scheduler for the actual wifi_stream.c functions extracted by test_network.py. */
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define HTTPD_RESP_USE_STRLEN -1
#define SHUT_RDWR 2
#define portMAX_DELAY UINT32_MAX
#define pdMS_TO_TICKS(ms) (ms)
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t ready;
    unsigned count;
} semaphore_t;
typedef semaphore_t *SemaphoreHandle_t;
typedef pthread_t TaskHandle_t;
typedef struct { bool alive; } server_t;
typedef server_t *httpd_handle_t;
typedef struct { int fd; } httpd_req_t;
static httpd_handle_t s_httpd;
static SemaphoreHandle_t s_http_state_lock, s_http_lifecycle_lock, s_http_refs_idle, s_http_closed;
static unsigned s_http_queue_users;
static TaskHandle_t s_http_task;
static atomic_bool s_sse_work_pending, s_sse_connected, s_http_stopping;
static httpd_req_t *s_sse_req;
static int s_sse_client_fd = -1;
static atomic_bool s_client_ever_connected;
static _Atomic uint32_t s_sse_disconnect_ms;
static atomic_uint now_ms;
static pthread_t worker;
static atomic_bool block_send, send_entered, block_queue, queue_entered, fail_queue, worker_stop;
static atomic_uint completed, sends;
static SemaphoreHandle_t send_release, queue_release, opened, direct_done;
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_ready = PTHREAD_COND_INITIALIZER;
static struct { void (*fn)(void *); void *arg; } jobs[8];
static unsigned read_idx, write_idx;

static SemaphoreHandle_t sem_new(unsigned count)
{
    SemaphoreHandle_t s = calloc(1, sizeof(*s));
    assert(s);
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->ready, NULL);
    s->count = count;
    return s;
}
static int xSemaphoreTake(SemaphoreHandle_t s, uint32_t timeout)
{
    pthread_mutex_lock(&s->mutex);
    while (!s->count && timeout) {
        pthread_cond_wait(&s->ready, &s->mutex);
    }
    int ok = s->count != 0;
    if (ok) {
        --s->count;
    }
    pthread_mutex_unlock(&s->mutex);
    return ok;
}
static void xSemaphoreGive(SemaphoreHandle_t s)
{
    pthread_mutex_lock(&s->mutex);
    s->count = 1;
    pthread_cond_broadcast(&s->ready);
    pthread_mutex_unlock(&s->mutex);
}
static void vTaskDelay(uint32_t ms)
{
    atomic_fetch_add(&now_ms, ms);
    const struct timespec delay = {.tv_nsec = 100000};
    nanosleep(&delay, NULL);
}
static uint32_t esp_log_timestamp(void) { return atomic_load(&now_ms); }
static TaskHandle_t xTaskGetCurrentTaskHandle(void) { return pthread_self(); }
static const char *esp_err_to_name(esp_err_t err) { (void)err; return "test"; }
static void wifi_mark_control_activity(void) {}
static int mock_shutdown(int fd, int how) { (void)fd; (void)how; return 0; }
#define shutdown mock_shutdown
static int httpd_req_async_handler_begin(httpd_req_t *req, httpd_req_t **out)
{
    *out = malloc(sizeof(**out));
    assert(*out);
    **out = *req;
    return ESP_OK;
}
static int httpd_req_async_handler_complete(httpd_req_t *req)
{
    assert(pthread_equal(worker, pthread_self()));
    assert(req && req == s_sse_req);
    atomic_fetch_add(&completed, 1);
    free(req);
    return ESP_OK;
}
static int httpd_req_to_sockfd(httpd_req_t *req) { return req->fd; }
static int httpd_resp_set_type(httpd_req_t *req, const char *type) { (void)req; (void)type; return 0; }
static int httpd_resp_set_hdr(httpd_req_t *req, const char *key, const char *value)
{ (void)req; (void)key; (void)value; return 0; }
static int httpd_resp_send_chunk(httpd_req_t *req, const char *text, int len)
{
    (void)len;
    assert(pthread_equal(worker, pthread_self()));
    assert(req && req == s_sse_req);
    if (text[0] != ':' && atomic_load(&block_send)) {
        atomic_store(&send_entered, true);
        xSemaphoreTake(send_release, portMAX_DELAY);
    }
    assert(req->fd == 7); /* ASan catches release during the simulated socket wait. */
    atomic_fetch_add(&sends, 1);
    return ESP_OK;
}
static int httpd_queue_work(httpd_handle_t server, void (*fn)(void *), void *arg)
{
    assert(server && server->alive);
    assert(!pthread_equal(worker, pthread_self())); /* HTTP callers must send directly. */
    if (atomic_load(&fail_queue)) {
        return ESP_FAIL;
    }
    if (atomic_load(&block_queue)) {
        atomic_store(&queue_entered, true);
        xSemaphoreTake(queue_release, portMAX_DELAY);
        assert(server->alive); /* Queue pin must outlive detach. */
    }
    pthread_mutex_lock(&queue_lock);
    assert(write_idx - read_idx < 8);
    jobs[write_idx % 8].fn = fn;
    jobs[write_idx++ % 8].arg = arg;
    pthread_cond_signal(&queue_ready);
    pthread_mutex_unlock(&queue_lock);
    return ESP_OK;
}
static int httpd_stop(httpd_handle_t server)
{
    assert(server->alive && !s_sse_req && !atomic_load(&s_sse_work_pending));
    pthread_mutex_lock(&queue_lock);
    assert(read_idx == write_idx);
    server->alive = false;
    atomic_store(&worker_stop, true);
    pthread_cond_signal(&queue_ready);
    pthread_mutex_unlock(&queue_lock);
    pthread_join(worker, NULL);
    return ESP_OK;
}

#include "network_lifecycle.inc"

static void *http_worker(void *unused)
{
    (void)unused;
    while (true) {
        pthread_mutex_lock(&queue_lock);
        while (read_idx == write_idx && !atomic_load(&worker_stop)) {
            pthread_cond_wait(&queue_ready, &queue_lock);
        }
        if (atomic_load(&worker_stop)) {
            pthread_mutex_unlock(&queue_lock);
            return NULL;
        }
        void (*fn)(void *) = jobs[read_idx % 8].fn;
        void *arg = jobs[read_idx++ % 8].arg;
        pthread_mutex_unlock(&queue_lock);
        fn(arg);
    }
}
static void wait_flag(atomic_bool *flag)
{
    for (unsigned i = 0; !atomic_load(flag); i++) {
        assert(i < 30000);
        vTaskDelay(1);
    }
}
static void open_sse(void *unused)
{
    (void)unused;
    httpd_req_t req = {.fd = 7};
    assert(sse_events_handler(&req) == ESP_OK);
    xSemaphoreGive(opened);
}
static void direct_send(void *unused)
{
    (void)unused;
    wifi_stream_send_text("snapshot");
    xSemaphoreGive(direct_done);
}
static void *send_thread(void *unused) { (void)unused; wifi_stream_send_text("sample"); return NULL; }
static void *stop_thread(void *unused) { (void)unused; stop_http_server(); return NULL; }

int main(void)
{
    s_http_state_lock = sem_new(1);
    s_http_lifecycle_lock = sem_new(1);
    s_http_refs_idle = sem_new(1);
    s_http_closed = sem_new(0);
    send_release = sem_new(0);
    queue_release = sem_new(0);
    opened = sem_new(0);
    direct_done = sem_new(0);
    for (unsigned scenario = 0; scenario < 3; scenario++) {
        server_t server = {.alive = true};
        s_httpd = &server;
        atomic_store(&s_http_stopping, false);
        atomic_store(&worker_stop, false);
        atomic_store(&block_send, false);
        atomic_store(&block_queue, false);
        atomic_store(&queue_entered, false);
        atomic_store(&send_entered, false);
        atomic_store(&completed, 0);
        pthread_create(&worker, NULL, http_worker, NULL);
        httpd_queue_work(&server, open_sse, NULL);
        xSemaphoreTake(opened, portMAX_DELAY);
        httpd_queue_work(&server, direct_send, NULL);
        xSemaphoreTake(direct_done, portMAX_DELAY);
        if (scenario == 0) {
            atomic_store(&block_send, true);
            wifi_stream_send_text("sample");
            wait_flag(&send_entered);
            pthread_t stopping;
            pthread_create(&stopping, NULL, stop_thread, NULL);
            wait_flag(&s_http_stopping);
            assert(atomic_load(&completed) == 0 && server.alive);
            wifi_stream_send_text("late sample");
            atomic_store(&block_send, false);
            xSemaphoreGive(send_release);
            pthread_join(stopping, NULL);
        } else if (scenario == 1) {
            atomic_store(&block_queue, true);
            pthread_t sending, stopping;
            pthread_create(&sending, NULL, send_thread, NULL);
            wait_flag(&queue_entered);
            pthread_create(&stopping, NULL, stop_thread, NULL);
            wait_flag(&s_http_stopping);
            assert(server.alive);
            atomic_store(&block_queue, false);
            xSemaphoreGive(queue_release);
            pthread_join(sending, NULL);
            pthread_join(stopping, NULL);
        } else {
            atomic_store(&fail_queue, true);
            stop_http_server();
            assert(s_httpd == &server && server.alive && atomic_load(&completed) == 0);
            atomic_store(&fail_queue, false);
            httpd_queue_work(&server, open_sse, NULL);
            xSemaphoreTake(opened, portMAX_DELAY);
            assert(atomic_load(&completed) == 1); /* Replacing a client completes once. */
            stop_http_server();
        }
        assert(!server.alive && !s_httpd && !s_sse_req);
        assert(atomic_load(&completed) == (scenario == 2 ? 2 : 1));
        wifi_stream_send_text("after stop"); /* No use of the closed server handle. */
    }
    SemaphoreHandle_t semaphores[] = {s_http_state_lock, s_http_lifecycle_lock, s_http_refs_idle,
                                     s_http_closed, send_release, queue_release, opened, direct_done};
    for (size_t i = 0; i < sizeof(semaphores) / sizeof(semaphores[0]); ++i) {
        pthread_mutex_destroy(&semaphores[i]->mutex);
        pthread_cond_destroy(&semaphores[i]->ready);
        free(semaphores[i]);
    }
    puts("SSE lifecycle: in-flight send, queue pin, HTTP direct send, replacement and stop retry passed");
    return 0;
}
