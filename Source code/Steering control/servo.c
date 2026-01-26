#include "modules/servo.h"
#include "main.h"

extern TIM_HandleTypeDef htim2;

extern int8_t ANGLE_DESIRED;

int8_t MAX_ANGLE = 127;
int8_t MIN_ANGLE = -127;

uint16_t COUNTER_MIN = 450;
uint16_t COUNTER_MAX = 2550;
uint16_t COUNTER_CURR = 1500;
uint16_t COUNTER_DESIRED;

int8_t angle = 0;
int8_t last_angle = 1;
uint16_t Pulse_length;

uint16_t get_counter(int8_t angle)
{
	uint32_t res = (127 + (uint32_t)angle);
	res *=  (uint32_t)(COUNTER_MAX - COUNTER_MIN);
	res /=  254;
	res +=  COUNTER_MIN;
	res = res / 10 * 10;
	return (uint16_t)(res);
}

void  perform_servo_control_step()
{
//	COUNTER_DESIRED = get_counter(ANGLE_DESIRED);
//	if (COUNTER_DESIRED != COUNTER_CURR){
//		if (COUNTER_DESIRED > COUNTER_CURR && COUNTER_CURR < COUNTER_MAX){
//			COUNTER_CURR += 10;
//		}
//		else if (COUNTER_DESIRED < COUNTER_CURR && COUNTER_CURR > COUNTER_MIN){
//			COUNTER_CURR -= 10;
//		}
//		TIM2->CCR2=COUNTER_CURR;
//	}
//	HAL_Delay(5);
	int8_t angle = ANGLE_DESIRED;
	if (last_angle != angle)
		{
			Pulse_length = 1000;
			if (angle > MAX_ANGLE) angle = MAX_ANGLE;
			else if (angle < MIN_ANGLE) angle = MIN_ANGLE;
	//		Pulse_length += 300 + 1000/127 * (angle);
			Pulse_length = get_counter(angle);
			if (Pulse_length > COUNTER_MAX) Pulse_length = COUNTER_MAX;
			TIM2->CCR2=Pulse_length;
		}
		last_angle = angle;
		COUNTER_CURR = get_counter(angle);

}

void  set_servo_angle(int8_t angle)
{
	if (last_angle != angle)
	{
		Pulse_length = 1000;
		if (angle > MAX_ANGLE) angle = MAX_ANGLE;
		else if (angle < MIN_ANGLE) angle = MIN_ANGLE;
//		Pulse_length += 300 + 1000/127 * (angle);
		Pulse_length = get_counter(angle);
		if (Pulse_length > COUNTER_MAX) Pulse_length = COUNTER_MAX;
		TIM2->CCR2=Pulse_length;
	}
	last_angle = angle;
	COUNTER_CURR = get_counter(angle);
}

// void  aaa(int8_t ANGLE)
// {
// 	ANGLE_DESIRED = ANGLE;
// 	perform_servo_control_step();

// 	}


void calibrate_servo(double calibrate_coefficient)
	{

		int sensor_status = 1;

		int angle = 0;

		set_servo_angle(angle);

		for (angle; angle < 90; angle++)
		{
			set_servo_angle(angle);

			if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11)) sensor_status = 0;

			HAL_Delay(50);

			if (sensor_status == 0)
			{
				MAX_ANGLE = angle;
				COUNTER_MAX = get_counter(angle);
				break;
			}

		}

		sensor_status = 1;

		for (angle; angle > -90; angle--)
		{
			set_servo_angle(angle);


			if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12)) sensor_status = 0;


			HAL_Delay(50);

			if (sensor_status == 0)
			{
				MIN_ANGLE = angle;
				COUNTER_MIN = get_counter(angle);
				break;
			}
		}

		angle = MIN_ANGLE + (MAX_ANGLE - MIN_ANGLE) * calibrate_coefficient;
		set_servo_angle(angle);
	}

void init_servo()
{
    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) != HAL_OK)
    {
	/* PWM generation Error */
	    Error_Handler();
    }
    calibrate_servo(0.7);
}
