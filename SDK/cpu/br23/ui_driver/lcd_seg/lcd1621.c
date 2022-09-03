#include "includes.h"
#include "app_config.h"
#include "app_task.h"

#include "ui/ui_api.h"

#if USER_UI_1621LCD_ENABLE
#include "lcd1621.h"

//////////////////显示命令字典/////////////////////
#define CMD_BYTE   8
#define MODE_BYTE  3
#define DATA_BYTE  4
#define ADDR_BYTE  6
#define MODE_CMD   0x80
#define MODE_WRITE 0xa0

#define SYS_EN          0x01
#define SYS_DIS         0x00
#define SYS_DIS         0x00
#define LCD_ON_1621     0x03
#define RC_256K         0x18
#define IRQ_DIS         0x80
#define LCD_OFF_1621    0x02
#define BIAS_1_3        0x29
#define BIAS_1_2        0x28

////////////////////////片选线///////////////////////////////////
#define SETB_OUT()              JL_PORTC->DIR &= ~BIT(3)

#define SETB_HIGH               JL_PORTC->OUT |=  BIT(3)
#define SETB_LOW                JL_PORTC->OUT &= ~BIT(3)

////////////////////////////////////////////////////////////////

////////////////////////数据线///////////////////////////////////
#define Sdata_Ouput_1621()      JL_PORTC->DIR &= ~BIT(5)

#define Sdata_h_1621()          JL_PORTC->OUT |=  BIT(5)
#define Sdata_l_1621()          JL_PORTC->OUT &= ~BIT(5)
////////////////////////////////////////////////////////////////

////////////////////////时钟线///////////////////////////////////
#define Sclk_Ouput_1621()       JL_PORTC->DIR &= ~BIT(4)

#define Sclk_h_1621()           JL_PORTC->OUT |=  BIT(4)
#define Sclk_l_1621()           JL_PORTC->OUT &= ~BIT(4)
////////////////////////////////////////////////////////////////

//#define LCD1621_IO_INIT()			GPIOADE |= (BIT(2)|BIT(3)|BIT(4))

u8 lcd1621_sendbuf[16] = {0};   ///2020-12-10 lcd1621传输中转

void delay_lcd(u8 n)
{
    while (n--)
    {
        asm("nop");
        asm("nop");
    }
}

void lcd1621_write_byte(u8 datas, u8 len)
{
    u8 cnt;
    Sdata_Ouput_1621();
    Sclk_Ouput_1621();
    delay_lcd(10);
    cnt = len;
    delay_lcd(10);
    while(cnt > 0)
    {
        Sclk_l_1621();
        delay_lcd(10);
        if((datas & 0x80)==0x80)
            Sdata_h_1621();
        else
            Sdata_l_1621();
        delay_lcd(10);
        Sclk_h_1621();
        datas<<=1;
        cnt--;
        delay_lcd(10);
    }

    if(len ==CMD_BYTE)
    {
        Sclk_l_1621();
        Sdata_l_1621();
        delay_lcd(10);
        Sclk_h_1621();
        delay_lcd(10);
    }
}

void lcd1621_write_cmd(u8 cmd)
{
    delay_lcd(5);
    SETB_LOW;
    delay_lcd(10);
    lcd1621_write_byte(MODE_CMD, MODE_BYTE);          //写入100命令模式
    lcd1621_write_byte(cmd, CMD_BYTE);
    delay_lcd(10);
    SETB_HIGH;
    delay_lcd(5);
}


void lcd1621_write_data(u8 addr)
{
    u8 i;
    delay_lcd(5);
    SETB_LOW;
    delay_lcd(10);
    lcd1621_write_byte(MODE_WRITE,MODE_BYTE) ;    //写模式101
    lcd1621_write_byte(addr,ADDR_BYTE);           //数据写入地址
    for(i=0;i<16;i++) //要写入的数据
    {
//        lcd1621_write_byte(lcd1621_sendbuf[i]>>8, DATA_BYTE);
//        lcd1621_write_byte(lcd1621_sendbuf[i]>>4, DATA_BYTE);
//        lcd1621_write_byte(lcd1621_sendbuf[i], DATA_BYTE);
//        lcd1621_write_byte(lcd1621_sendbuf[i]<<4, DATA_BYTE);
        lcd1621_write_byte(lcd1621_sendbuf[i], DATA_BYTE);
        lcd1621_write_byte(lcd1621_sendbuf[i]<<4, DATA_BYTE);
    }
    delay_lcd(10);
    SETB_HIGH;
    delay_lcd(5);
}

void lcd1621_reset(void)
{
    SETB_OUT();
    Sclk_Ouput_1621();
    Sdata_Ouput_1621();
    delay_lcd(5);
    lcd1621_write_cmd(SYS_EN);
    lcd1621_write_cmd(RC_256K);         //选取片内RC晶振
    lcd1621_write_cmd(BIAS_1_3);       //1/3偏压,4个COM端
    lcd1621_write_cmd(IRQ_DIS);        //时基电路失效（看门狗失效）
    lcd1621_write_cmd(LCD_ON_1621);           //点着显示屏
}

void lcd1621_value_set(void)
{
	lcd1621_reset();
	lcd1621_write_data(0);
}

void lcd1621_init(void)
{
	//LCD1621_IO_INIT();
	lcd1621_value_set();
}

void lcd1621_off(void)
{
	lcd1621_write_cmd(LCD_OFF_1621);
}


#endif
