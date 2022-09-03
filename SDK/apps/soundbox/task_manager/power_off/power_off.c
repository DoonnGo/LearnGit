
/*************************************************************
   此文件函数主 poweroff 关机流程

   如果蓝牙模式有开启，就会等待蓝牙流程关闭完成才会进入软关机

	power_set_soft_poweroff();
	软关机


**************************************************************/


#include "common/power_off.h"
#include "bt_tws.h"
#include <stdlib.h>
#include "app_config.h"
#include "app_task.h"
#include "system/includes.h"
#include "media/includes.h"
#include "app_power_manage.h"
#include "app_chargestore.h"
#include "btstack/avctp_user.h"
#include "app_main.h"
#include "ui/ui_api.h"
#include "ui_manage.h"
#include "tone_player.h"
#include "user_cfg.h"
#include "bt_tws.h"
#include "bt.h"
#include "key_event_deal.h"

#include "clock_cfg.h"
#include "rgb_module.h"

#define LOG_TAG_CONST       APP_ACTION
#define LOG_TAG             "[APP_ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#define POWER_OFF_CNT       10

static u8 goto_poweroff_cnt = 0;
u8 goto_poweroff_first_flag = 0;
static u8 goto_poweroff_flag = 0;
static u8 power_off_tone_play_flag = 0;

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 长按等待 关闭蓝牙
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
void power_off_deal(struct sys_event *event, u8 step)
{
    switch (step) {
    case 0:
    case 1:
        if (goto_poweroff_first_flag == 0) {
            goto_poweroff_first_flag = 1;
            goto_poweroff_cnt = 0;
            goto_poweroff_flag = 0;

#if TCFG_APP_BT_EN
            if ((BT_STATUS_CONNECTING == get_bt_connect_status()) ||
                (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) {
                /* if (get_call_status() != BT_CALL_HANGUP) {
                   log_info("call hangup\n");
                   user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
                	goto_poweroff_first_flag = 0;
                   goto_poweroff_flag = 0;
                   break;
                   } */
                if ((get_call_status() == BT_CALL_INCOMING) ||
                    (get_call_status() == BT_CALL_OUTGOING)) {
                    log_info("key call reject\n");
                    /* user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL); */
                    goto_poweroff_first_flag = 0;
                    goto_poweroff_flag = 0;
                    break;
                } else if (get_call_status() == BT_CALL_ACTIVE) {
                    log_info("key call hangup\n");
                    /* user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL); */
                    goto_poweroff_first_flag = 0;
                    goto_poweroff_flag = 0;
                    break;
                }
            }

#if (TCFG_USER_TWS_ENABLE && CONFIG_TWS_POWEROFF_SAME_TIME == 0)
            if ((u32)event->arg == KEY_EVENT_FROM_TWS) {
                goto_poweroff_first_flag = 0;
                break;
            }
#endif
            user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
#endif
            goto_poweroff_flag = 1;
            break;
        }

#if (TCFG_USER_TWS_ENABLE && CONFIG_TWS_POWEROFF_SAME_TIME == 0)
        if ((u32)event->arg == KEY_EVENT_FROM_TWS) {
            break;
        }
#endif
        log_info("poweroff flag:%d cnt:%d\n", goto_poweroff_flag, goto_poweroff_cnt);

        if (goto_poweroff_flag) {
            goto_poweroff_cnt++;

#if CONFIG_TWS_POWEROFF_SAME_TIME
            if (goto_poweroff_cnt == POWER_OFF_CNT) {
                if (get_tws_sibling_connect_state()) {
                    if ((u32)event->arg != KEY_EVENT_FROM_TWS) {
                        tws_api_sync_call_by_uuid('T', SYNC_CMD_POWER_OFF_TOGETHER, TWS_SYNC_TIME_DO);
                    }
                } else {
                    power_off_tone_play_flag = 1;
                    sys_enter_soft_poweroff(NULL);
                }
            }
#else
            if (goto_poweroff_cnt >= POWER_OFF_CNT) {
                goto_poweroff_cnt = 0;
#if TCFG_APP_BT_EN
                sys_enter_soft_poweroff(NULL);
#else
                app_var.goto_poweroff_flag = 1;
                app_task_switch_to(APP_POWEROFF_TASK);
#endif
            }
#endif //CONFIG_TWS_POWEROFF_SAME_TIME

        }
        break;
    }
}

extern u8 User_acc_sta;
///2020-11-13 poweroff 模式按键信息处理
static int poweroff_key_event_opr(struct sys_event *event)
{
    int ret = true;
    struct key_event *key = &event->u.key;

    int key_event = event->u.key.event;
    int key_value = event->u.key.value;//

    r_printf("key value:%d, event:%d \n", key->value, key->event);

    switch (key_event) {
    case KEY_POWER_ON:
    case KEY_POWEROFF:
        if(User_acc_sta == ACC_EVENT_IDLE)
        {
            if(app_task_switch_back() == 1)
                UI_SHOW_MENU(MENU_POWER_UP, 500, 0, NULL); ///2020-11-23 上电不显示HI
        }
        break;
    case KEY_SYSTEM_RESET:
        cpu_reset();
        break;
    default:
        ret = false;
        break;
    }
#if (SMART_BOX_EN)
    extern void smartbot_fm_msg_deal(int msg);
    smartbot_fm_msg_deal(key_event);
#endif

    return ret;
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 流程消息事件处理
  @param    无
  @return   无
  @note     只处理提示音消息
 */
/*----------------------------------------------------------------------------*/
static int poweroff_sys_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        poweroff_key_event_opr(event);
        return true;
    case SYS_BT_EVENT:
        return false;
    case SYS_DEVICE_EVENT:
        printf("pwroff dev event\n");
        return false;
    default:
        return true;
    }
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 等待关机显示完毕
  @param    无
  @return   无
  @note      */
/*----------------------------------------------------------------------------*/
static inline void poweroff_wait_ui()
{
#if TCFG_UI_ENABLE
    u8 count = 0;
__retry:
    if (UI_GET_WINDOW_ID() != ID_WINDOW_POWER_OFF) {
        os_time_dly(10);//增加延时防止没有关显示
        if (count < 3) {
            goto __retry;
        }
        count++;
    }
#endif
}

static void poweroff_done(void)
{
    poweroff_wait_ui();//等待关机显示完毕
    while (get_ui_busy_status()) {
    }
#if (TCFG_CHARGE_ENABLE && TCFG_CHARGE_POWERON_ENABLE)
    extern u8 get_charge_online_flag(void);
    if (get_charge_online_flag()) {
        cpu_reset();
    } else {
        power_set_soft_poweroff();
    }
#else
    power_set_soft_poweroff();
#endif
}



void poweroff_tone_end(void *priv, int flag)
{
    if (app_var.goto_poweroff_flag) {
        log_info("audio_play_event_end,enter soft poweroff");
        poweroff_done();
    }
}

///2020-11-21 RTC结构体
struct rtc_opr {
    void *dev_handle;
    u8  rtc_set_mode;
    u8  rtc_pos;
    u8  alm_enable;
    u8  alm_num;
    struct sys_time set_time;
};
static struct rtc_opr *rtc_background = NULL;

static user_poweroff_flag = 0;
static user_unmute_flag = 0;
void poweroff_user(void)
{
    user_poweroff_flag = 1;
    JL_PORTB->DIR &= ~BIT(11);  JL_PORTB->OUT &= ~BIT(11);
}

void unmute_user(void)
{
    user_unmute_flag = 1;
    USER_MUTE_DISABLE();
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 流程启动
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
static void poweroff_app_start()
{
    int ret = false;
    if (app_var.goto_poweroff_flag) {
        UI_SHOW_WINDOW(ID_WINDOW_POWER_OFF);
        syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);
        os_taskq_flush();

#if (CONFIG_TWS_POWEROFF_SAME_TIME)
        if (power_off_tone_play_flag == 0) {
            //不在这里播放提示音
            poweroff_done();
        } else
#endif/*CONFIG_TWS_POWEROFF_SAME_TIME*/
        {
            ret = tone_play_with_callback_by_name(tone_table[IDEX_TONE_POWER_OFF], 1,
                                                  poweroff_tone_end, (void *)IDEX_TONE_POWER_OFF);
            if (ret) {
                y_printf("power_off tone play err,enter soft poweroff");
                poweroff_done();
            }
        }
    }


//    if (!rtc_background) {
//        rtc_background = zalloc(sizeof(struct rtc_opr));
//        ASSERT(rtc_background, "%s %di \n", __func__, __LINE__);
//        rtc_background->dev_handle = dev_open("rtc", NULL);
//        if (!rtc_background->dev_handle) {
//            ASSERT(0, "%s %d \n", __func__, __LINE__);
//        }
//    }
//    rtc_background->rtc_set_mode =  RTC_SET_MODE;
//    rtc_background->rtc_pos = RTC_POS_NULL;

    ui_update_status(STATUS_RTC_MODE);
    clock_idle(RTC_IDLE_CLOCK);
    UI_SHOW_WINDOW(ID_WINDOW_CLOCK);


    ///2020-11-13 PowerOn & Unmute
    USER_MUTE_ENABLE();
    user_rgb_suspend();
    sys_timeout_add(NULL,poweroff_user,200);
//    JL_PORTB->DIR &= ~BIT(11);  JL_PORTB->OUT &= ~BIT(11);
    sys_key_event_enable();
    bsp_app_workmode();//blesend
}

static void poweroff_task_close(void)
{
    ///2020-11-13 PowerOn & Unmute
    JL_PORTB->DIR &= ~BIT(11);  JL_PORTB->OUT |= BIT(11);
    user_rgb_resume();

//    sys_timeout_add(NULL,unmute_user,200);
//    USER_MUTE_DISABLE();

    extern u8 User_mute_flag;  ///mute_flag
    extern u16 User_pa_unmute_timeid;  ///timeout id
    User_mute_flag = 0;
    extern void User_pa_unmute(void);
    if(User_pa_unmute_timeid == 0)
    {
        User_pa_unmute_timeid = sys_timeout_add(NULL, User_pa_unmute, 2000);
    }
    else
    {
        sys_timer_re_run(User_pa_unmute_timeid);
    }
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 主任务
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
void app_poweroff_task()
{
    int res;
    int msg[32];
    poweroff_app_start();
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);

        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            if (poweroff_sys_event_handler((struct sys_event *)(&msg[1])) == false) {
                app_default_event_deal((struct sys_event *)(&msg[1]));    //由common统一处理
            }
            break;
        default:
            break;
        }

        if (app_task_exitting()) {
            poweroff_task_close();
            return;
        }
    }
}

