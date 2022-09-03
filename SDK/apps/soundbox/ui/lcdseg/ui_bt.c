#include "includes.h"
#include "ui/ui_api.h"
#include "fm_emitter/fm_emitter_manage.h"
#include "btstack/avctp_user.h"
#include "app_main.h"

#if (TCFG_APP_BT_EN)

#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_USER_LCDSEG))

static void lcdseg_show_bt(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
#if CONFIG_CLIENT_DG
    dis->setXY(0, 0);
    if(get_bt_connect_status() >= BT_STATUS_CONNECTING)
        dis->show_string((u8 *)"BTON");
    else
        dis->show_string((u8 *)" BT");
#else
    dis->setXY(2, 0);
#if 0
    if(dis->flash_500ms() || (get_bt_connect_status() >= BT_STATUS_CONNECTING))
        dis->show_string((u8 *)" BT");
#else
    if(get_bt_connect_status() >= BT_STATUS_CONNECTING)
        dis->show_string((u8 *)"BTON");
    else
        dis->show_string((u8 *)" BT");
#endif
#endif

    dis->lock(0);
}

#if CONFIG_CLIENT_DG
    #define BT_CALL_SHOW_NUM    4   //显示电话号码数字位数
#else
    #define BT_CALL_SHOW_NUM    6   //显示电话号码数字位数
#endif
static void lcdseg_show_phone_num(void *hd, u8 input_flag)
{
    static bool flash_500ms_flag = 0;
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[7] = {0};             ///屏幕显示内容缓存
    u8 phone_num_buf[30] = {0};         ///号码缓存
    u8 *phone_num_select;               ///手机端号码获取/车机端输入号码 显示的选择

    if(input_flag)                      ///0:手机端号码, 1:车机端输入号码
        phone_num_select = bt_user_priv_var.input_phone_num;
    else
        phone_num_select = bt_user_priv_var.income_phone_num;

    memset(phone_num_buf, ' ', BT_CALL_SHOW_NUM-1);
    memcpy(&phone_num_buf[BT_CALL_SHOW_NUM-1], phone_num_select, 30);///将号码放进号码缓存

    sprintf((char *)bcd_number, "%-0.6s", &phone_num_buf[bt_user_priv_var.phone_num_cur]);  ///从号码缓存将该显示的内容放进显示缓存
    if(bcd_number[BT_CALL_SHOW_NUM] > 0)
        bcd_number[BT_CALL_SHOW_NUM] = 0;                           ///修正显示缓存,使其只在允许的位数显示

    if(dis->flash_500ms() != flash_500ms_flag && !input_flag)       ///显示手机端号码时,以500Ms的速度滚动
    {
        flash_500ms_flag = dis->flash_500ms();
        bt_user_priv_var.phone_num_cur++;
        if((bt_user_priv_var.income_phone_len+BT_CALL_SHOW_NUM) <= bt_user_priv_var.phone_num_cur)
            bt_user_priv_var.phone_num_cur = 0;
    }
    else if(input_flag)                                             ///显示车机端输入号码时,每刷新一次子显示函数,数字滚动一位
    {
        bt_user_priv_var.phone_num_cur++;
    }

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_dial_time(void *hd)
{
    u8 time_number[5] = {0};
    struct sys_time time = {0};
    time.min = bt_user_priv_var.dial_time / 60;
    time.sec = bt_user_priv_var.dial_time % 60;

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
#if CONFIG_CLIENT_DG
    dis->setXY(0, 0);
#else
    dis->setXY(2, 0);
#endif
    static bool flash_500ms_flag = 0;
    if(dis->flash_500ms() != flash_500ms_flag)
    {
        flash_500ms_flag = dis->flash_500ms();
        if(flash_500ms_flag == 0)
            bt_user_priv_var.dial_time++;
    }

    sprintf((char *)time_number, "%02d%02d", time.min, time.sec);
    dis->show_string(time_number);

    if(dis->flash_500ms())
        dis->show_icon(LCDSEG_2POINT);

    dis->lock(0);
}

static void lcdseg_show_call(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string((u8 *)" CAL");
    dis->lock(0);
}



static void lcdseg_fm_show_freq(void *hd, void *private, u32 arg)
{
    u8 bcd_number[5] = {0};
    LCD_API *dis = (LCD_API *)hd;
    u16 freq = 0;
    freq = arg;

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
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


static void lcdseg_show_wait(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string((u8 *)" Lod");
    dis->lock(0);
}



static void *ui_open_bt(void *hd)
{
    void *private = NULL;
    ///2020-11-10 改为100Ms刷新用于频谱跳动
    ui_set_auto_reflash(100);//设置主页500ms自动刷新
    return private;
}

static void ui_close_bt(void *hd, void *private)
{
    LCD_API *dis = (LCD_API *)hd;
    if (!dis) {
        return;
    }
    if (private) {
        free(private);
    }
}

static void ui_bt_main(void *hd, void *private) //主界面显示
{
    if (!hd) {
        return;
    }

#if TCFG_APP_FM_EMITTER_EN

    if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) {
        lcdseg_show_call(hd);
    } else {
        u16 fre = fm_emitter_manage_get_fre();
        if (fre != 0) {
            lcdseg_fm_show_freq(hd, private, fre);
        } else {
            lcdseg_show_wait(hd);
        }
    }
#else
    extern u8 User_mute_flag;
    if(User_mute_flag)
    {
        extern void lcdseg_show_mute(void *hd);
        lcdseg_show_mute(hd);
    }
    else
    {
        if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) {
            if(BT_CALL_INCOMING == get_call_status() || BT_CALL_OUTGOING == get_call_status())
            {
                lcdseg_show_phone_num(hd, 0);
            }
            else if(BT_CALL_ACTIVE == get_call_status())
            {
    //            lcdseg_show_call(hd);
                lcdseg_show_dial_time(hd);
            }
        } else {
            lcdseg_show_bt(hd);
            bt_user_priv_var.phone_num_cur = 0;
            bt_user_priv_var.dial_time = 0;

            extern void lcdseg_show_icon_online_dev(void *hd);
            lcdseg_show_icon_online_dev(hd);
        }
    }

#endif
}


static int ui_bt_user(void *hd, void *private, u8 menu, u32 arg)//子界面显示 //返回true不继续传递 ，返回false由common统一处理
{
    int ret = true;
    LCD_API *dis = (LCD_API *)hd;
    if (!hd) {
        return false;
    }

    switch (menu) {
    case MENU_BT:
        lcdseg_show_bt(hd);
        break;

    case MENU_BT_INPUT_NUMBER:
        lcdseg_show_phone_num(hd, 1);
        break;

    default:
        ret = false;
    }

//    extern void lcdseg_show_icon_online_dev(void *hd);
//    lcdseg_show_icon_online_dev(hd);
    return ret;

}

const struct ui_dis_api bt_main = {
    .ui      = UI_BT_MENU_MAIN,
    .open    = ui_open_bt,
    .ui_main = ui_bt_main,
    .ui_user = ui_bt_user,
    .close   = ui_close_bt,
};



#endif
#endif
