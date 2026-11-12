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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
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
// 定义结构体，方便管理两路独立状态
typedef struct {
    uint8_t mode;       // 0: 手动, 1: 自动
    uint8_t duty;       // 占空比 0~100
    uint8_t auto_dir;   // 1: 增加, 0: 减少
} LED_State;

LED_State led1 = {0, 50, 1}; // LED1 初始：手动，50%
LED_State led2 = {0, 50, 1}; // LED2 初始：手动，50%

// 串口接收相关
char rx_buf[50];      // 接收缓冲区
uint8_t rx_idx = 0;   // 缓冲区索引
uint8_t cmd_ready = 0;// 指令接收完成标志
uint8_t rx_data;      // 当前接收的字符

uint32_t auto_tick1 = 0;
uint32_t auto_tick2 = 0;
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
  /* USER CODE BEGIN 2 */
  // 启动两路 PWM
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, led1.duty);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, led2.duty);

  // 开启串口接收
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // ================== 1. 指令解析 ==================
	if (cmd_ready == 1)
	{
		cmd_ready = 0; // 清除标志位
		int val = 0;
		char cmd = 0;
		
		// 尝试解析 "LED1 A" 或 "LED2 M" 或 "LED1 30"
		if (sscanf(rx_buf, "LED1 %c", &cmd) == 1) {
			if (cmd == 'A') { led1.mode = 1; }
			else if (cmd == 'M') { led1.mode = 0; }
		}
		else if (sscanf(rx_buf, "LED2 %c", &cmd) == 1) {
			if (cmd == 'A') { led2.mode = 1; }
			else if (cmd == 'M') { led2.mode = 0; }
		}
		else if (sscanf(rx_buf, "LED1 %d", &val) == 1) {
			if(val >= 0 && val <= 100) {
				led1.duty = val;
				led1.mode = 0; // 调数值自动切手动
				__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, led1.duty);
			} else {
				printf("Error: Duty out of range\r\n");
			}
		}
		else if (sscanf(rx_buf, "LED2 %d", &val) == 1) {
			if(val >= 0 && val <= 100) {
				led2.duty = val;
				led2.mode = 0; 
				__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, led2.duty);
			} else {
				printf("Error: Duty out of range\r\n");
			}
		}
		else {
			printf("Error: Invalid Command\r\n");
		}

		// 统一反馈当前两个灯的状态
		printf("LED1:%s,Duty:%d\r\n", led1.mode ? "Auto" : "Manual", led1.duty);
		printf("LED2:%s,Duty:%d\r\n", led2.mode ? "Auto" : "Manual", led2.duty);
	}

	// ================== 2. 自动模式平滑处理 ==================
	// LED1 自动模式
	if (led1.mode == 1 && (HAL_GetTick() - auto_tick1 >= 20)) {
		auto_tick1 = HAL_GetTick();
		if (led1.auto_dir) { led1.duty++; if (led1.duty >= 80) led1.auto_dir = 0; }
		else { led1.duty--; if (led1.duty <= 20) led1.auto_dir = 1; }
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, led1.duty);
	}

	// LED2 自动模式
	if (led2.mode == 1 && (HAL_GetTick() - auto_tick2 >= 20)) {
		auto_tick2 = HAL_GetTick();
		if (led2.auto_dir) { led2.duty++; if (led2.duty >= 80) led2.auto_dir = 0; }
		else { led2.duty--; if (led2.duty <= 20) led2.auto_dir = 1; }
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, led2.duty);
	}
	
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
        // 如果没满，且不是换行符，存进缓冲区
        if(rx_idx < 49 && rx_data != '\n' && rx_data != '\r') {
            rx_buf[rx_idx++] = rx_data;
        }
        // 收到换行符或回车，说明一帧指令结束
        else if(rx_data == '\n' || rx_data == '\r') {
            rx_buf[rx_idx] = '\0'; // 加上字符串结束符
            cmd_ready = 1;         // 标记指令已就绪
            rx_idx = 0;            // 清空索引准备下一次
        }
        
        HAL_UART_Receive_IT(&huart1, &rx_data, 1); // 重新开启接收
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
