#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "../src/interface/ICComm.h"
#include "../PicoSSD1306Device/DisplayProtocol.h"
#include "hardware/adc.h"
#include "pico/binary_info.h"

float read_onboard_temperature(const char unit)
{

    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C')
    {
        return tempC;
    }
    else if (unit == 'F')
    {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}
void ConsoleSetChar(ICCommConfig config, char *buf, char x, char y, char c)
{
    buf[0] = ICComm_Cmd_Console_SetChar;
    buf[1] = x;
    buf[2] = y;
    buf[3] = c;
    ICComm_WriteData(config, buf, 32);
}
int main()
{
    stdio_init_all(); 
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    float max = 0;
    _ICCommConfig config;
    config.SPI_Inst = spi_default;
    config.OperateMode = ICComm_Mode_Slave;
    config.PIN_CSN = PICO_DEFAULT_SPI_CSN_PIN;
    config.PIN_RX = PICO_DEFAULT_SPI_RX_PIN;
    config.PIN_SCK = PICO_DEFAULT_SPI_SCK_PIN;
    config.PIN_TX = PICO_DEFAULT_SPI_TX_PIN;
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));
    ICComm_Setup(&config);
    char dataBuf[128];
    char strBuf[20];
    dataBuf[0] = ICComm_Cmd_ChangeMode;
    dataBuf[1] = ICComm_Display_Mode_Console;
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    ICComm_WriteData(&config, dataBuf, 32);
    bool v=false;
    while (true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, v);
        v++;
        v=v/2;
        float temp = read_onboard_temperature('C');
        if (max < temp + 10)
        {
            max = temp + 10;
        }
        float v = temp / max;

        sprintf(strBuf, "%f", temp);
        for (size_t chPos = 0; chPos < 20; chPos++)
        {
            char c = strBuf[chPos];
            if (c == 0)
                break;
            ConsoleSetChar(&config, dataBuf, 0, chPos, c);
        }
        sleep_ms(100);
    }
}
