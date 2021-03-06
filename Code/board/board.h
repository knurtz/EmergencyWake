/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#ifndef BOARD_H
#define BOARD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*
 * Setup for STMicroelectronics STM32F4-Discovery board.
 */

/*
 * Board identifier.
 */
#define BOARD_EMERGENCY_WAKE
#define BOARD_NAME                  "Emergency Wake"

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768U
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                8000000U
#endif

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   330U

/*
 * MCU type as defined in the ST header.
 */
#define STM32F407xx

/*
 * IO pins assignments.
 */
#define GPIOA_WAKEUP                0U
#define GPIOA_LEVER                 1U
#define GPIOA_PIN2                  2U
#define GPIOA_PIN3                  3U
#define GPIOA_SD_CS                 4U
#define GPIOA_SD_SCK                5U
#define GPIOA_LDR                   6U
#define GPIOA_SD_MOSI               7U
#define GPIOA_PIN8                  8U
#define GPIOA_UART_TX               9U
#define GPIOA_UART_RX               10U
#define GPIOA_USB_DM                11U
#define GPIOA_USB_DP                12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_I2S_WS                15U

#define GPIOB_I2C_INT               0U
#define GPIOB_BUZZER                1U
#define GPIOB_PIN2                  2U
#define GPIOB_PIN3                  3U
#define GPIOB_SD_MISO               4U
#define GPIOB_I2S_SD                5U
#define GPIOB_EEPROM_SCL            6U
#define GPIOB_EEPROM_SDA            7U
#define GPIOB_PIN8                  8U
#define GPIOB_PIN9                  9U
#define GPIOB_I2C_SCL               10U
#define GPIOB_I2C_SDA               11U
#define GPIOB_SPI_STB               12U
#define GPIOB_SPI_SCK               13U
#define GPIOB_SPI_MISO              14U
#define GPIOB_SPI_MOSI              15U

#define GPIOC_LED1                  0U
#define GPIOC_LED2                  1U
#define GPIOC_LED3                  2U
#define GPIOC_LED4                  3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_FILA                  6U
#define GPIOC_FILB                  7U
#define GPIOC_SD_D0                 8U
#define GPIOC_I2S_MCLK              9U
#define GPIOC_I2S_CK                10U
#define GPIOC_PIN11                 11U
#define GPIOC_SD_CK                 12U
#define GPIOC_PIN13                 13U
#define GPIOC_OSC32 IN              14U
#define GPIOC_OSC32_OUT             15U

#define GPIOD_PIN0                  0U
#define GPIOD_PIN1                  1U
#define GPIOD_SD_CMD                2U
#define GPIOD_PIN3                  3U
#define GPIOD_PIN4                  4U
#define GPIOD_SD_EN                 5U
#define GPIOD_SD_DET                6U
#define GPIOD_PIN7                  7U
#define GPIOD_PIN8                  8U
#define GPIOD_PROX_EN               9U
#define GPIOD_PIN10                 10U
#define GPIOD_PIN11                 11U
#define GPIOD_ENC_A                 12U
#define GPIOD_ENC_B                 13U
#define GPIOD_PIN14                 14U
#define GPIOD_DCDC_EN               15U

#define GPIOE_EEPROM_NWC            0U
#define GPIOE_PIN1                  1U
#define GPIOE_ENC_BUT               2U
#define GPIOE_TOGGLE_UP             3U
#define GPIOE_TOGGLE_DN             4U
#define GPIOE_USER_BUT              5U
#define GPIOE_PIN6                  6U
#define GPIOE_PIN7                  7U
#define GPIOE_PIN8                  8U
#define GPIOE_DCF_SIG               9U
#define GPIOE_DCF_EN                10U
#define GPIOE_PIN11                 11U
#define GPIOE_PIN12                 12U
#define GPIOE_PIN13                 13U
#define GPIOE_PIN14                 14U
#define GPIOE_5V_SENSE              15U

/*
 * EmergencyWake board assignments.
 */

#define LINE_LED1                   PAL_LINE(GPIOC, GPIOC_LED1)                     
#define LINE_LED2                   PAL_LINE(GPIOC, GPIOC_LED2)
#define LINE_LED3                   PAL_LINE(GPIOC, GPIOC_LED3)
#define LINE_LED4                   PAL_LINE(GPIOC, GPIOC_LED4)

#define LINE_USER_BUT               PAL_LINE(GPIOE, GPIOE_USER_BUT)

#define LINE_LEVER                  PAL_LINE(GPIOA, GPIOA_LEVER)
#define LEVER_DOWN                  0
#define LEVER_UP                    1

#define LINE_TOGGLE_UP              PAL_LINE(GPIOE, GPIOE_TOGGLE_UP)
#define LINE_TOGGLE_DN              PAL_LINE(GPIOE, GPIOE_TOGGLE_DN)

#define LINE_ENC_BUT                PAL_LINE(GPIOE, GPIOE_ENC_BUT)
#define LINE_ENC_A                  PAL_LINE(GPIOD, GPIOD_ENC_A)
#define LINE_ENC_B                  PAL_LINE(GPIOD, GPIOD_ENC_B)
#define ENC_AF                      2U

#define LINE_USB_DP                 PAL_LINE(GPIOA, GPIOA_USB_DP)
#define LINE_USB_DM                 PAL_LINE(GPIOA, GPIOA_USB_DM)
#define USB_AF                      10U

#define LINE_UART_TX                PAL_LINE(GPIOA, GPIOA_UART_TX)
#define LINE_UART_RX                PAL_LINE(GPIOA, GPIOA_UART_RX)
#define UART_AF                     7U

#define LINE_SD_CMD                 PAL_LINE(GPIOD, GPIOD_SD_CMD)
#define LINE_SD_CK                  PAL_LINE(GPIOC, GPIOC_SD_CK)
#define LINE_SD_D0                  PAL_LINE(GPIOC, GPIOC_SD_D0)
#define LINE_SD_EN                  PAL_LINE(GPIOD, GPIOD_SD_EN)
#define SD_AF                       12U

#define LINE_I2S_MCLK               PAL_LINE(GPIOC, GPIOC_I2S_MCLK)
#define MCO_AF                      0U
#define LINE_I2S_CK                 PAL_LINE(GPIOC, GPIOC_I2S_CK)        // I2S3 - CK         
#define LINE_I2S_SD                 PAL_LINE(GPIOB, GPIOB_I2S_SD)        // I2S3 - SD
#define LINE_I2S_WS                 PAL_LINE(GPIOA, GPIOA_I2S_WS)        // I2S3 - WS
#define I2S_AF                      6U

#define LINE_I2C_SCL                PAL_LINE(GPIOB, GPIOB_I2C_SCL)       // I2C2 - SCL
#define LINE_I2C_SDA                PAL_LINE(GPIOB, GPIOB_I2C_SDA)       // I2C2 - SDA
#define LINE_I2C_INT                PAL_LINE(GPIOB, GPIOB_I2C_INT)
#define I2C_AF                      4U

#define LINE_SPI_MISO               PAL_LINE(GPIOB, GPIOB_SPI_MISO)
#define LINE_SPI_MOSI               PAL_LINE(GPIOB, GPIOB_SPI_MOSI)
#define LINE_SPI_SCK                PAL_LINE(GPIOB, GPIOB_SPI_SCK)
#define LINE_SPI_STB                PAL_LINE(GPIOB, GPIOB_SPI_STB)
#define SPI_AF                      5U

#define LINE_DCDC_EN                PAL_LINE(GPIOD, GPIOD_DCDC_EN)

#define LINE_FILA                   PAL_LINE(GPIOC, GPIOC_FILA)
#define LINE_FILB                   PAL_LINE(GPIOC, GPIOC_FILB)
#define FIL_AF                      3U

#define LINE_BUZZER                 PAL_LINE(GPIOB, GPIOB_BUZZER)


/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
