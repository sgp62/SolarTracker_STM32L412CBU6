/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
    char Buffer[100];
} SerialBuffer;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2 //9 for USART1
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3 //10 for USART1
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

// GPS Message buffer definitions
#define PROCESS_COUNTER 15
#define PROCESS_RX_BUFFER_SIZE 600
#define PROCESS_PARSER_BUFFER_SIZE 600
static char current_parser_buf[ PROCESS_PARSER_BUFFER_SIZE ];

#define USART_GPS USART2//LPUART1
#define USART_GPS_IRQn USART2_IRQn//LPUART1_IRQn
#define USART_GPS_IRQHandler USART2_IRQHandler//LPUART1_IRQHandler

#ifdef USART_CR1_TXEIE_TXFNFIE // FIFO Support (L4R9)
#define USART_CR1_TXEIE USART_CR1_TXEIE_TXFNFIE
#define USART_ISR_TXE USART_ISR_TXE_TXFNF
#define USART_CR1_RXNEIE USART_CR1_RXNEIE_RXFNEIE
#define USART_ISR_RXNE USART_ISR_RXNE_RXFNE
#endif


#define LINEMAX 200 // Maximal allowed/expected line length
static char rx_buffer[LINEMAX + 1]; // Local holding buffer to build line, w/NUL
static int rx_index = 0;
static char got_nmea = 0;
static char nmea_header[6];



#define SPI_BUF_SIZE 100
static uint8_t spi_mout_buf[SPI_BUF_SIZE];
static uint8_t spi_min_buf[SPI_BUF_SIZE];
static uint16_t spi_addr = 0x00;
static uint8_t spi_wip;






/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
