#ifndef INC_MODULES_BLUETOOTH_H_
#define INC_MODULES_BLUETOOTH_H_

#include <stdint.h>

void init_bluetooth();
uint8_t try_connect_bt();
void handle_input_bt();
void send_data_bt(uint8_t* data, int data_len);

#endif /* INC_MODULES_BLUETOOTH_H_ */
