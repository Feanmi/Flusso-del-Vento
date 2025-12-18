#include "modules/motor.h"

extern TIM_HandleTypeDef htim4;

extern int8_t SPEED_DESIRED;

static int pwm_val;
static int pwm_val1;
static int pwm_val2;
static int past_pwm_val2 = 0;

void init_motor(){
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}

void perform_motor_control_step(){
          	  pwm_val = SPEED_DESIRED;
          	  pwm_val1 = pwm_val;
          	  if (pwm_val  > 0){
          		  pwm_val2 = pwm_val;
          	  }
          	  if (pwm_val < 0){
          	      pwm_val2 = pwm_val / (-1);
          	  }
          	  if (pwm_val1 > 3) {
          		  if (abs(abs(pwm_val2) - abs(past_pwm_val2)) > 20){
          		      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
          			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
          			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
          			  htim4.Instance-> CCR1 = past_pwm_val2 +1;
          			  past_pwm_val2 = past_pwm_val2 +1;
          		  }
          		  else {
					  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
					  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
					  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
					  htim4.Instance-> CCR1 = pwm_val2;
					  past_pwm_val2 = pwm_val2;
          		  }
          	  }

          	  if (pwm_val1 < 3) {
          		 if (abs(abs(pwm_val2) - abs(past_pwm_val2)) > 20){
          			 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
          			 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
          			 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
          			 htim4.Instance-> CCR1 = past_pwm_val2 +1;
          			 past_pwm_val2 = past_pwm_val2 +1;
          		 }
          		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
          		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
          		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
          		  htim4.Instance-> CCR1 = pwm_val2;
          		  past_pwm_val2 = pwm_val2;
          	  }

          	  if (pwm_val1 > -3 && pwm_val1 < 3) {
          		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
          		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
          		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
          		  htim4.Instance-> CCR1 = pwm_val2;
          		  past_pwm_val2 = pwm_val2;
          	  }
          	  HAL_Delay(1);
}
