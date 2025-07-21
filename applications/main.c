#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "key.h"
#include "adc.h"
#include <stdio.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include "debug_log.h"

void SystemClock_Config(void);

/**
 * @brief 主函数
 * @return int
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    lpuart1_init();
    usart2_init();

    /* 等待一小段时间确保UART初始化完成 */
    HAL_Delay(100);

    /* 启动时打印信息 */
    LOG_I("编译时间: %s %s", __DATE__, __TIME__);
    LOG_I("系统运行中...");

    /* 初始化按键 */
    key_config_t key_cfg = {
        .filter_time = 50,       // 滤波时间50ms
        .long_press_time = 1000, // 长按时间1000ms
        .repeat_time = 200       // 重复时间200ms
    };
    key_init(&key_cfg);
    LOG_I("按键驱动初始化完成");

    /* 初始化ADC */
    if (adc_init() == 0) {
        LOG_I("ADC驱动初始化完成");
    } else {
        LOG_E("ADC驱动初始化失败");
    }

    while (1)
    {
        uart_proc();
        
        /* 按键处理 */
        key_process();
        
        /* 按键事件检测 */
        for (int i = 0; i < (int)KEY_MAX; i++) {
            key_event_t event = key_get_event((key_id_t)i);
            if (event != KEY_EVENT_NONE) {
                switch (event) {
                    case KEY_EVENT_PRESS:
                        LOG_I("按键%d按下", i + 1);
                        break;
                    case KEY_EVENT_RELEASE:
                        LOG_I("按键%d松开", i + 1);
                        break;
                    case KEY_EVENT_LONG_PRESS:
                        LOG_I("按键%d长按", i + 1);
                        break;
                    case KEY_EVENT_REPEAT:
                        LOG_I("按键%d重复", i + 1);
                        break;
                    default:
                        break;
                }
                key_clear_event((key_id_t)i);
            }
        }
        
        /* ADC电压检测 */
        static uint32_t last_adc_time = 0;
        uint32_t current_time = HAL_GetTick();
        
        if (current_time - last_adc_time >= 1000) { // 每秒读取一次ADC
            adc_start_conversion();
            if (adc_is_conversion_complete(100)) {
                uint16_t raw_value = adc_get_raw_value();
                uint32_t voltage_mv = adc_get_voltage_mv();
                float voltage_v = adc_get_voltage_v();
                //LOG_I("ADC: 原始值=%d, 电压=%dmV (%.2fV)", raw_value, voltage_mv, voltage_v);
            }
            last_adc_time = current_time;
        }
        
        HAL_Delay(1);
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_LPUART1;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
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
