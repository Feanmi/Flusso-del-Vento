#ifndef INC_MODULES_WIFI_H_
#define INC_MODULES_WIFI_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"

#define TELEM_SIZE 15
#define MODULES_CNT 9
#define KEEP_ALIVE_TIME_MS 5000

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define DMA_BUFFER_SIZE 500
#define IPD_START_LEN 4
#define HTTP_MAGIC_LEN 4
#define GET_PREFIX_LEN 5

#define SYNC_BYTE1 0xAC
#define SYNC_BYTE2 0x53
#define DEVICE_ADDRESS 0x01
#define MAX_PACKET_LEN 256
#define MIN_PACKET_LEN 7  // Sync(2) + LEN(1) + SQN(1) + ADDR(1) + CMD(min 1) + CRC(1)

// Full packet
#pragma pack(push, 1)
typedef struct {
    uint8_t sync1;
    uint8_t sync2;
    uint8_t len;
    uint8_t sqn;
    uint8_t addr;
    uint8_t cmd_code;
    uint8_t params[250]; // Maximum size
    uint8_t crc;
} command_packet_t;
#pragma pack(pop)

// Protocol handler structure
typedef struct {
    UART_HandleTypeDef* huart;
    uint8_t rx_buffer[MAX_PACKET_LEN];
    uint8_t tx_buffer[MAX_PACKET_LEN];
    uint8_t rx_index;
    uint8_t expected_len;
    uint8_t last_sqn;
    uint8_t state;
    uint8_t packet_ready;
    command_packet_t current_packet;
} protocol_handler_t;

// Receiver states
typedef enum {
    STATE_WAIT_SYNC1,
    STATE_WAIT_SYNC2,
    STATE_WAIT_LEN,
    STATE_WAIT_DATA,
    STATE_PACKET_COMPLETE
} receiver_state_t;

// Command codes 
typedef enum {
    CMD_SET_SPEED = 0x01,
    CMD_SET_ANGLE = 0x02,
    CMD_GET_TELEMETRY = 0x04,
    CMD_SS_SA = 0x03,
    CMD_SS_SA_GT = 0x07,
    CMD_GET_VERSION = 0xff,
} command_codes_t;

// Acknowledgement codes
typedef enum{
    ACK_OK,
    ACK_ERR_WRONG_ADDR,
    ACK_ERR_WRONG_CRC,
    ACK_ERR_INVALID_PARAM
} ack_codes_t;

void perform_wifi_control_step();
void init_wifi();

#endif /* INC_MODULES_WIFI_H_ */
