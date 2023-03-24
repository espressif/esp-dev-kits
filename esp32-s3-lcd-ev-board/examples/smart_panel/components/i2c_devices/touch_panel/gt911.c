
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "gt911.h"
#include "bsp_i2c.h"

static const char *TAG = "GT911";
/* Reference: https://github.com/goodix/goodix_gt9xx_public
 * Datasheet: https://github.com/hadess/gt9xx/blob/master/specifications/GT911%20Datasheet.pdf
 */


/* Register defines */
#define GT_REG_CMD  0x8040

#define GT_REG_CFG  0x8047
#define GT_REG_DATA 0x8140


// Write only registers
#define GT911_REG_COMMAND        0x8040
#define GT911_REG_LED_CONTROL    0x8041
#define GT911_REG_PROXIMITY_EN   0x8042

// Read/write registers
// The version number of the configuration file
#define GT911_REG_CONFIG_DATA  0x8047
// X output maximum value (LSB 2 bytes)
#define GT911_REG_MAX_X        0x8048
// Y output maximum value (LSB 2 bytes)
#define GT911_REG_MAX_Y        0x804A
// Maximum number of output contacts: 1~5 (4 bit value 3:0, 7:4 is reserved)
#define GT911_REG_MAX_TOUCH    0x804C

// Module switch 1
// 7:6 Reserved, 5:4 Stretch rank, 3 X2Y, 2 SITO (Single sided ITO touch screen), 1:0 INT Trigger mode */
#define GT911_REG_MOD_SW1      0x804D
// Module switch 2
// 7:1 Reserved, 0 Touch key */
#define GT911_REG_MOD_SW2      0x804E

// Number of debuffs fingers press/release
#define GT911_REG_SHAKE_CNT    0x804F

// ReadOnly registers (device and coordinates info)
// Product ID (LSB 4 bytes, GT9110: 0x06 0x00 0x00 0x09)
#define GTP_REG_VERSION           0x8140
// Firmware version (LSB 2 bytes)
#define GT911_REG_FW_VER       0x8144

// Current output X resolution (LSB 2 bytes)
#define GT911_READ_X_RES       0x8146
// Current output Y resolution (LSB 2 bytes)
#define GT911_READ_Y_RES       0x8148
// Module vendor ID
#define GTP_REG_SENSOR_ID   0x814A

#define GTP_READ_COORD_ADDR  0x814E

/* Commands for REG_COMMAND */
//0: read coordinate state
#define GT911_CMD_READ         0x00
// 1: difference value original value
#define GT911_CMD_DIFFVAL      0x01
// 2: software reset
#define GT911_CMD_SOFTRESET    0x02
// 3: Baseline update
#define GT911_CMD_BASEUPDATE   0x03
// 4: Benchmark calibration
#define GT911_CMD_CALIBRATE    0x04
// 5: Off screen (send other invalid)
#define GT911_CMD_SCREEN_OFF   0x05

/* When data needs to be sent, the host sends command 0x21 to GT9x,
 * enabling GT911 to enter "Approach mode" and work as a transmitting terminal */
#define GT911_CMD_HOTKNOT_TX   0x21


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// 0x28/0x29 (0x14 7bit)
// 0xBA/0xBB (0x5D 7bit)
#define I2C_ADDR1 0x5D
#define I2C_ADDR2 0x14

#define MASK_BIT_8      0x80
#define GTP_TOOL_PEN    1
#define GTP_TOOL_FINGER 2

#define MAX_KEY_NUMS 4
#define GTP_CONFIG_MIN_LENGTH   186
#define GTP_CONFIG_MAX_LENGTH 240
#define GTP_ADDR_LENGTH       2

/***************************PART1:ON/OFF define*******************************/
#define GTP_DEBUG_ON          1
#define GTP_DEBUG_ARRAY_ON    0
#define GTP_DEBUG_FUNC_ON     0

#define GTP_DEFAULT_MAX_X    720    /* default coordinate max values */
#define GTP_DEFAULT_MAX_Y    1080
#define GTP_DEFAULT_MAX_WIDTH    1024
#define GTP_DEFAULT_MAX_PRESSURE 1024
#define GTP_DEFAULT_INT_TRIGGER  1 /* 1 rising, 2 falling */
#define GTP_MAX_TOUCH_ID     16

struct i2c_client {
    i2c_bus_device_handle_t i2c_handle;
};

struct goodix_point_t {
    int id;
    int x;
    int y;
    int w;
    int p;
    int tool_type;
};

struct goodix_config_data {
    int length;
    u8 data[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH];
};

struct goodix_ts_platform_data {
    int irq_gpio;
    int rst_gpio;
    u32 irq_flags;
    u32 abs_size_x;
    u32 abs_size_y;
    u32 max_touch_id;
    u32 max_touch_width;
    u32 max_touch_pressure;

    u32 pen_suppress_finger;
    struct goodix_config_data config;
};

enum {
    REPORT_THREAD_ENABLED = 0,
    HRTIMER_USED,
    FW_ERROR,

    DOZE_MODE,
    SLEEP_MODE,
    POWER_OFF_MODE,
    RAW_DATA_MODE,

    FW_UPDATE_RUNNING,
    PANEL_RESETTING
};


struct goodix_fw_info {
    u8 pid[6];
    u16 version;
    u8 sensor_id;
};

struct goodix_ts_data {
    unsigned long flags; /* This member record the device status */

    struct i2c_client client;
    // struct goodix_ts_platform_data *pdata;
    /* use pinctrl control int-pin output low or high */
    struct regulator *vdd_ana;
    struct regulator *vcc_i2c;

    struct goodix_fw_info fw_info;
    bool force_update;
    bool init_done;
};

static struct  goodix_ts_data g_ts_data = {0};

static uint8_t g911xFW[] = {
    0x42,
    0xD0, 0x02, 0x00, 0x05, 0x05, 0x75, 0x01, 0x01, 0x0F, 0x24,
    0x0F, 0x64, 0x3C, 0x03, 0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x16, 0x19, 0x1C, 0x14, 0x8C, 0x0E, 0x0E, 0x24, 0x00, 0x31,
    0x0D, 0x00, 0x00, 0x00, 0x83, 0x33, 0x1D, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x0A, 0x00, 0x2B, 0x1C, 0x3C, 0x94, 0xD5,
    0x03, 0x08, 0x00, 0x00, 0x04, 0x93, 0x1E, 0x00, 0x82, 0x23, 0x00,
    0x74, 0x29, 0x00, 0x69, 0x2F, 0x00, 0x5F, 0x37, 0x00, 0x5F, 0x20,
    0x40, 0x60, 0x00, 0xF0, 0x40, 0x30, 0x55, 0x50, 0x27, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x19, 0x00, 0x00,
    0x50, 0x50, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12,
    0x14, 0x16, 0x18, 0x1A, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24, 0x26, 0x28, 0x29, 0x2A, 0x1C,
    0x18, 0x16, 0x14, 0x13, 0x12, 0x10, 0x0F, 0x0C, 0x0A, 0x08, 0x06,
    0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x01
};

/*
 * return: 2 - ok, < 0 - i2c transfer error
 */
static int gtp_i2c_read(struct i2c_client *client, u8 *buf, int len)
{
    uint16_t address = (buf[0] << 8) + buf[1];
    esp_err_t ret = i2c_bus_read_reg16(client->i2c_handle, address, len - 2, &buf[2]);
    return ESP_OK == ret ? 2 : -1;
}

/*******************************************************
 * Function:
 *  Write data to the i2c slave device.
 * Input:
 *  client: i2c device.
 *  buf[0~1]: write start address.
 *  buf[2~len-1]: data buffer
 *  len: GTP_ADDR_LENGTH + write bytes count
 * Output:
 *  numbers of i2c_msgs to transfer:
 *      1: succeed, otherwise: failed
 *********************************************************/
static int gtp_i2c_write(struct i2c_client *client, u8 *buf, int len)
{
    uint16_t address = (buf[0] << 8) + buf[1];
    esp_err_t ret = i2c_bus_write_reg16(client->i2c_handle, address, len - 2, &buf[2]);
    return ESP_OK == ret ? 2 : -1;
}

static int gtp_get_fw_info(struct i2c_client *client, struct goodix_fw_info *fw_info)
{
    int ret = -1;
    u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};

    ret = gtp_i2c_read(client, buf, sizeof(buf));
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed read fw_info\n");
        return ret;
    }

    fw_info->version = (buf[7] << 8) | buf[6];

    /* product id */
    memset(fw_info, 0, sizeof(*fw_info));
    memcpy(fw_info->pid, buf + GTP_ADDR_LENGTH, 4);

    if (buf[5] == 0x00) {
        ESP_LOGI(TAG, "IC Version: %c%c%c_%02X%02X", buf[2], buf[3], buf[4], buf[7], buf[6]);
    } else {
        ESP_LOGI(TAG, "IC Version: %c%c%c%c_%02X%02X",  buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);
    }

    /* read sensor id */
    fw_info->sensor_id = 0xff;
    // ret = gtp_i2c_read_dbl_check(client, GTP_REG_SENSOR_ID,
    //                              &fw_info->sensor_id, 1);
    // if (SUCCESS != ret || fw_info->sensor_id >= 0x06) {
    //     ESP_LOGE(TAG, "Failed get valid sensor_id(0x%02X), No Config Sent", fw_info->sensor_id);

    //     fw_info->sensor_id = 0xff;
    // }
    return ret;
}

// static uint8_t fwResolution(struct goodix_ts_data *ts, uint16_t maxX, uint16_t maxY)
// {
//     uint8_t len = 0x8100 - GT_REG_CFG + 1;
//     uint8_t cfg[len] = {GT_REG_CFG >> 8,
//                         GT_REG_CFG & 0xFF, 0
//                        };
//     int error = gtp_i2c_read(&ts->client, cfg, len + 2);
//     if (error) {
//         return error;
//     }
// //   read(GT_REG_CFG, cfg, len);

//     cfg[1] = (maxX & 0xff);
//     cfg[2] = (maxX >> 8);
//     cfg[3] = (maxY & 0xff);
//     cfg[4] = (maxY >> 8);
//     cfg[len - 2] = calcChecksum(cfg, len - 2);
//     cfg[len - 1] = 1;

//     return write(GT_REG_CFG, cfg, len);
// }

/*
 * return touch state register value
 * pen event id fixed with 9 and set tool type TOOL_PEN
 *
 */
u8 gtp_get_points(struct goodix_ts_data *ts, struct goodix_point_t *points, u8 *key_value)
{
    int ret;
    int i;
    u8 *coor_data = NULL;
    u8 finger_state = 0;
    u8 touch_num = 0;
    u8 end_cmd[3] = { GTP_READ_COORD_ADDR >> 8,
                      GTP_READ_COORD_ADDR & 0xFF, 0
                    };
    u8 point_data[2 + 1 + 8 * GTP_MAX_TOUCH_ID + 1] = {
        GTP_READ_COORD_ADDR >> 8, GTP_READ_COORD_ADDR & 0xFF
    };

    ret = gtp_i2c_read(&ts->client, point_data, 12);
    if (ret < 0) {
        ESP_LOGE(TAG, "I2C transfer error. errno:%d", ret);
        return 0;
    }

    finger_state = point_data[GTP_ADDR_LENGTH];
    if (finger_state == 0x00) {
        return 0;
    }

    touch_num = finger_state & 0x0f;
    if ((finger_state & MASK_BIT_8) == 0 ) {
        ESP_LOGE(TAG, "Invalid touch state: 0x%x", finger_state);
        finger_state = 0;
        goto exit_get_point;
    }

    if (touch_num > 1) {
        u8 buf[8 * GTP_MAX_TOUCH_ID] = {
            (GTP_READ_COORD_ADDR + 10) >> 8,
                                       (GTP_READ_COORD_ADDR + 10) & 0xff
        };

        ret = gtp_i2c_read(&ts->client, buf, 2 + 8 * (touch_num - 1));
        if (ret < 0) {
            ESP_LOGE(TAG, "I2C error. %d", ret);
            finger_state = 0;
            goto exit_get_point;
        }
        memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));
    }

    /* panel have touch key */
    //if (ts->pdata->key_nums) {
    *key_value = point_data[3 + 8 * touch_num];

    /* 0x20_UPKEY 0X10_DOWNKEY 0X40_ALLKEYDOWN */
    //  *key_value &= 0x0F;
    //} else {
    //  *key_value = 0;
    //}

    memset(points, 0, sizeof(*points) * GTP_MAX_TOUCH_ID);
    for (i = 0; i < touch_num; i++) {
        coor_data = &point_data[i * 8 + 3];
        points[i].id = coor_data[0];
        points[i].x = coor_data[1] | (coor_data[2] << 8);
        points[i].y = coor_data[3] | (coor_data[4] << 8);
        points[i].w = coor_data[5] | (coor_data[6] << 8);
        /* if pen hover points[].p must set to zero */
        points[i].p = coor_data[5] | (coor_data[6] << 8);

        /* TODO: Following code only for debug use */
        ESP_LOGD(TAG, "[%d][%d %d %d]", points[i].id, points[i].x, points[i].y, points[i].p);

        /* pen device coordinate */
        if (points[i].id & 0x80) {
            points[i].tool_type = GTP_TOOL_PEN;
            // points[i].id = 10;
            // if (ts->pdata->pen_suppress_finger) {
            //     points[0] = points[i];
            //     memset(++points, 0, sizeof(*points) * (GTP_MAX_TOUCH_ID - 1));
            //     finger_state &= 0xf0;
            //     finger_state |= 0x01;
            //     break;
            // }
        } else {
            points[i].tool_type = GTP_TOOL_FINGER;
        }
    }

exit_get_point:
    if (1/*!test_bit(RAW_DATA_MODE, &ts->flags)*/) {
        ret = gtp_i2c_write(&ts->client, end_cmd, 3);
        if (ret < 0) {
            ESP_LOGE(TAG, "I2C write end_cmd error!");
        }
    }
    return finger_state;
}

int gt911_pos_read(uint16_t *xpos, uint16_t *ypos)
{
    u8 key_value = 0;
    struct goodix_point_t points[GTP_MAX_TOUCH_ID];
    u8 finger_state = gtp_get_points(&g_ts_data, &points, &key_value);

    if (finger_state & 0x0f) {
        *xpos = points[0].x;
        *ypos = points[0].y;
        return 1;
    }
    return 0;
}

int gt911_init(uint8_t i2c_addr)
{
    if (NULL != g_ts_data.client.i2c_handle) {
        return ESP_ERR_INVALID_STATE;
    }

    bsp_i2c_add_device(&g_ts_data.client.i2c_handle, i2c_addr);
    if (NULL == g_ts_data.client.i2c_handle) {
        return ESP_FAIL;
    }
    struct goodix_fw_info fw_info;
    gtp_get_fw_info(&g_ts_data.client, &fw_info);
    if (!(fw_info.pid[0] == '9' && fw_info.pid[1] == '1' && fw_info.pid[2] == '1')) {
        printf("pid error (%c%c%c)\n", fw_info.pid[0], fw_info.pid[1], fw_info.pid[2]);
        return ESP_FAIL;
    }

    return ESP_OK;
}
