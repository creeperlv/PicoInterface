#include "../../interface/Display.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "ssd1306_font_sarasa.h"

int SSD1306_HEIGHT = 32;
int SSD1306_WIDTH = 128;
#define SSD1306_PAGE_HEIGHT _u(8)
int SSD1306_NUM_PAGES;
int SSD1306_BUF_LEN;
// #define SSD1306_NUM_PAGES (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
// #define SSD1306_BUF_LEN (SSD1306_NUM_PAGES * SSD1306_WIDTH)
#define SSD1306_I2C_ADDR _u(0x3C)

// 400 is usual, but often these can be overclocked to improve display response.
// Tested at 1000 on both 32 and 84 pixel height devices and it worked.
#define SSD1306_I2C_CLK 400
// #define SSD1306_I2C_CLK             1000

// commands (see datasheet)
#define SSD1306_SET_MEM_MODE _u(0x20)
#define SSD1306_SET_COL_ADDR _u(0x21)
#define SSD1306_SET_PAGE_ADDR _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL _u(0x26)
#define SSD1306_SET_SCROLL _u(0x2E)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST _u(0x81)
#define SSD1306_SET_CHARGE_PUMP _u(0x8D)

#define SSD1306_SET_SEG_REMAP _u(0xA0)
#define SSD1306_SET_ENTIRE_ON _u(0xA4)
#define SSD1306_SET_ALL_ON _u(0xA5)
#define SSD1306_SET_NORM_DISP _u(0xA6)
#define SSD1306_SET_INV_DISP _u(0xA7)
#define SSD1306_SET_MUX_RATIO _u(0xA8)
#define SSD1306_SET_DISP _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV _u(0xD5)
#define SSD1306_SET_PRECHARGE _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG _u(0xDA)
#define SSD1306_SET_VCOM_DESEL _u(0xDB)

#define SSD1306_WRITE_MODE _u(0xFE)
#define SSD1306_READ_MODE _u(0xFF)

void calc_render_area_buflen(struct render_area *area)
{
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

void Display_SetMode(int W, int H)
{
    SSD1306_HEIGHT = H;
    SSD1306_WIDTH = W;
}
void SSD1306_send_cmd(uint8_t cmd)
{
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_buf(uint8_t buf[], int buflen)
{
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf + 1, buf, buflen);

    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}
void SSD1306_send_cmd_list(uint8_t *buf, int num)
{
    for (int i = 0; i < num; i++)
        SSD1306_send_cmd(buf[i]);
}
void Display_DriverInit(void *i2c_instance_ptr)
{
    i2c_init(i2c_instance_ptr, SSD1306_I2C_CLK * 1000);
}
void Display_DeviceInit()
{
    uint8_t MAGIC = 0x12;
    if (SSD1306_WIDTH == 128 && SSD1306_HEIGHT == 32)
    {
        MAGIC = 0x02;
    }
    else if (SSD1306_WIDTH == 128 && SSD1306_HEIGHT == 64)
    {
        MAGIC = 0x12;
    }
    else
    {
        MAGIC = 0x2;
    }
    uint8_t cmds[] = {
        SSD1306_SET_DISP, // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE, // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                 // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        SSD1306_HEIGHT - 1,             // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number.
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
        MAGIC,
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV, // set display clock divide ratio
        0x80,                     // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,    // set pre-charge period
        0xF1,                     // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,   // set VCOMH deselect level
        0x30,                     // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST, // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON,     // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,     // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,   // set charge pump
        0x14,                      // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00, // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01,   // turn display on
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}
void SSD1306_scroll(bool on)
{
    // configure horizontal scrolling
    uint8_t cmds[] = {
        SSD1306_SET_HORIZ_SCROLL | 0x00,
        0x00,                                // dummy byte
        0x00,                                // start page 0
        0x00,                                // time interval
        0x03,                                // end page 3 SSD1306_NUM_PAGES ??
        0x00,                                // dummy byte
        0xFF,                                // dummy byte
        SSD1306_SET_SCROLL | (on ? 0x01 : 0) // Start/stop scrolling
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}
void Display_SetPixel(uint8_t *buf, int x, int y, bool on)
{

    assert(x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT);

    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
    // byte 1 is x = 1, y=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    const int BytesPerRow = SSD1306_WIDTH; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = buf[byte_idx];

    if (on)
        byte |= 1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    buf[byte_idx] = byte;
}
void Display_DeviceCommand(void *cmdPtr)
{
    SSD1306_send_cmd(((uint8_t *)cmdPtr)[0]);
}
int Display_GetWidth()
{
    return SSD1306_WIDTH;
}
int Display_GetHeight()
{
    return SSD1306_HEIGHT;
}
int Display_GetBufferLen()
{
    return (SSD1306_NUM_PAGES * SSD1306_WIDTH);
}
void Display_RenderAreaInit(struct render_area *area)
{
    SSD1306_NUM_PAGES = SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT;
    struct render_area a = {
        start_col : 0,
        end_col : SSD1306_WIDTH - 1,
        start_page : 0,
        end_page : SSD1306_NUM_PAGES - 1
    };
    area[0] = a;
}
void Display_RenderAreaCalcBufferLen(struct render_area *area)
{

    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}
void Display_render(uint8_t *buf, struct render_area *area)
{

    // update a portion of the display with a render area
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page};

    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(buf, area->buflen);
}
int Display_GetTextBufferWidth()
{
    return SSD1306_WIDTH / 8;
}
int Display_GetTextBufferHeight()
{
    return SSD1306_HEIGHT / 8;
}

inline int GetFontIndex(uint8_t ch)
{
    return ch;
}
void Display_DrawLine(uint8_t *buf, int x0, int y0, int x1, int y1, bool on)
{

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true)
    {
        Display_SetPixel(buf, x0, y0, on);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}
void Display_WriteChar(uint8_t *buf, int16_t x, int16_t y, uint8_t ch)
{

    if (x > SSD1306_WIDTH - FONT_WIDTH || y > SSD1306_HEIGHT - FONT_HEIGHT)
        return;

    int idx = GetFontIndex(ch);
    switch (FONT_MODE)
    {
    case 0:
    {

        // For the moment, only write on Y row boundaries (every 8 vertical pixels)
        y = y / FONT_HEIGHT;
        int fb_idx = y * 128 + x;

        for (int i = 0; i < FONT_WIDTH; i++)
        {
            buf[fb_idx++] = font[idx * FONT_WIDTH + i];
        }
    }
    break;
    case 1:
    {
        int charSize = FONT_WIDTH * FONT_HEIGHT;
        int dataPtr = idx * charSize;
        for (size_t _x = 0; _x < FONT_WIDTH; _x++)
        {
            for (size_t _y = 0; _y < FONT_HEIGHT; _y++)
            {
                Display_SetPixel(buf, x + _x, y + _y, font[dataPtr + _x * FONT_HEIGHT + _y]);
            }
        }
    }
    break;
    }
}
void Display_WriteString(uint8_t *buf, int16_t x, int16_t y, char *str)
{

    // Cull out any string off the screen
    if (x > SSD1306_WIDTH - FONT_WIDTH || y > SSD1306_HEIGHT - FONT_HEIGHT)
        return;

    while (*str)
    {
        Display_WriteChar(buf, x, y, *str++);
        x += FONT_WIDTH;
    }
}