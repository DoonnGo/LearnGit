#include "includes.h"
#include "app_config.h"
#include "ui/ui_api.h"
#include "system/includes.h"
#include "system/timer.h"
#include "asm/spi.h"

#if TCFG_LCD_ST7735S_ENABLE

/* 初始化代码 */
static const InitCode LcdInit_code[] = {
    {0x01, 0},				// soft reset
    {REGFLAG_DELAY, 120},	// delay 120ms
    {0x11, 0},				// sleep out
    {REGFLAG_DELAY, 120},
    {0xB1, 3, {0x02, 0x35, 0x36}},
    {0xB2, 3, {0X02, 0X35, 0X36}},
    {0XB3, 6, {0X02, 0X35, 0X36, 0X02, 0X35, 0X36}},
    {0XB4, 1, {0X03}},
    {0XC0, 3, {0XA2, 0X02, 0X84}},
    {0XC1, 1, {0XC5}},
    {0XC2, 2, {0X0D, 0X00}},
    {0XC3, 2, {0X8D, 0X2A}},
    {0XC4, 2, {0X8D, 0XEE}},
    {0XC5, 1, {0X03}},
    {0X36, 1, {0XC8}},
    {0XE0, 16, {0X04, 0X16, 0X0D, 0X14, 0X3A, 0X33, 0X2A, 0X2E, 0X2C, 0X29, 0X30, 0X3C, 0X00, 0X01, 0X01, 0X10}},
    {0XE1, 16, {0X04, 0X16, 0X0D, 0X13, 0X3A, 0X33, 0X2A, 0X2E, 0X2C, 0X28, 0X2F, 0X3B, 0X00, 0X01, 0X01, 0X10}},
    {0X3A, 1, {0X05}},
    {0X2A, 4, {0X00, 0X00, 0X00, 0X7F}},
    {0X2B, 4, {0X00, 0X00, 0X00, 0X7F}},
    {0X29, 0},
    {REGFLAG_DELAY, 20},
    {0X2C, 0},
    {REGFLAG_DELAY, 20},
};


void TFT_Write_Cmd(u8 data)
{
    lcd_cs_l();
    lcd_rs_l();
    spi_dma_send_byte(data);
    lcd_cs_h();
}

void TFT_Write_Data(u8 data)
{
    lcd_cs_l();
    lcd_rs_h();
    spi_dma_send_byte(data);
    lcd_cs_h();
}

void TFT_Write_Map(char *map, int size)
{
    spi_dma_send_map(map, size);
}


void TFT_GPIO_Init(void)
{
}

void TFT_Set_Draw_Area(int xs, int xe, int ys, int ye)
{
    TFT_Write_Cmd(0x2A);
    TFT_Write_Data(xs >> 8);
    TFT_Write_Data(xs);
    TFT_Write_Data(xe >> 8);
    TFT_Write_Data(xe);

    TFT_Write_Cmd(0x2B);
    TFT_Write_Data(ys >> 8);
    TFT_Write_Data(ys);
    TFT_Write_Data(ye >> 8);
    TFT_Write_Data(ye);

    TFT_Write_Cmd(0x2C);

    lcd_cs_l();
    lcd_rs_h();
}

static void TFT_BackLightCtrl(u8 on)
{
    if (on) {
        lcd_bl_h();
    } else {
        lcd_bl_l();
    }
    printf("call %s \n", __FUNCTION__);
}

#define LCD_WIDTH 132
#define LCD_HIGHT 132
#define LINE_BUFF_SIZE  (10*2*LCD_WIDTH*2)
static u8 line_buffer[LINE_BUFF_SIZE] __attribute__((aligned(4)));

REGISTER_LCD_DRIVE() = {
    .name = "st7735s",
    .lcd_width = LCD_WIDTH,
    .lcd_height = LCD_HIGHT,
    .color_format = LCD_COLOR_RGB565,
    .interface = LCD_SPI,
    .column_addr_align = 1,
    .row_addr_align = 1,
    .dispbuf = line_buffer,
    .bufsize = LINE_BUFF_SIZE,
    .initcode = LcdInit_code,
    .initcode_cnt = sizeof(LcdInit_code) / sizeof(LcdInit_code[0]),
    .GpioInit = TFT_GPIO_Init,
    .WriteComm = TFT_Write_Cmd,
    .WriteData = TFT_Write_Data,
    .WriteMap = TFT_Write_Map,
    .SetDrawArea = TFT_Set_Draw_Area,
    .Reset = NULL,
    .BackLightCtrl = TFT_BackLightCtrl,
};

#endif


