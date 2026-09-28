/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int esp_err_t;
typedef uint32_t EventBits_t;
typedef void *esp_event_base_t;
typedef void *SemaphoreHandle_t;
typedef struct {
    struct {
        uint8_t ssid[32], password[64], bssid[6];
        struct { int authmode; } threshold;
        bool bssid_set;
        int channel, listen_interval;
    } sta;
} wifi_config_t;
typedef struct { char cc[3]; int schan, nchan, policy; } wifi_country_t;
typedef struct { uint32_t generation, delay_ms; } wifi_auto_connect_args_t;
#define ESP_OK 0
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define portMAX_DELAY UINT32_MAX
#define pdMS_TO_TICKS(ms) (ms)
#define pdPASS 1
#define WIFI_CONNECTED_BIT 1
#define WIFI_FAIL_BIT 2
#define WIFI_CONNECT_TIMEOUT_MS 30000
#define WIFI_RECONNECT_COOLDOWN_MS 15000
#define WIFI_MAX_RETRY 8
#define WIFI_DEFAULT_SSID ""
#define WIFI_DEFAULT_PASS ""
#define WIFI_STA_LISTEN_INTERVAL 10
#define WIFI_AUTH_WPA2_PSK 2
#define WIFI_AUTH_OPEN 0
#define WIFI_MODE_STA 1
#define WIFI_IF_STA 0
#define WIFI_PS_MAX_MODEM 2
#define WIFI_COUNTRY_POLICY_AUTO 0
#define WIFI_PWR_CONNECTING 1
#define WIFI_PWR_STOPPED 2
#define WIFI_PWR_AUTOSLEEP 3
#define WIFI_EVENT ((void *)1)
#define IP_EVENT ((void *)2)
#define WIFI_EVENT_STA_START 1
#define WIFI_EVENT_STA_DISCONNECTED 2
#define IP_EVENT_STA_GOT_IP 3

static uint32_t bits, s_wifi_generation, s_watchdog_token;
static uint32_t s_wifi_ready_ms, s_last_control_activity_ms, s_sse_disconnect_ms;
static uint32_t *s_wifi_events = &bits;
static SemaphoreHandle_t s_wifi_lock = (void *)1;
static bool s_wifi_started, s_connect_watchdog_running, s_wifi_auto_task_running;
static bool s_calibrating, s_sleep_preparing, s_fast_connect_applied, s_client_ever_connected, s_ps_mode_applied;
static int s_retry_count, s_wifi_power_state;
static char s_sta_ip[16], s_stored_ssid[33], s_stored_pass[65];
static bool have_credentials, clear_on_delay;
static char saved_ssid[33], saved_pass[65];
static unsigned starts, connects, cache_clears, task_count;
static wifi_config_t actual_config;
static struct { void (*fn)(void *); void *arg; } tasks[32];

static bool wifi_init_sta(const char *, const char *);
static void start_connect_watchdog(void);
static void wifi_schedule_auto_connect(uint32_t, const char *);
static void handle_serial_command(char *);
bool wifi_stream_connect(const char *, const char *);
static void wifi_lock_take(void) {}
static void wifi_lock_give(void) {}
static void xSemaphoreTakeRecursive(SemaphoreHandle_t s, uint32_t wait) { (void)s; (void)wait; }
static void xSemaphoreGiveRecursive(SemaphoreHandle_t s) { (void)s; }
static void xEventGroupClearBits(uint32_t *group, uint32_t mask) { *group &= ~mask; }
static void xEventGroupSetBits(uint32_t *group, uint32_t mask) { *group |= mask; }
static uint32_t xEventGroupGetBits(uint32_t *group) { return *group; }
static uint32_t esp_log_timestamp(void) { return 100; }
static const char *esp_err_to_name(int err) { (void)err; return "test"; }
static void log_wifi_status(const char *status) { (void)status; }
static void log_wifi_ip(void) {}
static void wifi_pm_release_lock(void) {}
static void wifi_mdns_stop(void) {}
static void wifi_mdns_start(void) {}
static void stop_http_server(void) {}
static int start_http_server(void) { return ESP_OK; }
static void save_wifi_fast_connect_info(void) {}
static void wifi_enter_power_state(int state, const char *reason) { (void)reason; s_wifi_power_state = state; }
static void wifi_set_ps_mode(int mode) { (void)mode; }
static int esp_wifi_disconnect(void) { return ESP_OK; }
static int esp_wifi_stop(void) { return ESP_OK; }
static int esp_wifi_connect(void) { ++connects; return ESP_OK; }
static int esp_wifi_start(void) { ++starts; return ESP_OK; }
static int esp_wifi_set_mode(int mode) { (void)mode; return ESP_OK; }
static int esp_wifi_set_config(int mode, const wifi_config_t *config)
{ (void)mode; actual_config = *config; return ESP_OK; }
static int esp_wifi_get_config(int mode, wifi_config_t *config)
{ (void)mode; *config = actual_config; return ESP_OK; }
static int esp_wifi_set_country(const wifi_country_t *country) { (void)country; return ESP_OK; }
static bool wifi_stack_init_once(void) { return true; }
static void apply_fast_connect(wifi_config_t *config) { (void)config; }
static void clear_wifi_fast_connect_cache(void) { ++cache_clears; }
static bool save_wifi_credentials(const char *ssid, const char *pass)
{
    snprintf(saved_ssid, sizeof(saved_ssid), "%s", ssid);
    snprintf(saved_pass, sizeof(saved_pass), "%s", pass);
    have_credentials = true;
    return true;
}
static bool load_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len)
{
    ssid[0] = pass[0] = '\0';
    if (!have_credentials) {
        return false;
    }
    snprintf(ssid, ssid_len, "%s", saved_ssid);
    snprintf(pass, pass_len, "%s", saved_pass);
    return true;
}
static void clear_wifi_credentials(void) { have_credentials = false; }
static void app_request_recalibration(void) {}
static void web_portal_handle_control_text(const char *text) { (void)text; }
static void vTaskDelay(uint32_t ms)
{
    (void)ms;
    if (clear_on_delay) {
        clear_on_delay = false;
        char clear[] = "WIFI_CLEAR";
        handle_serial_command(clear);
    }
}
static void vTaskDelete(void *task) { (void)task; }
static int xTaskCreate(void (*fn)(void *), const char *name, unsigned stack, void *arg, int priority, void *out)
{
    (void)name; (void)stack; (void)priority; (void)out;
    assert(task_count < 32);
    tasks[task_count].fn = fn;
    tasks[task_count++].arg = arg;
    return pdPASS;
}

#include "network_wifi.inc"

int main(void)
{
    char ssid[34], pass[66];
    memset(ssid, 's', 32); ssid[32] = '\0';
    memset(pass, 'a', 64); pass[64] = '\0';
    assert(wifi_credentials_valid(ssid, pass));
    assert(wifi_stream_connect(ssid, pass));
    assert(memcmp(actual_config.sta.ssid, ssid, 32) == 0);
    assert(memcmp(actual_config.sta.password, pass, 64) == 0);
    ssid[32] = 's'; ssid[33] = '\0';
    assert(!wifi_credentials_valid(ssid, pass));
    assert(!wifi_stream_connect(ssid, pass));
    ssid[32] = '\0'; pass[63] = 'g';
    assert(!wifi_credentials_valid(ssid, pass));
    assert(!wifi_credentials_valid(ssid, "short"));
    assert(wifi_stream_connect(" spaced ssid ", " spaced pass "));
    assert(strcmp((char *)actual_config.sta.ssid, " spaced ssid ") == 0);
    assert(strcmp((char *)actual_config.sta.password, " spaced pass ") == 0);
    assert(wifi_credentials_valid(ssid, ""));

    /* Existing watchdogs cannot affect a newer connection attempt. */
    s_watchdog_token = 100;
    s_connect_watchdog_running = true;
    unsigned before = starts;
    wifi_connect_watchdog_task((void *)(uintptr_t)99);
    assert(s_connect_watchdog_running && starts == before && s_wifi_started);

    /* Disconnect immediately leaves CONNECTED and retries without stale BSSID. */
    bits = WIFI_CONNECTED_BIT;
    s_fast_connect_applied = true;
    actual_config.sta.bssid_set = true;
    actual_config.sta.channel = 36;
    s_connect_watchdog_running = false;
    unsigned old_tasks = task_count;
    wifi_event_handler(NULL, WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, NULL);
    assert(!(bits & WIFI_CONNECTED_BIT) && s_connect_watchdog_running && task_count == old_tasks + 1);
    assert(!actual_config.sta.bssid_set && actual_config.sta.channel == 0 && !s_fast_connect_applied);
    assert(connects > 0 && cache_clears > 0);
    uint32_t old_token = s_watchdog_token;
    wifi_event_handler(NULL, IP_EVENT, IP_EVENT_STA_GOT_IP, NULL);
    assert((bits & WIFI_CONNECTED_BIT) && !s_connect_watchdog_running && s_watchdog_token != old_token);

    /* WIFI_CLEAR happens while a delayed auto-connect is asleep. */
    strcpy(s_stored_ssid, "cached"); strcpy(s_stored_pass, "password");
    wifi_auto_connect_args_t *work = malloc(sizeof(*work));
    *work = (wifi_auto_connect_args_t) {s_wifi_generation, 15000};
    s_wifi_auto_task_running = true;
    clear_on_delay = true;
    before = starts;
    wifi_auto_connect_task(work);
    assert(starts == before && !have_credentials && !s_wifi_started && !s_wifi_auto_task_running);
    assert(s_stored_ssid[0] == '\0' && s_stored_pass[0] == '\0');

    /* An old delayed task must not clear the running marker of a new one. */
    work = malloc(sizeof(*work));
    *work = (wifi_auto_connect_args_t) {s_wifi_generation - 1, 1000};
    s_wifi_auto_task_running = true;
    wifi_auto_connect_task(work);
    assert(s_wifi_auto_task_running);
    puts("Wi-Fi: full-length credentials, stale watchdogs, BSSID fallback, disconnect state and CLEAR cancellation passed");
    return 0;
}
