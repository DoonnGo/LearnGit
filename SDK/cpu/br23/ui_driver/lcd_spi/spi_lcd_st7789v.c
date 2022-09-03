#include "includes.h"
#include "app_config.h"
#include "ui/ui_api.h"
#include "system/includes.h"
#include "system/timer.h"
#include "asm/spi.h"

#if TCFG_LCD_ST7789V_ENABLE

static void delay_ms(unsigned int ms);

/* 初始化代码 */
static const InitCode LcdInit_code[] = {
    {0x01, 0},				// soft reset
    {REGFLAG_DELAY, 120},	// delay 120ms
    {0x11, 0},				// sleep out
    {REGFLAG_DELAY, 120},
    {0x36, 1, {0x00}},
    {0x3A, 1, {0x05}},
    {0xB2, 5, {0x0c, 0x0c, 0x00, 0x33, 0x33}},
    {0xB7, 1, {0x22}},
    {0xBB, 1, {0x36}},
    {0xC2, 1, {0x01}},
    {0xC3, 1, {0x19}},
    {0xC4, 1, {0x20}},
    {0xC6, 1, {0x0F}},
    {0xD0, 2, {0xA4, 0xA1}},
    /* {0xE0,14, {0x70,0x04,0x08,0x09,0x09,0x05,0x2A,0x33,0x41,0x07,0x13,0x13,0x29,0x2F}}, */
    /* {0xE1,14, {0x70,0x03,0x09,0x0A,0x09,0x06,0x2B,0x34,0x41,0x07,0x12,0x14,0x28,0x2E}}, */
    {0xE0, 14, {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}},
    {0xE1, 14, {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}},
    {0X21, 0},
    {0X2A, 4, {0x00, 0x00, 0x00, 0xEF}},
    {0X2B, 4, {0x00, 0x00, 0x00, 0xEF}},
    {0X29, 0},
    {REGFLAG_DELAY, 20},
    {0X2C, 0},
    {REGFLAG_DELAY, 20},
};


void TFT_Write_Cmd(u8 data)
{
    lcd_cs_l();
    lcd_rs_l();
    lcd_spi_send_byte(data);
    lcd_cs_h();
}

void TFT_Write_Data(u8 data)
{
    lcd_cs_l();
    lcd_rs_h();
    lcd_spi_send_byte(data);
    lcd_cs_h();
}

void TFT_Write_Map(char *map, u32 size)
{
    spi_dma_send_map(map, size);
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
}

static void TFT_EnterSleep()
{
    TFT_Write_Cmd(0x28);
    /* delay_ms(120); */
    TFT_Write_Cmd(0x10);
    delay_ms(120);
}

static void TFT_ExitSleep()
{
    TFT_Write_Cmd(0x11);
    delay_ms(120);
    TFT_Write_Cmd(0x29);
    /* delay_ms(120); */
}

static void delay_ms(unsigned int ms)
{
    delay_2ms((ms + 1) / 2);
}


static void lcd_reset()
{
    lcd_reset_h();
    delay_ms(1000);
    lcd_reset_l();
    delay_ms(1000);
    lcd_reset_h();
}

#define LCD_WIDTH 240
#define LCD_HIGHT 240
#define LINE_BUFF_SIZE  (10*2*LCD_WIDTH*2)
static u8 line_buffer[LINE_BUFF_SIZE] __attribute__((aligned(4)));

REGISTER_LCD_DRIVE() = {
    .name = "st7789v",
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
    .WriteComm = TFT_Write_Cmd,
    .WriteData = TFT_Write_Data,
    .WriteMap = TFT_Write_Map,
    .SetDrawArea = TFT_Set_Draw_Area,
    .Reset = lcd_reset,
    .BackLightCtrl = TFT_BackLightCtrl,
    .EnterSleep = TFT_EnterSleep,
    .ExitSleep = TFT_ExitSleep,
};

#endif


