#include "includes.h"
#include "app_config.h"
#include "app_task.h"

#include "ui/ui_api.h"
#include "lcdseg.h"

#if USER_UI_LCDSEG_ENABLE || USER_UI_1621LCD_ENABLE
#if USER_UI_1621LCD_ENABLE
#include "lcd1621.h"
#endif

#define LCDSEG_DEBUG_ENABLE
#ifdef LCDSEG_DEBUG_ENABLE
#define lcdseg_debug(fmt, ...) 	printf("[LCDSEG] "fmt, ##__VA_ARGS__)
#else
#define lcdseg_debug(...)
#endif

#define lcdseg_error(fmt, ...) 	printf("[LCDSEG ERR] "fmt, ##__VA_ARGS__)

//extern void (*timer_led_scan)(void *param); //Timer2定时器注册位

lcdseg_cb_t lcdseg_cb;

const u16 lcdseg_num_table[11] =
{
    LCDSEG_0, LCDSEG_1, LCDSEG_2, LCDSEG_3, LCDSEG_4,
    LCDSEG_5, LCDSEG_6, LCDSEG_7, LCDSEG_8, LCDSEG_9,
    LCDSEG_NC,
};

const u16 ID3_Table_Cap[] =
{
    LCDSEG_A_C,LCDSEG_B_C,LCDSEG_C_C,LCDSEG_D_C,LCDSEG_E_C,LCDSEG_F_C,LCDSEG_G_C,
    LCDSEG_H_C,LCDSEG_I_C,LCDSEG_J_C,LCDSEG_K_C,LCDSEG_L_C,LCDSEG_M_C,LCDSEG_N_C,
    LCDSEG_O_C,LCDSEG_P_C,LCDSEG_Q_C,LCDSEG_R_C,LCDSEG_S_C,LCDSEG_T_C,
    LCDSEG_U_C,LCDSEG_V_C,LCDSEG_W_C,LCDSEG_X_C,LCDSEG_Y_C,LCDSEG_Z_C,
};

const u16 ID3_Table[] =
{
    LCDSEG_A,LCDSEG_B,LCDSEG_C,LCDSEG_D,LCDSEG_E,LCDSEG_F,LCDSEG_G,
    LCDSEG_H,LCDSEG_I,LCDSEG_J,LCDSEG_K,LCDSEG_L,LCDSEG_M,LCDSEG_N,
    LCDSEG_O,LCDSEG_P,LCDSEG_Q,LCDSEG_R,LCDSEG_S,LCDSEG_T,
    LCDSEG_U,LCDSEG_V,LCDSEG_W,LCDSEG_X,LCDSEG_Y,LCDSEG_Z,
};

const u16 ID3_Symbol_Table[] =
{
    LCDSEG_AMP,LCDSEG_APO,LCDSEG_BRACKET_L,LCDSEG_BRACKET_R,LCDSEG_UNDEF,
    LCDSEG_PLUS,LCDSEG_COMMA,LCDSEG__,LCDSEG_POINT,LCDSEG_SLASH,
};

///const u8 Com_Table[] =
///{
///    /*COM1*/5,
///    /*COM2*/4,
///    /*COM3*/3,
///    /*COM4*/2,
///    /*COM5*/1,
///    /*COM6*/0,
///};
///
///const u8 Seg_Table[] =
///{
///    /*PA*/12,
///    /*PA*/11,
///    /*PA*/10,
///    /*PA*/9,
///    /*PA*/8,
///    /*PA*/7,
///    /*PA*/6,
///    /*PA*/5,
///    /*PA*/4,
///    /*PA*/3,
///    /*PA*/2,
///    /*PA*/1,
///    /*PA*/0,
///    /*PC*/7,
///    /*PB*/5,
///    /*PB*/4,
///};

AT_VOLATILE_RAM_CODE
void lcd_seg_flash_icon(void)
{
    static u8 flash_show_hide = 0;
    u8 bit_count = 0;
    if(lcdseg_cb.flash_500ms != flash_show_hide)
    {
        flash_show_hide = lcdseg_cb.flash_500ms;
    }
    else
    {
        return;
    }
    if(lcdseg_cb.flash_500ms || lcdseg_cb.ignore_next_flash)
    {
        while(lcdseg_cb.flash_char_bit>>bit_count)
        {
            if(lcdseg_cb.flash_char_bit & BIT(bit_count))
            {
                lcdseg_disp_buff(lcdseg_cb.screen_show_buf[bit_count], bit_count+1);
            }
            bit_count++;
        }
        lcdseg_buff2pin();
#if CONFIG_CLIENT_DG
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT) lcdseg_cb.pin_A[COM3] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)    lcdseg_cb.pin_A[COM1] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)    lcdseg_cb.pin_A[COM3] |= BIT(12);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)     lcdseg_cb.pin_A[COM4] |= BIT(12);
        //if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)   lcdseg_cb.pin_A[COM2] |= BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MP3)    lcdseg_cb.pin_A[COM6] |= BIT(12);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_INT)    lcdseg_cb.pin_A[COM6] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RPT)    lcdseg_cb.pin_A[COM5] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RDM)    lcdseg_cb.pin_A[COM4] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_EQ)     lcdseg_cb.pin_A[COM6] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_POP)    lcdseg_cb.pin_A[COM5] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_ROCK)   lcdseg_cb.pin_A[COM4] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_CLASS)  lcdseg_cb.pin_A[COM3] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_BT)     lcdseg_cb.pin_A[COM2] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_FM)     lcdseg_cb.pin_A[COM5] |= BIT(12);
#else
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT) lcdseg_cb.pin_A[COM2] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)    lcdseg_cb.pin_B[COM1] |= BIT(5);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)    lcdseg_cb.pin_A[COM2] |= BIT(6);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)     lcdseg_cb.pin_A[COM1] |= BIT(5);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)   lcdseg_cb.pin_A[COM2] |= BIT(3);
#endif
    }
    else
    {
        while(lcdseg_cb.flash_char_bit>>bit_count)
        {
            if(lcdseg_cb.flash_char_bit & BIT(bit_count))
            {
                lcdseg_disp_buff_hide(lcdseg_cb.screen_show_buf[bit_count], bit_count+1);
            }
            bit_count++;
        }
        lcdseg_buff2pin();
#if CONFIG_CLIENT_DG
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT) lcdseg_cb.pin_A[COM3] &= ~BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)    lcdseg_cb.pin_A[COM1] &= ~BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)    lcdseg_cb.pin_A[COM3] &= ~BIT(12);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)     lcdseg_cb.pin_A[COM4] &= ~BIT(12);
        //if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)   lcdseg_cb.pin_A[COM2] &= ~BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MP3)    lcdseg_cb.pin_A[COM6] &= ~BIT(12);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_INT)    lcdseg_cb.pin_A[COM6] &= ~BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RPT)    lcdseg_cb.pin_A[COM5] &= ~BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RDM)    lcdseg_cb.pin_A[COM4] &= ~BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_EQ)     lcdseg_cb.pin_A[COM6] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_POP)    lcdseg_cb.pin_A[COM5] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_ROCK)   lcdseg_cb.pin_A[COM4] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_CLASS)  lcdseg_cb.pin_A[COM3] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_BT)     lcdseg_cb.pin_A[COM2] &= ~BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_FM)     lcdseg_cb.pin_A[COM5] &= ~BIT(12);
#else
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT) lcdseg_cb.pin_A[COM2] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)    lcdseg_cb.pin_B[COM1] &= ~BIT(5);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)    lcdseg_cb.pin_A[COM2] &= ~BIT(6);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)     lcdseg_cb.pin_A[COM1] &= ~BIT(5);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)   lcdseg_cb.pin_A[COM2] &= ~BIT(3);
#endif
    }
    lcdseg_cb.ignore_next_flash = 0;
}

AT_VOLATILE_RAM_CODE
void lcd_seg_set(u8 com)
{
    u8 dir = (com + 1) % 2;

    com /= 2;

    lcd_seg_flash_icon();   //处理seg闪烁

//    JL_PORTC->DIR &= ~BIT(Com_Table[com]);    //这样用会死机复位
    JL_PORTC->DIR &= ~BIT(5-com);
#if CONFIG_CLIENT_DG
    JL_PORTA->DIR &= ~0x1FFC;
#else
    JL_PORTA->DIR &= ~0x1FFF;
    JL_PORTC->DIR &= ~BIT(7);
    JL_PORTB->DIR &= ~0x30;
#endif

    if(dir)
    {
//        JL_PORTC->OUT &= ~BIT(Com_Table[com]);
        JL_PORTC->OUT &= ~BIT(5-com);

#if CONFIG_CLIENT_DG
        JL_PORTA->OUT |= (~lcdseg_cb.pin_A[com])&0x1FFC;
#else
        JL_PORTA->OUT |= (~lcdseg_cb.pin_A[com])&0x1FFF;
        JL_PORTB->OUT |= (~lcdseg_cb.pin_B[com])&0x30;
        JL_PORTC->OUT |= (~lcdseg_cb.pin_C[com])&BIT(7);
#endif
    }
    else
    {
//        JL_PORTC->OUT |= BIT(Com_Table[com]);
        JL_PORTC->OUT |= BIT(5-com);

#if CONFIG_CLIENT_DG
        JL_PORTA->OUT |= lcdseg_cb.pin_A[com];
#else
        JL_PORTA->OUT |= lcdseg_cb.pin_A[com];
        JL_PORTB->OUT |= lcdseg_cb.pin_B[com];
        JL_PORTC->OUT |= lcdseg_cb.pin_C[com];
#endif
    }
}


AT_VOLATILE_RAM_CODE
void com_scan(void)
{
    static u8 com_cnt = 0;
	com_cnt++;
	if(com_cnt>=12)
		com_cnt = 0;
#if 1
///COM 输入打开10K上下拉
    JL_PORTC->DIR |= 0x3F;
    JL_PORTC->PU |= 0x3F;
    JL_PORTC->PD |= 0x3F;

#if 0 //17p lcd
///SEG 输出低电平
    JL_PORTA->OUT &= ~0x1FFF;
    JL_PORTC->OUT &= ~BIT(7);
    JL_PORTB->OUT &= ~0x30;
#else
    JL_PORTA->OUT &= ~0x1FFC;
#endif
#endif
    lcd_seg_set(com_cnt);
}


AT_VOLATILE_RAM_CODE
void lcdseg_port_clr(void)
{
#if 1
///COM 输出低电平
    JL_PORTC->DIR &= ~0x3F;
    JL_PORTC->PU &= ~0x3F;
    JL_PORTC->PD &= ~0x3F;
    JL_PORTC->OUT &= ~0x3F;

#if 0 //17p lcd
///SEG 输出低电平
    JL_PORTA->OUT &= ~0x1FFF;
    JL_PORTC->OUT &= ~BIT(7);
    JL_PORTB->OUT &= ~0x30;
#else
    JL_PORTA->OUT &= ~0x1FFC;
#endif
#endif
}

///knob
u8 knob_direction = 0;

AT_VOLATILE_RAM_CODE
void knob_detect(void)
{
	u8 state = 0;
	static u8  old_state = 0,knob_scan_data = 0;
#if USER_AD_KNOB_ENABLE
    extern u32 adc_get_knob_value(u32 ch);
    u16 ad_knob_val = adc_get_knob_value(USER_AD_KNOB_AD_CHANNEL);

#if ACC_DETECT_AD_REUSE_KNOB
    u8 acc_in = 0;
#endif
//    printf("k:%d", ad_knob_val);

#if USER_UI_1621LCD_ENABLE
    lcdseg_cb.rtc_time_ms++;
    if(lcdseg_cb.rtc_time_ms > 86400000)   ///1day = 24*60*60*1000ms
    {
        lcdseg_cb.rtc_time_ms = 0;
    }
    u16 flash_500ms = lcdseg_cb.rtc_time_ms % 1000;
	if(flash_500ms == 500)
    {
        lcdseg_cb.flash_500ms = 1;
    }
    else if(flash_500ms == 0/*1000*/)
    {
        lcdseg_cb.flash_500ms = 0;
        flash_500ms = 0;
    }
#endif

	if(ad_knob_val<349)
	{
	    if(state == 1 || state == 2 || state == 3)
            state = 0x03;
        else
            return;
	}
	else if(ad_knob_val<386)
	{
	    if(state == 0 || state == 2 || state == 3)
            state = 0x02;
        else
            return;
	}
	else if(ad_knob_val<432)
	{
	    if(state == 0 || state == 1 || state == 3)
            state = 0x01;
        else
            return;
	}
#if ACC_DETECT_AD_REUSE_KNOB
	else if(ad_knob_val>500)
    {
        acc_in = 1;
    }
    User_update_acc_det(acc_in);
#endif

#endif

	if(state != old_state)
	{
		old_state = state;
		knob_scan_data |= state;
		knob_scan_data<<=2;
//		printf("scan_data = 0x%02x\n",knob_scan_data);
		if(knob_scan_data==0x10||knob_scan_data==0x60)//(knob_scan_data==0xE0)
		{
			knob_direction= 2;
		}
		else if(knob_scan_data==0x20||knob_scan_data==0x90)//(knob_scan_data==0xd0)
		{
			knob_direction= 1;
		}
	}
}

extern int dac_energy_level;

AT_VOLATILE_RAM_CODE
void lcdseg_scan(void *param)
{
#if 1
	static u8 cnt;
	static u8 fft_wave_cnt = 0;
    lcdseg_cb.rtc_time_ms++;
    if(lcdseg_cb.rtc_time_ms > 86400000)   ///1day = 24*60*60*1000ms
    {
        lcdseg_cb.rtc_time_ms = 0;
    }
    u16 flash_500ms = lcdseg_cb.rtc_time_ms % 1000;
	if(flash_500ms == 500)
    {
        lcdseg_cb.flash_500ms = 1;
    }
    else if(flash_500ms == 0/*1000*/)
    {
        lcdseg_cb.flash_500ms = 0;
        flash_500ms = 0;
    }
    //if(lcdseg_cb.fft_enble)
    {
        //if((lcdseg_cb.rtc_time_ms % 50) == 0)
        {
            //lcdseg_cb.fft_level = rand32() % 4;
//            lcdseg_cb.fft_level = dac_energy_level;
        }

#if CONFIG_CLIENT_DG
        lcdseg_cb.pin_A[COM1] &= ~BIT(12);  //L1
        lcdseg_cb.pin_A[COM2] &= ~BIT(12);  //L2
        lcdseg_cb.pin_A[COM2] &= ~BIT(2);   //L3
        lcdseg_cb.pin_A[COM1] &= ~BIT(2);   //L4
        extern u8 User_mute_flag;
        extern s8 User_volume_value;
        if(dac_energy_level >= 1 && User_mute_flag == 0 && User_volume_value)
        {
            fft_wave_cnt++;
            if(fft_wave_cnt >= 100)
                dac_energy_level = 4;
            lcdseg_cb.pin_A[COM1] |= BIT(12);
            if(dac_energy_level >= 4)
            {
                fft_wave_cnt = 0;
                lcdseg_cb.pin_A[COM2] |= BIT(12);
                if(dac_energy_level >= 8)
                {
                    lcdseg_cb.pin_A[COM2] |= BIT(2);
                    if(dac_energy_level >= 11)
                    {
                        lcdseg_cb.pin_A[COM1] |= BIT(2);
                    }
                }
            }
        }
#else
        lcdseg_cb.pin_A[COM2] &= ~BIT(8);
        lcdseg_cb.pin_A[COM1] &= ~BIT(8);
        lcdseg_cb.pin_A[COM1] &= ~BIT(9);
        lcdseg_cb.pin_A[COM2] &= ~BIT(9);
        extern u8 User_mute_flag;
        if(dac_energy_level >= 1 && User_mute_flag == 0)
        {
            fft_wave_cnt++;
            if(fft_wave_cnt >= 100)
                dac_energy_level = 4;
            lcdseg_cb.pin_A[COM2] |= BIT(8);
            if(dac_energy_level >= 4)
            {
                fft_wave_cnt = 0;
                lcdseg_cb.pin_A[COM1] |= BIT(8);
                if(dac_energy_level >= 8)
                {
                    lcdseg_cb.pin_A[COM1] |= BIT(9);
                    if(dac_energy_level >= 11)
                    {
                        lcdseg_cb.pin_A[COM2] |= BIT(9);
                    }
                }
            }
        }
#endif
    }
	//if(sys_cb.disp_en)
	{
		knob_detect();
#if 0
		acc_detect();
#endif
#if 0
		cnt++;
		if(cnt<2 && lcdseg_cb.lock == 0)
		{
			com_scan();
		}
		else
		{
			cnt = 0;
			lcdseg_port_clr();
#if MUX_KEY_EN
			mux_key_detect();
			mux_key_io_clear();
#endif
		}
#else
		if(lcdseg_cb.lock == 0)
		{
			com_scan();
		}
#endif
	}
#else
    com_scan();
#endif
}

static void lcdseg_clear(void)
{
    for(u8 i = 0; i < 6; i++)
    {
        lcdseg_cb.seg_buf[i] = 0;
    }

    lcdseg_cb.icon_buf[0] = 0;
    lcdseg_cb.flash_icon_buf[0] = 0;
    lcdseg_cb.flash_char_bit = 0;
    memset(&lcd1621_sendbuf, 0, sizeof(lcd1621_sendbuf));
    memset(&lcdseg_cb.screen_show_buf, 0, sizeof(lcdseg_cb.screen_show_buf));
}

static void lcdseg_setXY(u32 x, u32 y)
{
    lcdseg_cb.pos_X = x;
}

static void lcdseg_show_icon(u32 x)
{
    //if(x == LED7_2POINT)    lcdseg_cb.icon_buf[0] |= LED7_2POINT;
    lcdseg_cb.icon_buf[0] |= x;
}

static void lcdseg_flash_icon(u32 x)
{
    lcdseg_cb.flash_icon_buf[0] |= x;
    lcdseg_cb.icon_buf[0] &= ~x;        //将需要闪烁的图标在常亮位里置0，避免图标不闪
}

static void lcdseg_show_string(u8* x)
{
    u8 i = 0;
    u16 tmp = 0;
    while(x[i] != 0)
    {
        if((x[i]>37)&&(x[i]<48))
            tmp = ID3_Symbol_Table[x[i]-38];
        else if((x[i]>47)&&(x[i]<58))
            tmp = lcdseg_num_table[x[i]-48];
        else if((x[i]>64)&&(x[i]<91))
            tmp = ID3_Table_Cap[x[i]-65];
        else if((x[i]>96)&&(x[i]<123))
            tmp = ID3_Table[x[i]-97];
        else if(x[i]==95)
            tmp = LCDSEG__;
        else if(x[i]==32)
            tmp = 0;
        else if(x[i]==0)
            tmp = 0;
        else
            tmp = LCDSEG_UNDEF;

        lcdseg_cb.screen_show_buf[lcdseg_cb.pos_X + i] = tmp;
        i++;
        lcdseg_disp_buff(tmp, lcdseg_cb.pos_X + i);
        if((lcdseg_cb.pos_X + i) >= 7)
            break;
    }
}

static void lcdseg_FlashChar(u32 x)
{
    lcdseg_cb.flash_char_bit |= BIT(x);
}

static void lcdseg_show_char(u8 x)
{
    u16 tmp = 0;
    lcdseg_cb.pos_X++;
    if(x != 0)
    {
        if((x>37)&&(x<48))
            tmp = ID3_Symbol_Table[x-38];
        else if((x>47)&&(x<58))
            tmp = lcdseg_num_table[x-48];
        else if((x>64)&&(x<91))
            tmp = ID3_Table_Cap[x-65];
        else if((x>96)&&(x<123))
            tmp = ID3_Table[x-97];
        else if(x==95)
            tmp = LCDSEG__;
        else if(x==32)
            tmp = 0;
        else if(x==0)
            tmp = 0;
        else
            tmp = LCDSEG_UNDEF;

        lcdseg_cb.screen_show_buf[lcdseg_cb.pos_X - 1] = tmp;
        lcdseg_disp_buff_hide(0, lcdseg_cb.pos_X);
        lcdseg_disp_buff(tmp, lcdseg_cb.pos_X);
    }
}

static void lcdseg_lock(u32 x)
{
    if(x == 0)
    {
#if USER_UI_LCDSEG_ENABLE
        lcdseg_buff2pin();
#elif USER_UI_1621LCD_ENABLE
        lcd1621_icon_update();
        lcd1621_value_set();
#endif
        lcdseg_cb.lock = 0;
    }
    else
    {
        lcdseg_cb.lock = 1;
    }
}

static bool lcdseg_flash_500ms(void)
{
    return lcdseg_cb.flash_500ms;
}

static u16 lcdseg_rtc_showtime(void)
{
    return lcdseg_cb.rtc_time_ms / 60000;
}

static void lcdseg_ignore_next_flash(void)
{
    lcdseg_cb.ignore_next_flash = 1;
}

static void lcdseg_show_fft(u8 en)
{
    lcdseg_cb.fft_enble = en;
}

static void lcdseg_show_fft_level(u8 level)
{
    lcdseg_cb.fft_level = level;
}

//static void lcdseg_clear(void)          {}
//static void lcdseg_setXY(u32 x, u32 y)  {}
//static void lcdseg_FlashChar(u32 x)     {}
static void lcdseg_Clear_FlashChar(u32 x) {}
//static void lcdseg_show_icon(u32 x)     {}
//static void lcdseg_flash_icon(u32 x)    {};
static void lcdseg_clear_icon(u32 x)    {}
//static void lcdseg_show_string(u8* x)   {lcdseg_debug("show_char:");while(*x != 0){printf("%c",*x);x++;};printf("\n");};
//static void lcdseg_show_char(u8 x)      {};
static void lcdseg_show_number(u8 x)    {};
static void lcdseg_show_pic(u32 x)      {};
static void lcdseg_hide_pic(u32 x)      {};
//static void lcdseg_lock(u32 x)          {};
//static void lcdseg_flash_500ms(void)    {};

static LCD_API LCDSEG_HW = {
    .clear             = lcdseg_clear,
    .setXY             = lcdseg_setXY,
    .FlashChar         = lcdseg_FlashChar,
    .Clear_FlashChar   = lcdseg_Clear_FlashChar,
    .show_icon         = lcdseg_show_icon,
    .flash_icon        = lcdseg_flash_icon,
    .clear_icon        = lcdseg_clear_icon,
    .show_string       = lcdseg_show_string,
    .show_char         = lcdseg_show_char,
    .show_number       = lcdseg_show_number,
    .show_pic          = lcdseg_show_pic,
    .hide_pic          = lcdseg_hide_pic,
    .lock              = lcdseg_lock,
    .flash_500ms       = lcdseg_flash_500ms,
    .rtc_showtime_get  = lcdseg_rtc_showtime,
    .ignore_next_flash = lcdseg_ignore_next_flash,
    .show_fft          = lcdseg_show_fft,
    .show_fft_level    = lcdseg_show_fft_level,
};

#if USER_UI_1621LCD_ENABLE
static void lcd1621_data_transfrom(void)
{
    for(u8 i=0;i<16;i++)
    {
        lcd1621_sendbuf[i] = 0x00;
    }
    lcd1621_sendbuf[0] = 0x10;
}

static void lcd1621_buf_clr(void)
{
    for(u8 i=0;i<16;i++)
    {
        lcd1621_sendbuf[i] = 0x00;
    }
}

static void lcd1621_fft_updata(void)
{
	static u8 fft_wave_cnt = 0;
    extern u8 User_mute_flag;
    extern s8 User_volume_value;
    lcd1621_sendbuf[0] &= ~BIT(7);  //L1
    lcd1621_sendbuf[0] &= ~BIT(6);  //L2
    lcd1621_sendbuf[0] &= ~BIT(5);  //L3
    lcd1621_sendbuf[0] &= ~BIT(4);  //L4
    if(dac_energy_level >= 1 && User_mute_flag == 0 && User_volume_value)
    {
        fft_wave_cnt++;
        if(fft_wave_cnt >= 5)
            dac_energy_level = 4;
        lcd1621_sendbuf[0] |= BIT(7);
        if(dac_energy_level >= 4)
        {
            fft_wave_cnt = 0;
            lcd1621_sendbuf[0] |= BIT(6);
            if(dac_energy_level >= 8)
            {
                lcd1621_sendbuf[0] |= BIT(5);
                if(dac_energy_level >= 11)
                {
                    lcd1621_sendbuf[0] |= BIT(4);
                }
            }
        }
    }
}

static void lcd1621_updata(void)
{
    if(lcdseg_cb.lock == 0)
    {
//        lcd1621_data_transfrom();
        lcd1621_fft_updata();
        lcd1621_flash_icon();
        lcd1621_value_set();
    }
    sys_timeout_add(NULL, lcd1621_updata, 49);
}
#endif

void *lcdseg_init(const struct led7_platform_data * _data)
{
    lcdseg_debug("Dong:lcdseg init DDDDDDDDDDDDDDDDDDDD\n");
    lcdseg_debug("%s", __func__);

    memset(&lcdseg_cb, 0, sizeof(lcdseg_cb));

#if USER_UI_LCDSEG_ENABLE
#if 0 //17p lcd
    JL_PORTA->DIE &= ~0x1FFF;
    JL_PORTC->DIE &= ~BIT(7);
    JL_PORTB->DIE &= ~0x30;

    JL_PORTA->DIEH &= ~0x1FFF;
    JL_PORTC->DIEH &= ~BIT(7);
    JL_PORTB->DIEH &= ~0x30;

    JL_PORTA->DIR &= ~0x1FFF;
    JL_PORTC->DIR &= ~BIT(7);
    JL_PORTB->DIR &= ~0x30;

    JL_PORTA->PU &= ~0x1FFF;
    JL_PORTC->PU &= ~BIT(7);
    JL_PORTB->PU &= ~0x30;

    JL_PORTA->PD &= ~0x1FFF;
    JL_PORTC->PD &= ~BIT(7);
    JL_PORTB->PD &= ~0x30;

    JL_PORTA->HD &= ~0x1FFF;
    JL_PORTC->HD &= ~BIT(7);
    JL_PORTB->HD &= ~0x30;

    JL_PORTA->HD0 &= ~0x1FFF;
    JL_PORTC->HD0 &= ~BIT(7);
    JL_PORTB->HD0 &= ~0x30;

    JL_PORTC->OUT &= ~0x3F;
    JL_PORTC->OUT &= ~0x3F;
    JL_PORTC->OUT &= ~0x3F;
    JL_PORTA->OUT &= ~0x1FFF;
    JL_PORTC->OUT &= ~BIT(7);
    JL_PORTB->OUT &= ~0x30;
#else
    JL_PORTC->DIE   &= ~0x3F;
    JL_PORTC->DIEH  &= ~0x3F;
    JL_PORTC->DIR   &= ~0x3F;
    JL_PORTC->PU    &= ~0x3F;
    JL_PORTC->PD    &= ~0x3F;
    JL_PORTC->HD    &= ~0x3F;
    JL_PORTC->HD0   &= ~0x3F;
    JL_PORTC->OUT   &= ~0x3F;

    JL_PORTA->DIE   &= ~0x1FFC;
    JL_PORTA->DIEH  &= ~0x1FFC;
    JL_PORTA->DIR   &= ~0x1FFC;
    JL_PORTA->PU    &= ~0x1FFC;
    JL_PORTA->PD    &= ~0x1FFC;
    JL_PORTA->HD    &= ~0x1FFC;
    JL_PORTA->HD0   &= ~0x1FFC;
    JL_PORTA->OUT   &= ~0x1FFC;
#endif

//     sys_hi_timer_add(NULL, lcdseg_scan, 1); //2ms
    void app_timer_led_scan(void (*led_scan)(void *));
    app_timer_led_scan(lcdseg_scan);
#elif USER_UI_1621LCD_ENABLE
    lcd1621_init();
    sys_timeout_add(NULL, lcd1621_updata, 50);   ///2020-12-10 需要实现50Ms定时传输1621数据

    void app_timer_led_scan(void (*led_scan)(void *));
    app_timer_led_scan(knob_detect);
#endif
    return (&LCDSEG_HW);
}

#if 0 //17p lcd
void lcdseg_disp_buff(u16 disp_data,u8 disp_pos)    //disp_pos最小为1
{
    if(disp_pos <= 5)
    {
        disp_pos--;
        if(disp_data&SEG_A) lcdseg_cb.seg_buf[COM6] |= BIT(SEG2 + 3 * disp_pos);
        if(disp_data&SEG_B) lcdseg_cb.seg_buf[COM6] |= BIT(SEG3 + 3 * disp_pos);
        if(disp_data&SEG_C) lcdseg_cb.seg_buf[COM4] |= BIT(SEG3 + 3 * disp_pos);
        if(disp_data&SEG_D) lcdseg_cb.seg_buf[COM3] |= BIT(SEG2 + 3 * disp_pos);
        if(disp_data&SEG_E) lcdseg_cb.seg_buf[COM4] |= BIT(SEG1 + 3 * disp_pos);
        if(disp_data&SEG_F) lcdseg_cb.seg_buf[COM6] |= BIT(SEG1 + 3 * disp_pos);
        if(disp_data&SEG_G) lcdseg_cb.seg_buf[COM4] |= BIT(SEG2 + 3 * disp_pos);
        if(disp_data&SEG_H) lcdseg_cb.seg_buf[COM5] |= BIT(SEG2 + 3 * disp_pos);
        if(disp_data&SEG_I) lcdseg_cb.seg_buf[COM5] |= BIT(SEG1 + 3 * disp_pos);
        if(disp_data&SEG_J) lcdseg_cb.seg_buf[COM3] |= BIT(SEG3 + 3 * disp_pos);
        if(disp_data&SEG_L) lcdseg_cb.seg_buf[COM3] |= BIT(SEG1 + 3 * disp_pos);
        if(disp_data&SEG_M) lcdseg_cb.seg_buf[COM5] |= BIT(SEG3 + 3 * disp_pos);
    }
    else if(disp_pos == 6)
    {
        if(disp_data&SEG_A) lcdseg_cb.seg_buf[COM2] |= BIT(SEG15);
        if(disp_data&SEG_B) lcdseg_cb.seg_buf[COM1] |= BIT(SEG12);
        if(disp_data&SEG_C) lcdseg_cb.seg_buf[COM1] |= BIT(SEG13);
        if(disp_data&SEG_D) lcdseg_cb.seg_buf[COM1] |= BIT(SEG14);
        if(disp_data&SEG_E) lcdseg_cb.seg_buf[COM2] |= BIT(SEG14);
        if(disp_data&SEG_F) lcdseg_cb.seg_buf[COM2] |= BIT(SEG12);
        if(disp_data&SEG_G) lcdseg_cb.seg_buf[COM2] |= BIT(SEG13);
    }
    else if(disp_pos == 7)
    {
        if(disp_data&SEG_A) lcdseg_cb.seg_buf[COM1] |= BIT(SEG16);
        if(disp_data&SEG_B) lcdseg_cb.seg_buf[COM1] |= BIT(SEG1);
        if(disp_data&SEG_C) lcdseg_cb.seg_buf[COM1] |= BIT(SEG2);
        if(disp_data&SEG_D) lcdseg_cb.seg_buf[COM1] |= BIT(SEG3);
        if(disp_data&SEG_E) lcdseg_cb.seg_buf[COM2] |= BIT(SEG3);
        if(disp_data&SEG_F) lcdseg_cb.seg_buf[COM2] |= BIT(SEG1);
        if(disp_data&SEG_G) lcdseg_cb.seg_buf[COM2] |= BIT(SEG2);
    }
}

void lcdseg_disp_buff_hide(u16 disp_data,u8 disp_pos)
{
    if(disp_pos <= 5)
    {
        disp_pos--;
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG2 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG3 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG3 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG2 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG1 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG1 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG2 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG2 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG1 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG3 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG1 + 3 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG3 + 3 * disp_pos);
    }
    else if(disp_pos == 6)
    {
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG15);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG12);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG13);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG14);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG14);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG12);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG13);
    }
    else if(disp_pos == 7)
    {
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG16);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG1);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG2);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG3);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG3);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG1);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG2);
    }
}
#elif 0
void lcdseg_disp_buff(u16 disp_data,u8 disp_pos)    //disp_pos最小为1
{
    if(disp_pos <= 2)
    {
        disp_pos--;
        if(disp_data&SEG_A) lcdseg_cb.seg_buf[COM6] |= BIT(SEG2 + 2 * disp_pos);
        if(disp_data&SEG_B) lcdseg_cb.seg_buf[COM6] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_C) lcdseg_cb.seg_buf[COM1] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_D) lcdseg_cb.seg_buf[COM1] |= BIT(SEG2 + 2 * disp_pos);
        if(disp_data&SEG_E) lcdseg_cb.seg_buf[COM3] |= BIT(SEG2 + 2 * disp_pos);
        if(disp_data&SEG_F) lcdseg_cb.seg_buf[COM4] |= BIT(SEG2 + 2 * disp_pos);
        if(disp_data&SEG_G) lcdseg_cb.seg_buf[COM3] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_H) lcdseg_cb.seg_buf[COM4] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_I) lcdseg_cb.seg_buf[COM5] |= BIT(SEG2 + 2 * disp_pos);
        if(disp_data&SEG_J) lcdseg_cb.seg_buf[COM2] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_L) lcdseg_cb.seg_buf[COM2] |= BIT(SEG2 + 2 * disp_pos);
        if(disp_data&SEG_M) lcdseg_cb.seg_buf[COM5] |= BIT(SEG3 + 2 * disp_pos);
    }
    else if(disp_pos <= 4)
    {
        disp_pos--;
        if(disp_data&SEG_A) lcdseg_cb.seg_buf[COM6] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_B) lcdseg_cb.seg_buf[COM6] |= BIT(SEG4 + 2 * disp_pos);
        if(disp_data&SEG_C) lcdseg_cb.seg_buf[COM1] |= BIT(SEG4 + 2 * disp_pos);
        if(disp_data&SEG_D) lcdseg_cb.seg_buf[COM1] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_E) lcdseg_cb.seg_buf[COM3] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_F) lcdseg_cb.seg_buf[COM4] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_G) lcdseg_cb.seg_buf[COM3] |= BIT(SEG4 + 2 * disp_pos);
        if(disp_data&SEG_H) lcdseg_cb.seg_buf[COM4] |= BIT(SEG4 + 2 * disp_pos);
        if(disp_data&SEG_I) lcdseg_cb.seg_buf[COM5] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_J) lcdseg_cb.seg_buf[COM2] |= BIT(SEG4 + 2 * disp_pos);
        if(disp_data&SEG_L) lcdseg_cb.seg_buf[COM2] |= BIT(SEG3 + 2 * disp_pos);
        if(disp_data&SEG_M) lcdseg_cb.seg_buf[COM5] |= BIT(SEG4 + 2 * disp_pos);
    }
}

void lcdseg_disp_buff_hide(u16 disp_data,u8 disp_pos)
{
    if(disp_pos <= 2)
    {
        disp_pos--;
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG2 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG2 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG2 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG2 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG2 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG2 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG3 + 2 * disp_pos);
    }
    else if(disp_pos <= 4)
    {
        disp_pos--;
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM6] &= ~BIT(SEG4 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG4 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM1] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM3] &= ~BIT(SEG4 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM4] &= ~BIT(SEG4 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG4 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM2] &= ~BIT(SEG3 + 2 * disp_pos);
        lcdseg_cb.seg_buf[COM5] &= ~BIT(SEG4 + 2 * disp_pos);
    }
}
#elif USER_UI_1621LCD_ENABLE
void lcdseg_disp_buff(u16 disp_data,u8 disp_pos)    //disp_pos最小为1
{
    if(disp_pos == 1||disp_pos == 3||disp_pos == 5)
    {///1,4,7
        //disp_pos--;
        if(disp_pos == 1) disp_pos = 0;
        else if(disp_pos == 3) disp_pos = 1;
        else if(disp_pos == 5) disp_pos = 2;
        if(disp_data&SEG_A) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(0);
        if(disp_data&SEG_B) lcd1621_sendbuf[3 * disp_pos + 2] |= BIT(4);
        if(disp_data&SEG_C) lcd1621_sendbuf[3 * disp_pos + 2] |= BIT(6);
        if(disp_data&SEG_D) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(3);
        if(disp_data&SEG_E) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(6);
        if(disp_data&SEG_F) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(4);
        if(disp_data&SEG_G) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(2);
        if(disp_data&SEG_H) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(1);
        if(disp_data&SEG_I) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(5);
        if(disp_data&SEG_J) lcd1621_sendbuf[3 * disp_pos + 2] |= BIT(7);
        if(disp_data&SEG_L) lcd1621_sendbuf[3 * disp_pos + 1] |= BIT(7);
        if(disp_data&SEG_M) lcd1621_sendbuf[3 * disp_pos + 2] |= BIT(5);
    }
    else if(disp_pos == 2||disp_pos == 4)
    {///2,5
        //disp_pos--;
        if(disp_pos == 2) disp_pos = 1;
        else if(disp_pos == 4) disp_pos = 2;
        if(disp_data&SEG_A) lcd1621_sendbuf[3 * disp_pos]     |= BIT(4);
        if(disp_data&SEG_B) lcd1621_sendbuf[3 * disp_pos]     |= BIT(0);
        if(disp_data&SEG_C) lcd1621_sendbuf[3 * disp_pos]     |= BIT(2);
        if(disp_data&SEG_D) lcd1621_sendbuf[3 * disp_pos]     |= BIT(7);
        if(disp_data&SEG_E) lcd1621_sendbuf[3 * disp_pos - 1] |= BIT(2);
        if(disp_data&SEG_F) lcd1621_sendbuf[3 * disp_pos - 1] |= BIT(0);
        if(disp_data&SEG_G) lcd1621_sendbuf[3 * disp_pos]     |= BIT(6);
        if(disp_data&SEG_H) lcd1621_sendbuf[3 * disp_pos]     |= BIT(5);
        if(disp_data&SEG_I) lcd1621_sendbuf[3 * disp_pos - 1] |= BIT(1);
        if(disp_data&SEG_J) lcd1621_sendbuf[3 * disp_pos]     |= BIT(3);
        if(disp_data&SEG_L) lcd1621_sendbuf[3 * disp_pos - 1] |= BIT(3);
        if(disp_data&SEG_M) lcd1621_sendbuf[3 * disp_pos]     |= BIT(1);
    }
    else if(disp_pos == 6)
    {
        if(disp_data&SEG_A) lcd1621_sendbuf[8] |= BIT(3);
        if(disp_data&SEG_B) lcd1621_sendbuf[8] |= BIT(2);
        if(disp_data&SEG_C) lcd1621_sendbuf[8] |= BIT(1);
        if(disp_data&SEG_D) lcd1621_sendbuf[8] |= BIT(0);
        if(disp_data&SEG_E) lcd1621_sendbuf[9] |= BIT(7);
        if(disp_data&SEG_F) lcd1621_sendbuf[9] |= BIT(6);
        if(disp_data&SEG_G) lcd1621_sendbuf[9] |= BIT(5);
    }
    else if(disp_pos == 7)
    {
        if(disp_data&SEG_A) lcd1621_sendbuf[9] |= BIT(3);
        if(disp_data&SEG_B) lcd1621_sendbuf[9] |= BIT(2);
        if(disp_data&SEG_C) lcd1621_sendbuf[9] |= BIT(1);
        if(disp_data&SEG_D) lcd1621_sendbuf[9] |= BIT(0);
        if(disp_data&SEG_E) lcd1621_sendbuf[10] |= BIT(7);
        if(disp_data&SEG_F) lcd1621_sendbuf[10] |= BIT(6);
        if(disp_data&SEG_G) lcd1621_sendbuf[10] |= BIT(5);
    }
}

void lcdseg_disp_buff_hide(u16 disp_data,u8 disp_pos)
{
    if(disp_pos == 1||disp_pos == 3||disp_pos == 5)
    {///1,4,7
        disp_pos--;
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(0);
        lcd1621_sendbuf[3 * disp_pos + 2] &=~BIT(4);
        lcd1621_sendbuf[3 * disp_pos + 2] &=~BIT(6);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(3);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(6);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(4);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(2);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(1);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(5);
        lcd1621_sendbuf[3 * disp_pos + 2] &=~BIT(3);
        lcd1621_sendbuf[3 * disp_pos + 1] &=~BIT(7);
        lcd1621_sendbuf[3 * disp_pos + 2] &=~BIT(1);
    }
    else if(disp_pos == 2||disp_pos == 4)
    {///2,5
        disp_pos--;
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(4);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(0);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(2);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(7);
        lcd1621_sendbuf[3 * disp_pos - 1] &=~BIT(2);
        lcd1621_sendbuf[3 * disp_pos - 1] &=~BIT(0);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(6);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(5);
        lcd1621_sendbuf[3 * disp_pos - 1] &=~BIT(1);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(3);
        lcd1621_sendbuf[3 * disp_pos - 1] &=~BIT(3);
        lcd1621_sendbuf[3 * disp_pos]     &=~BIT(1);
    }
    else if(disp_pos == 6)
    {
        lcd1621_sendbuf[8] &=~BIT(3);
        lcd1621_sendbuf[8] &=~BIT(2);
        lcd1621_sendbuf[8] &=~BIT(1);
        lcd1621_sendbuf[8] &=~BIT(0);
        lcd1621_sendbuf[9] &=~BIT(7);
        lcd1621_sendbuf[9] &=~BIT(6);
        lcd1621_sendbuf[9] &=~BIT(5);
    }
    else if(disp_pos == 7)
    {
        lcd1621_sendbuf[9] &=~BIT(3);
        lcd1621_sendbuf[9] &=~BIT(2);
        lcd1621_sendbuf[9] &=~BIT(1);
        lcd1621_sendbuf[9] &=~BIT(0);
        lcd1621_sendbuf[10] &=~BIT(7);
        lcd1621_sendbuf[10] &=~BIT(6);
        lcd1621_sendbuf[10] &=~BIT(5);
    }
}

void lcd1621_icon_update(void)
{
    if(lcdseg_cb.icon_buf[0] & LCDSEG_2POINT)   lcd1621_sendbuf[9] |= BIT(4);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_DOT)      lcd1621_sendbuf[10] |= BIT(4);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_USB)      lcd1621_sendbuf[0] |= BIT(0);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_SD)       lcd1621_sendbuf[0] |= BIT(1);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_MID_)     lcd1621_sendbuf[0] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_MP3)      lcd1621_sendbuf[11] |= BIT(6);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_INT)      lcd1621_sendbuf[10] |= BIT(1);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_RPT)      lcd1621_sendbuf[10] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_RDM)      lcd1621_sendbuf[10] |= BIT(3);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_EQ)       lcd1621_sendbuf[11] |= BIT(3);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_POP)      lcd1621_sendbuf[11] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_ROCK)     lcd1621_sendbuf[11] |= BIT(0);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_CLASS)    lcd1621_sendbuf[11] |= BIT(1);
//    if(lcdseg_cb.icon_buf[0] & LCDSEG_BT)       lcd1621_sendbuf[COM2] |= BIT(7);
//    if(lcdseg_cb.icon_buf[0] & LCDSEG_FM)       lcd1621_sendbuf[COM5] |= BIT(12);

    if(lcdseg_cb.flash_500ms)
    {
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT) lcd1621_sendbuf[9] |= BIT(4);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)    lcd1621_sendbuf[10] |= BIT(4);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)    lcd1621_sendbuf[0] |= BIT(0);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)     lcd1621_sendbuf[0] |= BIT(1);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)   lcd1621_sendbuf[0] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MP3)    lcd1621_sendbuf[11] |= BIT(6);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_INT)    lcd1621_sendbuf[10] |= BIT(1);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RPT)    lcd1621_sendbuf[10] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RDM)    lcd1621_sendbuf[10] |= BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_EQ)     lcd1621_sendbuf[11] |= BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_POP)    lcd1621_sendbuf[11] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_ROCK)   lcd1621_sendbuf[11] |= BIT(0);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_CLASS)  lcd1621_sendbuf[11] |= BIT(1);
//        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_BT)     lcd1621_sendbuf[COM2] |= BIT(7);
//        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_FM)     lcd1621_sendbuf[COM5] |= BIT(12);
    }
}

void lcd1621_flash_icon(void)
{
    static u8 flash_show_hide = 0;
    u8 bit_count = 0;
    if(lcdseg_cb.flash_500ms != flash_show_hide)
    {
        flash_show_hide = lcdseg_cb.flash_500ms;
    }
    else
    {
        return;
    }
    if(lcdseg_cb.flash_500ms || lcdseg_cb.ignore_next_flash)
    {
        while(lcdseg_cb.flash_char_bit>>bit_count)
        {
            if(lcdseg_cb.flash_char_bit & BIT(bit_count))
            {
                lcdseg_disp_buff(lcdseg_cb.screen_show_buf[bit_count], bit_count+1);
            }
            bit_count++;
        }
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT)   lcd1621_sendbuf[9] |= BIT(4);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)      lcd1621_sendbuf[10] |= BIT(4);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)      lcd1621_sendbuf[0] |= BIT(0);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)       lcd1621_sendbuf[0] |= BIT(1);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)     lcd1621_sendbuf[0] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MP3)      lcd1621_sendbuf[11] |= BIT(6);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_INT)      lcd1621_sendbuf[10] |= BIT(1);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RPT)      lcd1621_sendbuf[10] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RDM)      lcd1621_sendbuf[10] |= BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_EQ)       lcd1621_sendbuf[11] |= BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_POP)      lcd1621_sendbuf[11] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_ROCK)     lcd1621_sendbuf[11] |= BIT(0);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_CLASS)    lcd1621_sendbuf[11] |= BIT(1);
    }
    else
    {
        while(lcdseg_cb.flash_char_bit>>bit_count)
        {
            if(lcdseg_cb.flash_char_bit & BIT(bit_count))
            {
                lcdseg_disp_buff_hide(lcdseg_cb.screen_show_buf[bit_count], bit_count+1);
            }
            bit_count++;
        }
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT)   lcd1621_sendbuf[9] &= ~BIT(4);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)      lcd1621_sendbuf[10] &= ~BIT(4);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)      lcd1621_sendbuf[0] &= ~BIT(0);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)       lcd1621_sendbuf[0] &= ~BIT(1);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)     lcd1621_sendbuf[0] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MP3)      lcd1621_sendbuf[11] &= ~BIT(6);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_INT)      lcd1621_sendbuf[10] &= ~BIT(1);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RPT)      lcd1621_sendbuf[10] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RDM)      lcd1621_sendbuf[10] &= ~BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_EQ)       lcd1621_sendbuf[11] &= ~BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_POP)      lcd1621_sendbuf[11] &= ~BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_ROCK)     lcd1621_sendbuf[11] &= ~BIT(0);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_CLASS)    lcd1621_sendbuf[11] &= ~BIT(1);
    }
    lcdseg_cb.ignore_next_flash = 0;
}

#endif

void lcdseg_buff2pin(void)
{
    u8 com = 0;
    for(;com<6;com++)
    {
        lcdseg_cb.pin_A[com] = 0;
#if 0 //17P lcd
        lcdseg_cb.pin_B[com] = 0;
        lcdseg_cb.pin_C[com] = 0;
#endif

        if(lcdseg_cb.seg_buf[com] & BIT(0))     lcdseg_cb.pin_A[com] |= BIT(12);
        if(lcdseg_cb.seg_buf[com] & BIT(1))     lcdseg_cb.pin_A[com] |= BIT(11);
        if(lcdseg_cb.seg_buf[com] & BIT(2))     lcdseg_cb.pin_A[com] |= BIT(10);
        if(lcdseg_cb.seg_buf[com] & BIT(3))     lcdseg_cb.pin_A[com] |= BIT(9);
        if(lcdseg_cb.seg_buf[com] & BIT(4))     lcdseg_cb.pin_A[com] |= BIT(8);
        if(lcdseg_cb.seg_buf[com] & BIT(5))     lcdseg_cb.pin_A[com] |= BIT(7);
        if(lcdseg_cb.seg_buf[com] & BIT(6))     lcdseg_cb.pin_A[com] |= BIT(6);
        if(lcdseg_cb.seg_buf[com] & BIT(7))     lcdseg_cb.pin_A[com] |= BIT(5);
        if(lcdseg_cb.seg_buf[com] & BIT(8))     lcdseg_cb.pin_A[com] |= BIT(4);
        if(lcdseg_cb.seg_buf[com] & BIT(9))     lcdseg_cb.pin_A[com] |= BIT(3);
        if(lcdseg_cb.seg_buf[com] & BIT(10))    lcdseg_cb.pin_A[com] |= BIT(2);
#if 0 //17P lcd
        if(lcdseg_cb.seg_buf[com] & BIT(11))    lcdseg_cb.pin_A[com] |= BIT(1);
        if(lcdseg_cb.seg_buf[com] & BIT(12))    lcdseg_cb.pin_A[com] |= BIT(0);
        if(lcdseg_cb.seg_buf[com] & BIT(13))    lcdseg_cb.pin_C[com] |= BIT(7);
        if(lcdseg_cb.seg_buf[com] & BIT(14))    lcdseg_cb.pin_B[com] |= BIT(5);
        if(lcdseg_cb.seg_buf[com] & BIT(15))    lcdseg_cb.pin_B[com] |= BIT(4);
#endif
        //lcdseg_debug("pinA[%d] = 0x%04x\n", com, lcdseg_cb.pin_A[com]);
    }

    if(lcdseg_cb.icon_buf[0] & LCDSEG_2POINT)   lcdseg_cb.pin_A[COM3] |= BIT(7);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_DOT)      lcdseg_cb.pin_A[COM1] |= BIT(7);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_USB)      lcdseg_cb.pin_A[COM3] |= BIT(12);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_SD)       lcdseg_cb.pin_A[COM4] |= BIT(12);
    //if(lcdseg_cb.icon_buf[0] & LCDSEG_MID_)   lcdseg_cb.pin_A[COM2] |= BIT(3);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_MP3)      lcdseg_cb.pin_A[COM6] |= BIT(12);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_INT)      lcdseg_cb.pin_A[COM6] |= BIT(7);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_RPT)      lcdseg_cb.pin_A[COM5] |= BIT(7);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_RDM)      lcdseg_cb.pin_A[COM4] |= BIT(7);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_EQ)       lcdseg_cb.pin_A[COM6] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_POP)      lcdseg_cb.pin_A[COM5] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_ROCK)     lcdseg_cb.pin_A[COM4] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_CLASS)    lcdseg_cb.pin_A[COM3] |= BIT(2);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_BT)       lcdseg_cb.pin_A[COM2] |= BIT(7);
    if(lcdseg_cb.icon_buf[0] & LCDSEG_FM)       lcdseg_cb.pin_A[COM5] |= BIT(12);

    if(lcdseg_cb.flash_500ms)
    {
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_2POINT) lcdseg_cb.pin_A[COM3] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_DOT)    lcdseg_cb.pin_A[COM1] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_USB)    lcdseg_cb.pin_A[COM3] |= BIT(12);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_SD)     lcdseg_cb.pin_A[COM4] |= BIT(12);
        //if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MID_)   lcdseg_cb.pin_A[COM2] |= BIT(3);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_MP3)    lcdseg_cb.pin_A[COM6] |= BIT(12);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_INT)    lcdseg_cb.pin_A[COM6] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RPT)    lcdseg_cb.pin_A[COM5] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_RDM)    lcdseg_cb.pin_A[COM4] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_EQ)     lcdseg_cb.pin_A[COM6] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_POP)    lcdseg_cb.pin_A[COM5] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_ROCK)   lcdseg_cb.pin_A[COM4] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_CLASS)  lcdseg_cb.pin_A[COM3] |= BIT(2);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_BT)     lcdseg_cb.pin_A[COM2] |= BIT(7);
        if(lcdseg_cb.flash_icon_buf[0] & LCDSEG_FM)     lcdseg_cb.pin_A[COM5] |= BIT(12);
    }
}

#endif
