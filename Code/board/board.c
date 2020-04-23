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

#include "hal.h"
#include "stm32_gpio.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of STM32 GPIO port setup.
 */
typedef struct {
  uint32_t              moder;
  uint32_t              otyper;
  uint32_t              ospeedr;
  uint32_t              pupdr;
  uint32_t              odr;
  uint32_t              afrl;
  uint32_t              afrh;
} gpio_setup_t;

/**
 * @brief   Type of STM32 GPIO initialization data.
 */
typedef struct {
  gpio_setup_t          PAData;
  gpio_setup_t          PBData;
  gpio_setup_t          PCData;  
  gpio_setup_t          PDData;
  gpio_setup_t          PEData;
} gpio_config_t;

/**
 * @brief   STM32 GPIO static initialization data.
 */
/*
static const gpio_config_t gpio_default_config = {

  {VAL_GPIOA_MODER, VAL_GPIOA_OTYPER, VAL_GPIOA_OSPEEDR, VAL_GPIOA_PUPDR,
   VAL_GPIOA_ODR,   VAL_GPIOA_AFRL,   VAL_GPIOA_AFRH},

  {VAL_GPIOB_MODER, VAL_GPIOB_OTYPER, VAL_GPIOB_OSPEEDR, VAL_GPIOB_PUPDR,
   VAL_GPIOB_ODR,   VAL_GPIOB_AFRL,   VAL_GPIOB_AFRH},

  {VAL_GPIOC_MODER, VAL_GPIOC_OTYPER, VAL_GPIOC_OSPEEDR, VAL_GPIOC_PUPDR,
   VAL_GPIOC_ODR,   VAL_GPIOC_AFRL,   VAL_GPIOC_AFRH},

  {VAL_GPIOD_MODER, VAL_GPIOD_OTYPER, VAL_GPIOD_OSPEEDR, VAL_GPIOD_PUPDR,
   VAL_GPIOD_ODR,   VAL_GPIOD_AFRL,   VAL_GPIOD_AFRH},

  {VAL_GPIOE_MODER, VAL_GPIOE_OTYPER, VAL_GPIOE_OSPEEDR, VAL_GPIOE_PUPDR,
   VAL_GPIOE_ODR,   VAL_GPIOE_AFRL,   VAL_GPIOE_AFRH}

};
*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/*
static void gpio_init(stm32_gpio_t *gpiop, const gpio_setup_t *config) {
  gpiop->OTYPER  = config->otyper;
  gpiop->OSPEEDR = config->ospeedr;
  gpiop->PUPDR   = config->pupdr;
  gpiop->ODR     = config->odr;
  gpiop->AFRL    = config->afrl;
  gpiop->AFRH    = config->afrh;
  gpiop->MODER   = config->moder;
}
*/

static void stm32_gpio_init(void) {

  /* Enabling GPIO-related clocks, the mask comes from the
     registry header file.*/
  rccResetAHB1(STM32_GPIO_EN_MASK);
  rccEnableAHB1(STM32_GPIO_EN_MASK, true);

  /* Initializing all used GPIO ports.*/
  palSetLineMode(LINE_USER_BUT, PAL_MODE_INPUT);

  palSetLineMode(LINE_LED1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_LED2, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_LED3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_LED4, PAL_MODE_OUTPUT_PUSHPULL);

/*
  palSetLineMode(LINE_USB_DP, PAL_MODE_ALTERNATE(USB_AF));      // USB FS DM
  palSetLineMode(LINE_USB_DM, PAL_MODE_ALTERNATE(USB_AF));      // USB FS DP
*/

  palSetLineMode(LINE_UART_TX, PAL_MODE_ALTERNATE(UART_AF));    // UART1 TX
  palSetLineMode(LINE_UART_RX, PAL_MODE_ALTERNATE(UART_AF));    // UART1 RX

  palSetLineMode(LINE_SD_CMD, PAL_MODE_ALTERNATE(SD_AF));       // SDIO CMD
  palSetLineMode(LINE_SD_CK, PAL_MODE_ALTERNATE(SD_AF));        // SDIO CK
  palSetLineMode(LINE_SD_D0, PAL_MODE_ALTERNATE(SD_AF));        // SDIO D0
/*
  palSetPadMode(GPIOD, 2, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(12));    // SDIO CMD
  palSetPadMode(GPIOC, 12, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(12));   // SDIO CK
  palSetPadMode(GPIOC, 8, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(12));    // SDIO D0
*/
  palSetLineMode(LINE_SD_EN, PAL_MODE_OUTPUT_PUSHPULL);         // enable line for SD card power supply

  palSetLineMode(LINE_I2C_SCL, PAL_STM32_OTYPE_OPENDRAIN | PAL_MODE_ALTERNATE(I2C_AF));     // I2C2 SCL
  palSetLineMode(LINE_I2C_SDA, PAL_STM32_OTYPE_OPENDRAIN | PAL_MODE_ALTERNATE(I2C_AF));     // I2C2 SDA

  palSetLineMode(LINE_I2S_CK, PAL_MODE_ALTERNATE(I2S_AF));      // I2S3 clock
  palSetLineMode(LINE_I2S_WS, PAL_MODE_ALTERNATE(I2S_AF));      // I2S3 data
  palSetLineMode(LINE_I2S_SD, PAL_MODE_ALTERNATE(I2S_AF));      // I2S3 word select / left right clock

}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Early initialization code.
 * @details GPIO ports and system clocks are initialized before everything
 *          else.
 */
void __early_init(void) {

  stm32_gpio_init();
  stm32_clock_init();

}

#if HAL_USE_SDC || defined(__DOXYGEN__)

/**
 * @brief   SDC card detection.
 */
bool sdc_lld_is_card_inserted(SDCDriver *sdcp) {

  (void)sdcp;
  return true;
}

/**
 * @brief   SDC card write protection detection.
 */
bool sdc_lld_is_write_protected(SDCDriver *sdcp) {

  (void)sdcp;
  return false;
}

#endif // HAL_USE_SDC

#if HAL_USE_MMC_SPI || defined(__DOXYGEN__)
/**
 * @brief   MMC_SPI card detection.
 */
bool mmc_lld_is_card_inserted(MMCDriver *mmcp) {

  (void)mmcp;
  return true;
}

/**
 * @brief   MMC_SPI card write protection detection.
 */
bool mmc_lld_is_write_protected(MMCDriver *mmcp) {

  (void)mmcp;
  return false;
}
#endif // HAL_USE_MMC_SPI

/**
 * @brief   Board-specific initialization code.
 * @note    You can add your board-specific code here.
 */
void boardInit(void) {



}

