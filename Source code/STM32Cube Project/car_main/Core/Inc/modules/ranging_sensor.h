#ifndef DISTANCE_MEAS_H
#define DISTANCE_MEAS_H

#include "common.h"

#include "gpio.h"
#include "i2c.h"

#include "vl53l0x_api.h"

void init_ranging_sensor();
void read_distance();

#endif //DISTANCE_MEAS_H
