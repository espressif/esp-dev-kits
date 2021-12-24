#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "avifile.h"
#include "vidoplayer.h"

//#define CONFIG_AVI_AUDIO

#ifdef CONFIG_AVI_AUDIO
#include "pwm_audio.h"
#endif

static const char *TAG = "avi player";

#define PLAYER_CHECK(a, str, ret)  if(!(a)) {                                             \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);      \
        return (ret);                                                                   \
        }

/**
 * TODO: how to recognize each stream id
 */
#define  T_vids  _REV(0x30306463)
#define  T_auds  _REV(0x30317762)

extern AVI_TypeDef AVI_file;

static uint32_t _REV(uint32_t value){
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 | 
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24; 
}

#ifdef CONFIG_AVI_AUDIO
static void audio_init(void)
{
    pwm_audio_config_t pac;
    pac.duty_resolution    = LEDC_TIMER_10_BIT;
    pac.gpio_num_left      = 25;
    pac.ledc_channel_left  = LEDC_CHANNEL_0;
    pac.gpio_num_right     = -1;
    pac.ledc_channel_right = LEDC_CHANNEL_1;
    pac.ledc_timer_sel     = LEDC_TIMER_0;
    pac.tg_num             = TIMER_GROUP_0;
    pac.timer_num          = TIMER_0;
    pac.ringbuf_len        = 1024 * 8;
    pwm_audio_init(&pac);

    pwm_audio_set_volume(0);
}
#endif

static uint32_t read_frame(FILE *file, uint8_t *buffer, uint32_t length, uint32_t *fourcc)
{
    AVI_CHUNK_HEAD head;
    fread(&head, sizeof(AVI_CHUNK_HEAD), 1, file);
    if (head.FourCC) {
        /* code */
    }
    *fourcc = head.FourCC;
    if (head.size % 2) {
        head.size++;    //奇数加1
    }
    if (length < head.size) {
        ESP_LOGE(TAG, "frame size too large");
        return 0;
    }

    fread(buffer, head.size, 1, file);
    return head.size;
}

void avi_play(const char *filename, uint8_t *outbuffer,
                const size_t outbuffer_width, const size_t outbuffer_height,
                lcd_write_cb lcd_cb, const size_t lcd_width, const size_t lcd_height)
{
    FILE *avi_file;
    int ret;
    size_t  BytesRD;
    uint32_t  Strsize;
    uint32_t  Strtype;
    uint8_t *pbuffer;
    uint32_t buffer_size = 22*1024;

    avi_file = fopen(filename, "rb");
    if (avi_file == NULL) {
        ESP_LOGE(TAG, "Cannot open %s", filename);
        return;
    }

    pbuffer = malloc(buffer_size);
    if (pbuffer == NULL) {
        ESP_LOGE(TAG, "Cannot alloc memory for palyer");
        fclose(avi_file);
        return;
    }

    BytesRD = fread(pbuffer, 20480, 1, avi_file);
    ret = AVI_Parser(pbuffer, BytesRD);
    if (0 > ret) {
        ESP_LOGE(TAG, "parse failed (%d)", ret);
        return;
    }

#ifdef CONFIG_AVI_AUDIO
    audio_init();
    pwm_audio_set_param(AVI_file.auds_sample_rate, AVI_file.auds_bits, AVI_file.auds_channels);
    pwm_audio_start();
#endif

    uint16_t img_width = AVI_file.vids_width;
    uint16_t img_height = AVI_file.vids_height;
    uint8_t *img_rgb888 = heap_caps_malloc(img_width*img_height*2, MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL);
    if (NULL == img_rgb888){
        ESP_LOGE(TAG, "malloc for rgb888 failed");
        goto EXIT;
    }

    fseek(avi_file, AVI_file.movi_start, SEEK_SET); // 偏移到movi list
    Strsize = read_frame(avi_file, pbuffer, buffer_size, &Strtype);
    BytesRD = Strsize+8;

    while (1) { //播放循环
        if (BytesRD >= AVI_file.movi_size) {
            ESP_LOGI(TAG, "paly end");
            break;
        }
        if (Strtype == T_vids) { //显示帧
            int64_t fr_end = esp_timer_get_time();
            mjpegdraw(pbuffer, Strsize, outbuffer, outbuffer_width, outbuffer_height, lcd_cb, lcd_width, lcd_height);
            ESP_LOGI(TAG, "jpg decode %ums", (uint32_t)((esp_timer_get_time() - fr_end) / 1000));fr_end = esp_timer_get_time();
            // ESP_LOGI(TAG, "draw %ums", (uint32_t)((esp_timer_get_time() - fr_end) / 1000));fr_end = esp_timer_get_time();

        }//显示帧
        else if (Strtype == T_auds) { //音频输出
#ifdef CONFIG_AVI_AUDIO
            size_t cnt;
            pwm_audio_write((uint8_t *)pbuffer, Strsize, &cnt, 500 / portTICK_PERIOD_MS);
#endif
        } else {
            ESP_LOGE(TAG, "unknow frame");
            break;
        }
        Strsize = read_frame(avi_file, pbuffer, buffer_size, &Strtype); //读入整帧
        ESP_LOGD(TAG, "type=%x, size=%d", Strtype, Strsize);
        BytesRD += Strsize+8;
    }
EXIT:
#ifdef CONFIG_AVI_AUDIO
pwm_audio_deinit();
#endif
free(img_rgb888);
free(pbuffer);
fclose(avi_file);
}
