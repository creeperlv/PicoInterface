#ifndef __PICOINTERFACE_DISPLAY__
#define __PICOINTERFACE_DISPLAY__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

struct render_area
{
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
};
void Display_RenderAreaInit(struct render_area *area);
void Display_RenderAreaCalcBufferLen(struct render_area *area);
void Display_DriverInit(void *i2c_instance_ptr);
void Display_DeviceInit();
void Display_DeviceCommand(void* cmdPtr);
void Display_SetPixel(uint8_t *buf, int x, int y, bool on);
int Display_GetWidth();
int Display_GetBufferLen();
int Display_GetHeight();
int Display_GetTextBufferWidth();
int Display_GetTextBufferHeight();
void Display_render(uint8_t *buf, struct render_area *area);
void Display_DrawLine(uint8_t *buf, int x0, int y0, int x1, int y1, bool on);
void Display_WriteChar(uint8_t *buf, int16_t x, int16_t y, uint8_t ch);
void Display_WriteString(uint8_t *buf, int16_t x, int16_t y, char *str);
#endif