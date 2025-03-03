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

#include "hardware/adc.h"

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
int main()
{
    stdio_init_all();
    Display_SetMode(128,64);
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
    Display_WriteString(buf, 40, 15, "C E F I");
    Display_render(buf, &frame_area);
    sleep_ms(500);
    size_t W=Display_GetWidth();
    size_t h=Display_GetHeight();
    char* strBuf=malloc(20*sizeof(char));
    float max=0;
    while (true)
    {
        float temp=read_onboard_temperature('C');
        if(max<temp+10){
            max=temp+10;
        }
        float v=temp/max;
        for (size_t i = 0; i < h; i++)
        {
            Display_DrawLine(buf,0,i,W*v,i,true);
            Display_DrawLine(buf,W*v,i,W,i,false);
        }
        sprintf(strBuf,"%f",temp);
        Display_WriteString(buf, W/2-20, h/2-8, strBuf);
        Display_render(buf, &frame_area);
        
    }
    
}
