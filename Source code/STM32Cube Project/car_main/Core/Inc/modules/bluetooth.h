#ifndef INC_MODULES_BLUETOOTH_H_
#define INC_MODULES_BLUETOOTH_H_

#include <stdint.h>

#define SYNC_BYTE1 0xAC
#define SYNC_BYTE2 0x53
#define DEVICE_ADDRESS 0x01
#define MAX_PACKET_LEN 256
#define MIN_PACKET_LEN 7  // Sync(2) + LEN(1) + SQN(1) + ADDR(1) + CMD(min 1) + CRC(1)

// Command packet structure
#pragma pack(push, 1)
//typedef struct {
//    uint8_t sync1;
//    uint8_t sync2;
//    uint8_t len;
//    uint8_t sqn;
//    uint8_t addr;
//    uint8_t cmd_code;
//    uint8_t params[250]; // Maximum size
//    uint8_t crc;
//} command_packet_t;
//#pragma pack(pop)
//
//// Protocol handler structure
//typedef struct {
//    UART_HandleTypeDef* huart;
//    uint8_t rx_buffer[MAX_PACKET_LEN];
//    uint8_t tx_buffer[MAX_PACKET_LEN];
//    uint8_t rx_index;
//    uint8_t expected_len;
//    uint8_t last_sqn;
//    uint8_t state;
//    uint8_t packet_ready;
//    command_packet_t current_packet;
//} protocol_handler_t;
//
//// Receiver states
//typedef enum {
//    STATE_WAIT_SYNC1,
//    STATE_WAIT_SYNC2,
//    STATE_WAIT_LEN,
//    STATE_WAIT_DATA,
//    STATE_PACKET_COMPLETE
//} receiver_state_t;
//
//// Command codes
//typedef enum {
//    CMD_SET_SPEED = 0x01,
//    CMD_SET_ANGLE = 0x02,
//    CMD_SS_SA = 0x03,
//    CMD_GET_TEL = 0x04,
//    CMD_SS_SA_GT = 0x07,
//    CMD_VERSION = 0xFF
//} command_codes_t;
//
//// Initialize handler
//void protocol_init(protocol_handler_t* handler, UART_HandleTypeDef* huart);
//
//// Process received byte
//void protocol_process_byte(protocol_handler_t* handler, uint8_t byte);
//
//// Create response packet
//void protocol_create_response(protocol_handler_t* handler,
//                              uint8_t cmd_code,
//                              uint8_t* response_data,
//                              uint8_t data_len);
//
//// Send response
//void protocol_send_response(protocol_handler_t* handler);
//
//// Command processor
//void protocol_process_command(protocol_handler_t* handler);
//
//// Check CRC
//uint8_t protocol_check_crc(protocol_handler_t* handler);
//
//// Calculate CRC8
//uint8_t calculate_crc8(uint8_t* data, uint8_t len);

#endif /* INC_MODULES_BLUETOOTH_H_ */
