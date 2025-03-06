#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "../src/interface/Display.h"
#include "../src/interface/ICComm.h"
#include "pico/multicore.h"
#include "DisplayProtocol.h"

#include "hardware/adc.h"
void ICCommThread()
{
}

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
#define CUSTOM_PIN_CSN 0
#define CUSTOM_PIN_RX 1
#define CUSTOM_PIN_SCK 2
#define CUSTOM_PIN_TX 3
int main()
{
    stdio_init_all();
    Display_SetMode(128, 64);
    Display_DriverInit(i2c_default);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    Display_DeviceInit();

    struct render_area frame_area;
    Display_RenderAreaInit(&frame_area);

    Display_RenderAreaCalcBufferLen(&frame_area);
    uint8_t buf[Display_GetBufferLen()];
    uint8_t cmdBuf[128];
    memset(buf, 0, Display_GetBufferLen());
    Display_render(buf, &frame_area);
    uint8_t SSD1306_SET_ALL_ON = 0xA5;
    uint8_t SSD1306_SET_ENTIRE_ON = 0xA4;
    for (int i = 0; i < 1; i++)
    {
        Display_DeviceCommand(&SSD1306_SET_ALL_ON); // Set all pixels on
        sleep_ms(500);
        Display_DeviceCommand(&SSD1306_SET_ENTIRE_ON); // go back to following RAM for pixel state
        sleep_ms(500);
    }
    sleep_ms(500);
    Display_WriteString(buf, 0, 0, "SSD1306");
    Display_WriteString(buf, 0, 16, "Driver");
    Display_WriteString(buf, 0, 32, "v.0.0.1.0");
    Display_render(buf, &frame_area);
    sleep_ms(500);
    size_t W = Display_GetWidth();
    size_t h = Display_GetHeight();
    char *strBuf = malloc(20 * sizeof(char));
    float max = 0;
    _ICCommConfig config;
    config.SPI_Inst = spi_default;
    config.OperateMode = ICComm_Mode_Slave;
    config.PIN_CSN = CUSTOM_PIN_CSN;
    config.PIN_RX = CUSTOM_PIN_RX;
    config.PIN_SCK = CUSTOM_PIN_SCK;
    config.PIN_TX = CUSTOM_PIN_TX;
    ICComm_Setup(&config);
    bi_decl(bi_4pins_with_func(CUSTOM_PIN_RX, CUSTOM_PIN_TX, CUSTOM_PIN_SCK, CUSTOM_PIN_CSN, GPIO_FUNC_SPI));

    // multicore_launch_core1(ICCommThread);
    char DispMode = -1;
    while (true)
    {
        ICComm_ReadData(&config, cmdBuf, 32);
        switch (cmdBuf[0])
        {
        case ICComm_Cmd_ChangeMode:
            DispMode = cmdBuf[1];
            Display_WriteString(buf, 0, 0, "Console Mode");
            Display_render(buf, &frame_area);
            break;
        case ICComm_Cmd_Console_SetChar:
            {
                short X=cmdBuf[1];
                short Y=cmdBuf[2];
                char Char=cmdBuf[3];
                Display_WriteChar(buf, X,Y,Char);
            }
            break;
        default:
            break;
        }
        switch (DispMode)
        {
        case ICComm_Display_Mode_Console:

            break;

        default:
            break;
        }
        float temp = read_onboard_temperature('C');
        if (max < temp + 10)
        {
            max = temp + 10;
        }
        float v = temp / max;
        for (size_t i = 0; i < h / 5; i++)
        {
            Display_DrawLine(buf, 0, i, W * v, i, true);
            Display_DrawLine(buf, W * v, i, W, i, false);
        }
        sprintf(strBuf, "%f", temp);
        Display_WriteString(buf, 0, h / 2 - 16, strBuf);
        Display_WriteString(buf, 0, h - 16, "This is a test of screen.");
        Display_render(buf, &frame_area);
    }
}
