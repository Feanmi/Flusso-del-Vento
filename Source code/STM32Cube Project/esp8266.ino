#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define SYNC_BYTE1 0xAC
#define SYNC_BYTE2 0x53
#define DEVICE_ADDRESS 0x01
#define MAX_PACKET_LEN 256
#define MIN_PACKET_LEN 7  // Sync(2) + LEN(1) + SQN(1) + ADDR(1) + CMD(min 1) + CRC(1)

#define TELEM_SIZE 15
#define MODULES_CNT 9

static uint8_t telem [TELEM_SIZE] = {0};
static uint8_t magic_versions [MODULES_CNT] = {0};

static int8_t last_speed, last_angle;

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

const char* ssid = "PoPaEsp";
const char* pass = "bauns1337";

IPAddress local_ip(192,168,5,1);
IPAddress gateway(192,168,5,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

protocol_handler_t protocol_handler;

static void protocol_init(protocol_handler_t* handler) {
    memset(handler, 0, sizeof(protocol_handler_t));
    handler->state = STATE_WAIT_SYNC1;
    handler->last_sqn = 0;
    handler->packet_ready = 0;
}

void onWsEvent(AsyncWebSocket *srv, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  if(type==WS_EVT_DATA){
    AwsFrameInfo *info=(AwsFrameInfo*)arg;
    if(info->final && info->index==0 && info->len==len && info->opcode==WS_BINARY){
      String msg; msg.reserve(len);
      for(size_t i=0;i<len;i++) msg+=(char)data[i];
      handle_input(msg);
    }
  }
}

void setup() {
  Serial.begin(115200);

  protocol_init(&protocol_handler);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);  

  server.begin();

  Serial.print("\r\n");

}

uint8_t idx = 0;

bool serial_start = false;

void loop() {
  while (Serial.available() > 0) {
    uint8_t c = Serial.read();
    if (c == 0xac){
        idx = 0;
        serial_start = true;
        continue;
    }

    if (serial_start){
        telem[idx] = c;
        idx++;
    }

    if(idx >= TELEM_SIZE){
        serial_start = false;
        idx = 0;
    }

  }
}


static void protocol_process_byte(protocol_handler_t* handler, uint8_t byte) {
    switch (handler->state) {
        case STATE_WAIT_SYNC1:
            if (byte == SYNC_BYTE1) {
                handler->rx_buffer[0] = byte;
                handler->rx_index = 1;
                handler->state = STATE_WAIT_SYNC2;
            }
            break;

        case STATE_WAIT_SYNC2:
            if (byte == SYNC_BYTE2) {
                handler->rx_buffer[1] = byte;
                handler->rx_index = 2;
                handler->state = STATE_WAIT_LEN;
            } else {
                handler->state = STATE_WAIT_SYNC1;
            }
            break;

        case STATE_WAIT_LEN:
            if (byte >= 4 && byte <= 253) {
                handler->rx_buffer[2] = byte;
                handler->expected_len = byte + 3; // +3 for sync bytes and LEN
                handler->rx_index = 3;
                handler->state = STATE_WAIT_DATA;
            } else {
                handler->state = STATE_WAIT_SYNC1;
            }
            break;

        case STATE_WAIT_DATA:
            handler->rx_buffer[handler->rx_index++] = byte;

            if (handler->rx_index >= handler->expected_len) {
                handler->state = STATE_PACKET_COMPLETE;
                handler->packet_ready = 1;

                // Copy data to structure
                if (handler->expected_len >= MIN_PACKET_LEN) {
                    memcpy(&handler->current_packet, handler->rx_buffer,
                           handler->expected_len - 1); // LEN - 1 stands for len of packet without CRC
                    handler->current_packet.crc = handler->rx_buffer[handler->rx_index - 1];
                }
            }
            break;

        default:
            handler->state = STATE_WAIT_SYNC1;
            break;
    }
}

// Calculate CRC8
static uint8_t calculate_crc8(uint8_t* data, uint16_t len) {
    uint8_t crc = 0;
    uint8_t poly = 0x8c;
    if (len == 0) return 0;

    while(len--){
        crc ^= *data++;
        for(int i = 0; i < 8; i++){
            if(crc & 0x01){
                crc = (crc >> 1) ^ poly;
            }
            else{
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Check CRC
uint8_t protocol_check_crc(protocol_handler_t* handler) {
    // CRC is calculated from LEN to last byte before CRC
    uint8_t crc_data_len = handler->current_packet.len;
    uint8_t calculated_crc = calculate_crc8(&handler->rx_buffer[2], crc_data_len);

    return (calculated_crc == handler->current_packet.crc);
}

// Validate packet
ack_codes_t protocol_validate_packet(protocol_handler_t* handler) {
    // Check address
    if (handler->current_packet.addr != DEVICE_ADDRESS) {
        return ACK_ERR_WRONG_ADDR;
    }

//    // Check CRC
//    if (!protocol_check_crc(handler)) {
//        return ACK_ERR_WRONG_CRC;
//    }

    // Check length
    uint8_t data_len = handler->current_packet.len - 3; // SQN, ADDR, CRC
    if (data_len < 1) { // At least command code
        return ACK_ERR_INVALID_PARAM;
    }

    // Check valid params
    for(uint16_t i = 0; i < data_len - 1; i++) {  // -1 to exclude CMD
        if((int8_t)(handler->current_packet.params[i]) == -128) {
           return ACK_ERR_INVALID_PARAM;
        }
    } 

    switch(handler->current_packet.cmd_code) {
        case CMD_SET_SPEED:
        case CMD_SET_ANGLE:
            if(data_len != 2){ // cmd + param
                return ACK_ERR_INVALID_PARAM;
            }
            break;

        case CMD_GET_VERSION:
        case CMD_GET_TELEMETRY:
            if(data_len != 1){ // only cmd
                return ACK_ERR_INVALID_PARAM;
            }
            break;

        case CMD_SS_SA:
        case CMD_SS_SA_GT:
            if(data_len != 3){ // cmd + 2 params
                return ACK_ERR_INVALID_PARAM;
            }
            break;

        default:
            break;
    }

    return ACK_OK;
}


// Create response packet
static void protocol_create_response(protocol_handler_t* handler,
                             uint8_t ack_code,
                             uint8_t* response_data,
                             uint8_t data_len) {
    // Header
    handler->tx_buffer[0] = SYNC_BYTE1;
    handler->tx_buffer[1] = SYNC_BYTE2;

    // Length: SQN(1) + ADDR(1) + cmd_code(1) + data_len + CRC(1)
    uint8_t total_len = 1 + 1 + 1 + data_len + 1; // SQN + ADDR + CMD + data + CRC
    handler->tx_buffer[2] = total_len;

    // SQN (take from request)
    handler->tx_buffer[3] = handler->current_packet.sqn;

    // Address
    handler->tx_buffer[4] = DEVICE_ADDRESS;

    // Acknowledgement
    handler->tx_buffer[5] = ack_code;

    // Response data
    if (data_len > 0 && response_data != NULL) {
        memcpy(&handler->tx_buffer[6], response_data, data_len);
    }

    // Calculate CRC (from LEN to end of data before CRC)
    uint8_t crc_len = total_len; // CRC from LEN field to data inclusive
    uint8_t crc = calculate_crc8(&handler->tx_buffer[2], crc_len);
    handler->tx_buffer[6 + data_len] = crc;
}


static void protocol_send_response(protocol_handler_t* handler) {
    uint8_t total_packet_len = handler->tx_buffer[2] + 3; // LEN + sync bytes

    ws.binaryAll(handler->tx_buffer, total_packet_len);
}

static void protocol_process_command(protocol_handler_t* handler) {
    if (!handler->packet_ready) {
        handler->state = STATE_WAIT_SYNC1;
        return;
    }

    ack_codes_t ack = protocol_validate_packet(handler);

    uint8_t serial_data[3] = {SYNC_BYTE1, 0x0, 0x0}; // 0xAC, data[0], data[1]

    if(ack == ACK_OK) {
        switch (handler->current_packet.cmd_code) {
            case CMD_SET_SPEED:
                // Set speed
                last_speed = (int8_t)(handler->current_packet.params[0]);
                serial_data[1] = last_speed;
                serial_data[2] = last_angle;
                protocol_create_response(handler, (uint8_t)ack, NULL, 0);
                Serial.write((const char*)serial_data, 3);
                break;

            case CMD_SET_ANGLE:
                // Set angle
                last_angle = (int8_t)(handler->current_packet.params[0]);
                serial_data[1] = last_speed;
                serial_data[2] = last_angle;
                protocol_create_response(handler, (uint8_t)ack, NULL, 0);
                Serial.write((const char*)serial_data, 3);
                break;

            case CMD_GET_TELEMETRY:
                // Form telemetry data
                protocol_create_response(handler, (uint8_t)ack, telem, TELEM_SIZE);
                break;

            case CMD_SS_SA:
                // Set speed and angle
                last_speed = (int8_t)(handler->current_packet.params[0]);
                last_angle = (int8_t)(handler->current_packet.params[1]);
                serial_data[1] = last_speed;
                serial_data[2] = last_angle;

                protocol_create_response(handler, (uint8_t)ack, NULL, 0);
                Serial.write((const char*)serial_data, 3);
                break;

            case CMD_SS_SA_GT:
                // Set speed, angle and get telemetry
                last_speed = (int8_t)(handler->current_packet.params[0]);
                last_angle = (int8_t)(handler->current_packet.params[1]);
                serial_data[1] = last_speed;
                serial_data[2] = last_angle;

                protocol_create_response(handler, (uint8_t)ack, telem, TELEM_SIZE);
                Serial.write((const char*)serial_data, 3);
                break;

            case CMD_GET_VERSION:
                // Get version
                protocol_create_response(handler, (uint8_t)ack, magic_versions, MODULES_CNT);
                break;


            default:
                // // Unknown command
                // response_data[0] = 0xFF; // Error code
                // response_len = 1;
                // protocol_create_response(handler, handler->current_packet.cmd_code, response_data, response_len);
                break;
        }
    }
    else {
        // Send error code
        protocol_create_response(handler, (uint8_t)ack, NULL, 0);
    }

    // Send response
    protocol_send_response(handler);

    // Reset state
    handler->packet_ready = 0;
    handler->state = STATE_WAIT_SYNC1;
}

void handle_input(String msg){
  
  const char* msg_c = msg.c_str();

  int i = 0;
  while(!protocol_handler.packet_ready){
    protocol_process_byte(&protocol_handler, *(msg_c + i));
    i++;
  }
  protocol_process_command(&protocol_handler); 
}




