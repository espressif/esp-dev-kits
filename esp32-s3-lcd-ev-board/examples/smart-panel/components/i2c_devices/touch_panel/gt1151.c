
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "gt1151.h"
#include "bsp_i2c.h"

static const char *TAG = "GT1511";

/* Reference: https://github.com/goodix/gt1x_driver_generic */

#ifndef GT1151_DEBUG
#define GT1151_DEBUG   0
#endif

#define GT1151_ADDRESS         (0x14)

#define GTP_READ_COOR_ADDR     (0x814E)
#define GT1151_PRODUCT_ID      (0x8140)
#define GT1151_CONFIG          (0x8050)
#define GT1151_COMMAND         (0x8040)

#define GTP_MAX_TOUCH           10
#define GTP_DATA_BUFF_LEN(buf_len)  (1 + 8 * (buf_len) + 2)  //STATUS_TEG(1) + TOUCH_DATA(8*GTP_MAX_TOUCH) + KeyValue(1) + CheckSum(1)

/* bit operation */
#define SET_BIT(data, flag) ((data) |= (flag))
#define CLR_BIT(data, flag) ((data) &= ~(flag))
#define CHK_BIT(data, flag) ((data) & (flag))

#define IS_NUM_OR_CHAR(x)    (((x) >= 'A' && (x) <= 'Z') || ((x) >= '0' && (x) <= '9'))

/* touch states */
#define BIT_TOUCH           0x01

#define GT1151_HW_CONFIG_LEN   239

#define GT1151_I2C_BUS I2C_NUM_0
#define GT1151_TIMEOUT            1000

typedef enum {
    TOUCH_EVENT_DOWN = 1,
    TOUCH_EVENT_UP = 2
} gt1151_touch_event_t;

typedef struct {
    gt1151_touch_event_t event;
    uint32_t track_id;
    uint32_t x_coordinate;
    uint32_t y_coordinate;
    uint32_t width;
    uint32_t timestamp;
} gt1151_touch_info_t;

struct gt1x_version_info {
    uint8_t product_id[5];
    uint32_t patch_id;
    uint32_t mask_id;
    uint8_t sensor_id;
    uint8_t match_opt;
};

typedef struct {
    // rt_base_t irq_pin;
    // rt_base_t rst_pin;
    i2c_bus_device_handle_t i2c_handle;
    uint8_t *hw_config;     // GT1151 IC register config array, size is GT1151_HW_CONFIG_LEN
} gt1151_obj_t;

gt1151_obj_t *gt1151_obj = NULL;

#if GT1151_DEBUG
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
/* Reference: https://my.oschina.net/u/4581400/blog/4760201 */
/**
 * dump_hex
 *
 * @brief dump data in hex format
 *
 * @param buf: User buffer
 * @param size: Dump data size
 * @param number: The number of outputs per line
 *
 * @return void
*/
void dump_hex(const uint8_t *buf, uint32_t size, uint32_t number)
{
    int i, j;

    for (i = 0; i < size; i += number) {
        printf("%08X: ", i);

        for (j = 0; j < number; j++) {
            if (j % 8 == 0) {
                printf(" ");
            }
            if (i + j < size) {
                printf("%02X ", buf[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");

        for (j = 0; j < number; j++) {
            if (i + j < size) {
                printf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        printf("\n");
    }
}
#endif

static void gt1151_delay_ms(uint32_t t)
{
    vTaskDelay(pdMS_TO_TICKS(t));
}

static int gt1151_write_regs(gt1151_obj_t *gt1151_obj, uint16_t reg_start_addr, uint8_t *write_data, uint16_t len)
{
    assert(gt1151_obj);
    assert(write_data);
    assert(len > 0);

    esp_err_t ret = i2c_bus_write_reg16(gt1151_obj->i2c_handle, reg_start_addr, len, write_data);
    return ESP_OK == ret ? 0 : -1;
}

static int gt1151_read_regs(gt1151_obj_t *gt1151_obj, uint16_t reg_start_addr, uint8_t *read_buf, uint16_t len)
{
    assert(gt1151_obj);
    assert(read_buf);
    assert(len > 0);

    esp_err_t ret = i2c_bus_read_reg16(gt1151_obj->i2c_handle, reg_start_addr, len, read_buf);
    return ESP_OK == ret ? 0 : -1;

    // i2c_cmd_handle_t cmd = NULL;
    // cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (GT1151_ADDRESS << 1) | 0, 0x1);
    // i2c_master_write_byte(cmd, (reg_start_addr >> 8) & 0xFF, 0x1);
    // i2c_master_write_byte(cmd, reg_start_addr & 0xFF, 0x1);
    // i2c_master_stop(cmd);
    // ret = i2c_master_cmd_begin(GT1151_I2C_BUS, cmd, pdMS_TO_TICKS(GT1151_TIMEOUT));
    // i2c_cmd_link_delete(cmd);
    // if (ret != ESP_OK) {
    //     return ESP_FAIL;
    // }
    // cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (GT1151_ADDRESS << 1) | 1, 0x1);
    // if (len > 1) {
    //     i2c_master_read(cmd, read_buf, len - 1, 0x0);
    // }
    // i2c_master_read_byte(cmd, &read_buf[len - 1], 0x1);
    // i2c_master_stop(cmd);
    // ret = i2c_master_cmd_begin(GT1151_I2C_BUS, cmd, pdMS_TO_TICKS(GT1151_TIMEOUT));
    // i2c_cmd_link_delete(cmd);

    // return ret;
}

/**
 * This function read the product id
 *
 * @param dev the pointer of rt_i2c_client
 * @param read_buf the buffer for product id
 * @param len the length for read_buf, should be 4.
 *
 * @return the read status, 0 reprensents  read the product id successfully.
 */
static int gt1151_get_product_id(gt1151_obj_t *gt1151_obj, uint8_t *read_buf)
{
    assert(gt1151_obj);

    uint8_t buf[4];

    if (gt1151_read_regs(gt1151_obj, GT1151_PRODUCT_ID, buf, 4) != 0) {
        printf("gt1151 get product id failed\n");
        return -1;
    }

    memcpy(read_buf, buf, 4);
    return 0;
}

static int gt1x_read_version(struct gt1x_version_info *ver_info)
{
    int ret = -1;
    uint8_t buf[12] = { 0 };
    uint32_t mask_id = 0;
    uint32_t patch_id = 0;
    uint8_t product_id[5] = { 0 };
    uint8_t sensor_id = 0;
    uint8_t match_opt = 0;
    int i, retry = 3;
    uint8_t checksum = 0;

    while (retry--) {
        ret = gt1151_read_regs(gt1151_obj, GT1151_PRODUCT_ID, buf, sizeof(buf));
        if (!ret) {
            checksum = 0;

            for (i = 0; i < sizeof(buf); i++) {
                checksum += buf[i];
            }

            if (checksum == 0 &&    /* first 3 bytes must be number or char */
                    IS_NUM_OR_CHAR(buf[0]) && IS_NUM_OR_CHAR(buf[1]) &&
                    IS_NUM_OR_CHAR(buf[2]) && buf[10] != 0xFF) {
                /*sensor id == 0xFF, retry */
                break;
            } else {
                ESP_LOGE(TAG, "Read version failed!(checksum error)");
            }
        } else {
            ESP_LOGE(TAG, "Read version failed!");
        }
        printf("Read version : %d\n", retry);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (retry <= 0) {
        if (ver_info) {
            ver_info->sensor_id = 0;
        }
        return -1;
    }

    mask_id = (uint32_t) ((buf[7] << 16) | (buf[8] << 8) | buf[9]);
    patch_id = (uint32_t) ((buf[4] << 16) | (buf[5] << 8) | buf[6]);
    memcpy(product_id, buf, 4);
    sensor_id = buf[10] & 0x0F;
    match_opt = (buf[10] >> 4) & 0x0F;

    printf("IC VERSION:GT%s_%06X(Patch)_%04X(Mask)_%02X(SensorID)\n",
           product_id, patch_id, mask_id >> 8, sensor_id);

    if (ver_info != NULL) {
        ver_info->mask_id = mask_id;
        ver_info->patch_id = patch_id;
        memcpy(ver_info->product_id, product_id, 5);
        ver_info->sensor_id = sensor_id;
        ver_info->match_opt = match_opt;
    }
    return 0;
}

static int gt1151_get_hw_config(gt1151_obj_t *gt1151_obj)
{
    assert(gt1151_obj);

    if (gt1151_read_regs(gt1151_obj, GT1151_CONFIG, gt1151_obj->hw_config, GT1151_HW_CONFIG_LEN) != 0) {
        printf("gt1151 get hw config ERROR\n");
        return -1;
    }

#if GT1151_DEBUG
    dump_hex(gt1151_obj->hw_config, GT1151_HW_CONFIG_LEN, 16);
    uint8_t *hw_config = gt1151_obj->hw_config;
    uint16_t cfg_len = GT1151_HW_CONFIG_LEN;
    uint16_t config = 0;
    uint16_t checksum = ((uint16_t)hw_config[cfg_len - 3] << 8) + hw_config[cfg_len - 2];
    for (int i = 0; i < cfg_len - 3; i += 2) {
        config += ((uint16_t)hw_config[i] << 8) + hw_config[i + 1];
    }

    printf("config: 0x%2X\tcheck_sum: 0x%2X\r\n", config, checksum);
    printf("total: 0x%2X\r\n", (uint16_t)(config + checksum));
#endif

    return 0;
}

static int gt1151_update_hw_config(gt1151_obj_t *gt1151_obj)
{
    assert(gt1151_obj);

    int ret;

    uint8_t *config = gt1151_obj->hw_config;
    uint16_t cfg_len = GT1151_HW_CONFIG_LEN;
    uint16_t checksum = 0;
    for (int i = 0; i < cfg_len - 3; i += 2) {
        checksum += ((uint16_t)config[i] << 8) + config[i + 1];
    }
    checksum = 0 - checksum;

    config[cfg_len - 3] = (uint8_t)(checksum >> 8);
    config[cfg_len - 2] = (uint8_t)(checksum & 0xFF);
    config[cfg_len - 1] = 0x1;

    uint8_t retry = 0;
    while (retry++ < 3) {
        ret = gt1151_write_regs(gt1151_obj, GT1151_CONFIG, config, GT1151_HW_CONFIG_LEN);
        if (ret == 0) {
            /* at least 200ms, wait for storing config into flash. */
            gt1151_delay_ms(200);
            return 0;
        }
    }
    printf("Send config failed!\n");

    return ret;
}

static void gt1151_clear_status(gt1151_obj_t *gt1151_obj)
{
    uint8_t status = 0;
    if (gt1151_write_regs(gt1151_obj, GTP_READ_COOR_ADDR, &status, 1) != 0) {
        printf("clear status ERROR\n");
    }
}

static void gt1151_set_xy_range(gt1151_obj_t *gt1151_obj, int isx, void *arg)
{
    assert(gt1151_obj);
    assert(arg);

    uint8_t *config = gt1151_obj->hw_config;
    uint16_t range = *(uint16_t *)arg;

    if (isx) {
        config[1] = range & 0xFF;
        config[2] = range >> 8;
    } else {
        config[3] = range & 0xFF;
        config[4] = range >> 8;
    }
}

/**
 * soft reset, but don't find in dataset, just write here.
 */
static int gt1151_soft_reset(gt1151_obj_t *gt1151_obj)
{
    uint8_t buf = 0x02;

    if (gt1151_write_regs(gt1151_obj, GT1151_COMMAND, &buf, 1) != 0) {
        printf("soft reset failed\n");
        return -1;
    }

    return 0;
}

static int gt1151_touch_event_handler(gt1151_obj_t *gt1151_obj, uint8_t *data, uint8_t data_len, gt1151_touch_info_t *touch_info, uint8_t user_touch_num)
{
    assert(data_len == 11);

    uint8_t touch_data[GTP_DATA_BUFF_LEN(GTP_MAX_TOUCH)] = {0};
    uint8_t touch_num = 0;

    touch_num = data[0] & 0x0F;
    if (touch_num > GTP_MAX_TOUCH) {
        printf("Illegal finger number!\n");
        return 0;
    }

    memcpy(touch_data, data, data_len);

    /* read the remaining coor data
        * 0x814E(touch status) + 8(every coordinate
        * consist of 8 bytes data) * touch num +
        * keycode + checksum
        */
    if (touch_num > 1) {
        gt1151_read_regs(gt1151_obj, (GTP_READ_COOR_ADDR + data_len), &touch_data[data_len], (GTP_DATA_BUFF_LEN(touch_num) - data_len));
    }

    /* cacl checksum */
    uint8_t checksum = 0;
    for (int i = 0; i < GTP_DATA_BUFF_LEN(touch_num); i++) {
        checksum += touch_data[i];
    }
    if (checksum) { /* checksum error, read again */
        gt1151_read_regs(gt1151_obj, GTP_READ_COOR_ADDR, touch_data, GTP_DATA_BUFF_LEN(touch_num));

        checksum = 0;
        for (int i = 0; i < GTP_DATA_BUFF_LEN(touch_num); i++) {
            checksum += touch_data[i];
            printf("touch_data[%d]=%x\n", i, touch_data[1]);
        }
        if (checksum) {
            printf("Checksum error[%x]\n", checksum);
            return 0;
        }
    }

    /*
    * cur_event , pre_event bit defination
    * bits:       bit0
    * event:     touch
    */
    uint16_t cur_event = 0;
    static uint16_t pre_event = 0;
    static uint16_t pre_index = 0;
    gt1151_touch_info_t *pdata = (gt1151_touch_info_t *)touch_info;

    if (touch_num > 0) {
        SET_BIT(cur_event, BIT_TOUCH);
    }

    /* finger touch event */
    if (CHK_BIT(cur_event, BIT_TOUCH)) {
        uint8_t *coor_data = &touch_data[1];
        int id = coor_data[0] & 0x0F;
        for (int i = 0; i < GTP_MAX_TOUCH; i++) {
            if (i == id) {
                // TOUCH DOWN data
                if (i < user_touch_num) {
                    pdata[i].track_id     = id;
                    pdata[i].event        = TOUCH_EVENT_DOWN;
                    pdata[i].x_coordinate = coor_data[1] | ((uint16_t)coor_data[2] << 8);
                    pdata[i].y_coordinate = coor_data[3] | ((uint16_t)coor_data[4] << 8);
                    pdata[i].width        = coor_data[5] | ((uint16_t)coor_data[6] << 8);
                    // pdata[i].timestamp    = rt_touch_get_ts();
                }

                if (i < touch_num) {
                    coor_data += 8;
                    id = coor_data[0] & 0x0F;
                }
                pre_index |= (0x01 << i);
            } else if (pre_index & (0x01 << 1)) {   /* i != id */
                // TOUCH UP data
                if (i < user_touch_num) {
                    pdata[i].track_id     = id;
                    pdata[i].event        = TOUCH_EVENT_UP;
                    // pdata[i].timestamp    = rt_touch_get_ts();
                }

                pre_index &= ~(0x01 << i);
            }
        }
    } else if (CHK_BIT(pre_event, BIT_TOUCH)) {
        // TOUCH UP data
        if (user_touch_num > 0) {
            pdata[0].track_id     = 0;
            pdata[0].event        = TOUCH_EVENT_UP;
            // pdata[0].timestamp    = rt_touch_get_ts();
        }

        // printf("Released Touch\n");
        pre_index = 0;
    }

    if (!pre_event && !cur_event) {
        // printf("Additional Pulse\n");
    } else {
        pre_event = cur_event;
    }

    return touch_num;
}

static uint8_t touch_gt1151_readpoint(gt1151_obj_t *gt1151_obj, gt1151_touch_info_t *touch_info, uint8_t touch_num)
{
    assert(touch_num <= 10);

    assert(gt1151_obj);

    uint8_t point_data[11] = {0};
    uint8_t res = 0;

    gt1151_read_regs(gt1151_obj, GTP_READ_COOR_ADDR, (uint8_t *)&point_data, sizeof(point_data));

    if ((point_data[0]) < 0x80)  {// no data ready
        return -1;
    }

    res = gt1151_touch_event_handler(gt1151_obj, point_data, sizeof(point_data), touch_info, touch_num);

    gt1151_clear_status(gt1151_obj);
    return res;
}

int gt1151_pos_read(uint16_t *xpos, uint16_t *ypos)
{
    gt1151_touch_info_t touch_info[10] = {0};
    if (touch_gt1151_readpoint(gt1151_obj, touch_info, 10) > 0) {
        // for (int i = 0; i < 10; i++) {
        //     printf("id: %d, event: %d, x: %d, y: %d, width: %d\n", touch_info[i].track_id, touch_info[i].event, touch_info[i].x_coordinate, touch_info[i].y_coordinate, touch_info[i].width);
        // }

        if (touch_info[0].event == TOUCH_EVENT_DOWN) {
            *xpos = touch_info[0].x_coordinate;
            *ypos = touch_info[0].y_coordinate;
            return 1;
        }
    }

    return 0;
}

int gt1151_hw_init(gt1151_obj_t *gt1151_obj)
{
    /* initial hw config array */
    gt1151_obj->hw_config = (uint8_t *)calloc(1, GT1151_HW_CONFIG_LEN);
    if (gt1151_obj->hw_config == NULL) {
        printf("calloc ERROR\n");
        goto __exit;
    }

    gt1151_delay_ms(100);

    gt1151_soft_reset(gt1151_obj);

    gt1151_get_hw_config(gt1151_obj);

    return 0;

__exit:
    if (gt1151_obj->hw_config) {
        free(gt1151_obj->hw_config);
    }

    return 1;
}

int gt1151_init(uint8_t i2c_addr)
{
    gt1151_obj = (gt1151_obj_t *)calloc(1, sizeof(gt1151_obj_t));
    if (gt1151_obj == NULL) {
        goto _exit;
    }

    bsp_i2c_add_device(&gt1151_obj->i2c_handle, i2c_addr);
    if (NULL == gt1151_obj->i2c_handle) {
        goto _exit;
    }

    struct gt1x_version_info ver_info = {0};
    if (0 != gt1x_read_version(&ver_info)) {
        goto _exit;
    }

    if (gt1151_hw_init(gt1151_obj) != 0) {
        goto _exit;
    }

    int32_t x = GT1151_TOUCH_WIDTH;
    int32_t y = GT1151_TOUCH_HEIGHT;
    gt1151_set_xy_range(gt1151_obj, 1, &x);
    gt1151_set_xy_range(gt1151_obj, 0, &y);
    gt1151_update_hw_config(gt1151_obj);

    return ESP_OK;

_exit:
    if (gt1151_obj) {
        free(gt1151_obj);
    }

    printf("gt1151 init failed\n");
    return ESP_FAIL;
}
