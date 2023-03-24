/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_check.h"
#include "app_sr.h"

#include "lv_example_pub.h"

#include "bsp_board.h"
#include "bsp/esp-bsp.h"
#include "app_sr_handler.h"
#include "ui_sr.h"
#include "settings.h"


static const char *TAG = "sr_handler";

static bool b_audio_playing = false;

//extern file_iterator_instance_t* file_iterator;

typedef enum {
    AUDIO_WAKE,
    AUDIO_OK,
    AUDIO_END,
    AUDIO_PRESS,
    AUDIO_MAX,
} audio_segment_t;

typedef struct {
    uint8_t *audio_buffer;
    size_t len;
} audio_data_t;

static audio_data_t g_audio_data[AUDIO_MAX];

static esp_err_t sr_echo_play(audio_segment_t audio)
{
    bsp_audio_poweramp_enable(false); // turn off the speaker to avoid play some noise

    typedef struct {
        // The "RIFF" chunk descriptor
        uint8_t ChunkID[4];
        int32_t ChunkSize;
        uint8_t Format[4];
        // The "fmt" sub-chunk
        uint8_t Subchunk1ID[4];
        int32_t Subchunk1Size;
        int16_t AudioFormat;
        int16_t NumChannels;
        int32_t SampleRate;
        int32_t ByteRate;
        int16_t BlockAlign;
        int16_t BitsPerSample;
        // The "data" sub-chunk
        uint8_t Subchunk2ID[4];
        int32_t Subchunk2Size;
    } wav_header_t;

    printf("audio:%d", audio);
    /**
     * read head of WAV file
     */
    bsp_codec_config_t *codec_handle = bsp_board_get_codec_handle();
    uint8_t *p = g_audio_data[audio].audio_buffer;
    wav_header_t *wav_head = (wav_header_t *)p;
    if (NULL == strstr((char *)wav_head->Subchunk1ID, "fmt") &&
            NULL == strstr((char *)wav_head->Subchunk2ID, "data")) {
        ESP_LOGE(TAG, "Header of wav format error");
        return ESP_FAIL;
    }
    p += sizeof(wav_header_t);
    size_t len = g_audio_data[audio].len - sizeof(wav_header_t);
    len = len & 0xfffffffc;
    ESP_LOGD(TAG, "frame_rate=%d, ch=%d, width=%d", wav_head->SampleRate, wav_head->NumChannels, wav_head->BitsPerSample);
    codec_handle->i2s_reconfig_clk_fn(wav_head->SampleRate, wav_head->BitsPerSample, I2S_SLOT_MODE_STEREO);

    //i2s_zero_dma_buffer(I2S_NUM_0);
    sys_param_t *param = settings_get_parameter();
    codec_handle->volume_set_fn(85, NULL);
    codec_handle->mute_set_fn(false);
    ESP_LOGD(TAG, "bsp_codec_set_voice_volume=%d", param->volume);

    vTaskDelay(pdMS_TO_TICKS(50));
    bsp_audio_poweramp_enable(true);

    size_t bytes_written = 0;
    b_audio_playing = true;
    codec_handle->i2s_write_fn((char *)p, len, &bytes_written, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(20));
    b_audio_playing = false;
    return ESP_OK;
}

bool sr_echo_is_playing(void)
{
    return b_audio_playing;
}

sr_language_t sr_detect_language()
{
    static sr_language_t sr_current_lang = SR_LANG_MAX;
    esp_err_t ret;
    FILE *fp = NULL;
    const sys_param_t * param = settings_get_parameter();

    if(param->sr_lang^sr_current_lang){
        sr_current_lang = param->sr_lang;
        ESP_LOGI(TAG, "boardcast language change to = %s", (SR_LANG_EN == param->sr_lang ? "EN" : "CN"));

        const char *files[2][4] = {
            {"/spiffs/echo_en_wake.wav", "/spiffs/echo_en_ok.wav", "/spiffs/echo_en_end.wav", "/spiffs/echo_en_wake.wav"},
            {"/spiffs/echo_cn_wake.wav", "/spiffs/echo_cn_ok.wav", "/spiffs/echo_cn_end.wav", "/spiffs/echo_en_wake.wav"},
        };

        char audio_file[48] = {0};
        for (size_t i = 0; i < AUDIO_MAX; i++) {
            strcpy(audio_file, files[param->sr_lang][i]);
            fp = fopen(audio_file, "rb");
            ESP_GOTO_ON_FALSE(NULL != fp, ESP_ERR_NOT_FOUND, err, TAG, "Open file %s failed", audio_file);
            fseek(fp, 0, SEEK_END);
            size_t file_size = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            if(g_audio_data[i].audio_buffer){
                heap_caps_free(g_audio_data[i].audio_buffer);
                g_audio_data[i].len = 0;
            }
            g_audio_data[i].len = file_size;
            g_audio_data[i].audio_buffer = heap_caps_calloc(1, file_size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
            ESP_GOTO_ON_FALSE(NULL != g_audio_data[i].audio_buffer, ESP_ERR_NO_MEM, err, TAG,  "No mem for sr echo buffer");
            fread(g_audio_data[i].audio_buffer, 1, file_size, fp);
            fclose(fp);
        }
    }
    return sr_current_lang;

err:
    if(fp){
        fclose(fp);
    }
    return sr_current_lang;
}

void sr_handler_task(void *pvParam)
{
    static lv_event_info_t event;
    static sr_language_t sr_current_lang;
    static sys_param_t * sys_param;

    event.event = LV_EVENT_EXIT_CLOCK;

    sr_current_lang = sr_detect_language();
    sr_echo_play(AUDIO_WAKE);

    while (true) {
        sr_result_t result;
        app_sr_get_result(&result, portMAX_DELAY);

        sr_current_lang = sr_detect_language();
        sys_param = settings_get_parameter();

        if(true == result.beep_enable){
            sr_echo_play(AUDIO_PRESS);
            continue;
        }

        if (ESP_MN_STATE_TIMEOUT == result.state) {
            if (SR_LANG_EN == sr_current_lang) {
                sr_anim_set_text("Timeout");
            } else {
                sr_anim_set_text("超时");
            }
#if !SR_RUN_TEST
            event.event = LV_EVENT_EXIT_CLOCK;
            app_lvgl_send_event(&event);

            sr_echo_play(AUDIO_END);
#endif
            sr_anim_stop();
            /*if (AUDIO_PLAYER_STATE_PLAYING == last_player_state) {
                audio_player_resume();
            }*/
            continue;
        }

        if (WAKENET_DETECTED == result.wakenet_mode) {
            sr_anim_start();
            /*
            last_player_state = audio_player_get_state();
            audio_player_pause();
            */
            if (SR_LANG_EN == sr_current_lang) {
                sr_anim_set_text("Say command");
            } else {
                sr_anim_set_text("请说");
            }
#if !SR_RUN_TEST
            // reload_screenOff_timer();
            // event.event = LV_EVENT_EXIT_CLOCK;
            // app_lvgl_send_event(&event);
            sr_echo_play(AUDIO_WAKE);
#endif
            continue;
        }

        if (ESP_MN_STATE_DETECTED & result.state) {
            const sr_cmd_t *cmd = app_sr_get_cmd_from_id(result.command_id);
            if(NULL == cmd){
                continue;
            }

            ESP_LOGI(TAG, "command:%s, act:%d", cmd->str, cmd->cmd);
            if(cmd->cmd < SR_CMD_MAX){
                sr_anim_set_text((char *) cmd->str);
            }
            else{
                sr_anim_set_text((char *) cmd->phoneme);
            }
#if !SR_CONTINUE_DET
            /*sr_anim_stop();
            if (PLAYER_STATE_PLAYING == last_player_state) {
                app_player_play();
            }*/
#endif
            switch (cmd->cmd) {
            case SR_CMD_SET_RED:
                //sys_param->light_colour = lIGHT_COLOUR_RED;
                event.event = LV_EVENT_LIGHT_RGB_SET;
                event.event_data = sys_param;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_SET_WHITE:
                //sys_param->light_colour = lIGHT_COLOUR_WHITE;
                event.event = LV_EVENT_LIGHT_RGB_SET;
                event.event_data = sys_param;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_SET_YELLOW:
                //sys_param->light_colour = lIGHT_COLOUR_YELLOW;
                event.event = LV_EVENT_LIGHT_RGB_SET;
                event.event_data = sys_param;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_SET_BLUE:
                //sys_param->light_colour = lIGHT_COLOUR_BLUE;
                event.event = LV_EVENT_LIGHT_RGB_SET;
                event.event_data = sys_param;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_LIGHT_ON:
                sys_param->light_onoff = true;
                event.event = LV_EVENT_LIGHT_ONOFF_SET;
                event.event_data = sys_param;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_LIGHT_OFF:
                sys_param->light_onoff = false;
                event.event = LV_EVENT_LIGHT_ONOFF_SET;
                event.event_data = sys_param;
                app_lvgl_send_event(&event);
                break;
            case SR_CMD_CUSTOMIZE_COLOR: {
                // uint16_t h;
                // uint8_t s, v;
                //app_pwm_led_get_customize_color(&h, &s, &v);
                //app_pwm_led_set_all_hsv(h, s, v);
            } break;

            case SR_CMD_AC_SET_ON:
                event.event = LV_EVENT_AC_SET_ON;
                app_lvgl_send_event(&event);
            break;
            case SR_CMD_AC_SET_OFF:
                event.event = LV_EVENT_AC_SET_OFF;
                app_lvgl_send_event(&event);
            break;
            case SR_CMD_AC_TEMP_ADD:
             event.event = LV_EVENT_AC_TEMP_ADD;
                app_lvgl_send_event(&event);
            break;
            case SR_CMD_AC_TEMP_DEC:
                event.event = LV_EVENT_AC_TEMP_DEC;
                app_lvgl_send_event(&event);
            break;
            default:
                ESP_LOGE(TAG, "Unknow cmd");
                break;
            }
#if !SR_RUN_TEST
            sr_echo_play(AUDIO_OK);
#endif
        }
    }
    vTaskDelete(NULL);
}
