#include "modules/wifi.h"

static void handle_input_wifi(uint8_t type);
static void receive_dma(uint16_t num_bytes);
static void send_data_w_len(const uint8_t* data, uint16_t data_size);
static int find(const char* str, int str_len, const char* sub, int sub_len, int start_idx, int end_idx);
static int rfind(const char* str, int str_len, const char* sub, int sub_len, int start_idx, int end_idx);
static int parse_uart_message(char* text, int max_length, uint8_t* output);
static void uart_it_receiver();
static void get_uart_send_command(uint16_t data_len, uint8_t* cmd_buf, uint16_t* cmd_len);
static void get_http_reply(const uint8_t* text, uint16_t text_len, uint8_t* reply_buf, uint16_t* reply_len);

static void protocol_init(protocol_handler_t* handler, UART_HandleTypeDef* huart);
static void protocol_process_byte(protocol_handler_t* handler, uint8_t byte);
static uint8_t calculate_crc8(uint8_t* data, uint16_t len);
static void protocol_create_response(protocol_handler_t* handler,
                                     uint8_t ack_code,
                                     uint8_t* response_data,
                                     uint8_t data_len);
static uint8_t protocol_check_crc(protocol_handler_t* handler);
static ack_codes_t protocol_validate_packet(protocol_handler_t* handler);
static void protocol_send_response(protocol_handler_t* handler);
static void protocol_process_command(protocol_handler_t* handler);

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;

// DATA FROM REMOTE
extern int8_t ANGLE_DESIRED;
extern int8_t SPEED_DESIRED;

// METRICS
extern uint16_t DISTANCE_MEASURED;
extern int16_t SPEED_MEASURED_ACC;
extern int16_t RPM_MEASURED_MAG;
extern int16_t RPM_MEASURED_OPT;
extern int16_t SPEED_MEASURED;
extern int16_t PITCH;
extern int16_t ROLL;
extern uint8_t BATTERY_CHARGE;

// FLAGS
extern uint8_t MALFUNCTION;
extern uint8_t CONNECTED;
extern uint8_t DATA_VALID_FLAGS;


static char DMA_BUFFER[DMA_BUFFER_SIZE];
static char INPUT_DATA_BUFFER[DMA_BUFFER_SIZE];
static uint8_t INPUT_PACKET[DMA_BUFFER_SIZE];

static char CURRENT_CONNECTION[5];

static int flag_send_data = 0;
static int flag_start_parse = 0;

static int INPUT_DATA_REMAINDER = 0;

static protocol_handler_t protocol_handler;

static const char ipd [] = {'+', 'I', 'P', 'D'};
static const char colon [] = {':'};
static const char comma [] = {','};
// static const char slash [] = {'/'};
// static const char http_magic[] = {'\r', '\n', '\r', '\n'};
static const char get_prefixK[] = {'G', 'E', 'T', ' ', '/'};

static uint8_t telem [TELEM_SIZE];
static uint16_t telem_cnt = 0;

static uint8_t magic_versions [MODULES_CNT] = {0};

static uint32_t keep_alive_tick = 0;
 
// Collecting telemetry
static void get_telem(uint8_t* telem, uint16_t telem_size){
    telem[0] = telem_cnt++;

    telem[1] = SPEED_MEASURED_ACC >> 8;
    telem[2] = SPEED_MEASURED_ACC & 0x00FF;

    telem[3] = PITCH >> 8; 
    telem[4] = PITCH & 0x00FF;

    telem[5] = ROLL >> 8; 
    telem[6] = ROLL & 0x00FF;

    telem[7] = RPM_MEASURED_MAG >> 8; 
    telem[8] = RPM_MEASURED_MAG & 0x00FF;

    telem[9] = RPM_MEASURED_OPT >> 8; 
    telem[10] = RPM_MEASURED_OPT & 0x00FF;

    telem[11] = DISTANCE_MEASURED >> 8; 
    telem[12] = DISTANCE_MEASURED & 0x00FF;

    telem[13] = BATTERY_CHARGE;
    
    telem[telem_size - 1] = DATA_VALID_FLAGS;
}

static void receive_dma(uint16_t num_bytes){
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t*)DMA_BUFFER, num_bytes);
}

// for RST noise
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
	__HAL_UART_CLEAR_IDLEFLAG(&huart2);
	HAL_UART_DMAStop(&huart2);
	receive_dma(DMA_BUFFER_SIZE);

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
		flag_start_parse = 1;
}

void init_wifi(){
	memset(DMA_BUFFER, 0, DMA_BUFFER_SIZE);
	protocol_init(&protocol_handler, &huart2);

	HAL_UART_Transmit(&huart2, (uint8_t*)"AT+RST\r\n", sizeof("AT+RST\r\n"), 100);
	HAL_Delay(3000);

    // Configure WIFI module to work in access point(AP) mode
	HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CWMODE_CUR=2\r\n", sizeof("AT+CWMODE_CUR=2\r\n"), 100);
	HAL_Delay(300);

    // Setting up the AP
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CWSAP_CUR=\"PoPaESP\",\"bauns1337\",1,3,1,0\r\n", sizeof("AT+CWSAP_CUR=\"PoPaESP\",\"bauns1337\",1,3,1,0\r\n"), 100);
	HAL_Delay(300);

    // Setting up ip address for AP
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPAP_CUR=\"192.168.5.1\",\"192.168.5.1\",\"255.255.255.0\"\r\n", sizeof("AT+CIPAP_CUR=\"192.168.5.1\",\"192.168.5.1\",\"255.255.255.0\"\r\n"), 100);
    HAL_Delay(300);

    // Activating multiple connections (needs for creating TCP server)
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPMUX=1\r\n", sizeof("AT+CIPMUX=1\r\n"), 100);
    HAL_Delay(300);

    // Creating TCP server on <port>
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPSERVER=1,1001\r\n", sizeof("AT+CIPSERVER=1,1001\r\n"), 100);
    HAL_Delay(300);

    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE)){
    	__HAL_UART_CLEAR_IDLEFLAG(&huart2);
     }

    receive_dma(DMA_BUFFER_SIZE);

    keep_alive_tick = HAL_GetTick();
    
//    HAL_Delay(100);
//    HAL_UART_Transmit(&huart2, (uint8_t*)"ATE0\r\n", sizeof("ATE0\r\n"), 100);
}

// Send data to ESP
static void send_data_w_len(const uint8_t* data, uint16_t data_size){
    uint8_t reply [150] = {0};
    uint8_t cmd [50] = {0};
    uint16_t cmd_len = 0;
    uint16_t reply_len = 0;
    get_http_reply(data, data_size, reply, &reply_len);
    get_uart_send_command(reply_len, cmd, &cmd_len);
    HAL_UART_Transmit(&huart2, cmd, cmd_len, 100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart2, reply, reply_len, 100);
}

// Custom find
static int find(const char* str, int str_len, const char* sub, int sub_len, int start_idx, int end_idx) {
    if(str == NULL || sub == NULL || sub_len == 0) {
        return -1;
    }

    if(sub_len > str_len){
        return -1;
    }

    if(start_idx >= end_idx){
        return -1;
    }

    int search_end = end_idx - sub_len;
    if (search_end < start_idx) {
        return -1;
    }

    for (int pos = start_idx; pos <= search_end; pos++) {
        int match = 1;

        for(int i = 0; i < sub_len; i++) {
            if(str[pos + i] != sub[i]) {
                match = 0;
                break;
            }
        }

        if(match) {
            return pos;
        }
    }

    return -1;
}

// Custom reverse find
static int rfind(const char* str, int str_len, const char* sub, int sub_len, int start_idx, int end_idx) {
    if(str == NULL || sub == NULL || sub_len == 0) {
        return -1;
    }

    if(sub_len > str_len){
        return -1;
    }

    if(start_idx >= end_idx){
        return -1;
    }

    int search_start = end_idx - sub_len;
    if (search_start < start_idx) {
        return -1;
    }
    for (int pos = search_start; pos >= start_idx; pos--) {
        int match = 1;

        for(int i = 0; i < sub_len; i++) {
            if(str[pos + i] != sub[i]) {
                match = 0;
                break;
            }
        }

        if(match) {
            return pos;
        }
    }

    return -1;
}

// Parse message from ESP
static int parse_uart_message(char* text, int max_length, uint8_t* output) {
    int msg_index = rfind(text, max_length, ipd, IPD_START_LEN, 0, max_length - 1);
    if (msg_index == -1) {
        return -1;
    }
    int msg_http_start = find(text, max_length, colon, 1, msg_index, max_length - 1) + 1;
    int msg_len_start = rfind(text, max_length, comma, 1, msg_index, msg_http_start - 1) + 1;

    int get_prefix = find(text, max_length, get_prefixK, GET_PREFIX_LEN, msg_http_start, max_length - 1);

    if(get_prefix == 0) return -1;

    int msg_connection_start = rfind(text, max_length, comma, 1, msg_index, msg_len_start - 1) + 1;

    if(msg_connection_start != -1){
        memcpy(CURRENT_CONNECTION, text + msg_connection_start, msg_len_start - msg_connection_start - 1);
    }
    else{
        memset(CURRENT_CONNECTION, '0', 1);
        memset(CURRENT_CONNECTION + 1, 0, 4);
    }

    uint8_t dig_buff [2];

    int hex_data_start = get_prefix + 5;

    for(int i = 0; *(text + i + hex_data_start) != ' '; i = i + 2){
    	dig_buff[0] = text[i + hex_data_start];
    	dig_buff[1] = text[i + 1 + hex_data_start];
    	uint8_t hex = strtol((char*)dig_buff, NULL, 16);
    	*(output + i/2) = hex;
    }

//    int msg_http_end = msg_http_start + msg_len;

//    if (msg_http_end >= max_length) {
//        int msg_data_start = rfind(text, max_length, http_magic, HTTP_MAGIC_LEN, msg_http_start, max_length - 1) + 4;
////        memcpy(output, text + msg_data_start, max_length - msg_data_start);
//        return msg_http_end - max_length;
//    }

//    int msg_data_start = rfind(text, max_length, http_magic, HTTP_MAGIC_LEN, msg_http_start, msg_http_end) + 4;
//    memcpy(output, text + msg_data_start, msg_http_end - msg_data_start);

    return 0;
}

static void uart_it_receiver(){
//    memset(INPUT_PACKET, 0, DMA_BUFFER_SIZE); // хуйня из-за того что вынесли INPUT_PACKET в глобальную перменную

    // мб еще возвращать количество байт в записанных в data
    int rest = parse_uart_message(DMA_BUFFER, DMA_BUFFER_SIZE, INPUT_PACKET);
    if (rest != -1){
        if(rest == 0){
            handle_input_wifi(1);
        }
        else {
            memcpy(INPUT_DATA_BUFFER, INPUT_PACKET, DMA_BUFFER_SIZE);
            INPUT_DATA_REMAINDER = rest;
        }
    }
    // тут подумать еще
    else if(INPUT_DATA_REMAINDER != 0){
        int can_read_size = MIN(rest, DMA_BUFFER_SIZE - 1);
        uint16_t current_len = DMA_BUFFER_SIZE - INPUT_DATA_REMAINDER;
        memcpy(INPUT_DATA_BUFFER + current_len, DMA_BUFFER, can_read_size);
        INPUT_DATA_REMAINDER -= can_read_size;
        if(INPUT_DATA_REMAINDER < 0){
            INPUT_DATA_REMAINDER = 0;
            handle_input_wifi(2);
        }
    }
//    else {
//        memcpy(INPUT_DATA_BUFFER, DMA_BUFFER, DMA_BUFFER_SIZE - 13);
//    }
}

// Prepare command to send data via UART to ESP
static void get_uart_send_command(uint16_t data_len, uint8_t* cmd_buf, uint16_t* cmd_len) {
    uint8_t len_str[10];
    snprintf((char*)len_str, sizeof(len_str), "%d", data_len);

    strcpy((char*)cmd_buf, "AT+CIPSEND=");
    strcat((char*)cmd_buf, CURRENT_CONNECTION);
    strcat((char*)cmd_buf, ",");
    strcat((char*)cmd_buf, (char*)len_str);
    strcat((char*)cmd_buf, "\r\n");

    *cmd_len = strlen((char*)cmd_buf);
}


// Wrapping in HTTP headers
static void get_http_reply(const uint8_t* text, uint16_t text_len, uint8_t* reply_buf, uint16_t* reply_len) {
    uint8_t len_str[10];
    snprintf((char*)len_str, sizeof(len_str), "%d", text_len);

    strcpy((char*)reply_buf, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: ");
    strcat((char*)reply_buf, (char*)len_str);
    strcat((char*)reply_buf, "\r\n\r\n");

    uint16_t header_len = strlen((char*)reply_buf);
    memcpy(reply_buf + header_len, text, text_len);

    *reply_len = header_len + text_len;
}

// Protocol parser initialize
static void protocol_init(protocol_handler_t* handler, UART_HandleTypeDef* huart) {
    memset(handler, 0, sizeof(protocol_handler_t));
    handler->huart = huart;
    handler->state = STATE_WAIT_SYNC1;
    handler->last_sqn = 0;
    handler->packet_ready = 0;
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

   // Check CRC
   if (!protocol_check_crc(handler)) {
       return ACK_ERR_WRONG_CRC;
   }

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

// Send response
static void protocol_send_response(protocol_handler_t* handler) {
    uint8_t total_packet_len = handler->tx_buffer[2] + 3; // LEN + sync bytes

    // Send packet
    send_data_w_len(handler->tx_buffer, total_packet_len);
}

// Command processor
static void protocol_process_command(protocol_handler_t* handler) {
    if (!handler->packet_ready) {
        handler->state = STATE_WAIT_SYNC1;
        return;
    }

    ack_codes_t ack = protocol_validate_packet(handler);

    if(ack == ACK_OK) {
        switch (handler->current_packet.cmd_code) {
            case CMD_SET_SPEED:
                // Set speed
                SPEED_DESIRED = (int8_t)(handler->current_packet.params[0]);
                protocol_create_response(handler, (uint8_t)ack, NULL, 0);
                break;

            case CMD_SET_ANGLE:
                // Set angle
                ANGLE_DESIRED = (int8_t)(handler->current_packet.params[0]);
                protocol_create_response(handler, (uint8_t)ack, NULL, 0);
                break;

            case CMD_GET_TELEMETRY:
                // Form telemetry data
                get_telem(telem, TELEM_SIZE);
                protocol_create_response(handler, (uint8_t)ack, telem, TELEM_SIZE);
                break;

            case CMD_SS_SA:
                // Set speed and angle
                SPEED_DESIRED = (int8_t)(handler->current_packet.params[0]);
                ANGLE_DESIRED = (int8_t)(handler->current_packet.params[1]);
                break;

            case CMD_SS_SA_GT:
                // Set speed, angle and get telemetry
                SPEED_DESIRED = (int8_t)(handler->current_packet.params[0]);
                ANGLE_DESIRED = (int8_t)(handler->current_packet.params[1]);

                get_telem(telem, TELEM_SIZE);
                protocol_create_response(handler, (uint8_t)ack, telem, TELEM_SIZE);
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

void perform_wifi_control_step(){
    if (HAL_GetTick() - keep_alive_tick > KEEP_ALIVE_TIME_MS){
        CONNECTED = 0;
    }

    if(flag_start_parse == 1){
        HAL_Delay(1);
        HAL_UART_DMAStop(&huart2);
        uart_it_receiver();
        flag_start_parse = 0;
        receive_dma(DMA_BUFFER_SIZE);
    }

    if (flag_send_data == 1){
        flag_send_data = 0;
        int i = 0;
        while(!protocol_handler.packet_ready){
            protocol_process_byte(&protocol_handler, *(INPUT_PACKET + i));
            i++;
        }
        protocol_process_command(&protocol_handler); 
}
}


uint8_t try_connect_wifi(){
	return 0;
}

void handle_input_wifi(uint8_t type){
    CONNECTED = 1;
    keep_alive_tick = HAL_GetTick();
    flag_send_data = type;
}

void send_data_wifi(uint8_t* data, int data_len){

}

uint8_t try_connect(){
	return 0;
}

void handle_input(uint8_t* data, int data_len){

}

uint8_t send_data(){
	return 0;
}

uint8_t detect_disconnect(){
	return 0;
}
