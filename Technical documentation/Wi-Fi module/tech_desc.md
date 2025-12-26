## Техническое описание Wi-Fi модуля Wemos D1 mini

### 1. Элементная база
Модуль/Микросхема
**Wemos D1 mini** - компактный Wi-Fi-модуль на основе микроконтроллера ESP8266.

### Основные характеристики

<table> 
    <thead>
         <tr> 
            <th>Параметр</th> <th>Значение/Описание</th>
         </tr> 
    </thead> 
    <tbody> 
        <tr> 
            <td><strong>Основной чип</strong></td> <td>ESP8266</td> 
        </tr>
        <tr> 
            <td><strong>Тактовая частота</strong></td> <td>80 МГц / 160 МГц</td>
        </tr> 
        <tr>
            <td><strong>Память</strong></td> <td>80 + 32 (data + instr) КБ SRAM, 4 МБ Flash</td> 
        </tr>
        <tr>
            <td><strong>Wi-Fi</strong></td> <td>802.11 b/g/n (2.4 ГГц), WPA/WPA2</td>
        </tr>
        <tr>
            <td><strong>GPIO</strong></td> <td>11 (D0-D8, RX, TX, A0). Все кроме D0 поддерживают прерывания, I2C, 1-wire</td>
        </tr> 
        <tr> 
            <td><strong>Аналоговые входы</strong></td> <td>1 × 10-бит АЦП (A0)</td> 
        </tr> 
        <tr>
            <td><strong>Интерфейсы</strong></td> <td>I2C, SPI, UART, I2S, 1-wire</td>
        </tr> 
        <tr> 
            <td><strong>Питание</strong></td> <td>5V через USB или VIN, 3.3V через стабилизатор</td> 
        </tr>
        <tr>
            <td><strong>Потребление тока</strong></td>
            <td>200 мА - в режиме передачи данных по WiFi.<br>
             60 мА - в режиме приема данных по WiFi.<br>
             40 мА - в режиме ожидания.</td>
        </tr>
        <tr>
            <td><strong>Режимы энергосбережения</strong></td>
            <td>Modem sleep (15 mA), Light sleep (0.4 mA), Deep sleep (15 uA).</td>
        </tr>
        <tr> 
            <td><strong>Габариты</strong></td> <td>34.2 × 25.6 мм</td>
        </tr>
    </tbody>
</table>

### Ключевые особенности

- Встроенный полнофункциональный стек TCP/IP и Wi-Fi-драйвер.

- Поддерживает создание веб-серверов и клиентов HTTP, MQTT и др.

- Возможность программирования через UART

## 2. Схема подключения

Типовая схема подключения к микроконтроллеру

```
Wemos D1 mini (ESP8266)           Микроконтроллер(STM32)
┌─────────────────────┐          ┌───────────────────────────┐
│                     │          │                           │
│       3V3 ──────────┼──────────┼───► 3.3V                  │
│                     │          │                           │
│       GND ──────────┼──────────┼───► GND                   │
│                     │          │                           │
│       TX  ──────────┼──────────┼───► PA3  (USART2_RX)      │
│                     │          │                           │
│       RX  ──────────┼──────────┼───► PA2  (USART2_TX)      │
│                     │          │                           │
│                     │          │                           │
└─────────────────────┘          └───────────────────────────┘
```

### Описание выводов

<table>
  <thead>
    <tr>
      <th>Вывод<br>Wemos D1 Mini</th>
      <th>Назначение</th>
      <th>Описание</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><strong>D0</strong></td>
      <td>GPIO16</td>
      <td>Прерывания, Wake-up из Deep Sleep, без pull-up</td>
    </tr>
    <tr>
      <td><strong>D1</strong></td>
      <td>GPIO5 / SCL (I2C)</td>
      <td>ШИМ, прерывания, pull-up</td>
    </tr>
    <tr>
      <td><strong>D2</strong></td>
      <td>GPIO4 / SDA (I2C)</td>
      <td>ШИМ, прерывания, pull-up</td>
    </tr>
    <tr>
      <td><strong>D3</strong></td>
      <td>GPIO0</td>
      <td>ШИМ, прерывания, pull-up, режим загрузки (Flash)</td>
    </tr>
    <tr>
      <td><strong>D4</strong></td>
      <td>GPIO2</td>
      <td>ШИМ, прерывания, pull-up, встроенный LED</td>
    </tr>
    <tr>
      <td><strong>D5</strong></td>
      <td>GPIO14</td>
      <td>ШИМ, HSPI CLK (SCK), прерывания</td>
    </tr>
    <tr>
      <td><strong>D6</strong></td>
      <td>GPIO12</td>
      <td>ШИМ, HSPI MISO, прерывания, влияет на Flash</td>
    </tr>
    <tr>
      <td><strong>D7</strong></td>
      <td>GPIO13</td>
      <td>ШИМ, HSPI MOSI, прерывания</td>
    </tr>
    <tr>
      <td><strong>D8</strong></td>
      <td>GPIO15</td>
      <td>ШИМ, HSPI CS (SS), прерывания, pull-down, режим загрузки</td>
    </tr>
    <tr>
      <td><strong>RX</strong></td>
      <td>GPIO3</td>
      <td>UART0 RX (серийная связь)</td>
    </tr>
    <tr>
      <td><strong>TX</strong></td>
      <td>GPIO1</td>
      <td>UART0 TX (серийная связь, отладка)</td>
    </tr>
    <tr>
      <td><strong>A0</strong></td>
      <td>ADC0</td>
      <td>10-бит АЦП (0–3.2 В), только вход</td>
    </tr>
    <tr>
      <td><strong>3V3</strong></td>
      <td>Питание</td>
      <td>3.3V выход/вход</td>
    </tr>
    <tr>
      <td><strong>5V / VIN</strong></td>
      <td>Питание</td>
      <td>5V USB или 5-12V внешнее</td>
    </tr>
    <tr>
      <td><strong>GND</strong></td>
      <td>Земля</td>
      <td>Общий провод (несколько пинов)</td>
    </tr>
  </tbody>
</table>


## 3. Принцип функционирования

### Режимы работы

- Нормальный режим: Основной режим выполнения программ

- Deep Sleep: Минимальное потребление (~20 мкА), пробуждение по GPIO16

- Light Sleep: ~0.5 мА, сохранение RAM

- Modem Sleep: WiFi активен, CPU спит

## 4. Программный интерфейс

Протокол связи **UART**

