#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "../src/interface/Display.h"

int main()
{
    stdio_init_all();

    Display_DriverInit(i2c_default);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    Display_DeviceInit();

    struct render_area frame_area;
    Display_RenderAreaInit(&frame_area);

    Display_RenderAreaCalcBufferLen(&frame_area);
    uint8_t buf[Display_GetBufferLen()];
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
    char *blank[] = {
        "_______________",
        "_______________",
        "_______________",
        "_______________",
    };
    char *text[] = {
        "CEFI 0.0.1     ",
        "[0000]CEFI Loaded",
        "[0001]INIT: S1306",
        "[0002]INIT: UART",
    };
    char *text2[] = {
        "[0001]INIT: S1306",
        "[0002]INIT: UART",
        "[0003]INIT: CPU",
        "[0004]INIT: SDLDR",
    };
    char *text3[] = {
        "[0002]INIT: UART",
        "[0003]INIT: CPU",
        "[0004]INIT: SDLDR",
        "[0005]FAIL: SDLDR",
    };
    char *text4[] = {
        "[0003]INIT: CPU",
        "[0004]INIT: SDLDR",
        "[0005]FAIL: SDLDR",
        "REBOOT         ",
    };
    int y = 0;
display:
    y = 0;
    for (uint i = 0; i < count_of(blank); i++)
    {
        Display_WriteString(buf, 5, y, blank[i]);
        y += 8;
    }
    Display_render(buf, &frame_area);
    sleep_ms(500);
    Display_WriteString(buf, 40, 15, "C E F I");
    Display_render(buf, &frame_area);
    sleep_ms(500);
    y = 0;
    for (uint i = 0; i < count_of(text); i++)
    {
        Display_WriteString(buf, 5, y, text[i]);
        y += 8;
    }
    Display_render(buf, &frame_area);
    sleep_ms(200);
    y = 0;
    for (uint i = 0; i < count_of(text2); i++)
    {
        Display_WriteString(buf, 5, y, text2[i]);
        y += 8;
    }
    Display_render(buf, &frame_area);
    sleep_ms(200);
    y = 0;
    for (uint i = 0; i < count_of(text3); i++)
    {
        Display_WriteString(buf, 5, y, text3[i]);
        y += 8;
    }
    Display_render(buf, &frame_area);
    sleep_ms(200);
    y = 0;
    for (uint i = 0; i < count_of(text4); i++)
    {
        Display_WriteString(buf, 5, y, text4[i]);
        y += 8;
    }
    Display_render(buf, &frame_area);
    sleep_ms(500);
    goto display;
}
