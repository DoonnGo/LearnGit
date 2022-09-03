#include "includes.h"
#include "ui/ui_api.h"
#include "fm_emitter/fm_emitter_manage.h"
#include "app_main.h"
#include "application/audio_eq.h"
#include "rtc/rtc_ui.h"

#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_USER_LCDSEG))
static void lcdseg_show_hi(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_GX
    dis->show_string((u8 *)" HELLO");
#else
    dis->show_string((u8 *)" HI");
#endif
    dis->lock(0);
}

static void lcdseg_show_wait(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    dis->show_string((u8 *)"LOAD");
#else
    dis->show_string((u8 *)"  LOAD");
#endif
    dis->lock(0);
}

static void lcdseg_show_bt(void *hd)
{

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string((u8 *)" bt");
    dis->lock(0);
}

#if TCFG_APP_FM_EMITTER_EN
static void lcdseg_fm_ir_set_freq(void *hd, u16 freq)
{

    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[5] = {0};	  ///<换算结果显示缓存
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    sprintf((char *)bcd_number, "%4d", freq);
    /* itoa4(freq,bcd_number); */
    if (freq > 1080) {
        dis->show_string((u8 *)" Err");
    } else if (freq >= 875) {
        dis->show_string(bcd_number);
        /* os_time_dly(100); */
        fm_emitter_manage_set_fre(freq);
        UI_REFLASH_WINDOW(TRUE);//设置回主页
    } else {
        dis->FlashChar(BIT(0) | BIT(1) | BIT(2) | BIT(3)); //设置闪烁
        dis->show_string(bcd_number);
    }
    dis->lock(0);

}
#endif


static void lcdseg_fm_set_freq(void *hd, u32 arg)
{
    u8 bcd_number[5] = {0};
    LCD_API *dis = (LCD_API *)hd;
    u16 freq = 0;
    freq = arg;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->FlashChar(BIT(0) | BIT(1) | BIT(2) | BIT(3)); //设置闪烁
    itoa4(freq, (u8 *)bcd_number);
    if (freq > 999 && freq <= 1999) {
        bcd_number[0] = '1';
    } else {
        bcd_number[0] = ' ';
    }
    dis->show_string(bcd_number);
    dis->show_icon(LCDSEG_DOT);
    dis->lock(0);
}

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void lcdseg_show_volume(void *hd, u8 vol)
{
    u8 bcd_number[3] = {0};
    sprintf((char *)bcd_number, "%2d", vol);
    if(vol < 10)
        bcd_number[0] = '0';

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_char('V');
#if CONFIG_CLIENT_DG
    dis->setXY(2, 0);
#else
    dis->show_char('O');
    dis->show_char('L');
    dis->setXY(4, 0);
#endif
    dis->show_string(bcd_number);
//    dis->show_number(vol / 10);
//    dis->show_number(vol % 10);
    dis->lock(0);
}

static void lcdseg_show_bass(void *hd, s32 bass)
{
    u8 bcd_number[7] = {0};
    u8 bass_vol;
    if(bass < 0)
        bass_vol = 0 - bass;
    else
        bass_vol = bass;

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "BS+%1d", bass_vol);
    if(bass < 0)
        bcd_number[2] = '-';
    else if(bass == 0)
        bcd_number[2] = ' ';
#else
    sprintf((char *)bcd_number, "BAS +%1d", bass_vol);
    if(bass < 0)
        bcd_number[4] = '-';
    else if(bass == 0)
        bcd_number[4] = ' ';
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_treble(void *hd, s32 treble)
{
    u8 bcd_number[7] = {0};
    u8 treble_vol;
    if(treble < 0)
        treble_vol = 0 - treble;
    else
        treble_vol = treble;

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "TR+%1d", treble_vol);
    if(treble < 0)
        bcd_number[2] = '-';
    else if(treble == 0)
        bcd_number[2] = ' ';
#else
    sprintf((char *)bcd_number, "TRE +%1d", treble_vol);
    if(treble < 0)
        bcd_number[4] = '-';
    else if(treble == 0)
        bcd_number[4] = ' ';
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_eq_freq(void *hd, s32 gain, section)
{
    u8 bcd_number[7] = {0};
    u8 show_gain;
    if(gain < 0)
        show_gain = 0 - gain;
    else
        show_gain = gain;

    const char *eq_freq[7] = {"60", "330", "630", "1K", "3K", "6K", "10K"};

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);

    sprintf((char *)bcd_number, "%-0.3s +%1d", show_gain);
    if(gain < 0)
        bcd_number[4] = '-';
    else if(gain == 0)
        bcd_number[4] = ' ';

    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_bal(void *hd, s32 bal)
{
    u8 bcd_number[7] = {0};
    u8 bal_vol;
    if(bal < 0)
        bal_vol = 0 - bal;
    else
        bal_vol = bal;

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "BLR%1d", bal_vol);
    if(bal < 0)
        bcd_number[2] = 'L';
    else if(bal == 0)
        bcd_number[2] = ' ';
#else
    sprintf((char *)bcd_number, "BAL R%1d", bal_vol);
    if(bal < 0)
        bcd_number[4] = 'L';
    else if(bal == 0)
        bcd_number[4] = '0';
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_fad(void *hd, s32 fad)
{
    u8 bcd_number[7] = {0};
    u8 fad_vol;
    if(fad < 0)
        fad_vol = 0 - fad;
    else
        fad_vol = fad;

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "FAF%1d", fad_vol);
    if(fad < 0)
        bcd_number[2] = 'R';
    else if(fad == 0)
        bcd_number[2] = ' ';
#else
    sprintf((char *)bcd_number, "FAD F%1d", fad_vol);
    if(fad < 0)
        bcd_number[4] = 'R';
    else if(fad == 0)
        bcd_number[4] = '0';
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_loud(void *hd, s32 loud_flag)
{
    u8 bcd_number[7] = {0};

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "LDOF");
    if(loud_flag > 0)
    {
        bcd_number[3] = 'N';
    }
#else
    sprintf((char *)bcd_number, "LD OFF");
    if(loud_flag > 0)
    {
        bcd_number[4] = 'N';
        bcd_number[5] = 0;
    }
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

const char *ifx_char_tbl[6] = {
    "IFXOFF",   //IFX OFF
    "FULLSH",   //FULLSHOT
    "MBASS",    //MEGABASS
    "VOCAL",    //VOCAL
    "VIRTUA",   //VIRTUAL
    "3DROTA",   //3DROTARY
};
static void lcdseg_show_ifx(void *hd, s32 ifx_mode)
{
    u8 bcd_number[7] = {0};

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    sprintf((char *)bcd_number, "%s", ifx_char_tbl[ifx_mode]);

    dis->show_string(bcd_number);
    dis->lock(0);
}

///2020-11-12 顺序在audio_eq.h里设置
#if CONFIG_CLIENT_DG
const char *eq_char_tbl[EQ_MODE_MAX] = {
    [EQ_MODE_NORMAL]  = "EQOF",
    [EQ_MODE_POP]     = "POP",
    [EQ_MODE_ROCK]    = "ROCK",
    [EQ_MODE_JAZZ]    = "JAZZ",
    [EQ_MODE_CLASSIC] = "CLAS",
    [EQ_MODE_COUNTRY] = "COUN",
    [EQ_MODE_CUSTOM]  = "EQOF"
};
#else
const char *eq_char_tbl[EQ_MODE_MAX] = {
    [EQ_MODE_NORMAL]  = "EQOFF",
    [EQ_MODE_POP]     = " POP",
    [EQ_MODE_ROCK]    = "ROCK",
    [EQ_MODE_JAZZ]    = "JAZZ",
    [EQ_MODE_CLASSIC] = "CLASS",
    [EQ_MODE_COUNTRY] = "COUNT",
    [EQ_MODE_CUSTOM]  = "EQOFF"
};
#endif
static void lcdseg_show_eq(void *hd, s32 eq_mode)
{
    u8 bcd_number[7] = {0};

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    sprintf((char *)bcd_number, "%s", eq_char_tbl[eq_mode]);

    dis->show_string(bcd_number);
    dis->lock(0);
}


struct rtc_ui_opr {
    void *dev_handle;
    struct ui_rtc_display ui_rtc;
};
static struct rtc_ui_opr *ui_opr = NULL;
struct sys_time sys_current_time = {0};

static void User_rtc_load_save(u8 save)
{
    //struct sys_time sys_current_time = {0};

    if (!ui_opr) {
        ui_opr =  zalloc(sizeof(struct rtc_ui_opr));
    }
    ui_opr->dev_handle = dev_open("rtc", NULL);
    if (!ui_opr->dev_handle) {
        free(ui_opr);
        ui_opr = NULL;
    }
    if(ui_opr){
        if(save)
        {
            dev_ioctl(ui_opr->dev_handle, IOCTL_SET_SYS_TIME, (u32)&sys_current_time);
        }
        dev_ioctl(ui_opr->dev_handle, IOCTL_GET_SYS_TIME, (u32)&sys_current_time);
        free(ui_opr);
        ui_opr = NULL;
    }
}

static void lcdseg_show_time(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;

    User_rtc_load_save(0);

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);

    u8 time_number[7] = {0};
#if CONFIG_CLIENT_DG
    sprintf((char *)time_number, "%02d%02d", sys_current_time.hour, sys_current_time.min);
#else
    sprintf((char *)time_number, "  %02d%02d", sys_current_time.hour, sys_current_time.min);
#endif
    dis->show_string(time_number);

    dis->flash_icon(LCDSEG_2POINT);

    dis->lock(0);
}

static void lcdseg_show_set_time(void *hd, u8 setting_flag, u8 dir)
{
    LCD_API *dis = (LCD_API *)hd;

    if(setting_flag == 0)
    {
        if(dir == 1)
        {
            sys_current_time.hour++;
            if(sys_current_time.hour >= 24)
                sys_current_time.hour = 0;
        }
        else if(dir == 2)
        {
            if(sys_current_time.hour == 0)
                sys_current_time.hour = 23;
            else
                sys_current_time.hour--;
        }
    }
    else
    {
        if(dir == 1)
        {
            sys_current_time.min++;
            if(sys_current_time.min >= 60)
                sys_current_time.min = 0;
        }
        else if(dir == 2)
        {
            if(sys_current_time.min == 0)
                sys_current_time.min = 59;
            else
                sys_current_time.min--;
        }
    }

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);

    u8 time_number[7] = {0};
#if CONFIG_CLIENT_DG
    sprintf((char *)time_number, "%02d%02d", sys_current_time.hour, sys_current_time.min);
    dis->show_string(time_number);
    if(setting_flag == 0)
    {
        dis->FlashChar(0);
        dis->FlashChar(1);
    }
    else if(setting_flag == 1)
    {
        dis->FlashChar(2);
        dis->FlashChar(3);
    }
#else
    sprintf((char *)time_number, "  %02d%02d", sys_current_time.hour, sys_current_time.min);
    dis->show_string(time_number);
    if(setting_flag == 0)
    {
        dis->FlashChar(2);
        dis->FlashChar(3);
    }
    else if(setting_flag == 1)
    {
        dis->FlashChar(4);
        dis->FlashChar(5);
    }
#endif

    dis->flash_icon(LCDSEG_2POINT);
    dis->ignore_next_flash();

    dis->lock(0);
}

void User_rtc_timeout(void)
{
    printf("rtc timeout save\n");

    User_rtc_load_save(1);
}

///MUTE 显示
void lcdseg_show_mute(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[7] = {0};

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "MUTE");
#else
    sprintf((char *)bcd_number, " MUTE");
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

void lcdseg_show_information(void *hd, u32 arg)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[7] = {0};

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    bcd_number[0] = (arg >> 24) & 0xFF;
    bcd_number[1] = (arg >> 16) & 0xFF;
    bcd_number[2] = (arg >> 8) & 0xFF;
    bcd_number[3] = (arg) & 0xFF;
    dis->show_string(bcd_number);
    dis->lock(0);
}

void ui_common(void *hd, void *private, u8 menu, u32 arg)//公共显示
{

    u16 fre = 0;

    if (!hd) {
        return;
    }

    switch (menu) {
    case MENU_POWER_UP:
        lcdseg_show_hi(hd);
        break;
    case MENU_MAIN_VOL:
        lcdseg_show_volume(hd, arg & 0xff);
        break;

    case MENU_BASS:
        lcdseg_show_bass(hd, arg);
        break;
    case MENU_TREBLE:
        lcdseg_show_treble(hd, arg);
        break;
    case MENU_EQ_FREQ_1:
    case MENU_EQ_FREQ_2:
    case MENU_EQ_FREQ_3:
    case MENU_EQ_FREQ_4:
    case MENU_EQ_FREQ_5:
    case MENU_EQ_FREQ_6:
    case MENU_EQ_FREQ_7:
        lcdseg_show_eq_freq(hd, arg, menu-MENU_EQ_FREQ_1);
        break;
    case MENU_BAL:
        lcdseg_show_bal(hd, arg);
        break;
    case MENU_FAD:
        lcdseg_show_fad(hd, arg);
        break;
    case MENU_LOUD:
        lcdseg_show_loud(hd, arg);
        break;
    case MENU_IFX:
        lcdseg_show_ifx(hd, arg);
        break;
    case MENU_SET_EQ:
        lcdseg_show_eq(hd, arg);
        break;
    case MENU_RTC_SHOW:
        lcdseg_show_time(hd);
        break;
    case MENU_RTC_SET_HOUR:
    case MENU_RTC_SET_MIN:
        lcdseg_show_set_time(hd, menu - MENU_RTC_SET_HOUR, arg);
        break;
    case MENU_USER_INFO:
        lcdseg_show_information(hd, arg);
        break;

    case MENU_WAIT:
        lcdseg_show_wait(hd);
        break;
    case MENU_BT:
        lcdseg_show_bt(hd);
        break;
    case MENU_IR_FM_SET_FRE:
#if TCFG_APP_FM_EMITTER_EN
        lcdseg_fm_ir_set_freq(hd, arg);
#endif
        break;
    case MENU_FM_SET_FRE:

#if TCFG_APP_FM_EMITTER_EN
        fre = fm_emitter_manage_get_fre();
        lcdseg_fm_set_freq(hd, arg);
#endif
        break;
    default:
        break;
    }

    extern void lcdseg_show_icon_online_dev(void *hd);
    lcdseg_show_icon_online_dev(hd);
}

#endif
