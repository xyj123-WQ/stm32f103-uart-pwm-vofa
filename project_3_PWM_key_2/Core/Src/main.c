/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t duty = 50;       // 当前占空比
uint8_t mode = 0;        // 0: 手动, 1: 自动
uint8_t auto_dir = 1;    // 1: 增, 0: 减
uint32_t auto_tick = 0;  // 自动模式时间戳
uint8_t rx_data;         // 接收的串口字符
uint8_t tx_flag = 0;     // 反馈标志位
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(1, 1, "Duty:");
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
  HAL_UART_Receive_IT(&huart1, &rx_data, 1); // 开启第一次接收
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // 1. 处理自动模式下的平滑呼吸（周期不小于2秒）
		if (mode == 1) // 自动模式
		{
			// 每 25ms 变一次。从20到80需要变60次，60*25ms = 1.5秒半个周期，往返约3秒，完美符合题目>2秒的要求
			if (HAL_GetTick() - auto_tick >= 50) 
			{
				auto_tick = HAL_GetTick(); // 更新时间戳
				if (auto_dir == 1) {
					duty++;
					if (duty >= 80) auto_dir = 0; // 到80%改为减
				} else {
					duty--;
					if (duty <= 20) auto_dir = 1; // 到20%改为加
				}
				__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
			}
		}

		// 2. 处理串口反馈（必须放在主循环里，千万别放中断里）
		if (tx_flag == 1)
		{
			tx_flag = 0;
			if (mode == 0) {
				printf("Mode: Manual, Duty: %d\r\n", duty);
			} else {
				printf("Mode: Auto, Duty: %d\r\n", duty);
			}
		}

		// 3. OLED 刷新（如果你保留了OLED的话）
		   OLED_ShowString(1, 1, mode==0 ? "Manual" : "Auto  ");
		   OLED_ShowNum(2, 1, duty, 3);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        if(rx_data == '+') {
            if(mode == 0) { // 手动模式下才生效
                if(duty <= 90) duty += 10; else duty = 100;
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
                tx_flag = 1; 
            } else {
                // 题目要求：自动模式下收到无效指令要反馈
                printf("Invalid in Auto Mode\r\n"); 
            }
        }
        else if(rx_data == '-') {
            if(mode == 0) {
                if(duty >= 10) duty -= 10; else duty = 0;
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
                tx_flag = 1;
            } else {
                printf("Invalid in Auto Mode\r\n");
            }
        }
        else if(rx_data == 'A') {
            mode = 1;
            // 题目要求：进入自动模式时如果在 20~80 之外，要先拉回来
            if(duty > 80) auto_dir = 0;
            else if(duty < 20) auto_dir = 1;
            tx_flag = 1;
        }
        else if(rx_data == 'M') {
            mode = 0;
            tx_flag = 1;
        }
        
        // 重新开启接收中断
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
