#ifndef __BME_H__
#define __BME_H__

#include "bme280.h"

struct identifier {
    uint8_t dev_addr;
    int8_t fd;
};

struct bme280_dev *init_bme280(char *device);

int8_t get_sensor_data_forced_mode(struct bme280_dev *dev, float *temperature);

#endif