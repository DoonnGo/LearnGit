#include "ui/ui_api.h"
#include "fm/fm_manage.h"

#if TCFG_APP_FM_EN
#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_USER_LCDSEG))
void ui_fm_temp_finsh(u8 menu)//子菜单被打断或者显示超时
{
    switch (menu) {
    default:
        break;
    }
}

static void led7_show_fm(void *hd)
{


    LCD_API *dis = (LCD_API *)hd;

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string((u8 *)" FM");
    dis->lock(0);
}


static void lcdseg_fm_show_freq(void *hd, u32 arg, u16 channel)
{
    u8 bcd_number[7] = {0};
    LCD_API *dis = (LCD_API *)hd;
    u16 freq = arg;

    if (freq > 7600 && freq <= 1080 * 10) {
        freq = freq / 10;
    }
    ///show_channel
    u8 show_channel = channel >> 7;
    channel &= 0x7F;
    ///band
    u8 band = (channel + 5) / 6;
    ///seg
    u8 seg = channel;
    if(band > 1)
    {
        seg -= (band - 1) * 6;
    }


    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);

#if CONFIG_CLIENT_DG
    itoa4(freq, (u8 *)bcd_number);
    if (freq > 999 && freq <= 9999) {
        bcd_number[0] = freq / 1000 % 10 + '0';
    } else {
        bcd_number[0] = ' ';
    }
#else
    sprintf((char *)bcd_number, "F%01d%04d%01d", band, freq, seg);
    if(bcd_number[2] == '0')
        bcd_number[2] = ' ';
    if(channel == 0)
    {
        bcd_number[0] = ' ';
        bcd_number[1] = ' ';
        bcd_number[6] = ' ';
    }
    if(show_channel)
    {
        bcd_number[6] = ' ';
    }
#endif

    dis->show_string(bcd_number);
    dis->show_icon(LCDSEG_DOT);
    dis->show_icon(LCDSEG_FM);
    dis->lock(0);
}

static void lcdseg_fm_show_station(void *hd, u32 arg, u16 freq)
{
    u8 bcd_number[7] = {0};
    LCD_API *dis = (LCD_API *)hd;

    if (freq > 7600 && freq <= 1080 * 10) {
        freq = freq / 10;
    }
    ///show_channel
    u8 show_channel = arg >> 7;
    arg &= 0x7F;
    ///band
    u8 band = (arg + 5) / 6;
    ///seg
    u8 seg = arg;
    if(band > 1)
    {
        seg -= (band - 1) * 6;
    }

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    if(show_channel)
        sprintf((char *)bcd_number, "%01d P0", band);
    else
        sprintf((char *)bcd_number, "%01d P%01d", band, seg);
#else
    sprintf((char *)bcd_number, "F%01d%04d%01d", band, freq, seg);
    if(bcd_number[2] == '0')
        bcd_number[2] = ' ';
    if(show_channel)
    {
        bcd_number[6] = ' ';
    }
#endif
    dis->show_string(bcd_number);
#if !CONFIG_CLIENT_DG
    dis->show_icon(LCDSEG_DOT);
#endif
    dis->show_icon(LCDSEG_FM);
    dis->lock(0);
}

static void lcdseg_fm_show_step_in(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[7] = {0};

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "STEP");
#else
    sprintf((char *)bcd_number, " STEP");
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_fm_show_step_out(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[7] = {0};

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "AUTO");
#else
    sprintf((char *)bcd_number, " AUTO");
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void *ui_open_fm(void *hd)
{
    ui_set_auto_reflash(1000);//设置主页500ms自动刷新
    return NULL;
}

static void ui_close_fm(void *hd, void *private)
{
    LCD_API *dis = (LCD_API *)hd;
    if (!dis) {
        return ;
    }
    if (private) {
        free(private);
    }
}

static void ui_fm_main(void *hd, void *private) //主界面显示
{
    if (!hd) {
        return;
    }
    extern u8 User_mute_flag;
    if(User_mute_flag)
    {
        extern void lcdseg_show_mute(void *hd);
        lcdseg_show_mute(hd);
    }
    else
    {
        u16 fre = fm_manage_get_fre();
        u16 channel = fm_manage_get_channel();
        if (fre != 0) {
            lcdseg_fm_show_freq(hd, fre, channel);
        }
    }

    extern void lcdseg_show_icon_online_dev(void *hd);
    lcdseg_show_icon_online_dev(hd);
}


static int ui_fm_user(void *hd, void *private, u8 menu, u32 arg)//子界面显示 //返回true不继续传递 ，返回false由common统一处理
{
    int ret = true;
    LCD_API *dis = (LCD_API *)hd;
    if (!dis) {
        return false;
    }

    switch (menu) {
    case MENU_FM_STATION:
        lcdseg_fm_show_station(hd, arg, fm_manage_get_fre());
        break;
    case MENU_FM_STEP_IN:
        lcdseg_fm_show_step_in(hd);
        break;
    case MENU_FM_STEP_OUT:
        lcdseg_fm_show_step_out(hd);
        break;
    default:
        ret = false;
    }

    extern void lcdseg_show_icon_online_dev(void *hd);
    lcdseg_show_icon_online_dev(hd);
    return ret;

}



const struct ui_dis_api fm_main = {
    .ui      = UI_FM_MENU_MAIN,
    .open    = ui_open_fm,
    .ui_main = ui_fm_main,
    .ui_user = ui_fm_user,
    .close   = ui_close_fm,
};


#endif
#endif
