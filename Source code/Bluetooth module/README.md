# Программное описание Bluetooth
Данные представленны в виде структуры, для приема и отправки данных созданны специальные функции:

**protocol_init** - функция для инициализации протокола

**protocol_process_byte** - функция для обработки входящего байта

**protocol_create_response** - функция для создания ответа

**protocol_send_response** - функция для отправки ответа

**protocol_process_command** - функция для обработки команд

**protocol_check_crc** - функция проверки crc

**calculate_crc8** - функция расчёта crc
## Пример использования:

Для использования программы, необходимо подключить модуль к свободному UART на плате, настроив его так, как описано выше. В примере используется UART2. 

```c
//main.c
//PROTOCOL INIT
#include "bluetooth_protocol.h"

protocol_handler_t protocol_handler;


//MAIN FUNCTION
int main(void)
{
    protocol_init(&protocol_handler, &huart2);
    
    // Enable receive interrupt
    uint8_t rx_byte;
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    
    // Start timer
    HAL_TIM_Base_Start_IT(&htim2);
    while (1)
  {
    if (protocol_handler.packet_ready) {
			protocol_process_command(&protocol_handler);
    }
  }
}


//UART CALLBACK
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        uint8_t received_byte = huart->Instance->DR;
        
        // Process byte
        protocol_process_byte(&protocol_handler, received_byte);
        
        // Reset timeout timer
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        HAL_TIM_Base_Start(&htim2);
        
        // Restart reception
        HAL_UART_Receive_IT(&huart2, &received_byte, 1);
    }
}
```
Структура protocol_handler хранит в себе состояние конечного автомата bluetooth, готовность принимаемого пакета, а также принимаемые и отправляемые данные
