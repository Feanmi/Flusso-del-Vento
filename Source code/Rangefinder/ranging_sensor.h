#ifndef DISTANCE_MEAS_H
#define DISTANCE_MEAS_H

// подключение стандартных библиотек C
#include "common.h" 

// подключение библиотек переферии МК
#include "gpio.h"
#include "i2c.h"

// подключение дравйвера
#include "vl53l0x_api.h"

// объявления функций

void init_ranging_sensor();
void read_distance();

#endif //DISTANCE_MEAS_H
