## Чучуть информации
1. Весь код в acc.c и acc.h
2. Код использует локальную библиотеку акселерометра TJ_MPU что-то там
3. Код использует две внешине глобальные переменные для передачи данных ```SPEED_MEASURED_ACC``` и ```DATA_VALID_FLAG[2]```.

## Типикал юз
```
#include "acc.h"

extern I2C_HandleTypeDef i2c_acc_handle;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    init_gyroscope(&i2c_acc_handle);

    while (1)
    {
        read_accelleration();
        ...
    }
}

```

## Отладка
Для проверки модуля можно воспользоваться кодом и скомпилировать только акселерометр в папке debug