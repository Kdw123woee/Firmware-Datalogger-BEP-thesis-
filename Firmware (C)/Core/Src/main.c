/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

/* Defines -------------------------------------------------------------------*/
#define STANDBY_WAIT_TIME_MIN 59 //hourly measurement
#define STANDBY_WAIT_TIME_S 3  // Debug/demo purpose

/* Peripheral Handles ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
LPTIM_HandleTypeDef hlptim1;
RTC_HandleTypeDef hrtc;

/* Function Prototypes --------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C3_Init(void);
static void MX_LPTIM1_Init(void);
void WriteToMem(I2C_HandleTypeDef *hi2c, uint16_t data);
static int16_t TI_ReadTemperature(I2C_HandleTypeDef *hi2c);

/* Read temperature from TI sensor */
static int16_t TI_ReadTemperature(I2C_HandleTypeDef *hi2c) {
    const uint8_t TEMP_SENSOR_ADDR = 0x9E; // i2c device address
    const uint8_t TEMP_REG_ADDR = 0x00;

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);  // Sensor power ON, sensor automatically starts measurement

    //15ms stop2 mode during wait for sensor measurement
    HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 15);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    HAL_LPTIM_TimeOut_Stop_IT(&hlptim1);

    HAL_PWREx_EnableUltraLowPowerMode();
    HAL_PWREx_EnableLowPowerRunMode(); //re-enable low power run mode

    //receive data from sensor register
    uint8_t reg = TEMP_REG_ADDR;
    uint8_t temp_raw[2];
    if (HAL_I2C_Master_Transmit(hi2c, TEMP_SENSOR_ADDR, &reg, 1, HAL_MAX_DELAY) == HAL_OK &&
        HAL_I2C_Master_Receive(hi2c, TEMP_SENSOR_ADDR, temp_raw, 2, HAL_MAX_DELAY) == HAL_OK) {
        int16_t temp = (int16_t)((temp_raw[0] << 8) | temp_raw[1]);
        temp >>= 4;  // 12-bit data, right-justified
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);  // Sensor power OFF
        return temp;
    } else {
        return 0;
    }
}

/* Write temperature data to external EEPROM memory */
void WriteToMem(I2C_HandleTypeDef *hi2c, uint16_t data) {
    const uint8_t devaddr = 0x53 << 1; //i2c device address
    const uint16_t BASE_ADDRESS = 0x0008; //optional offset of 8 bytes for subgroup 3 purposes, else 0x0000

    uint16_t last_addr = READ_REG(TAMP->BKP0R) & 0xFFFF; //backup register which saves last written address
    //if last address = 0, start writing from byte 8
    uint16_t next_addr = (last_addr == 0) ? BASE_ADDRESS : last_addr + 2; 

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);  // Memory power ON

    // wait until the memory is ready
    while (HAL_I2C_IsDeviceReady(hi2c, devaddr, 5, HAL_MAX_DELAY) != HAL_OK) {}

    //write 2 data bytes
    uint8_t data_buf[2] = {(uint8_t)(data >> 8), (uint8_t)(data & 0xFF)};
    if (HAL_I2C_Mem_Write(hi2c, devaddr, next_addr, I2C_MEMADD_SIZE_16BIT, data_buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);  // Memory power OFF
        return;
    }

    while (HAL_I2C_IsDeviceReady(hi2c, devaddr, 5, HAL_MAX_DELAY) != HAL_OK) {}

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);  // Memory power OFF
    WRITE_REG(TAMP->BKP0R, next_addr); //store MSB address
}

/* Main program */
int main(void) {
    HAL_Init();
    SystemClock_Config();

    HAL_PWREx_EnableUltraLowPowerMode();
    HAL_PWREx_EnableLowPowerRunMode();

    MX_GPIO_Init();
    MX_I2C2_Init();
    MX_RTC_Init();
    MX_I2C3_Init();
    MX_LPTIM1_Init();

    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET) {  // Wake from standby
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
        __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
        HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

        int16_t temperaturedata = TI_ReadTemperature(&hi2c2);
        WriteToMem(&hi2c3, temperaturedata);

    } else { // first startup
        WRITE_REG(TAMP->BKP0R, 0); //initial startup set backup register (used for last address)
    }

    HAL_NVIC_ClearPendingIRQ(RTC_TAMP_IRQn); //clear interrupt from wake-up timer

    //set RTC timer for wake up from standby
    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, STANDBY_WAIT_TIME_MIN, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0) != HAL_OK) {
        Error_Handler();
    }

    HAL_PWR_EnterSTANDBYMode(); //enter standby mode
}

/* System Clock Configuration */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_3; //800kHz
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/* I2C2 Initialization */
static void MX_I2C2_Init(void) {
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x00000000;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigFastModePlus(&hi2c2, I2C_FASTMODEPLUS_ENABLE) != HAL_OK) {
        Error_Handler();
    }
}

/* I2C3 Initialization */
static void MX_I2C3_Init(void) {
    hi2c3.Instance = I2C3;
    hi2c3.Init.Timing = 0x00000000;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigFastModePlus(&hi2c3, I2C_FASTMODEPLUS_ENABLE) != HAL_OK) {
        Error_Handler();
    }
}

/* LPTIM1 Initialization */
static void MX_LPTIM1_Init(void) {
    hlptim1.Instance = LPTIM1;
    hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
    hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim1.Init.Period = 20;
    hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    hlptim1.Init.RepetitionCounter = 0;

    if (HAL_LPTIM_Init(&hlptim1) != HAL_OK) {
        Error_Handler();
    }
    HAL_LPTIM_TimeOut_Stop_IT(&hlptim1);
}

/* RTC Initialization */
static void MX_RTC_Init(void) {
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;

    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        Error_Handler();
    }
}

/* GPIO Initialization */
static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_5;  // Sensor power control
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;  // Memory power control
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* Error Handler */
void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}