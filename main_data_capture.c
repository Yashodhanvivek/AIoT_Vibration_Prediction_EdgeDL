// Capture Accelerometer Data from Sensors

/* Includes ------------------------------------------------------------------*/
#include "main.h"




/* accelerometer module address*/
#define AccelAddress (0x53<<1)

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

 uint8_t Rx_buffer[4];
 uint8_t TS_buffer[4] = {0 ,0 ,0 ,0};
 uint8_t CheckSum = 0;
 uint32_t TimeStamp;
 uint8_t SendFlag  = 0;
 uint8_t AccelData[6];
 uint8_t chipid=0;



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Write the Value into corresponding register address */
void AccelWrite (uint8_t reg, uint8_t value)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit (&hi2c1, AccelAddress, data, 2, 100);
}

/* Read the values from corresponding register address */
void AccelReadValues (uint8_t reg , uint8_t numberofbytes)
{
	HAL_I2C_Mem_Read (&hi2c1, AccelAddress, reg, 1, (uint8_t *)AccelData, numberofbytes, 100);
}

/* Read the address of chip */
void AccelReadAddress (uint8_t reg)
{
	HAL_I2C_Mem_Read (&hi2c1, AccelAddress, reg, 1, &chipid, 1, 100);

}

/* initialize the accelerometer */
void AccelInit (void)
{
	AccelReadValues(0x00 ,1);
	AccelWrite (0x2d, 0x00);  // reset all bits
	AccelWrite (0x2d, 0x08);  // power_cntl measure and wake up 8hz*/
	AccelWrite (0x31, 0x00);  // data_format range= +- 2g
	//AccelWrite (0x2c, 0x0d);  // 800 hz output data rate

}

/* UART receive interrupt call back */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

		if (huart->Instance == USART2)             //is current uart?
		{
			/* check for the correct received confirmation code */
			if((Rx_buffer[0] == 0xFF) && (Rx_buffer[1] == 0x00) && (Rx_buffer[2] == 0xFF) && (Rx_buffer[3] == 0x00))
			{
				/*Set send flag to 1*/
				SendFlag = 1;

			}else
			{
            /* Transmit the received time stamp */
			HAL_UART_Transmit(&huart2 ,Rx_buffer ,4 ,20);

			/*Convert timestamp from bytes to integer */
			TimeStamp |= (Rx_buffer[0]);
			TimeStamp =  (TimeStamp << 8);
			TimeStamp |= (Rx_buffer[1]);
			TimeStamp =  (TimeStamp << 8);
			TimeStamp |= (Rx_buffer[2]);
			TimeStamp =  (TimeStamp << 8);
			TimeStamp |= (Rx_buffer[3]);

			}


		}
		/*Enable receive interrupt*/
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
		HAL_UART_Receive_IT (&huart2, Rx_buffer, 4);
	}

/*Increment the timer */
/*This function is called in stm32f4xx_hal.c in HAL_IncTick function() */
void IncrementTimer(void)
{
	if(SendFlag == 1)
	{
	TimeStamp = TimeStamp+1;
	}
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  /*Initialize the accelerometer*/
  AccelInit ();

  /*Enable the uart receive interrupt */
   __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
   HAL_UART_Receive_IT (&huart2, Rx_buffer, 4);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	//  Printf("Print: AR=%s\r\n", "testing");
      /*If send flag is 0 collect the received data in Rx_buffer */
      if(SendFlag == 0)
  	  {
  		  HAL_UART_Receive_IT (&huart2, Rx_buffer, 4);
  	  }
  	  else if(SendFlag == 1)
  	  {
  		  /*Read the accelerometer values  */
  		  /*2 bytes represent the one single value for one axis for example x axis */
  		  /*So we have 6 bytes for x , y and z axis respectively*/
  		    AccelReadValues (0x32 , 6);

            /*Convert the time stamp value again to bytes */
            TS_buffer[3] = (uint8_t) ((TimeStamp >> 0)  & 0x000000FF);
  		    TS_buffer[2] = (uint8_t) ((TimeStamp >> 8)  & 0x000000FF);
  		    TS_buffer[1] = (uint8_t) ((TimeStamp >> 16) & 0x000000FF);
  		    TS_buffer[0] = (uint8_t) ((TimeStamp >> 24) & 0x000000FF);

            /*Calculate the checksum of 4 bytes of time stamp and 6 bytes of accelerometr*/
            CheckSum = TS_buffer[0] + TS_buffer[1] + TS_buffer[2] + TS_buffer[3] + AccelData[0]
				   + AccelData[1]+ AccelData[2]+ AccelData[3]+ AccelData[4]+ AccelData[5];

            HAL_Delay(20);

            /*Transmit 4 bytes of TS_buffer*/
            HAL_UART_Transmit(&huart2 ,TS_buffer ,4 ,10);
            /*Transmit 6 bytes of Accelerometr Data*/
            HAL_UART_Transmit(&huart2 ,AccelData ,6 ,10);
            /*Transmit 1 byte of checksum*/
  		    HAL_UART_Transmit(&huart2 ,&CheckSum ,1 ,10);

  	}
    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
