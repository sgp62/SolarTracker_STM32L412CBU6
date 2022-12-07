/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NMEA_LEN	72
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define FIND_AND_NUL(s, p, c) ( \
   (p) = strchr(s, c), \
   *(p) = '\0', \
   ++(p), \
   (p))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

osThreadId uart1TaskHandle;
osThreadId uart2TaskHandle;
osThreadId spi1TaskHandle;
osMutexId spi_mutexHandle;
osSemaphoreId spi_semHandle;
osSemaphoreId uart_semHandle;
osSemaphoreId external_semHandle;
/* USER CODE BEGIN PV */

// FM25V02A FRAM SPI Commands
const uint8_t READ = 0b00000011;
const uint8_t WRITE = 0b00000010;
const uint8_t WREN = 0b00000110;
const uint8_t RDSR = 0b00000101;
const uint8_t WRSR = 0b00000001;
const uint8_t FSTRD = 0b00001011;
const uint8_t SLEEP = 0b10111001;
const uint8_t RDID = 0b10011111;

SerialBuffer SerialBufferReceived;
SerialBuffer test_spi_buf;
volatile uint32_t tim1_counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
void Startuart1Task(void const * argument);
void Startuart2Task(void const * argument);
void startspi1Task(void const * argument);

/* USER CODE BEGIN PFP */
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
float GpsToDecimalDegrees(char* nmeaPos, char quadrant);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
QueueHandle_t xQueueSerialDataReceived;

/**
 * Convert NMEA absolute position to decimal degrees
 * "ddmm.mmmm" or "dddmm.mmmm" really is D+M/60,
 * then negated if quadrant is 'W' or 'S'
 */
float GpsToDecimalDegrees(char* nmeaPos, char quadrant)
{
  float v= 0;
  if(strlen(nmeaPos)>5)
  {
    char integerPart[3+1];
    int digitCount= (nmeaPos[4]=='.' ? 2 : 3);
    memcpy(integerPart, nmeaPos, digitCount);
    integerPart[digitCount]= 0;
    nmeaPos+= digitCount;
    v= atoi(integerPart) + atof(nmeaPos)/60.;
    if(quadrant=='W' || quadrant=='S')
      v= -v;
  }
  return v;
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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //Enable Uart Interrupts
  HAL_NVIC_SetPriority(USART_GPS_IRQn, 12, 0);
  HAL_NVIC_EnableIRQ(USART_GPS_IRQn);
  USART_GPS->CR1 |= USART_CR1_RXNEIE; // Enable Interrupt
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of spi_mutex */
  osMutexDef(spi_mutex);
  spi_mutexHandle = osMutexCreate(osMutex(spi_mutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
//  /* definition and creation of spi_sem */
//  osSemaphoreDef(spi_sem);
//  spi_semHandle = osSemaphoreCreate(osSemaphore(spi_sem), 1);
//
//  /* definition and creation of uart_sem */
//  osSemaphoreDef(uart_sem);
//  uart_semHandle = osSemaphoreCreate(osSemaphore(uart_sem), 1);
//
//  /* definition and creation of external_sem */
//  osSemaphoreDef(external_sem);
//  external_semHandle = osSemaphoreCreate(osSemaphore(external_sem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  spi_semHandle = xSemaphoreCreateCounting( 1, 0 );
  uart_semHandle = xSemaphoreCreateCounting( 1, 1 );
  external_semHandle = xSemaphoreCreateCounting( 1, 0 );
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  xQueueSerialDataReceived = xQueueCreate( 2, sizeof( SerialBuffer) );
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of uart1Task */
  osThreadDef(uart1Task, Startuart1Task, osPriorityHigh, 0, 512);
  uart1TaskHandle = osThreadCreate(osThread(uart1Task), NULL);

  /* definition and creation of uart2Task */
  osThreadDef(uart2Task, Startuart2Task, osPriorityNormal, 0, 512);
  uart2TaskHandle = osThreadCreate(osThread(uart2Task), NULL);

  /* definition and creation of spi1Task */
  osThreadDef(spi1Task, startspi1Task, osPriorityNormal, 0, 512);
  spi1TaskHandle = osThreadCreate(osThread(spi1Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */
  SPI1->CR1 |= SPI_CR1_SSM;
  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void USART_GPS_IRQHandler(void) // Sync and Queue NMEA Sentences
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static char rx_buffer[LINEMAX + 1]; // Local holding buffer to build line, w/NUL
	static int rx_index = 0;
	if (USART_GPS->ISR & USART_ISR_ORE) // Overrun Error
		USART_GPS->ICR = USART_ICR_ORECF;
	if (USART_GPS->ISR & USART_ISR_NE) // Noise Error
		USART_GPS->ICR = USART_ICR_NCF;
	if (USART_GPS->ISR & USART_ISR_FE) // Framing Error
		USART_GPS->ICR = USART_ICR_FECF;
	if (USART_GPS->ISR & USART_ISR_RXNE) // Received character?
	{
		char rx = (char)(USART_GPS->RDR & 0xFF);
		if ((rx == '\r') || (rx == '\n')) // Is this an end-of-line condition, either will suffice?
		{
			if (rx_index != 0) // Line has some content?
			{
				rx_buffer[rx_index++] = 0; // Add NUL if required down stream
				//QueueBuffer(rx_buffer, rx_index); // Copy to queue from live dynamic receive buffer
				xQueueSendFromISR(xQueueSerialDataReceived,(void *)&rx_buffer,&xHigherPriorityTaskWoken);
				rx_index = 0; // Reset content pointer
				got_nmea = 1;
			}
		}
		else
		{
			if ((rx == '$') || (rx_index == LINEMAX)) // If resync or overflows pull back to start
				rx_index = 0;
			rx_buffer[rx_index++] = rx; // Copy to buffer and increment
		}
	}
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_11)
  {
	  xSemaphoreGiveFromISR(external_semHandle, NULL);
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_Startuart1Task */
/**
  * @brief  Function implementing the uart1Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Startuart1Task */
void Startuart1Task(void const * argument)
{
  /* USER CODE BEGIN 5 */
	static int spi_gps_read_addr = 0;
	static SerialBuffer gps_ext_buffer;
	static int statusbuf[8];
	int num_messages = 500; //Number of FRAM messages for offset 72B -> 256KB storage =
	//Code used for external UART write, reading SPI data
  /* Infinite loop */
	for(;;)
	{
		xSemaphoreTake(external_semHandle, portMAX_DELAY);
		//Take both semaphores to keep other tasks from running (might be unnecessary bc higher priority)
		//xSemaphoreTake(uart_semHandle, portMAX_DELAY);
		//xSemaphoreTake(spi_semHandle, portMAX_DELAY);
		USART_GPS->CR1 &= ~(USART_CR1_RXNEIE); // Disable UART Interrupt
		spi_gps_read_addr = 0;

		for(int i = 0; i < num_messages; i++){

			//Read NMEA_LEN bytes of data
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_SPI_Transmit(&hspi1, (uint8_t *)&READ, 1, 100);
			HAL_SPI_Transmit(&hspi1, (uint8_t *)&spi_gps_read_addr, 2, 100);
			HAL_SPI_Receive(&hspi1, (uint8_t *)&gps_ext_buffer.Buffer, NMEA_LEN, 100);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

			spi_gps_read_addr += NMEA_LEN; //Increase offset to read next data value

			// Read status register
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_SPI_Transmit(&hspi1, (uint8_t *)&RDSR, 1, 100);
			HAL_SPI_Receive(&hspi1, (uint8_t *)statusbuf, 1, 100);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

			//Write NMEA message to external UART
			HAL_UART_Transmit(&huart1, (uint8_t*)&gps_ext_buffer.Buffer, NMEA_LEN, 100);
		}

		//Let other tasks continue running
		USART_GPS->CR1 |= USART_CR1_RXNEIE; // Enable UART Interrupt
		//xSemaphoreGive(uart_semHandle);
		//xSemaphoreGive(spi_semHandle);

	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_Startuart2Task */
/**
* @brief Function implementing the uart2Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Startuart2Task */
void Startuart2Task(void const * argument)
{
  /* USER CODE BEGIN Startuart2Task */
	float latitude, longitude;

	char* message_id, *time, *data_valid, *raw_latitude, *raw_longitude, *latdir, *longdir;

	//Set RF Switch to 0 for internal antenna:
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

	/* Infinite loop */
	for(;;)
	{
	  if(uxQueueMessagesWaitingFromISR(xQueueSerialDataReceived)>0)
	  {


		  xQueueReceive(xQueueSerialDataReceived,&(SerialBufferReceived),1);
		  //Fill and check header
		  for(int c = 0; c < 6; c++){
			  nmea_header[c] = SerialBufferReceived.Buffer[c];
		  }
		  if(!strcmp(nmea_header, "$GPRMC")){
			  if(SerialBufferReceived.Buffer[18] == 'V'){
				  //No fix, turn on LED
				  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//
//			  }
//			  if(SerialBufferReceived.Buffer[18] == 'A'){
//				  //Got a fix, turn off LED
//				  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//
//				  message_id = SerialBufferReceived.Buffer;
//				  time = FIND_AND_NUL(message_id, time, ',');
//				  data_valid = FIND_AND_NUL(time, data_valid, ',');
//				  raw_latitude = FIND_AND_NUL(data_valid, raw_latitude, ',');
//				  latdir = FIND_AND_NUL(raw_latitude, latdir, ',');
//				  raw_longitude = FIND_AND_NUL(latdir, raw_longitude, ',');
//				  longdir = FIND_AND_NUL(raw_longitude, longdir, ',');
//
//				  latitude = GpsToDecimalDegrees(raw_latitude, *latdir);
//				  longitude = GpsToDecimalDegrees(raw_longitude, *longdir);


				  if(tim1_counter > 1000){ //Post SPI write semaphore every 1s there is a valid message
					  xSemaphoreGive(spi_semHandle);
					  tim1_counter = 0;
					  xSemaphoreTake(uart_semHandle, portMAX_DELAY); //Wait until SPI is posted
				  }
			  }
		  }
		  got_nmea=0;
	  }
	//osDelay(1);
	}
  /* USER CODE END Startuart2Task */
}

/* USER CODE BEGIN Header_startspi1Task */
/**
* @brief Function implementing the spi1Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startspi1Task */
void startspi1Task(void const * argument)
{
  /* USER CODE BEGIN startspi1Task */
	HAL_StatusTypeDef response = HAL_ERROR;

	//SPI Initialization **************************
	//Write CS Pin high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	// Enable write enable latch (allow write operations)
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&WREN, 1, 100);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

	// Test bytes to write to EEPROM
	spi_mout_buf[0] = 0xAB;
	spi_mout_buf[1] = 0xCD;
	spi_mout_buf[2] = 0xEF;

	// Write 3 bytes starting at given address
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&WRITE, 1, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&spi_addr, 2, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)spi_mout_buf, 3, 100);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	//IO Driver for output pin enable

	// Clear buffer
	spi_mout_buf[0] = 0;
	spi_mout_buf[1] = 0;
	spi_mout_buf[2] = 0;

	// Wait until WIP bit is cleared
	spi_wip = 1;
	while (spi_wip)
	{
	 // Read status register
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	 HAL_SPI_Transmit(&hspi1, (uint8_t *)&RDSR, 1, 100);
	 response = HAL_SPI_Receive(&hspi1, (uint8_t *)spi_mout_buf, 1, 100);
	 if (response == HAL_OK) {
	  printf("Status Reg: %02x \r\n", spi_mout_buf[0]);
	 } else {
	  printf("Got error response as %d\r\n", response);
	 }
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

	 // Mask out WIP bit
	 spi_wip = spi_mout_buf[0] & 0b00000001;
	}

	// Read the 3 bytes back
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&READ, 1, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&spi_addr, 2, 100);
	HAL_SPI_Receive(&hspi1, (uint8_t *)spi_mout_buf, 3, 100);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

	// Read status register
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&RDSR, 1, 100);
	HAL_SPI_Receive(&hspi1, (uint8_t *)spi_mout_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

	/* Infinite loop */
	for(;;)
	{
	  //osStatus stat = osSemaphoreAcquire(SPI_semHandle, osWaitForever); //Wait for nmea sem to be posted
		xSemaphoreTake(spi_semHandle, portMAX_DELAY);
	  //osDelay(1);

	  //Send over SPI to FRAM
	  //osSemaphoreRelease(UART_semHandle); //Tell UART to gather more data
		//Set Write enable latch
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_SPI_Transmit_IT(&hspi1, (uint8_t *)&WREN, 1);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

		// Write 64 bytes starting at given address
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&WRITE, 1, 100);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&spi_addr, 2, 100);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&SerialBufferReceived.Buffer, NMEA_LEN, 100);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

		// TEST READ ECHO
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&READ, 1, 100);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&spi_addr, 2, 100);
		HAL_SPI_Receive(&hspi1, (uint8_t *)&test_spi_buf.Buffer, NMEA_LEN, 100);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

		spi_addr += NMEA_LEN; //Offset within destination device to hold NMEA message

		if(spi_addr > 0x7FFF) spi_addr = 0;


		xSemaphoreGive(uart_semHandle);

		//Blink LED to signal SPI write happened
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
		vTaskDelay( 200 / portTICK_PERIOD_MS );
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	}
  /* USER CODE END startspi1Task */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	tim1_counter++; //Incrementing at 1kHz (1000 in 1 second)
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
