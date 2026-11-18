// JD9366 触摸屏驱动程序 - v2.5
// 按照原厂代码完全重新实现BackDoor协议

#include <string.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_touch_jd9366.h"

static const char *TAG = "JD9366";

/* --- Debug counters for touch-issue diagnosis (dumped by touch_diag_task
 * in ksdiy_lvgl_port.c every few seconds) --- */
volatile uint32_t jd9366_dbg_reads  = 0;  /* read_data() invocations       */
volatile uint32_t jd9366_dbg_errors = 0;  /* I2C transport failures         */
volatile uint32_t jd9366_dbg_hits   = 0;  /* frames containing a touch      */
volatile uint32_t jd9366_dbg_last_x = 0;
volatile uint32_t jd9366_dbg_last_y = 0;

// 按照原厂代码定义的地址常量
#define JD9366T_SOC_BASE_ADDR 0x400080                                      // SOC基地址
#define JD9366T_SOC_REG_ADDR_CHIP_ID2 ((JD9366T_SOC_BASE_ADDR << 8) + 0x76) // 芯片ID2寄存器
#define JD9366T_ID 0x9032                                                   // 期望的芯片ID
#define JD9366T_TOUCH_DATA_REG 0x20011120                                   // 触摸数据寄存器

// 配置常量
#define JD9366T_TOUCH_DATA_LEN 60  // 简化：只读取10字节用于测试
#define JD9366T_MAX_TOUCH_POINTS 5 // 最大触摸点数
#define JD9366T_COORD_DATA_SIZE 5  // 每个触摸点的数据大小

// 硬件复位时序
#define RESET_LOW_TIME_MS 50   // 复位低电平时间
#define RESET_HIGH_TIME_MS 100 // 复位后等待时间

// 函数声明
static esp_err_t jd9366_enter_backdoor(esp_lcd_touch_handle_t tp);
static esp_err_t jd9366_read_chip_id(esp_lcd_touch_handle_t tp, uint16_t *chip_id);
static esp_err_t touch_jd9366_read_data(esp_lcd_touch_handle_t tp);
static bool touch_jd9366_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
static esp_err_t touch_jd9366_del(esp_lcd_touch_handle_t tp);
static esp_err_t touch_jd9366_reset(esp_lcd_touch_handle_t tp);
esp_err_t esp_lcd_touch_new_i2c_jd9366(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *out_touch)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(io, ESP_ERR_INVALID_ARG, err, TAG, "Invalid io handle");
    ESP_GOTO_ON_FALSE(config, ESP_ERR_INVALID_ARG, err, TAG, "Invalid config");
    ESP_GOTO_ON_FALSE(out_touch, ESP_ERR_INVALID_ARG, err, TAG, "Invalid output handle");

    // 分配内存
    esp_lcd_touch_handle_t esp_lcd_touch_jd9366 = heap_caps_calloc(1, sizeof(esp_lcd_touch_t), MALLOC_CAP_DEFAULT);
    ESP_GOTO_ON_FALSE(esp_lcd_touch_jd9366, ESP_ERR_NO_MEM, err, TAG, "no mem for JD9366 controller");

    // 初始化基本配置
    esp_lcd_touch_jd9366->io = io;

    // 设置基础接口函数
    esp_lcd_touch_jd9366->read_data = touch_jd9366_read_data;
    esp_lcd_touch_jd9366->get_xy = touch_jd9366_get_xy;
    esp_lcd_touch_jd9366->del = touch_jd9366_del;
    /* Mutex */
    esp_lcd_touch_jd9366->data.lock.owner = portMUX_FREE_VAL;
    /* Save config */
    memcpy(&esp_lcd_touch_jd9366->config, config, sizeof(esp_lcd_touch_config_t));
    esp_lcd_touch_io_jd9366_config_t *jd9366_config = (esp_lcd_touch_io_jd9366_config_t *)esp_lcd_touch_jd9366->config.driver_data;

    /* Configure interrupt GPIO if specified (JD9366: edge-triggered, falling edge, Type II) */
    if (esp_lcd_touch_jd9366->config.int_gpio_num != GPIO_NUM_NC)
    {
        const gpio_config_t int_gpio_config = {
            .mode = GPIO_MODE_INPUT,
            .intr_type = GPIO_INTR_NEGEDGE,  /* JD9366 touch INT: falling edge */
            .pull_down_en = 0,
            .pull_up_en = 1,  /* Pull-up: INT pin is open-drain, active low */
            .pin_bit_mask = BIT64(esp_lcd_touch_jd9366->config.int_gpio_num),
        };
        ret = gpio_config(&int_gpio_config);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "INT GPIO config failed");
        ESP_LOGI(TAG, "Touch INT GPIO configured: %d (falling edge)", esp_lcd_touch_jd9366->config.int_gpio_num);
    }

    if (esp_lcd_touch_jd9366->config.rst_gpio_num != GPIO_NUM_NC)
    {
        /* Prepare pin for touch controller reset */
        ESP_RETURN_ON_ERROR(gpio_set_level(esp_lcd_touch_jd9366->config.rst_gpio_num, 0), TAG, "GPIO set level error!");

        vTaskDelay(pdMS_TO_TICKS(200));
        ESP_RETURN_ON_ERROR(gpio_set_level(esp_lcd_touch_jd9366->config.rst_gpio_num, 1), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    else
    {
        /* Reset controller */
        ret = touch_jd9366_reset(esp_lcd_touch_jd9366);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "JD9366 reset failed");
    }
    // 初始化触摸芯片
    ESP_LOGI(TAG, "开始初始化JD9366触摸芯片");

  
    // 读取并验证芯片ID
    uint16_t chip_id = 0;
    ret = jd9366_read_chip_id(esp_lcd_touch_jd9366, &chip_id);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "读取到芯片ID: 0x%04X (期望: 0x%04X)", chip_id, JD9366T_ID);
        if (chip_id != JD9366T_ID)
        {
            ESP_LOGW(TAG, "芯片ID不匹配，但继续初始化");
        }
    }
    else
    {
        ESP_LOGE(TAG, "读取芯片ID失败");
        goto err_free;
    }
  // 进入BackDoor模式
    ret = jd9366_enter_backdoor(esp_lcd_touch_jd9366);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "进入BackDoor模式失败");
        goto err_free;
    }

    ESP_LOGI(TAG, "JD9366驱动初始化完成");

    *out_touch = esp_lcd_touch_jd9366;
    return ESP_OK;

err_free:
    free(esp_lcd_touch_jd9366);
err:
    return ret;
}

// 进入BackDoor模式 - 按照原厂代码实现
static esp_err_t jd9366_enter_backdoor(esp_lcd_touch_handle_t tp)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "进入BackDoor模式");

    // 按照原厂代码：jadard_bus_write(addrBuf, 5, writeBuf, 1)
    // 模拟: 发送 {0xF2, 0xAA, 0x55, 0x0F, 0xF0, 0x68} 作为一个完整命令
    uint8_t backdoor_cmd[] = {0xF2, 0xAA, 0x55, 0x0F, 0xF0, 0x68};

    ret = esp_lcd_panel_io_tx_param(tp->io, 0x40008004, (uint8_t[]){0xa5}, 1);
    /* 0x40008081: Jadard SOC private config register (not in public DS).
     * EXPERIMENT LOG (closed): bit0 was suspected TP INT enable — DISPROVED.
     * Writing 0x01 vs 0x00 changes nothing observable: TSIX stays undriven
     * (ANYEDGE counter on GPIO21 stayed 0 while polling kept reporting
     * touches), and the register READS BACK 0x01 even after writing 0x00
     * -> likely read-only or write-ignored (default 0x01), so the vendor
     * 0x00 write was a no-op all along. Kept at vendor-reference 0x00.
     * Enabling TSIX requires the Jadard BackDoor register map (NDA doc) —
     * request from panel vendor. */
    ret = esp_lcd_panel_io_tx_param(tp->io, 0x40008081, (uint8_t[]){0x00}, 1);

    /* Boot-time check: chip ID + this readback prove the I2C register READ
     * path works. NOTE: the value read here is the register default, NOT
     * the written value (see experiment log above). */
    {
        uint8_t rb = 0xAA;
        esp_err_t rret = esp_lcd_panel_io_rx_param(tp->io, 0x40008081, &rb, 1);
        ESP_LOGI(TAG, "backdoor 0x40008081 readback: ret=%s val=0x%02X",
                 esp_err_to_name(rret), rb);
    }

    // ret = esp_lcd_panel_io_tx_param(tp->io, 0x20011120, backdoor_cmd, sizeof(backdoor_cmd));
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "发送BackDoor进入命令失败: %s", esp_err_to_name(ret));
    //     return ret;
    // }

    // 等待BackDoor建立
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "进入BackDoor模式成功");
    return ESP_OK;
}

// 读取芯片ID - 按照原厂代码实现
static esp_err_t jd9366_read_chip_id(esp_lcd_touch_handle_t tp, uint16_t *chip_id)
{

    esp_err_t ret;
    uint8_t data[2];

    // 然后读取数据
    ret = esp_lcd_panel_io_rx_param(tp->io, 0x40008076, data, 2);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "读取芯片ID数据失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 按照原厂代码的字节序组合: (buf[1] << 8) + buf[0]
    *chip_id = (uint16_t)((data[1] << 8) + data[0]);

    ESP_LOGI(TAG, "原始数据: [0]=0x%02X, [1]=0x%02X", data[0], data[1]);

    return ESP_OK;
}

// 读取触摸数据
static esp_err_t touch_jd9366_read_data(esp_lcd_touch_handle_t tp)
{
    esp_err_t ret;
    uint8_t cmd[6] = {0xF3, 0x20, 0x02, 0x11, 0x20, 0x01};
    uint8_t buf[80] = {0};
    
    memset(buf, 0, sizeof(buf));
    // esp_lcd_panel_io_tx_param(tp->io, 0x20021120, cmd, 6);
    
    jd9366_dbg_reads++;
    if (jd9366_dbg_reads <= 3) {
        ESP_LOGI(TAG, "read_data called #%u (indev polling alive)",
                 (unsigned)jd9366_dbg_reads);
    }

    // 读取触摸数据寄存器
    ret = esp_lcd_panel_io_rx_param(tp->io, 0x20021120, &buf, JD9366T_TOUCH_DATA_LEN);
    if (ret != ESP_OK) {
        jd9366_dbg_errors++;
        /* throttled: one line per 50 failures so a dead bus cannot flood log */
        if ((jd9366_dbg_errors % 50) == 1) {
            ESP_LOGW(TAG, "I2C read failed (err #%u, last %s)",
                     (unsigned)jd9366_dbg_errors, esp_err_to_name(ret));
        }
        return ret;
    }
    
    // 显示原始数据用于调试
    ESP_LOGD(TAG, "触摸原始数据: %02x %02x %02x %02x %02x %02x %02x %02x", 
             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    
    // portENTER_CRITICAL(&tp->data.lock);
    
    // 解析触摸数据
    if (buf[0] != 0x00) {  // 检查是否有触摸点
        // 数据格式: [触摸ID] [00] [FF] [X区域] [X坐标] [Y区域] [Y坐标] [?]
        uint8_t touch_id = buf[0];
        uint8_t x_region = buf[3];  // X轴区域ID (0-2)
        uint8_t x_coord = buf[4];   // X轴坐标
        uint8_t y_region = buf[5];  // Y轴区域ID (0-4)
        uint8_t y_coord = buf[6];   // Y轴坐标
        
        // 计算实际屏幕坐标
        // X轴：屏幕宽度均分为3个区域
        uint16_t screen_width = tp->config.x_max;
        uint16_t x_region_width = screen_width / 3;
        // 改进的坐标计算：使用更精确的比例计算
        uint16_t actual_x = x_region * x_region_width + ((uint32_t)x_coord * x_region_width) / 255;
        
        // Y轴：屏幕高度均分为5个区域  
        uint16_t screen_height = tp->config.y_max;
        uint16_t y_region_height = screen_height / 5;
        uint16_t actual_y = y_region * y_region_height + ((uint32_t)y_coord * y_region_height) / 255;
        
        // 边界检查
        if (actual_x > screen_width) actual_x = screen_width;
        if (actual_y > screen_height) actual_y = screen_height;
        
        // 存储坐标数据
        tp->data.coords[0].x = actual_x;
        tp->data.coords[0].y = actual_y;
        tp->data.coords[0].strength = 50;  // 默认压力值
        tp->data.points = 1;

        jd9366_dbg_hits++;
        jd9366_dbg_last_x = actual_x;
        jd9366_dbg_last_y = actual_y;
        /* throttled: ~1 line per 20 touch frames (~0.6 s while pressed) */
        if ((jd9366_dbg_hits % 20) == 1) {
            ESP_LOGI(TAG, "touch: raw[%02x %02x %02x %02x %02x %02x] -> (%u,%u)",
                     buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
                     (unsigned)actual_x, (unsigned)actual_y);
        }
    } else {
        // 没有触摸点
        tp->data.points = 0;
        ESP_LOGD(TAG, "无触摸点");
    }
    
    // portEXIT_CRITICAL(&tp->data.lock);
    
    return ESP_OK;
}
static esp_err_t touch_jd9366_reset(esp_lcd_touch_handle_t tp)
{
    assert(tp != NULL);

    if (tp->config.rst_gpio_num != GPIO_NUM_NC)
    {
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, tp->config.levels.reset), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, !tp->config.levels.reset), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return ESP_OK;
}
// 获取坐标数据 - 简化实现
static bool touch_jd9366_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{

    assert(tp != NULL);
    assert(x != NULL);
    assert(y != NULL);
    assert(point_num != NULL);
    assert(max_point_num > 0);

    portENTER_CRITICAL(&tp->data.lock);

    /* Count of points */
    *point_num = (tp->data.points > max_point_num ? max_point_num : tp->data.points);

    for (size_t i = 0; i < *point_num; i++)
    {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;

        if (strength)
        {
            strength[i] = tp->data.coords[i].strength;
        }
    }

    /* Invalidate */
    tp->data.points = 0;

    portEXIT_CRITICAL(&tp->data.lock);

    return (*point_num > 0);
}

// 删除驱动实例
static esp_err_t touch_jd9366_del(esp_lcd_touch_handle_t tp)
{

    esp_lcd_touch_handle_t jd9366 = tp;

    // 释放内存
    free(jd9366);

    return ESP_OK;
}
