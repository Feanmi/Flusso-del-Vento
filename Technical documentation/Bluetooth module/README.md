# Bluetooth

<img width="300" height="400" alt="Bluetooth fsm" src="https://github.com/user-attachments/assets/15ce6d5c-9ac5-4f97-b2b5-18e5b6c065c5" />

Программа обмена данными между микроконтроллером STM32 и Bluetooth-модулем. Обеспечивает надёжную передачу команд, контроль целостности данных и механизм запрос-ответ.

Основные характеристики
Скорость: 19200 бод 

Формат: 8N1 (8 бит данных, без контроля чётности, 1 стоп-бит)

Контроль ошибок: CRC-8 (полином 0x131)

Максимальный размер пакета: 256 байт

Тайм-ауты: Автоматический сброс при обрыве связи

## Конечный автомат приёма:
<img width="3696" height="544" alt="Bluetooth fsm" src="https://github.com/user-attachments/assets/73ad6323-b862-477a-8d31-eb91fa1cc58e" />

Работа приёмника и передатчика соответствуепт протоколу управления. Данные представленны в виде структуры, для приема и отправки данных созданны специальные функции:

**protocol_init** - функция для инициализации протокола

**protocol_process_byte** - функция для обработки входящего байта

**protocol_create_response** - функция для создания ответа

**protocol_send_response** - функция для отправки ответа

**protocol_process_command** - функция для обработки команд

**protocol_check_crc** - функция проверки crc

**calculate_crc8** - функция расчёта crc
## Пример использования:

```c
#include "bluetooth_protocol.h"

protocol_handler_t protocol_handler;

//UART2 INIT

//YOUR CODE
int main(void)
{
  //YOUR CODE
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
//YOUR CODE
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
//YOUR CODE
