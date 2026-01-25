#include "modules/wifi.h"

static void handle_input_wifi();
static void receive_dma(uint16_t num_bytes);
static int find(const char* str, int str_len, const char* sub, int sub_len, int start_idx, int end_idx);
static int rfind(const char* str, int str_len, const char* sub, int sub_len, int start_idx, int end_idx);
static int parse_uart_message(char* text, int max_length, uint8_t* output);
static void uart_it_receiver();

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
//

const char PREAMBULE [] = {0xac};

static char DMA_BUFFER[DMA_BUFFER_SIZE];
static uint8_t INPUT_PACKET[DMA_BUFFER_SIZE];

extern int flag_send_data;
static int flag_start_parse = 0;

static uint8_t telem [TELEM_SIZE + 1];

static uint16_t telem_cnt = 0;

static uint32_t keep_alive_tick = 0;

static int msg_index;
 
// Collecting telemetry
static void get_telem(uint8_t* telem){
	telem[0] = 0xac;
    telem[1] = telem_cnt++;

    telem[2] = SPEED_MEASURED_ACC >> 8;
    telem[3] = SPEED_MEASURED_ACC & 0x00FF;

    telem[4] = PITCH >> 8;
    telem[5] = PITCH & 0x00FF;

    telem[6] = ROLL >> 8;
    telem[7] = ROLL & 0x00FF;

    telem[8] = RPM_MEASURED_MAG >> 8;
    telem[9] = RPM_MEASURED_MAG & 0x00FF;

    telem[10] = RPM_MEASURED_OPT >> 8;
    telem[11] = RPM_MEASURED_OPT & 0x00FF;

    telem[12] = DISTANCE_MEASURED >> 8;
    telem[13] = DISTANCE_MEASURED & 0x00FF;

    telem[14] = BATTERY_CHARGE;
    
    telem[15] = DATA_VALID_FLAGS;
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

    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE)){
    	__HAL_UART_CLEAR_IDLEFLAG(&huart2);
     }

    receive_dma(DMA_BUFFER_SIZE);

    keep_alive_tick = HAL_GetTick();
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
    msg_index = rfind(text, max_length, PREAMBULE, 1, 0, max_length - 1);
    if (msg_index == -1) {
        return -1;
    }
    int msg_data_start = msg_index + 1;

    if (msg_data_start + 2 > max_length) {
        return -1;
    }

    for(int i = 0; i < 2; i++){
    	output[i] = text[i + msg_data_start];
    }

    return 0;
}

static void uart_it_receiver(){
    int rest = parse_uart_message(DMA_BUFFER, DMA_BUFFER_SIZE, INPUT_PACKET);
    if(rest == 0){
        handle_input_wifi(1);
    }
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
        get_telem(telem);
        HAL_UART_Transmit(&huart2, telem, TELEM_SIZE + 1, 100);
    }
}

void handle_input_wifi(){
    SPEED_DESIRED = INPUT_PACKET[0];
    ANGLE_DESIRED = INPUT_PACKET[1];

    if (msg_index > DMA_BUFFER_SIZE - 50){
        memset(DMA_BUFFER, 0, DMA_BUFFER_SIZE);
    }

    CONNECTED = 1;
    keep_alive_tick = HAL_GetTick();
}
