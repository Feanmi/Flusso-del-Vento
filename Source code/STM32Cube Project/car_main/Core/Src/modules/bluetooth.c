#include "modules/bluetooth.h"


//// Initialize handler
//void protocol_init(protocol_handler_t* handler, UART_HandleTypeDef* huart) {
//    memset(handler, 0, sizeof(protocol_handler_t));
//    handler->huart = huart;
//    handler->state = STATE_WAIT_SYNC1;
//    handler->last_sqn = 0;
//    handler->packet_ready = 0;
//}
//
//// Process received byte
//void protocol_process_byte(protocol_handler_t* handler, uint8_t byte) {
//    static uint8_t packet_len = 0;
//
//    switch (handler->state) {
//        case STATE_WAIT_SYNC1:
//            if (byte == SYNC_BYTE1) {
//                handler->rx_buffer[0] = byte;
//                handler->rx_index = 1;
//                handler->state = STATE_WAIT_SYNC2;
//            }
//            break;
//
//        case STATE_WAIT_SYNC2:
//            if (byte == SYNC_BYTE2) {
//                handler->rx_buffer[1] = byte;
//                handler->rx_index = 2;
//                handler->state = STATE_WAIT_LEN;
//            } else {
//                handler->state = STATE_WAIT_SYNC1;
//            }
//            break;
//
//        case STATE_WAIT_LEN:
//            if (byte >= 4 && byte <= 253) {
//                handler->rx_buffer[2] = byte;
//                handler->expected_len = byte + 3; // +3 for sync bytes and LEN
//                handler->rx_index = 3;
//                handler->state = STATE_WAIT_DATA;
//            } else {
//                handler->state = STATE_WAIT_SYNC1;
//            }
//            break;
//
//        case STATE_WAIT_DATA:
//            handler->rx_buffer[handler->rx_index++] = byte;
//
//            if (handler->rx_index >= handler->expected_len) {
//                handler->state = STATE_PACKET_COMPLETE;
//                handler->packet_ready = 1;
//
//                // Copy data to structure
//                if (handler->expected_len >= MIN_PACKET_LEN) {
//                    memcpy(&handler->current_packet, handler->rx_buffer,
//                           sizeof(command_packet_t));
//                }
//            }
//            break;
//
//        default:
//            handler->state = STATE_WAIT_SYNC1;
//            break;
//    }
//}
//
//// Validate packet
//uint8_t protocol_validate_packet(protocol_handler_t* handler) {
//    if (!handler->packet_ready) {
//        return 0;
//    }
//
//    // Check address
//    if (handler->current_packet.addr != DEVICE_ADDRESS) {
//        return 0;
//    }
//
//    // Check CRC
//    if (!protocol_check_crc(handler)) {
//        return 0;
//    }
//
//    // Check length
//    uint8_t data_len = handler->current_packet.len - 3; // SQN, ADDR, CMD
//    if (data_len < 1) { // At least command code
//        return 0;
//    }
//
//    return 1;
//}
//
//// Calculate CRC8
//uint8_t calculate_crc8(uint8_t* data, uint8_t len) {
//    uint8_t crc = 0;
//		uint8_t i;
//
//    while (len--) {
//        crc ^= *data++;
//        for (i = 0; i < 8; i++) {
//            if (crc & 0x01) {
//                crc = (crc >> 1) ^ 0x8C; // Polynomial x^8 + x^5 + x^4 + 1
//            } else {
//                crc >>= 1;
//            }
//        }
//    }
//
//    return crc;
//}
//
//// Check CRC
//uint8_t protocol_check_crc(protocol_handler_t* handler) {
//    // CRC is calculated from LEN to last byte before CRC
//    uint8_t crc_data_len = handler->current_packet.len;
//    uint8_t calculated_crc = calculate_crc8(&handler->rx_buffer[2], crc_data_len);
//
//    return (calculated_crc == handler->current_packet.crc);
//}
//
//// Create response packet
//void protocol_create_response(protocol_handler_t* handler,
//                             uint8_t cmd_code,
//                             uint8_t* response_data,
//                             uint8_t data_len) {
//    // Header
//    handler->tx_buffer[0] = SYNC_BYTE1;
//    handler->tx_buffer[1] = SYNC_BYTE2;
//
//    // Length: SQN(1) + ADDR(1) + cmd_code(1) + data_len + CRC(1)
//    uint8_t total_len = 1 + 1 + 1 + data_len + 1; // SQN + ADDR + CMD + data + CRC
//    handler->tx_buffer[2] = total_len;
//
//    // SQN (take from request)
//    handler->tx_buffer[3] = handler->current_packet.sqn;
//
//    // Address
//    handler->tx_buffer[4] = DEVICE_ADDRESS;
//
//    // Response code (can be same as command or special response code)
//    handler->tx_buffer[5] = cmd_code;
//
//    // Response data
//    if (data_len > 0 && response_data != NULL) {
//        memcpy(&handler->tx_buffer[6], response_data, data_len);
//    }
//
//    // Calculate CRC (from LEN to end of data before CRC)
//    uint8_t crc_len = total_len; // CRC from LEN field to data inclusive
//    uint8_t crc = calculate_crc8(&handler->tx_buffer[2], crc_len);
//    handler->tx_buffer[6 + data_len] = crc;
//}
//
//// Send response
//void protocol_send_response(protocol_handler_t* handler) {
//    uint8_t total_packet_len = handler->tx_buffer[2] + 3; // LEN + sync bytes
//
//    // Send packet
//    HAL_UART_Transmit(handler->huart, handler->tx_buffer, total_packet_len, 100);
//}
//
//// Command processor
//void protocol_process_command(protocol_handler_t* handler) {
//    if (!protocol_validate_packet(handler)) {
//        handler->packet_ready = 0;
//        handler->state = STATE_WAIT_SYNC1;
//        return;
//    }
//
//    uint8_t response_data[32];
//    uint8_t response_len = 0;
//
//    switch (handler->current_packet.cmd_code) {
//        case CMD_SET_SPEED:
//            response_data[0] = 0x01;
//            response_data[1] = 0x00;
//            response_data[2] = 0x01;
//            response_len = 3;
//            protocol_create_response(handler, CMD_SET_SPEED, response_data, response_len);
//            break;
//
//        case CMD_SET_ANGLE:
//            response_data[0] = 0x00;
//            response_data[1] = 0x64;
//            response_data[2] = 0x80;
//            response_len = 3;
//            protocol_create_response(handler, CMD_SET_ANGLE, response_data, response_len);
//            break;
//
//        case CMD_GET_TEL:
//            response_data[0] = 0x01;
//            response_len = 1;
//            protocol_create_response(handler, CMD_GET_TEL, response_data, response_len);
//            break;
//
//				case CMD_SS_SA:
//            response_data[0] = 0x01;
//            response_len = 1;
//            protocol_create_response(handler, CMD_SS_SA, response_data, response_len);
//            break;
//
//        case CMD_SS_SA_GT:
//            response_data[0] = 0x01;
//            response_len = 1;
//            protocol_create_response(handler, CMD_SS_SA_GT, response_data, response_len);
//            break;
//
//				case CMD_VERSION:
//            response_data[0] = 0x01;
//            response_len = 1;
//            protocol_create_response(handler, CMD_VERSION, response_data, response_len);
//            break;
//
//        default:
//            // Unknown command error response
//            response_data[0] = 0xFF;
//            response_len = 1;
//            protocol_create_response(handler, handler->current_packet.cmd_code, response_data, response_len);
//            break;
//    }
//
//    // Send response
//    protocol_send_response(handler);
//
//    // Reset state
//    handler->packet_ready = 0;
//    handler->state = STATE_WAIT_SYNC1;
//}
