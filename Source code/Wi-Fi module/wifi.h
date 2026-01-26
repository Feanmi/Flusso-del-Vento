#ifndef INC_MODULES_WIFI_H_
#define INC_MODULES_WIFI_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"

#define TELEM_SIZE 15
#define KEEP_ALIVE_TIME_MS 5000

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define DMA_BUFFER_SIZE 500

void perform_wifi_control_step();
void init_wifi();

#endif /* INC_MODULES_WIFI_H_ */
