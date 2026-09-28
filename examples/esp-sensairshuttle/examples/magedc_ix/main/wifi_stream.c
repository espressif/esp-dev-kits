/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "wifi_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "mdns.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#if CONFIG_PM_ENABLE
#include "esp_pm.h"
#endif
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "web_portal.h"
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED
#include "driver/usb_serial_jtag.h"
#endif

#if !ENABLE_WIFI

void wifi_stream_init(void)
{
    ESP_LOGI("WIFI_STREAM", "Wi-Fi disabled (ENABLE_WIFI=0)");
}

bool wifi_stream_connect(const char *ssid, const char *password)
{
    (void)ssid;
    (void)password;
    return false;
}

void wifi_stream_send_text(const char *text)
{
    (void)text;
}

bool wifi_stream_get_sta_ip(char *buf, size_t len)
{
    (void)buf;
    (void)len;
    return false;
}

bool wifi_stream_http_active(void)
{
    return false;
}

bool wifi_stream_inhibits_sleep(void)
{
    return false;
}

void wifi_stream_prepare_sleep(void) {}

bool wifi_stream_is_connected(void)
{
    return false;
}

#else

static const char *TAG = "WIFI_STREAM";

/* 由 main.cpp 实现：请求重新进行 9 点磁校准（外壳无按键，改用串口触发） */
extern void app_request_recalibration(void);

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define NVS_NS_WIFI "wifi_cfg"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASS "pass"
#define NVS_KEY_CHAN "chan"
#define NVS_KEY_BSSID "bssid"
#define CMD_BUF_SIZE 256
#define WIFI_CONNECT_TIMEOUT_MS 30000
#define WIFI_MAX_RETRY 8
/* 开源默认留空：固件不内置任何账号，首次需经串口 WIFI_CFG 配网（凭据存 NVS） */
#define WIFI_DEFAULT_SSID ""
#define WIFI_DEFAULT_PASS ""
/* STA DHCP 主机名（便于在路由器租约里识别设备；非 mDNS） */
#define WIFI_STA_HOSTNAME "magedc"
/* DTIM 监听间隔（单位：AP beacon 间隔，通常 100ms）。
 * 仅在 WIFI_PS_MAX_MODEM 下生效：station 每 N 个 beacon 才唤醒收包，
 * 配合 Auto Light Sleep 显著降低空闲功耗，代价是下行（HTTP POST 命令）延迟最高约 N*100ms。 */
#define WIFI_STA_LISTEN_INTERVAL 10
/* Web 控制活跃窗口（毫秒）：窗口内禁止自动 Light Sleep，保障操控跟手 */
#define WIFI_ACTIVE_WINDOW_MS 4000
/* Wi-Fi 就绪后给网页连接的配对窗口；仅在“尚无任何客户端连接过”时生效。
 * 超过该时长仍无网页(SSE)客户端接入，则允许进深睡省电。
 * 太长=空联网白耗电；太短=可能在你打开网页前就睡了。默认 60s 为平衡值。 */
#define WIFI_WEB_PAIRING_GRACE_MS (60 * 1000)
/* SSE 客户端断开后的短暂保活（毫秒）：容忍页面刷新/EventSource 自动重连，避免来回抖动 */
#define WIFI_CLIENT_LINGER_MS 10000
/* 运行态可选无控断链停机（毫秒）；0=禁用（推荐先禁用，仅用 Modem Sleep） */
#define WIFI_IDLE_STOP_MS 0
/* autosleep 轮询节拍（毫秒） */
#define WIFI_AUTOSLEEP_TICK_MS 500
/* 常连策略：快速重试耗尽后，停射频冷却再重连，降低离线发热 */
#define WIFI_RECONNECT_COOLDOWN_MS 15000

static EventGroupHandle_t s_wifi_events;
static SemaphoreHandle_t s_wifi_lock;
static uint32_t s_wifi_generation;
static uint32_t s_watchdog_token;
static bool s_fast_connect_applied;
static httpd_handle_t s_httpd = NULL;
/* Only the HTTP task owns the asynchronous request. External callers pin the
 * server while queueing work; stop detaches it before waiting for those pins. */
static SemaphoreHandle_t s_http_state_lock;
static SemaphoreHandle_t s_http_lifecycle_lock;
static SemaphoreHandle_t s_http_refs_idle;
static SemaphoreHandle_t s_http_closed;
static atomic_bool s_http_stopping;
static unsigned s_http_queue_users;
static TaskHandle_t s_http_task;
static atomic_bool s_sse_work_pending;
static atomic_bool s_sse_connected;
static httpd_req_t *s_sse_req = NULL;
static int s_sse_client_fd = -1;
static atomic_bool s_client_ever_connected = false;
static _Atomic uint32_t s_sse_disconnect_ms = 0;
static esp_netif_t *s_sta_netif = NULL;
static bool s_wifi_started = false;
static bool s_wifi_stack_ready = false;
static bool s_sta_netif_ready = false;
static bool s_connect_watchdog_running = false;
static bool s_wifi_power_task_started = false;
static bool s_wifi_auto_task_running = false;
static bool s_mdns_started = false;
static bool s_sleep_preparing = false;
static int s_retry_count = 0;
static char s_sta_ip[16] = {0};
static char s_stored_ssid[33] = {0};
static char s_stored_pass[65] = {0};
static uint32_t s_wifi_ready_ms = 0;
static _Atomic uint32_t s_last_control_activity_ms = 0;
/* 9 点校准进行中：禁止 Auto Light Sleep，保证主循环采样节奏稳定（否则稳定窗口难收敛、卡住不推进） */
static volatile bool s_calibrating = false;
static wifi_ps_type_t s_ps_mode = WIFI_PS_MAX_MODEM;
static bool s_ps_mode_applied = false;
#if CONFIG_PM_ENABLE
static bool s_pm_ready = false;
static esp_pm_lock_handle_t s_pm_no_ls_lock = NULL;
static bool s_pm_lock_held = false;
#endif

typedef enum {
    WIFI_PWR_UNKNOWN = 0,
    WIFI_PWR_CONNECTING,
    WIFI_PWR_ACTIVE,
    WIFI_PWR_AUTOSLEEP,
    WIFI_PWR_SLEEP_PREP,
    WIFI_PWR_STOPPED,
} wifi_power_state_t;

static wifi_power_state_t s_wifi_power_state = WIFI_PWR_UNKNOWN;

static void stop_http_server(void);
static void wifi_stop_internal(void);
static bool save_wifi_credentials(const char *ssid, const char *password);
static void clear_wifi_fast_connect_cache(void);
static bool load_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
static bool wifi_stack_init_once(void);
static bool wifi_init_sta(const char *ssid, const char *password);
static void wifi_configure_band(void);
static void start_connect_watchdog(void);
static void wifi_update_power_state(void);
static void wifi_enter_power_state(wifi_power_state_t state, const char *reason);
static void wifi_pm_init(void);
static void wifi_pm_set_lock(bool hold, const char *reason);
static void wifi_pm_release_lock(void);
static void wifi_auto_connect_task(void *arg);
static void wifi_schedule_auto_connect(uint32_t delay_ms, const char *reason);
static void sse_close_client(void);
static void wifi_mdns_start(void);
static void wifi_mdns_stop(void);

static void log_wifi_status(const char *status)
{
    ESP_LOGI(TAG, "WIFI_STATUS:%s", status);
}

static void log_wifi_ip(void)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        snprintf(s_sta_ip, sizeof(s_sta_ip), IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "WIFI_IP:" IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "WIFI_READY:" IPSTR, IP2STR(&ip_info.ip));
    }
}

static void wifi_pm_set_lock(bool hold, const char *reason)
{
#if CONFIG_PM_ENABLE
    if (!s_pm_ready || s_pm_no_ls_lock == NULL) {
        return;
    }
    if (hold) {
        if (s_pm_lock_held) {
            return;
        }
        if (esp_pm_lock_acquire(s_pm_no_ls_lock) == ESP_OK) {
            s_pm_lock_held = true;
            ESP_LOGI(TAG, "PM_LOCK:ACQUIRE%s%s", reason ? " @" : "", reason ? reason : "");
        }
        return;
    }

    if (!s_pm_lock_held) {
        return;
    }
    if (esp_pm_lock_release(s_pm_no_ls_lock) == ESP_OK) {
        s_pm_lock_held = false;
        ESP_LOGI(TAG, "PM_LOCK:RELEASE%s%s", reason ? " @" : "", reason ? reason : "");
    }
#else
    (void)hold;
    (void)reason;
#endif
}

static void wifi_pm_release_lock(void)
{
    wifi_pm_set_lock(false, "release-all");
}

static void wifi_pm_init(void)
{
#if CONFIG_PM_ENABLE
    if (s_pm_ready) {
        return;
    }

    int cpu_freq_mhz = 240;
#ifdef CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ
    cpu_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
#endif
    esp_pm_config_t pm_cfg = {
        .max_freq_mhz = cpu_freq_mhz,
        .min_freq_mhz = cpu_freq_mhz,
        .light_sleep_enable = true,
    };
    esp_err_t err = esp_pm_configure(&pm_cfg);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "esp_pm_configure failed: %s", esp_err_to_name(err));
        return;
    }

    if (s_pm_no_ls_lock == NULL) {
        err = esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "wifi_ws", &s_pm_no_ls_lock);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "esp_pm_lock_create(NO_LIGHT_SLEEP) failed: %s", esp_err_to_name(err));
            return;
        }
    }

    s_pm_ready = true;
    ESP_LOGI(TAG, "PM auto light sleep ready (cpu=%dMHz)", cpu_freq_mhz);

    /* 冷启动校准：start_zone_wizard_calibration() 早于本函数被调用，
     * s_calibrating 已置位但当时锁还没创建，这里补获取，确保整个向导期间禁睡。 */
    if (s_calibrating) {
        wifi_pm_set_lock(true, "calib-pm-init");
    }
#else
    ESP_LOGI(TAG, "PM disabled in sdkconfig");
#endif
}

static const char *wifi_power_state_name(wifi_power_state_t state)
{
    switch (state) {
    case WIFI_PWR_CONNECTING:
        return "CONNECTING";
    case WIFI_PWR_ACTIVE:
        return "ACTIVE";
    case WIFI_PWR_AUTOSLEEP:
        return "AUTOSLEEP";
    case WIFI_PWR_SLEEP_PREP:
        return "SLEEP_PREP";
    case WIFI_PWR_STOPPED:
        return "STOPPED";
    case WIFI_PWR_UNKNOWN:
    default:
        return "UNKNOWN";
    }
}

static void wifi_mark_control_activity(void)
{
    s_last_control_activity_ms = esp_log_timestamp();
}

static void wifi_lock_take(void)
{
    if (s_wifi_lock) {
        xSemaphoreTakeRecursive(s_wifi_lock, portMAX_DELAY);
    }
}

static void wifi_lock_give(void)
{
    if (s_wifi_lock) {
        xSemaphoreGiveRecursive(s_wifi_lock);
    }
}

void wifi_stream_set_calibrating(bool busy)
{
    wifi_lock_take();
    s_calibrating = busy;
    if (busy) {
        /* 校准期间彻底停 Wi-Fi：Wi-Fi 关联（DTIM/省电协调）会破坏 9 点向导的采样
         * 节奏，导致卡红灯不推进；这里复刻“无网首次烧录校准”这一已知可用条件。
         * 串口命令任务独立运行，不受影响，RECALIB 仍可用。 */
        if (s_wifi_events) {
            wifi_stop_internal();
        }
        /* 双保险：再禁 Auto Light Sleep（PM 未就绪时为 no-op，由 wifi_pm_init 补获取） */
        wifi_pm_set_lock(true, "calib");
    } else {
        /* 校准完成：放锁并自动重连 Wi-Fi（凭据从 NVS 读取） */
        wifi_pm_set_lock(false, "calib-done");
        wifi_schedule_auto_connect(1000, "calib-done");
    }
    wifi_lock_give();
}

static void wifi_set_ps_mode(wifi_ps_type_t mode)
{
    if (!s_wifi_started) {
        return;
    }
    if (s_ps_mode_applied && s_ps_mode == mode) {
        return;
    }
    if (esp_wifi_set_ps(mode) == ESP_OK) {
        s_ps_mode = mode;
        s_ps_mode_applied = true;
    }
}

static void wifi_enter_power_state(wifi_power_state_t state, const char *reason)
{
    /* 校准进行中：强制保持 ACTIVE（持有 NO_LIGHT_SLEEP 锁）。
     * 否则 got-ip/idle 等事件会调用 AUTOSLEEP 释放锁，使关联态 light sleep 把
     * 校准主循环的 I2C 读取与控制台拖死，表现为卡红灯且无日志输出。 */
    if (s_calibrating) {
        state = WIFI_PWR_ACTIVE;
    }

    if (s_wifi_power_state == state) {
        return;
    }

    if (state == WIFI_PWR_ACTIVE) {
        wifi_pm_set_lock(true, reason);
    } else {
        wifi_pm_set_lock(false, reason);
    }

    s_wifi_power_state = state;
    ESP_LOGI(TAG, "WIFI_PWR:%s%s%s", wifi_power_state_name(state), reason ? " @" : "", reason ? reason : "");
}

static void wifi_power_task(void *arg)
{
    (void)arg;
    while (1) {
        wifi_lock_take();
        wifi_update_power_state();
        wifi_lock_give();
        vTaskDelay(pdMS_TO_TICKS(WIFI_AUTOSLEEP_TICK_MS));
    }
}

static void wifi_update_power_state(void)
{
    if (!s_wifi_started || !s_wifi_events) {
        return;
    }

    if (s_calibrating) {
        wifi_enter_power_state(WIFI_PWR_ACTIVE, "calibrating");
        return;
    }

    const EventBits_t bits = xEventGroupGetBits(s_wifi_events);
    if (bits & WIFI_CONNECTED_BIT) {
        const uint32_t now = esp_log_timestamp();
        const bool control_connected = atomic_load(&s_sse_connected);
        const bool control_active = control_connected && (now - s_last_control_activity_ms <= WIFI_ACTIVE_WINDOW_MS);

        if (control_active) {
            wifi_enter_power_state(WIFI_PWR_ACTIVE, "control-active");
        } else {
            wifi_enter_power_state(WIFI_PWR_AUTOSLEEP, "idle");
        }

#if WIFI_IDLE_STOP_MS > 0
        if (!control_connected && (now - s_last_control_activity_ms) > WIFI_IDLE_STOP_MS) {
            log_wifi_status("IDLE_STOP");
            wifi_stop_internal();
            xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
            wifi_enter_power_state(WIFI_PWR_STOPPED, "idle-stop");
        }
#endif
        return;
    }

    if ((bits & WIFI_FAIL_BIT) == 0) {
        wifi_enter_power_state(WIFI_PWR_CONNECTING, "linking");
    }
}

static void wifi_stop_internal(void)
{
    ++s_wifi_generation;
    ++s_watchdog_token;
    s_wifi_auto_task_running = false;
    s_connect_watchdog_running = false;
    wifi_pm_release_lock();
    wifi_mdns_stop();
    stop_http_server();
    if (s_wifi_started) {
        s_wifi_started = false;
        esp_wifi_disconnect();
        esp_wifi_stop();
    }
    s_connect_watchdog_running = false;
    xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    s_sta_ip[0] = '\0';
    s_wifi_ready_ms = 0;
    s_last_control_activity_ms = 0;
    s_client_ever_connected = false;
    s_sse_disconnect_ms = 0;
    s_wifi_power_state = WIFI_PWR_STOPPED;
    s_ps_mode_applied = false;
}

static void wifi_mdns_start(void)
{
    if (s_mdns_started) {
        return;
    }
    esp_err_t err = mdns_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "mDNS init failed: %s", esp_err_to_name(err));
        return;
    }
    err = mdns_hostname_set(WIFI_STA_HOSTNAME);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "mDNS hostname set failed: %s", esp_err_to_name(err));
    }
    err = mdns_instance_name_set("MagEDC");
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "mDNS instance set failed: %s", esp_err_to_name(err));
    }
    err = mdns_service_add("MagEDC HTTP", "_http", "_tcp", 80, NULL, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "mDNS service add failed: %s", esp_err_to_name(err));
    }
    s_mdns_started = true;
    ESP_LOGI(TAG, "MDNS_READY:%s.local", WIFI_STA_HOSTNAME);
}

static void wifi_mdns_stop(void)
{
    if (!s_mdns_started) {
        return;
    }
    mdns_service_remove("_http", "_tcp");
    mdns_free();
    s_mdns_started = false;
}

static void wifi_connect_watchdog_task(void *arg)
{
    const uint32_t token = (uint32_t)(uintptr_t)arg;
    vTaskDelay(pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));
    xSemaphoreTakeRecursive(s_wifi_lock, portMAX_DELAY);
    if (token == s_watchdog_token && s_connect_watchdog_running) {
        EventBits_t bits = xEventGroupGetBits(s_wifi_events);
        s_connect_watchdog_running = false;
        if (s_wifi_started && !(bits & WIFI_CONNECTED_BIT)) {
            ESP_LOGW(TAG, "Wi-Fi connect timeout (%ds)", WIFI_CONNECT_TIMEOUT_MS / 1000);
            clear_wifi_fast_connect_cache();
            wifi_stop_internal();
            xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
            log_wifi_status("TIMEOUT");
            wifi_schedule_auto_connect(WIFI_RECONNECT_COOLDOWN_MS, "timeout-retry");
        }
    }
    xSemaphoreGiveRecursive(s_wifi_lock);
    vTaskDelete(NULL);
}

static void save_wifi_fast_connect_info(void)
{
    wifi_ap_record_t ap = {0};
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) {
        return;
    }

    nvs_handle_t handle;
    if (nvs_open(NVS_NS_WIFI, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }
    nvs_set_u8(handle, NVS_KEY_CHAN, ap.primary);
    nvs_set_blob(handle, NVS_KEY_BSSID, ap.bssid, sizeof(ap.bssid));
    nvs_commit(handle);
    nvs_close(handle);
    ESP_LOGI(TAG, "Saved fast-connect info: channel=%d", ap.primary);
}

static void clear_wifi_fast_connect_cache(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_WIFI, NVS_READWRITE, &handle) == ESP_OK) {
        nvs_erase_key(handle, NVS_KEY_CHAN);
        nvs_erase_key(handle, NVS_KEY_BSSID);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static void apply_fast_connect(wifi_config_t *wifi_config)
{
    s_fast_connect_applied = false;
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_WIFI, NVS_READONLY, &handle) != ESP_OK) {
        return;
    }

    uint8_t chan = 0;
    uint8_t bssid[6] = {0};
    size_t bssid_len = sizeof(bssid);
    esp_err_t chan_err = nvs_get_u8(handle, NVS_KEY_CHAN, &chan);
    esp_err_t bssid_err = nvs_get_blob(handle, NVS_KEY_BSSID, bssid, &bssid_len);
    nvs_close(handle);

    if (chan_err == ESP_OK && bssid_err == ESP_OK && bssid_len == 6 && chan > 0) {
        wifi_config->sta.channel = chan;
        memcpy(wifi_config->sta.bssid, bssid, 6);
        wifi_config->sta.bssid_set = true;
        s_fast_connect_applied = true;
        ESP_LOGI(TAG, "Using fast-connect cache (channel=%d)", chan);
    }

    wifi_config->sta.scan_method = WIFI_FAST_SCAN;
    wifi_config->sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
}

typedef struct {
    uint32_t generation;
    uint32_t delay_ms;
} wifi_auto_connect_args_t;

static void wifi_auto_connect_task(void *arg)
{
    const wifi_auto_connect_args_t work = *(wifi_auto_connect_args_t *)arg;
    free(arg);
    vTaskDelay(pdMS_TO_TICKS(work.delay_ms));
    xSemaphoreTakeRecursive(s_wifi_lock, portMAX_DELAY);
    if (work.generation == s_wifi_generation) {
        s_wifi_auto_task_running = false;
        if (!s_sleep_preparing && !s_calibrating) {
            if (load_wifi_credentials(s_stored_ssid, sizeof(s_stored_ssid), s_stored_pass, sizeof(s_stored_pass))) {
                wifi_init_sta(s_stored_ssid, s_stored_pass);
            } else if (WIFI_DEFAULT_SSID[0] != '\0') {
                wifi_init_sta(WIFI_DEFAULT_SSID, WIFI_DEFAULT_PASS);
            }
        }
    }
    xSemaphoreGiveRecursive(s_wifi_lock);
    vTaskDelete(NULL);
}

static void wifi_schedule_auto_connect(uint32_t delay_ms, const char *reason)
{
    if (s_wifi_auto_task_running || s_sleep_preparing || s_calibrating) {
        return;
    }
    if (!load_wifi_credentials(s_stored_ssid, sizeof(s_stored_ssid), s_stored_pass, sizeof(s_stored_pass)) &&
            WIFI_DEFAULT_SSID[0] == '\0') {
        return;
    }
    wifi_auto_connect_args_t *work = malloc(sizeof(*work));
    if (!work) {
        return;
    }
    *work = (wifi_auto_connect_args_t) {s_wifi_generation, delay_ms};
    s_wifi_auto_task_running = true;
    if (xTaskCreate(wifi_auto_connect_task, "wifi_auto", 4096, work, 3, NULL) != pdPASS) {
        s_wifi_auto_task_running = false;
        free(work);
        ESP_LOGW(TAG, "Failed to schedule wifi_auto task");
    } else {
        ESP_LOGI(TAG, "Auto-connect scheduled in %ums%s%s", (unsigned)delay_ms, reason ? " @" : "",
                 reason ? reason : "");
    }
}

/* HTTP task only. shutdown lets EventSource observe replacement/disconnection;
 * httpd retains ownership of the fd and closes it after async completion. */
static void sse_close_client(void)
{
    if (s_sse_req) {
        shutdown(s_sse_client_fd, SHUT_RDWR);
        httpd_req_async_handler_complete(s_sse_req);
        s_sse_req = NULL;
    }
    s_sse_client_fd = -1;
    atomic_store(&s_sse_connected, false);
    s_sse_disconnect_ms = esp_log_timestamp();
}

static esp_err_t sse_events_handler(httpd_req_t *req)
{
    if (atomic_load(&s_http_stopping)) {
        return ESP_FAIL;
    }
    httpd_req_t *async_req = NULL;
    esp_err_t err = httpd_req_async_handler_begin(req, &async_req);
    if (err != ESP_OK) {
        return err;
    }
    sse_close_client();
    s_sse_req = async_req;
    s_sse_client_fd = httpd_req_to_sockfd(req);
    xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
    s_http_task = xTaskGetCurrentTaskHandle();
    xSemaphoreGive(s_http_state_lock);
    s_client_ever_connected = true;
    s_sse_disconnect_ms = 0;
    atomic_store(&s_sse_connected, true);
    wifi_mark_control_activity();

    httpd_resp_set_type(async_req, "text/event-stream");
    httpd_resp_set_hdr(async_req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(async_req, "Connection", "keep-alive");
    httpd_resp_set_hdr(async_req, "Access-Control-Allow-Origin", "*");
    err = httpd_resp_send_chunk(async_req, ": connected\n\n", HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK) {
        sse_close_client();
        return err;
    }
    ESP_LOGI(TAG, "SSE client connected (fd=%d)", s_sse_client_fd);
    return ESP_OK;
}

static void sse_send_text(const char *text)
{
    if (!s_sse_req) {
        return;
    }
    const size_t frame_len = strlen(text) + 16;
    char *frame = malloc(frame_len);
    if (!frame) {
        return;
    }
    snprintf(frame, frame_len, "data: %s\n\n", text);
    if (httpd_resp_send_chunk(s_sse_req, frame, HTTPD_RESP_USE_STRLEN) != ESP_OK) {
        sse_close_client();
    } else {
        wifi_mark_control_activity();
    }
    free(frame);
}

static void sse_send_work(void *arg)
{
    char *text = arg;
    sse_send_text(text);
    free(text);
    atomic_store(&s_sse_work_pending, false);
}

static void sse_stop_work(void *arg)
{
    sse_close_client();
    xSemaphoreGive((SemaphoreHandle_t)arg);
}

static esp_err_t start_http_server(void)
{
    xSemaphoreTake(s_http_lifecycle_lock, portMAX_DELAY);
    if (s_httpd) {
        xSemaphoreGive(s_http_lifecycle_lock);
        return ESP_OK;
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = 4;
    config.stack_size = 16384;
    config.recv_wait_timeout = 1;
    config.send_wait_timeout = 1;
    httpd_handle_t server = NULL;
    atomic_store(&s_http_stopping, false);
    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK) {
        xSemaphoreGive(s_http_lifecycle_lock);
        return err;
    }
    const httpd_uri_t events_uri = {
        .uri = "/events",
        .method = HTTP_GET,
        .handler = sse_events_handler,
    };
    httpd_register_uri_handler(server, &events_uri);
    web_portal_register_handlers(server);
    xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
    s_httpd = server;
    xSemaphoreGive(s_http_state_lock);
    xSemaphoreGive(s_http_lifecycle_lock);
    ESP_LOGI(TAG, "HTTP/SSE server started on port %d", config.server_port);
    return ESP_OK;
}

static void stop_http_server(void)
{
    /* Never called from an HTTP handler. No lock needed by HTTP work is held
     * while waiting, including when queue_work uses its blocking mode. */
    xSemaphoreTake(s_http_lifecycle_lock, portMAX_DELAY);
    xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
    httpd_handle_t server = s_httpd;
    atomic_store(&s_http_stopping, true);
    s_httpd = NULL;
    xSemaphoreGive(s_http_state_lock);
    if (server) {
        xSemaphoreTake(s_http_refs_idle, portMAX_DELAY);
        xSemaphoreGive(s_http_refs_idle);
        xSemaphoreTake(s_http_closed, 0);
        const uint32_t started_ms = esp_log_timestamp();
        esp_err_t err;
        do {
            err = httpd_queue_work(server, sse_stop_work, s_http_closed);
            if (err == ESP_OK || esp_log_timestamp() - started_ms >= 2000) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        } while (true);
        if (err == ESP_OK) {
            xSemaphoreTake(s_http_closed, portMAX_DELAY);
            err = httpd_stop(server);
        }
        if (err != ESP_OK) {
            /* Retain ownership so a subsequent stop can retry safely. */
            ESP_LOGE(TAG, "HTTP stop failed: %s", esp_err_to_name(err));
            xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
            s_httpd = server;
            atomic_store(&s_http_stopping, false);
            xSemaphoreGive(s_http_state_lock);
        }
    }
    xSemaphoreGive(s_http_lifecycle_lock);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    xSemaphoreTakeRecursive(s_wifi_lock, portMAX_DELAY);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (s_sleep_preparing || s_calibrating || !s_wifi_started) {
            goto done;
        }
        wifi_enter_power_state(WIFI_PWR_CONNECTING, "sta-event-start");
        log_wifi_status("CONNECTING");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT);
        s_sta_ip[0] = '\0';
        wifi_mdns_stop();
        stop_http_server();
        if (s_sleep_preparing || s_calibrating || !s_wifi_started) {
            if (s_wifi_events) {
                xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
            }
            goto done;
        }
        if (s_fast_connect_applied) {
            wifi_config_t config;
            if (esp_wifi_get_config(WIFI_IF_STA, &config) == ESP_OK) {
                config.sta.bssid_set = false;
                config.sta.channel = 0;
                memset(config.sta.bssid, 0, sizeof(config.sta.bssid));
                esp_wifi_set_config(WIFI_IF_STA, &config);
            }
            s_fast_connect_applied = false;
            clear_wifi_fast_connect_cache();
        }
        start_connect_watchdog();
        if (s_retry_count < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_count++;
            wifi_enter_power_state(WIFI_PWR_CONNECTING, "sta-retry");
            log_wifi_status("RECONNECTING");
        } else {
            wifi_stop_internal();
            wifi_enter_power_state(WIFI_PWR_STOPPED, "sta-failed");
            xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
            log_wifi_status("FAILED");
            wifi_schedule_auto_connect(WIFI_RECONNECT_COOLDOWN_MS, "cooldown-retry");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        if (s_sleep_preparing || s_calibrating || !s_wifi_started) {
            goto done;
        }
        s_retry_count = 0;
        s_connect_watchdog_running = false;
        ++s_watchdog_token;
        s_wifi_ready_ms = esp_log_timestamp();
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
        log_wifi_status("CONNECTED");
        log_wifi_ip();
        save_wifi_fast_connect_info();
        wifi_mdns_start();
        wifi_enter_power_state(WIFI_PWR_AUTOSLEEP, "got-ip");
        if (start_http_server() != ESP_OK) {
            log_wifi_status("HTTPD_FAILED");
        }
    }

done:
    xSemaphoreGiveRecursive(s_wifi_lock);
}

static bool save_wifi_credentials(const char *ssid, const char *password)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_WIFI, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = nvs_set_str(handle, NVS_KEY_SSID, ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(handle, NVS_KEY_PASS, password);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err == ESP_OK;
}

static bool load_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len)
{
    ssid[0] = '\0';
    pass[0] = '\0';
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_WIFI, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }

    size_t s_len = ssid_len;
    size_t p_len = pass_len;
    esp_err_t err1 = nvs_get_str(handle, NVS_KEY_SSID, ssid, &s_len);
    esp_err_t err2 = nvs_get_str(handle, NVS_KEY_PASS, pass, &p_len);
    nvs_close(handle);
    return (err1 == ESP_OK && err2 == ESP_OK && ssid[0] != '\0');
}

static void clear_wifi_credentials(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_WIFI, NVS_READWRITE, &handle) == ESP_OK) {
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static bool wifi_stack_init_once(void)
{
    if (s_wifi_stack_ready) {
        return true;
    }

    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return false;
    }

    if (!s_sta_netif_ready) {
        s_sta_netif = esp_netif_create_default_wifi_sta();
        if (!s_sta_netif) {
            ESP_LOGE(TAG, "esp_netif_create_default_wifi_sta failed");
            return false;
        }
        err = esp_netif_set_hostname(s_sta_netif, WIFI_STA_HOSTNAME);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "STA hostname set failed: %s", esp_err_to_name(err));
        }
        s_sta_netif_ready = true;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WIFI event register failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "IP event register failed: %s", esp_err_to_name(err));
        return false;
    }

    wifi_configure_band();

    s_wifi_stack_ready = true;
    return true;
}

#if CONFIG_IDF_TARGET_ESP32C5
static void wifi_configure_band(void)
{
    esp_err_t err = esp_wifi_set_band_mode(WIFI_BAND_MODE_AUTO);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_set_band_mode(AUTO) failed: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "Wi-Fi band mode: AUTO (2.4G + 5G)");
}
#else
static void wifi_configure_band(void) {}
#endif

static bool wifi_credentials_valid(const char *ssid, const char *password)
{
    if (!ssid || ssid[0] == '\0' || strnlen(ssid, 33) > 32) {
        return false;
    }
    const size_t pass_len = password ? strnlen(password, 65) : 0;
    if (pass_len != 0 && (pass_len < 8 || pass_len > 64)) {
        return false;
    }
    if (pass_len == 64) {
        for (size_t i = 0; i < pass_len; i++) {
            const char c = password[i];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                return false;
            }
        }
    }
    return true;
}

static bool wifi_init_sta(const char *ssid, const char *password)
{
    if (!wifi_credentials_valid(ssid, password) || !wifi_stack_init_once()) {
        log_wifi_status("INIT_FAILED");
        return false;
    }
    /* Inputs can alias the saved buffers, which stopping/reloading may change. */
    char desired_ssid[33] = {0};
    char desired_pass[65] = {0};
    snprintf(desired_ssid, sizeof(desired_ssid), "%s", ssid);
    snprintf(desired_pass, sizeof(desired_pass), "%s", password ? password : "");
    wifi_stop_internal();
    s_retry_count = 0;
    s_sleep_preparing = false;

    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.sta.ssid, desired_ssid, strlen(desired_ssid));
    memcpy(wifi_config.sta.password, desired_pass, strlen(desired_pass));
    wifi_config.sta.threshold.authmode = desired_pass[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    wifi_config.sta.listen_interval = WIFI_STA_LISTEN_INTERVAL;
    apply_fast_connect(&wifi_config);

    esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err == ESP_OK) {
        err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    }
    if (err == ESP_OK) {
        err = esp_wifi_start();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi start failed: %s", esp_err_to_name(err));
        xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
        log_wifi_status("INIT_FAILED");
        return false;
    }
    s_wifi_started = true;
    wifi_set_ps_mode(WIFI_PS_MAX_MODEM);
    wifi_enter_power_state(WIFI_PWR_CONNECTING, "sta-start");
    wifi_country_t country = {
        .cc = "CN",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
    };
    esp_wifi_set_country(&country);
    memcpy(s_stored_ssid, desired_ssid, sizeof(s_stored_ssid));
    memcpy(s_stored_pass, desired_pass, sizeof(s_stored_pass));
    start_connect_watchdog();
    return true;
}

static void start_connect_watchdog(void)
{
    if (s_connect_watchdog_running) {
        return;
    }
    s_connect_watchdog_running = true;
    const uint32_t token = ++s_watchdog_token;
    if (xTaskCreate(wifi_connect_watchdog_task, "wifi_wdog", 3072, (void *)(uintptr_t)token, 3, NULL) != pdPASS) {
        s_connect_watchdog_running = false;
        ESP_LOGE(TAG, "Cannot start Wi-Fi connection watchdog");
    }
}

static void handle_wifi_cfg_command(const char *ssid, const char *password)
{
    if (!wifi_stream_connect(ssid, password)) {
        log_wifi_status("INVALID_OR_FAILED_CFG");
        return;
    }
    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
}

static void handle_serial_command(char *line)
{
    while (*line == ' ' || *line == '\r') {
        line++;
    }
    if (*line == '\0') {
        return;
    }

    if (*line == '{') {
        web_portal_handle_control_text(line);
        return;
    }

    if (strcmp(line, "WIFI_STATUS") == 0) {
        wifi_lock_take();
        if (s_wifi_started) {
            EventBits_t bits = xEventGroupGetBits(s_wifi_events);
            if (bits & WIFI_CONNECTED_BIT) {
                log_wifi_status("CONNECTED");
                log_wifi_ip();
            } else {
                log_wifi_status("CONNECTING");
            }
        } else if (load_wifi_credentials(s_stored_ssid, sizeof(s_stored_ssid), s_stored_pass, sizeof(s_stored_pass))) {
            log_wifi_status("STORED");
        } else if (WIFI_DEFAULT_SSID[0] != '\0') {
            log_wifi_status("DEFAULT");
        } else {
            log_wifi_status("IDLE");
        }
        wifi_lock_give();
        return;
    }

    if (strcmp(line, "WIFI_STOP") == 0) {
        wifi_lock_take();
        wifi_stop_internal();
        wifi_lock_give();
        log_wifi_status("STOPPED");
        return;
    }

    if (strcmp(line, "WIFI_CLEAR") == 0) {
        wifi_lock_take();
        clear_wifi_credentials();
        memset(s_stored_ssid, 0, sizeof(s_stored_ssid));
        memset(s_stored_pass, 0, sizeof(s_stored_pass));
        wifi_stop_internal();
        wifi_lock_give();
        log_wifi_status("CLEARED");
        return;
    }

    if (strcmp(line, "RECALIB") == 0) {
        app_request_recalibration();
        ESP_LOGI(TAG, "RECALIB requested via serial; restarting 9-point wizard");
        return;
    }

    if (strncmp(line, "WIFI_CFG\t", 9) == 0) {
        char *ssid = line + 9;
        char *pass = strchr(ssid, '\t');
        if (!pass) {
            log_wifi_status("INVALID_CMD");
            return;
        }
        *pass = '\0';
        pass++;
        handle_wifi_cfg_command(ssid, pass);
        return;
    }

    log_wifi_status("UNKNOWN_CMD");
}

static int serial_read_char(char *out)
{
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED
    int n = usb_serial_jtag_read_bytes((uint8_t *)out, 1, 0);
    if (n > 0) {
        return n;
    }
#endif
    return read(fileno(stdin), out, 1);
}

static void serial_cmd_task(void *arg)
{
    char line[CMD_BUF_SIZE];
    int pos = 0;

    /* 等待主业务（磁感采样/LED）完成启动，避免与控制台初始化抢占 */
    vTaskDelay(pdMS_TO_TICKS(1500));

#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED
    /* 确保 USB 串口可读（与上位机 pyserial 同一路） */
    usb_serial_jtag_driver_config_t usb_cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    esp_err_t usb_err = usb_serial_jtag_driver_install(&usb_cfg);
    if (usb_err != ESP_OK && usb_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "usb_serial_jtag_driver_install: %s", esp_err_to_name(usb_err));
    }
#endif

    int stdin_fd = fileno(stdin);
    int flags = fcntl(stdin_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(stdin_fd, F_SETFL, flags | O_NONBLOCK);
    }

    while (1) {
        char c;
        int n = serial_read_char(&c);
        if (n <= 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (c == '\n' || c == '\r') {
            if (pos > 0) {
                line[pos] = '\0';
                ESP_LOGD(TAG, "Serial cmd: %s", line);
                handle_serial_command(line);
                pos = 0;
            }
            continue;
        }

        if (pos < (int)sizeof(line) - 1) {
            line[pos++] = c;
        }
    }
}

void wifi_stream_init(void)
{
    if (!s_wifi_lock) {
        s_wifi_lock = xSemaphoreCreateRecursiveMutex();
        s_http_state_lock = xSemaphoreCreateMutex();
        s_http_lifecycle_lock = xSemaphoreCreateMutex();
        s_http_refs_idle = xSemaphoreCreateBinary();
        s_http_closed = xSemaphoreCreateBinary();
        s_wifi_events = xEventGroupCreate();
        configASSERT(s_wifi_lock && s_http_state_lock && s_http_lifecycle_lock && s_http_refs_idle &&
                     s_http_closed && s_wifi_events);
        xSemaphoreGive(s_http_refs_idle);
    }
    wifi_lock_take();
    s_sleep_preparing = false;
    wifi_pm_init();

    web_portal_init();
    xTaskCreate(serial_cmd_task, "serial_cmd", 4096, NULL, 3, NULL);
    if (!s_wifi_power_task_started) {
        xTaskCreate(wifi_power_task, "wifi_power", 3072, NULL, 2, NULL);
        s_wifi_power_task_started = true;
    }

    /* 默认 STA-only：不自动启动 SoftAP，避免常驻发热。 */
    ESP_LOGI(TAG, "Wi-Fi runtime mode: STA-only");
    ESP_LOGI(TAG, "Wi-Fi power knobs: active_window=%dms idle_stop=%dms", WIFI_ACTIVE_WINDOW_MS, WIFI_IDLE_STOP_MS);

    if (load_wifi_credentials(s_stored_ssid, sizeof(s_stored_ssid), s_stored_pass, sizeof(s_stored_pass))) {
        log_wifi_status("STORED");
        /* 校准进行中（冷启动首校）：推迟自动连网，待校准完成由 set_calibrating(false) 触发 */
        if (!s_calibrating) {
            /* 传感器/LED 启动后再连 WiFi，减轻 USB 烧录阶段冲突 */
            wifi_schedule_auto_connect(2000, "boot-saved");
        } else {
            ESP_LOGI(TAG, "Calibration in progress; deferring Wi-Fi auto-connect");
        }
    } else if (WIFI_DEFAULT_SSID[0] != '\0') {
        snprintf(s_stored_ssid, sizeof(s_stored_ssid), "%s", WIFI_DEFAULT_SSID);
        snprintf(s_stored_pass, sizeof(s_stored_pass), "%s", WIFI_DEFAULT_PASS);
        log_wifi_status("DEFAULT");
        if (!s_calibrating) {
            wifi_schedule_auto_connect(2000, "boot-default");
        } else {
            ESP_LOGI(TAG, "Calibration in progress; deferring Wi-Fi auto-connect");
        }
    } else {
        s_stored_ssid[0] = '\0';
        s_stored_pass[0] = '\0';
        log_wifi_status("IDLE");
        ESP_LOGI(TAG, "No Wi-Fi credentials; waiting for serial WIFI_CFG provisioning");
    }
    wifi_lock_give();
}

bool wifi_stream_connect(const char *ssid, const char *password)
{
    if (!s_wifi_lock || !wifi_credentials_valid(ssid, password)) {
        return false;
    }
    wifi_lock_take();
    bool ok = save_wifi_credentials(ssid, password ? password : "");
    if (ok) {
        clear_wifi_fast_connect_cache();
        if (s_calibrating) {
            /* Remember provisioning during the wizard without starting RF. */
            wifi_stop_internal();
        } else {
            ok = wifi_init_sta(ssid, password);
        }
    }
    wifi_lock_give();
    return ok;
}

static bool wifi_inhibits_sleep_locked(void)
{
    if (!s_wifi_started || !s_wifi_events) {
        return false;
    }

    EventBits_t bits = xEventGroupGetBits(s_wifi_events);
    if (bits & WIFI_CONNECTED_BIT) {
        /* 实时有网页（SSE）客户端：保持唤醒。 */
        if (atomic_load(&s_sse_connected)) {
            return true;
        }
        const uint32_t now = esp_log_timestamp();
        /* 已经有客户端连接过：配对宽限的使命已完成。此后只在断开后做短暂
         * linger（容忍页面刷新/EventSource 自动重连），到点即允许深睡。
         * 这修复了“关闭网页后设备仍被长宽限拖着不休眠”的功耗问题。 */
        if (s_client_ever_connected) {
            if (s_sse_disconnect_ms > 0 && (now - s_sse_disconnect_ms) < WIFI_CLIENT_LINGER_MS) {
                return true;
            }
            return false;
        }
        /* 尚无任何客户端连接过：保留配对宽限窗口，给网页首次连接留时间。 */
        if (s_wifi_ready_ms > 0 && (now - s_wifi_ready_ms) < WIFI_WEB_PAIRING_GRACE_MS) {
            return true;
        }
        return false;
    }

    /* 连接/重试阶段禁止休眠；失败后允许系统按 idle 进入深睡 */
    return (bits & WIFI_FAIL_BIT) == 0;
}

bool wifi_stream_inhibits_sleep(void)
{
    wifi_lock_take();
    const bool busy = wifi_inhibits_sleep_locked();
    wifi_lock_give();
    return busy;
}

void wifi_stream_prepare_sleep(void)
{
    wifi_lock_take();
    s_sleep_preparing = true;
    wifi_pm_release_lock();
    wifi_enter_power_state(WIFI_PWR_SLEEP_PREP, "deep-sleep");
    wifi_stop_internal();
    wifi_lock_give();
}

bool wifi_stream_get_sta_ip(char *buf, size_t len)
{
    if (!buf || len == 0) {
        return false;
    }
    wifi_lock_take();
    const bool available = s_sta_ip[0] != '\0';
    snprintf(buf, len, "%s", s_sta_ip);
    wifi_lock_give();
    return available;
}

bool wifi_stream_http_active(void)
{
    if (!s_http_state_lock) {
        return false;
    }
    xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
    const bool active = s_httpd != NULL;
    xSemaphoreGive(s_http_state_lock);
    return active;
}

bool wifi_stream_is_connected(void)
{
    if (!s_wifi_events) {
        return false;
    }
    return (xEventGroupGetBits(s_wifi_events) & WIFI_CONNECTED_BIT) != 0;
}

void wifi_stream_send_text(const char *text)
{
    if (!text || !s_http_state_lock) {
        return;
    }
    xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
    httpd_handle_t server = s_httpd;
    if (!server || !atomic_load(&s_sse_connected)) {
        xSemaphoreGive(s_http_state_lock);
        return;
    }
    const bool in_http_task = xTaskGetCurrentTaskHandle() == s_http_task;
    if (!in_http_task && atomic_exchange(&s_sse_work_pending, true)) {
        xSemaphoreGive(s_http_state_lock);
        return; /* Bounded telemetry backlog; never delay sensor sampling. */
    }
    if (s_http_queue_users++ == 0) {
        xSemaphoreTake(s_http_refs_idle, 0);
    }
    xSemaphoreGive(s_http_state_lock);

    if (in_http_task) {
        sse_send_text(text);
    } else {
        char *copy = strdup(text);
        if (!copy || httpd_queue_work(server, sse_send_work, copy) != ESP_OK) {
            free(copy);
            atomic_store(&s_sse_work_pending, false);
        }
    }

    xSemaphoreTake(s_http_state_lock, portMAX_DELAY);
    if (--s_http_queue_users == 0) {
        xSemaphoreGive(s_http_refs_idle);
    }
    xSemaphoreGive(s_http_state_lock);
}

#endif /* ENABLE_WIFI */
