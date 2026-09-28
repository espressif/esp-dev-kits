/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* The production parser/handler runs against real cJSON and bounded fake I/O. */
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "zone_tone.h"
#include "audio_tuning.h"
#include "speaker_gesture.h"

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_ARG 2
#define HTTPD_SOCK_ERR_TIMEOUT -3
typedef struct { size_t content_len; } httpd_req_t;
static uint32_t now_ms;
static const char *body;
static size_t offset;
static unsigned reads, gesture_writes, enabled_writes, tone_writes;
static bool persist_ok = true;
static int receive_mode;
static char status[64], response[128];
static zone_tone_config_t tone = {50, 500, 10, 30, 10, 10};
static audio_gate_config_t gate = {100, 0.2f, 200};

static uint32_t esp_log_timestamp(void) { return now_ms; }
static void set_cors_headers(httpd_req_t *req) { (void)req; }
static int httpd_resp_set_status(httpd_req_t *req, const char *value)
{ (void)req; snprintf(status, sizeof(status), "%s", value); return ESP_OK; }
static int httpd_resp_set_type(httpd_req_t *req, const char *value) { (void)req; (void)value; return ESP_OK; }
static int httpd_resp_sendstr(httpd_req_t *req, const char *value)
{ (void)req; snprintf(response, sizeof(response), "%s", value); return ESP_OK; }
static int httpd_req_recv(httpd_req_t *req, char *out, size_t len)
{
    (void)req;
    ++reads;
    assert(reads <= 6); /* A peer retaining an incomplete body must not loop forever. */
    if (receive_mode == 1) {
        now_ms += 1000;
        return HTTPD_SOCK_ERR_TIMEOUT;
    }
    if (receive_mode == 2) {
        now_ms += 1100;
        len = 1;
    } else {
        now_ms += 20;
    }
    memcpy(out, body + offset, len);
    offset += len;
    return (int)len;
}
void zone_tone_get_config(zone_tone_config_t *out) { *out = tone; }
void zone_tone_set_config(const zone_tone_config_t *cfg) { tone = *cfg; ++tone_writes; }
void zone_tone_reset_config(void) {}
void zone_tone_play_test(int zone) { assert(zone >= 1 && zone <= 9); }
void zone_tone_set_enabled(bool enabled) { (void)enabled; ++enabled_writes; }
void audio_gate_get_config(audio_gate_config_t *out) { *out = gate; }
void audio_gate_set_config(const audio_gate_config_t *cfg) { gate = *cfg; }
void audio_gate_reset_config(void) {}
bool speaker_gesture_set_sequence(const int *seq, size_t len)
{
    assert(len >= 2 && len <= 8 && seq[0] >= 1 && seq[0] <= 9);
    if (persist_ok) {
        ++gesture_writes;
    }
    return persist_ok;
}
static void web_portal_push_audio_config(void) {}
static void web_portal_push_speaker_gesture_config(void) {}
esp_err_t web_portal_handle_control_text(const char *text);

#include "network_control.inc"

static int request(const char *text, int mode)
{
    body = text;
    offset = reads = now_ms = 0;
    receive_mode = mode;
    strcpy(status, "200 OK");
    response[0] = '\0';
    httpd_req_t req = {.content_len = strlen(text)};
    return audio_post_handler(&req);
}

int main(void)
{
    const char *set = "{\"t\" : \"speaker_gesture_set\",\"len\":2,\"s1\":1,\"s2\":9}";
    assert(request(set, 0) == ESP_OK);
    assert(gesture_writes == 1 && strcmp(response, "{\"ok\":true}") == 0);
    assert(request("{\"t\":\"speaker_gesture_set\",\"len\":9}", 0) == ESP_OK);
    assert(strncmp(status, "400", 3) == 0 && gesture_writes == 1);
    const char *invalid[] = {
        "{\"t\":\"speaker_gesture_set\",\"len\":2.5,\"s1\":1,\"s2\":9}",
        "{\"t\":\"speaker_gesture_set\",\"len\":2,\"s1\":0,\"s2\":9}",
        "{\"t\":\"audio_set\",\"enabled\":false,\"volume\":1e309}",
        "{\"t\":\"audio_set\",\"gate_cooldown\":4294967296}",
        "{\"t\":\"audio_set\",\"gate_conf\":-0.1}",
        "{\"t\":\"audio_get\"} trailing",
        "{\"t\":\"unsupported\"}",
        "{\"t\":\"audio_set\",\"volume\":NaN}",
    };
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        request(invalid[i], 0);
        assert(strncmp(status, "400", 3) == 0);
    }
    assert(gesture_writes == 1 && enabled_writes == 0 && tone_writes == 0);
    persist_ok = false;
    request(set, 0);
    assert(strncmp(status, "500", 3) == 0 && strstr(response, "apply_failed"));
    assert(gesture_writes == 1);
    assert(request("{\"t\":\"audio_get\"}", 1) == ESP_FAIL);
    assert(reads == 5 && now_ms == 5000 && strncmp(status, "408", 3) == 0);
    assert(request("{\"t\":\"audio_set\",\"volume\":75}", 2) == ESP_FAIL);
    assert(reads == 5 && now_ms == 5500 && strncmp(status, "408", 3) == 0 && tone_writes == 0);
    request("{\"t\":\"audio_set\",\"volume\":75,\"enabled\":true}", 0);
    assert(strncmp(status, "200", 3) == 0 && tone.volume_pct == 75 && enabled_writes == 1);
    const char *commands[] = {"audio_get", "speaker_gesture_get", "audio_reset", "audio_test"};
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        char text[80];
        snprintf(text, sizeof(text), "{\"t\":\"%s\"}", commands[i]);
        request(text, 0);
        assert(strncmp(status, "200", 3) == 0 && strcmp(response, "{\"ok\":true}") == 0);
    }
    httpd_req_t oversized = {.content_len = 512};
    reads = 0;
    assert(audio_post_handler(&oversized) == ESP_FAIL && reads == 0);
    assert(strncmp(status, "400", 3) == 0);
    puts("HTTP control: real JSON validation, persistence errors and stalled/drip-fed body deadlines passed");
    return 0;
}
