
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl.h"
#include "lv_port.h"
#include "bsp_board.h"
#include "bsp_lcd.h"
#include "indev/indev.h"

static const char *TAG = "lv_port";

static lv_disp_drv_t disp_drv;

static void lv_port_disp_init(void);
static esp_err_t lv_port_indev_init(void);
static esp_err_t lv_port_tick_init(void);

static SemaphoreHandle_t lvgl_mutex = NULL;
static TaskHandle_t g_lvgl_task_handle;
static lv_indev_t *indev_touchpad;
static lv_indev_t *indev_button;

void lv_port_sem_take(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (g_lvgl_task_handle != task) {
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
    }
}

void lv_port_sem_give(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (g_lvgl_task_handle != task) {
        xSemaphoreGive(lvgl_mutex);
    }
}

static void lvgl_task(void *args)
{
    while (1) {
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
        lv_task_handler();
        xSemaphoreGive(lvgl_mutex);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void lv_port_init(void)
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    lv_port_tick_init();
    lvgl_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(lvgl_mutex);
    xTaskCreatePinnedToCore(&lvgl_task, "lvgl_task", 4096, NULL, 5, NULL, tskNO_AFFINITY);
}

/**
 * @brief Tell LVGL that LCD flush done.
 *
 * @return true Call `portYIELD_FROM_ISR()` after esp-lcd ISR return.
 * @return false Do nothing after esp-lcd ISR return.v
 */
static bool lv_port_flush_ready(void *args)
{
    /* Inform the graphics library that you are ready with the flushing */
    lv_disp_flush_ready(&disp_drv);

    /* portYIELD_FROM_ISR (true) or not (false). */
    return false;
}

/**
 * @brief LCD flush function callback for LVGL.
 *
 * @param disp_drv
 * @param area
 * @param color_p
 */
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    (void) disp_drv;

    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    bsp_lcd_flush(area->x1, area->y1, area->x2 + 1, area->y2 + 1, (uint8_t *) color_p);
    // printf("%d, %d, %d, %d\n", area->x1, area->y1, area->x2 + 1, area->y2 + 1);
}

static bool button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    // static uint8_t prev_btn_id = 0;
    static uint32_t last_key = 0;

    /* Read touch point(s) via touch IC */
    indev_data_t indev_data;
    if (ESP_OK != indev_get_major_value(&indev_data)) {
        ESP_LOGE(TAG, "Failed read input device value");
        return false;;
    }

    /*Get the pressed button's ID*/
    if (indev_data.btn_val & 0x02) {
        last_key = LV_KEY_ENTER;
        data->state = LV_BTN_STATE_PRESSED;
        ESP_LOGD(TAG, "ok");
    } else if (indev_data.btn_val & 0x04) {
        data->state = LV_BTN_STATE_PRESSED;
        last_key = LV_KEY_PREV;
        ESP_LOGD(TAG, "prev");
    } else if (indev_data.btn_val & 0x01) {
        data->state = LV_BTN_STATE_PRESSED;
        last_key = LV_KEY_NEXT;
        ESP_LOGD(TAG, "next");
    } else {
        data->state = LV_BTN_STATE_RELEASED;
    }
    data->key = last_key;
    return false;
}

/**
 * @brief Read touchpad data.
 *
 * @param indev_drv
 * @param data
 * @return IRAM_ATTR
 */
static bool touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    /* Read touch point(s) via touch IC */
    indev_data_t indev_data;
    if (ESP_OK != indev_get_major_value(&indev_data)) {
        return false;
    }

    ESP_LOGD(TAG, "Touch (%u) : [%3u, %3u] - 0x%02X", indev_data.pressed, indev_data.x, indev_data.y, indev_data.btn_val);

    /* FT series touch IC might return 0xff before first touch. */
    data->point.x = indev_data.x;
    data->point.y = indev_data.y;

    if (indev_data.pressed) {
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    return false;
}

/**
 * @brief Initialize display driver for LVGL.
 *
 */
static void lv_port_disp_init(void)
{
    // size_t disp_buf_height = 10;
    const board_res_desc_t *brd = bsp_board_get_description();

    /* Option 1 : Allocate memories from heap */
    static lv_disp_buf_t disp_buf;

    const size_t disp_buf_width = brd->LCD_WIDTH, disp_buf_height = 10;
    lv_color_t *lv_buf = (lv_color_t *)heap_caps_malloc(1 * disp_buf_width * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (NULL == lv_buf) {
        ESP_LOGE(TAG, "No free mem for allocating buffer");
        return ;
    }
    lv_disp_buf_init(&disp_buf, lv_buf, NULL, 1 * disp_buf_width * disp_buf_height);

    /*Create a display*/
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = brd->LCD_WIDTH;
    disp_drv.ver_res = brd->LCD_HEIGHT;
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = disp_flush;
    // disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    /**
     * @brief Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.
     *
     */
    // disp_drv.gpu_fill_cb = gpu_fill;

    /* Use lcd_trans_done_cb to inform the graphics library that flush already done */
    bsp_lcd_set_cb(lv_port_flush_ready, NULL);
}

/**
 * @brief Initialize input device for LVGL.
 *
 * @return esp_err_t
 */
static esp_err_t lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */
    static lv_indev_drv_t indev_drv_tp;
    static lv_indev_drv_t indev_drv_btn;

    /* Initialize your touchpad if you have */
    const board_res_desc_t *brd = bsp_board_get_description();
    if (brd->BSP_INDEV_IS_TP) {
        ESP_LOGI(TAG, "Add TP input device to LVGL");
        lv_indev_drv_init(&indev_drv_tp);
        indev_drv_tp.type = LV_INDEV_TYPE_POINTER;
        indev_drv_tp.read_cb = touchpad_read;
        indev_touchpad = lv_indev_drv_register(&indev_drv_tp);

    } else {
        ESP_LOGI(TAG, "Add KEYPAD input device to LVGL");
        lv_indev_drv_init(&indev_drv_btn);
        indev_drv_btn.type = LV_INDEV_TYPE_KEYPAD;
        indev_drv_btn.read_cb = button_read;
        indev_button = lv_indev_drv_register(&indev_drv_btn);
    }

#if CONFIG_LV_PORT_SHOW_MOUSE_CURSOR
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t *cursor_obj = lv_img_create(lv_scr_act());  /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(indev_touchpad, cursor_obj);             /*Connect the image  object to the driver*/
#endif
    return ESP_OK;
}

/**
 * @brief Task to generate ticks for LVGL.
 *
 * @param pvParam Not used.
 */
static void lv_tick_inc_cb(void *data)
{
    uint32_t tick_inc_period_ms = *((uint32_t *) data);

    lv_tick_inc(tick_inc_period_ms);
}

/**
 * @brief Create tick task for LVGL.
 *
 * @return esp_err_t
 */
static esp_err_t lv_port_tick_init(void)
{
    static const uint32_t tick_inc_period_ms = 2;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = lv_tick_inc_cb,
        .name = "",     /* name is optional, but may help identify the timer when debugging */
        .arg = (void*)&tick_inc_period_ms,
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = true,
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    /* The timer has been created but is not running yet. Start the timer now */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));

    return ESP_OK;
}

