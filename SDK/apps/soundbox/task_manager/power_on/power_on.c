#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "tone_player.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "app_main.h"
#include "ui_manage.h"
#include "vm.h"
#include "app_chargestore.h"
#include "user_cfg.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "key_event_deal.h"
#include "bt.h"


#define LOG_TAG_CONST       APP_IDLE
#define LOG_TAG             "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


static void  lcd_ui_power_on_timeout(void *p)
{
#if (TCFG_SPI_LCD_ENABLE)
    /* sys_key_event_enable(); */
    /* logo_time = timer_get_ms(); */
    /* while (timer_get_ms() - logo_time <= 2 * 1000) { //显示开机logo */
    /* os_time_dly(10); */
    /* } */

    UI_HIDE_WINDOW(ID_WINDOW_POWER_ON);
    UI_SHOW_WINDOW(ID_WINDOW_MAIN);
#if TCFG_APP_BT_EN
    app_task_switch_to(APP_BT_TASK);
#else
    app_task_switch_to(APP_MUSIC_TASK);
#endif
#endif

}

static void  lcd_ui_power_on()
{
#if (TCFG_SPI_LCD_ENABLE)
    int logo_time = 0;
    UI_SHOW_WINDOW(ID_WINDOW_POWER_ON);
    sys_timeout_add(NULL, lcd_ui_power_on_timeout, 1000);
#endif
}

static int power_on_init(void)
{
    ///有些需要在开机提示完成之后再初始化的东西， 可以在这里初始化
#if (TCFG_SPI_LCD_ENABLE)
    lcd_ui_power_on();//由ui决定切换的模式
    return 0;
#endif

    JL_PORTB->DIR &= ~BIT(11);  JL_PORTB->OUT &= ~BIT(11);  //2020-12-4 power_off
    ///上电清除VM music断点信息
//    extern void breakpoint_clear(void);
//    breakpoint_clear();

    bt_direct_init();///2020-11-16 上电直接打开蓝牙后台

///2020-11-13
#if 1//USER_POWERUP_TO_POWEROFF //上电进入关机模式
    app_task_switch_to(APP_POWEROFF_TASK);
#else
#if TCFG_APP_BT_EN
    app_task_switch_to(APP_BT_TASK);
#else
    /* app_task_switch_to(APP_MUSIC_TASK); */
    app_task_switch_to(APP_LINEIN_TASK);//如果带检测，设备不在线，则不跳转
#endif
#endif

    return 0;
}

static int power_on_unint(void)
{

    tone_play_stop();
    UI_HIDE_CURR_WINDOW();

    extern void eq_setting_init_2(void);
    eq_setting_init_2();    ///2020-12-7 重新设置一次EQ值

    return 0;
}






static int poweron_sys_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        break;
    case SYS_BT_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        break;
    default:
        return false;
    }
    return false;
}


static void  tone_play_end_callback(void *priv, int flag)
{
    int index = (int)priv;

    if (APP_POWERON_TASK != app_get_curr_task()) {
        log_error("tone callback task out \n");
        return;
    }

    switch (index) {
    case IDEX_TONE_POWER_ON:
        power_on_init();
        break;
    }
}

int dac_energy_level = 0;
static void get_dac_pwr2fft(void)
{
    extern int audio_dac_energy_get(void);
    dac_energy_level = audio_dac_energy_get() / 1000;

    sys_timeout_add(NULL, get_dac_pwr2fft, 50);
}

#if ACC_DETECT_EN
extern void idle_app_open_module();
extern void idle_app_close_module();
u8 acc_det_count = 0;
u8 acc_last_mode_flag = 0;
u16 acc_ignore_time = ACC_DETECT_POWERUP_IGNORE_TIME;

static void acc_detect(void)
{
    if(acc_ignore_time)
    {
        acc_ignore_time -= 100;
        goto __timeout_acc_detect;
    }
    switch(User_acc_sta)
    {
    case ACC_EVENT_IDLE:
        if(User_get_acc_det()) ///2020-11-25 当正常状态时检测到ACC+12V断开,进入500Ms的滤波计时
            User_acc_sta = ACC_EVENT_IN_DETECTED;
        break;

    case ACC_EVENT_IN_DETECTED:
        if(User_get_acc_det())
        {
            acc_det_count++;
            log_info("acc det in:%d\n", acc_det_count);
            if(acc_det_count >= 5)  ///2020-11-25 500Ms内电压都正确则进入准备状态,让模式进入关机模式
            {
                User_acc_sta = ACC_EVENT_ENTER;
                if(app_get_curr_task() > APP_POWEROFF_TASK)
                {
                    acc_last_mode_flag = 1;
                    app_task_switch_to(APP_POWEROFF_TASK);
                }
                else
                {
                    acc_last_mode_flag = 0;
                }
            }
        }
        else
        {
            acc_det_count = 0;
            User_acc_sta = ACC_EVENT_IDLE;
        }
        break;

    case ACC_EVENT_ENTER:           ///2020-11-25 在准备状态中，检测到模式切换到关机后,把高功耗功能关闭
        log_info("changing mode to pwroff...\n");
        if(app_get_curr_task() == APP_POWEROFF_TASK)
        {
            log_info("close bt,udisk,sd... for acc_in\n");
            acc_det_count = 0;
            bt_direct_close();
            idle_app_close_module();
            User_acc_sta = ACC_EVENT_COMPLETE;
            log_info("acc in\n");
        }
        break;

    case ACC_EVENT_COMPLETE:
        if(User_get_acc_det() == 0)       ///2020-11-25 当ACC状态时检测到ACC+12V,进入500Ms的滤波计时
            User_acc_sta = ACC_EVENT_OUT_DETECTED;
        break;

    case ACC_EVENT_OUT_DETECTED:
        if(User_get_acc_det() == 0)
        {
            acc_det_count++;
            log_info("acc det out:%d\n", acc_det_count);
            if(acc_det_count >= 5)  ///2020-11-25 500Ms内电压都正确则进入准备状态,复原之前所关闭的功能
            {
                log_info("open bt,udisk,sd... for acc_out\n");
                acc_det_count = 0;
                idle_app_open_module();
                bt_direct_init();
                User_acc_sta = ACC_EVENT_EXIT;
            }
        }
        else
        {
            User_acc_sta = ACC_EVENT_COMPLETE;
        }
        break;

    case ACC_EVENT_EXIT:
        if(acc_last_mode_flag)      ///2020-11-25 返回ACC前的模式
        {
            log_info("changing mode back to last...\n");
            app_task_switch_back();
        }
        acc_last_mode_flag = 0;
        User_acc_sta = ACC_EVENT_IDLE;
        log_info("acc out\n");
        break;
    }

__timeout_acc_detect:
    sys_timeout_add(NULL, acc_detect, 100);
}
#endif

void app_poweron_task()
{
    int msg[32];

    user_rgb_online();///检测RGB是否在线

#if TCFG_EQ_ENABLE
    ///2020-11-7
    extern void eq_setting_init(void);
    eq_setting_init();
#endif

    get_dac_pwr2fft();  ///2020-11-23 开启150Ms循环获取DAC能量值

#if ACC_DETECT_EN
    ///2020-11-25 User ACC Init
    acc_detect();
#endif

    if(app_var.volmcu_type == 0)
    {
        ///2020-12-17 查看DSP模块是否在线
//        app_var.volmcu_type = is_dsp_module_online();
    }

    //UI_SHOW_MENU(MENU_POWER_UP, 0, 0, NULL); ///2020-11-23 上电不显示HI
    UI_SHOW_MENU(MENU_RTC_SHOW, 0, 0, NULL); ///2020-11-23 上电直接显示时钟

    int err =  tone_play_with_callback_by_name(tone_table[IDEX_TONE_POWER_ON], 1, tone_play_end_callback, (void *)IDEX_TONE_POWER_ON);
    /* if (err) { //提示音没有,播放失败，直接init流程 */
    /* power_on_init(); */
    /* } */


    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            if (poweron_sys_event_handler((struct sys_event *)(msg + 1)) == false) {
                app_default_event_deal((struct sys_event *)(&msg[1]));    //由common统一处理
            }
            break;
        default:
            break;
        }

        if (app_task_exitting()) {
            power_on_unint();
            return;
        }
    }

}

