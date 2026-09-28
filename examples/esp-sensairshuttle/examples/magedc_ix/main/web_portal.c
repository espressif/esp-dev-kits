/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "web_portal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cJSON.h"
#include "audio_tuning.h"
#include "esp_log.h"
#include "wifi_stream.h"
#include "zone_tone.h"
#include "speaker_gesture.h"

static const char *TAG = "WEB_PORTAL";
static volatile int s_action_zone_id = -1;
static volatile float s_action_zone_conf = 0.0f;

static void set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

/* Validate before converting or changing any runtime state. */
static bool json_number(const cJSON *json, const char *key, bool required, double min_value,
                        double max_value, double *out)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!item) {
        return !required;
    }
    if (!cJSON_IsNumber(item) || !isfinite(item->valuedouble) || item->valuedouble < min_value ||
            item->valuedouble > max_value) {
        return false;
    }
    *out = item->valuedouble;
    return true;
}

static bool json_integer(const cJSON *json, const char *key, bool required, int min_value,
                         int max_value, int *out)
{
    double value = *out;
    if (!json_number(json, key, required, min_value, max_value, &value) || trunc(value) != value) {
        return false;
    }
    *out = (int)value;
    return true;
}

static void web_portal_push_audio_config(void)
{
    zone_tone_config_t tone;
    audio_gate_config_t gate;
    zone_tone_get_config(&tone);
    audio_gate_get_config(&gate);

    char buf[384];
    snprintf(buf, sizeof(buf),
             "{\"t\":\"audio_cfg\",\"enabled\":%s,"
             "\"volume\":%d,\"max_ms\":%u,\"pa_settle\":%u,\"pa_hold\":%u,"
             "\"attack\":%d,\"release\":%d,"
             "\"gate_settle\":%u,\"gate_conf\":%.3f,\"gate_cooldown\":%u,"
             "\"sample_rate\":24000,\"upsample_fs\":480}",
             zone_tone_is_enabled() ? "true" : "false", tone.volume_pct, (unsigned)tone.max_play_ms,
             (unsigned)tone.pa_settle_ms, (unsigned)tone.pa_hold_ms, tone.attack_pct, tone.release_pct,
             (unsigned)gate.settle_hold_ms, gate.min_conf, (unsigned)gate.cooldown_ms);
    wifi_stream_send_text(buf);
    printf("%s\n", buf);
    fflush(stdout);
}

static void web_portal_push_speaker_gesture_config(void)
{
    int seq[SPEAKER_GESTURE_MAX_LEN] = {0};
    const size_t len = speaker_gesture_get_sequence(seq, SPEAKER_GESTURE_MAX_LEN);
    char seq_json[96] = {0};
    size_t pos = 0;
    for (size_t i = 0; i < len && pos < (sizeof(seq_json) - 8); i++) {
        const int n = snprintf(seq_json + pos, sizeof(seq_json) - pos, "%s%d", (i == 0) ? "" : ",", seq[i]);
        if (n <= 0) {
            break;
        }
        pos += (size_t)n;
    }

    char buf[160];
    snprintf(buf, sizeof(buf), "{\"t\":\"speaker_gesture_cfg\",\"len\":%u,\"seq\":[%s]}", (unsigned)len, seq_json);
    wifi_stream_send_text(buf);
}

static esp_err_t audio_options_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t control_error(httpd_req_t *req, const char *status, const char *error)
{
    char body[96];
    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "application/json");
    snprintf(body, sizeof(body), "{\"ok\":false,\"err\":\"%s\"}", error);
    return httpd_resp_sendstr(req, body);
}

static esp_err_t audio_post_handler(httpd_req_t *req)
{
    set_cors_headers(req);

    if (req->content_len <= 0 || req->content_len >= 512) {
        control_error(req, "400 Bad Request", "bad_length");
        return ESP_FAIL; /* Close instead of draining an arbitrarily long body. */
    }

    char buf[512];
    size_t received = 0;
    const uint32_t started_ms = esp_log_timestamp();
    while (received < req->content_len) {
        /* Each recv is bounded by the server's one-second socket timeout. */
        if (esp_log_timestamp() - started_ms >= 5000) {
            control_error(req, "408 Request Timeout", "recv_timeout");
            return ESP_FAIL;
        }
        const int ret = httpd_req_recv(req, buf + received, req->content_len - received);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }
        if (ret <= 0) {
            control_error(req, "400 Bad Request", "recv");
            return ESP_FAIL;
        }
        received += (size_t)ret;
    }
    if (esp_log_timestamp() - started_ms >= 5000) {
        control_error(req, "408 Request Timeout", "recv_timeout");
        return ESP_FAIL;
    }
    buf[received] = '\0';
    if (memchr(buf, '\0', received) != NULL) {
        return control_error(req, "400 Bad Request", "invalid_command");
    }

    const esp_err_t err = web_portal_handle_control_text(buf);
    if (err != ESP_OK) {
        return control_error(req, err == ESP_ERR_INVALID_ARG ? "400 Bad Request" : "500 Internal Server Error",
                             err == ESP_ERR_INVALID_ARG ? "invalid_command" : "apply_failed");
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"ok\":true}");
}

void web_portal_init(void)
{
    ESP_LOGI(TAG, "Web game runtime ready");
}

void web_portal_register_handlers(httpd_handle_t server)
{
    if (!server) {
        return;
    }

    const httpd_uri_t routes[] = {
        {.uri = "/api/audio", .method = HTTP_OPTIONS, .handler = audio_options_handler},
        {.uri = "/api/audio", .method = HTTP_POST, .handler = audio_post_handler},
    };

    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        httpd_register_uri_handler(server, &routes[i]);
    }
    ESP_LOGI(TAG, "Game routes registered");
}

void web_portal_set_action_snapshot(int action_zone_id, float action_confidence)
{
    s_action_zone_id = action_zone_id;
    s_action_zone_conf = action_confidence;
}

void web_portal_push_mag(float dx, float dy, float dz, int zone_id, float confidence, float r_xy, float theta_deg)
{
    static uint32_t last_ms = 0;
    uint32_t now = esp_log_timestamp();
    if (now - last_ms < 50) {
        return;
    }
    last_ms = now;

    char buf[256];
    snprintf(buf, sizeof(buf),
             "{\"t\":\"mag\",\"dx\":%.1f,\"dy\":%.1f,\"dz\":%.1f,"
             "\"zone\":%d,\"vzone\":%d,\"azone\":%d,"
             "\"conf\":%.3f,\"vconf\":%.3f,\"aconf\":%.3f,"
             "\"r\":%.0f,\"th\":%.1f}",
             dx, dy, dz, zone_id, zone_id, s_action_zone_id, confidence, confidence, s_action_zone_conf, r_xy,
             theta_deg);
    wifi_stream_send_text(buf);
}

void web_portal_push_zone_change(int zone_id, float confidence)
{
    char buf[96];
    snprintf(buf, sizeof(buf), "{\"t\":\"zone\",\"id\":%d,\"conf\":%.3f}", zone_id, confidence);
    wifi_stream_send_text(buf);

    char legacy[48];
    snprintf(legacy, sizeof(legacy), "Position changed to [%d]", zone_id);
    wifi_stream_send_text(legacy);
}

esp_err_t web_portal_handle_control_text(const char *text)
{
    if (!text || text[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }
    cJSON *json = cJSON_ParseWithLengthOpts(text, strlen(text) + 1, NULL, true);
    const cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "t");
    esp_err_t result = ESP_ERR_INVALID_ARG;
    if (!cJSON_IsObject(json) || !cJSON_IsString(type)) {
        goto done;
    }

    if (strcmp(type->valuestring, "audio_get") == 0) {
        web_portal_push_audio_config();
        web_portal_push_speaker_gesture_config();
        result = ESP_OK;
    } else if (strcmp(type->valuestring, "speaker_gesture_get") == 0) {
        web_portal_push_speaker_gesture_config();
        result = ESP_OK;
    } else if (strcmp(type->valuestring, "speaker_gesture_set") == 0) {
        int len = 0;
        if (!json_integer(json, "len", true, 2, SPEAKER_GESTURE_MAX_LEN, &len)) {
            goto done;
        }
        int seq[SPEAKER_GESTURE_MAX_LEN] = {0};
        for (int i = 0; i < len; i++) {
            char key[16];
            snprintf(key, sizeof(key), "s%d", i + 1);
            if (!json_integer(json, key, true, 1, 9, &seq[i])) {
                goto done;
            }
        }
        if (!speaker_gesture_set_sequence(seq, (size_t)len)) {
            result = ESP_FAIL;
            goto done;
        }
        web_portal_push_speaker_gesture_config();
        result = ESP_OK;
    } else if (strcmp(type->valuestring, "audio_reset") == 0) {
        zone_tone_reset_config();
        audio_gate_reset_config();
        web_portal_push_audio_config();
        result = ESP_OK;
    } else if (strcmp(type->valuestring, "audio_test") == 0) {
        int zone = 1;
        if (!json_integer(json, "zone", false, 1, 9, &zone)) {
            goto done;
        }
        zone_tone_play_test(zone);
        web_portal_push_audio_config();
        result = ESP_OK;
    } else if (strcmp(type->valuestring, "audio_set") == 0) {
        zone_tone_config_t tone;
        audio_gate_config_t gate;
        zone_tone_get_config(&tone);
        audio_gate_get_config(&gate);
        int max_ms = (int)tone.max_play_ms;
        int pa_settle = (int)tone.pa_settle_ms;
        int pa_hold = (int)tone.pa_hold_ms;
        int gate_settle = (int)gate.settle_hold_ms;
        int gate_cooldown = (int)gate.cooldown_ms;
        double gate_conf = gate.min_conf;
        const cJSON *enabled = cJSON_GetObjectItemCaseSensitive(json, "enabled");
        if ((enabled && !cJSON_IsBool(enabled)) ||
                !json_integer(json, "volume", false, 0, 100, &tone.volume_pct) ||
                !json_integer(json, "max_ms", false, 80, 1200, &max_ms) ||
                !json_integer(json, "pa_settle", false, 0, 40, &pa_settle) ||
                !json_integer(json, "pa_hold", false, 0, 120, &pa_hold) ||
                !json_integer(json, "attack", false, 0, 20, &tone.attack_pct) ||
                !json_integer(json, "release", false, 0, 35, &tone.release_pct) ||
                !json_integer(json, "gate_settle", false, 40, 800, &gate_settle) ||
                !json_number(json, "gate_conf", false, 0.02, 0.80, &gate_conf) ||
                !json_integer(json, "gate_cooldown", false, 0, 1500, &gate_cooldown)) {
            goto done;
        }
        tone.max_play_ms = (uint32_t)max_ms;
        tone.pa_settle_ms = (uint32_t)pa_settle;
        tone.pa_hold_ms = (uint32_t)pa_hold;
        gate.settle_hold_ms = (uint32_t)gate_settle;
        gate.min_conf = (float)gate_conf;
        gate.cooldown_ms = (uint32_t)gate_cooldown;
        zone_tone_set_config(&tone);
        audio_gate_set_config(&gate);
        if (enabled) {
            zone_tone_set_enabled(cJSON_IsTrue(enabled));
        }
        web_portal_push_audio_config();
        result = ESP_OK;
    }

done:
    cJSON_Delete(json);
    return result;
}
