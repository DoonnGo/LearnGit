#include "common/app_common.h"
#include "app_task.h"
#include "app_main.h"
#include "key_event_deal.h"
#include "music/music.h"
#include "pc/pc.h"
#include "record/record.h"
#include "linein/linein.h"
#include "fm/fm.h"
#include "btstack/avctp_user.h"
#include "app_power_manage.h"
#include "app_chargestore.h"
#include "usb/otg.h"
#include "usb/host/usb_host.h"
#include <stdlib.h>
#include "bt/bt_tws.h"
#include "audio_config.h"
#include "common/power_off.h"
#include "common/user_msg.h"
#include "audio_config.h"
#include "audio_enc.h"
#include "ui/ui_api.h"
#include "fm_emitter/fm_emitter_manage.h"
#include "common/fm_emitter_led7_ui.h"
#if TCFG_CHARGE_ENABLE
#include "app_charge.h"
#endif
#include "dev_multiplex_api.h"
#include "chgbox_ctrl.h"
#include "device/chargebox.h"
#include "app_online_cfg.h"
#include "soundcard/soundcard.h"
#include "smartbox_bt_manage.h"
#include "bt.h"
#include "common/dev_status.h"
#include "tone_player.h"
#include "ui_manage.h"
#include "soundbox.h"
#include "audio_recorder_mix.h"
#include "application/eq_config.h"
#include "rgb_module.h"

#define LOG_TAG_CONST       APP_ACTION
#define LOG_TAG             "[APP_ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


int JL_rcsp_event_handler(struct rcsp_event *rcsp);
int bt_background_event_handler(struct sys_event *event);
extern u32 timer_get_ms(void);
extern int alarm_sys_event_handler(struct sys_event *event);
extern void bt_tws_sync_volume();

///2020-11-21 RTC函数声明
static void User_rtc_show(void);
static void User_rtc_set(void);
static void User_rtc_set_hour_dir(u8 dir);
static void User_rtc_set_min_dir(u8 dir);

u8 User_menu_refresh_by_key(void);
void User_menu_change_by_key(u8 app_menu, s32 arg);
u8 menu_refresh_flag = 0;   ///2020-12-4 当menu_refresh_flag为1时,旋转编码器会重置会音量调节,实现按键和编码分离

u8 User_mute_flag = 0;  ///mute_flag
u16 User_pa_unmute_timeid = 0;
void User_pa_unmute(void);

int app_common_key_msg_deal(struct sys_event *event)
{
    int ret = false;
    struct key_event *key = &event->u.key;
    int key_event = event->u.key.event;
    int key_value = event->u.key.value;

    if (key_event == KEY_NULL) {
        return false;
    }

    if (key_is_ui_takeover()) {
        ui_key_msg_post(key_event);
        return false;
    }

#if (TCFG_UI_ENABLE && TCFG_APP_FM_EMITTER_EN)
    if (!ui_fm_emitter_common_key_msg(key_event)) {
        return false;
    }
#endif

    ///log_info("common_key_event:%d\n", key_event);

    if ((key_event != KEY_POWEROFF) && (key_event != KEY_POWEROFF_HOLD)) {
        extern u8 goto_poweroff_first_flag;
        goto_poweroff_first_flag = 0;
    }

#if (SMART_BOX_EN)
    extern bool smartbox_key_event_filter_before(int key_event);
    if (smartbox_key_event_filter_before(key_event)) {
        return true;
    }
#endif

    switch (key_event) {
#if TCFG_APP_BT_EN

#if TCFG_USER_TWS_ENABLE
    case KEY_TWS_CONN:
        log_info("    KEY_TWS_CONN \n");
        bt_open_tws_conn(0);
        break;
    case KEY_TWS_DISCONN:
        log_info("    KEY_TWS_DISCONN \n");
        bt_disconnect_tws_conn();
        break;
    case KEY_TWS_REMOVE_PAIR:
        log_info("    KEY_TWS_REMOVE_PAIR \n");
        bt_tws_remove_tws_pair();
        break;
    case KEY_TWS_SEARCH_PAIR:
        log_info("    KEY_TWS_SEARCH_PAIR \n");
        bt_tws_start_search_and_pair();
        break;
    case KEY_TWS_SEARCH_REMOVE_PAIR:
        log_info("    KEY_TWS_SEARCH_REMOVE_PAIR \n");
        bt_tws_search_or_remove_pair();
        break;
#endif

    case KEY_BT_DIRECT_INIT:
        bt_direct_init();
        break;
    case KEY_BT_DIRECT_CLOSE:
        bt_direct_close();
        break;
#endif

#if 0
    case  KEY_POWEROFF:
    case  KEY_POWEROFF_HOLD:
        power_off_deal(event, key_event - KEY_POWEROFF);
        break;
#else
    case  KEY_POWEROFF:
        User_mute_flag = 1;
        bsp_app_mute_status();//blesend,关机发出mute
        User_mute_flag = 0;
        app_task_switch_to(APP_POWEROFF_TASK);
        break;
#endif
    case KEY_IR_NUM_0:
    case KEY_IR_NUM_1:
    case KEY_IR_NUM_2:
    case KEY_IR_NUM_3:
    case KEY_IR_NUM_4:
    case KEY_IR_NUM_5:
    case KEY_IR_NUM_6:
    case KEY_IR_NUM_7:
    case KEY_IR_NUM_8:
    case KEY_IR_NUM_9:
        break;

    case KEY_MODE_HANG_UP:  ///2020-11-16 MOD按键做挂机
    case KEY_CHANGE_MODE:
#if (TCFG_DEC2TWS_ENABLE)
        if (!key->init) {
            break;
        }
#endif
#if TWFG_APP_POWERON_IGNORE_DEV
        if ((timer_get_ms() - app_var.start_time) > TWFG_APP_POWERON_IGNORE_DEV)
#endif//TWFG_APP_POWERON_IGNORE_DEV

        {
            printf("KEY_CHANGE_MODE\n");
            app_task_switch_next();

            ///2020-11-24 User转模式进行Mute操作与定时解Mute
            USER_MUTE_ENABLE();
            User_mute_flag = 1;
            bsp_app_mute_status();//blesend,先发出mute待进入下个模式后再更新
            User_mute_flag = 0;
            if(User_pa_unmute_timeid == 0)
            {
                User_pa_unmute_timeid = sys_timeout_add(NULL, User_pa_unmute, 2000);
            }
            else
            {
                sys_timer_re_run(User_pa_unmute_timeid);
            }
        }
        break;

#if 0   ///2020-11-27 已重写音量与EQ控制
    case KEY_VOL_UP:
        log_info("COMMON KEY_VOL_UP\n");
        if (!tone_get_status()) {
            app_audio_volume_up(1);
            printf("common vol+: %d", app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
        }
        if (app_audio_get_volume(APP_AUDIO_CURRENT_STATE) == app_audio_get_max_volume()) {
            if (tone_get_status() == 0) {
#if TCFG_MAX_VOL_PROMPT
                tone_play_by_path(tone_table[IDEX_TONE_MAX_VOL], 0);
#endif
            }
        }

#if (TCFG_DEC2TWS_ENABLE)
        bt_tws_sync_volume();
#endif
        UI_SHOW_MENU(MENU_MAIN_VOL, 1000, app_audio_get_volume(APP_AUDIO_CURRENT_STATE), NULL);
        break;

    case KEY_VOL_DOWN:
        log_info("COMMON KEY_VOL_DOWN\n");
        app_audio_volume_down(1);
        printf("common vol-: %d", app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
#if (TCFG_DEC2TWS_ENABLE)
        bt_tws_sync_volume();
#endif
        UI_SHOW_MENU(MENU_MAIN_VOL, 1000, app_audio_get_volume(APP_AUDIO_CURRENT_STATE), NULL);
        break;

    case  KEY_EQ_MODE:
#if(TCFG_EQ_ENABLE == 1)
        eq_mode_sw();
#endif
        break;
#endif

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)


    case KEY_BT_EMITTER_RECEIVER_SW:
        printf(" KEY_BT_EMITTER_RECEIVER_SW\n");
        bt_emitter_receiver_sw();
        break;

    case KEY_BT_EMITTER_PAUSE:
        r_printf(" KEY_BT_EMITTER_PAUSE\n");
        bt_emitter_pp(0);
        break;

    case KEY_BT_EMITTER_PLAY:
        r_printf(" KEY_BT_EMITTER_PLAY\n");
        bt_emitter_pp(1);
        break;

    case KEY_BT_EMITTER_SW:
        r_printf("KEY_BT_EMITTER_SW\n");
        {
            extern u8 bt_emitter_stu_sw(void);

            if (bt_emitter_stu_sw()) {
                printf("bt emitter start \n");
            } else {
                printf("bt emitter stop \n");
            }
        }
        break;
#endif


#if(TCFG_CHARGE_BOX_ENABLE)
    case  KEY_BOX_POWER_CLICK:
    case  KEY_BOX_POWER_LONG:
    case  KEY_BOX_POWER_HOLD:
    case  KEY_BOX_POWER_UP:
        charge_box_key_event_handler(key_event);
        break;
#endif
#if (TCFG_MIC_EFFECT_ENABLE)
    case KEY_REVERB_OPEN:
#if TCFG_USER_TWS_ENABLE
        if (!key->init) {
            break;
        }
#endif

        if (mic_effect_get_status()) {
            mic_effect_stop();
        } else {
            mic_effect_start();
        }
        ret = true;
        break;
#endif
    case KEY_ENC_START:
#if (RECORDER_MIX_EN)
        if (recorder_mix_get_status()) {
            printf("recorder_encode_stop\n");
            recorder_mix_stop();
        } else {
            printf("recorder_encode_start\n");
            recorder_mix_start();
        }
#endif/*RECORDER_MIX_EN*/
        break;

    case KEY_EQ_MODE:
        User_menu_change_by_key(MENU_SET_EQ, 0);
//        menu_refresh_flag = 1;
//        UI_SHOW_MENU(MENU_SET_EQ, 3000, 0, User_menu_refresh_by_key);
        goto __encoder_up;
    case KEY_LOUD:
        if(User_get_eq_mode() != EQ_MODE_CUSTOM)    ///暂不支持在非normal的eq调loud
            break;
        User_menu_change_by_key(MENU_LOUD, 0);
//        menu_refresh_flag = 1;
//        UI_SHOW_MENU(MENU_LOUD, 3000, 0, User_menu_refresh_by_key);
        goto __encoder_up;
    case KEY_VOL_UP:
        User_menu_change_by_key(MENU_MAIN_VOL, 0);
//        menu_refresh_flag = 1;
//        UI_SHOW_MENU(MENU_MAIN_VOL, 3000, 0, User_menu_refresh_by_key);
        goto __encoder_up;
    case KEY_VOL_DOWN:
        User_menu_change_by_key(MENU_MAIN_VOL, 0);
//        menu_refresh_flag = 1;
//        UI_SHOW_MENU(MENU_MAIN_VOL, 3000, 0, User_menu_refresh_by_key);
        goto __encoder_down;

    case KEY_ENCODER_UP:
__encoder_up:
        User_menu_gain_dir(1);
        break;

    case KEY_ENCODER_DOWN:
__encoder_down:
        User_menu_gain_dir(0);
        break;

    case KEY_MENU_CHANGE:
        User_menu_change(0);
        break;

    case KEY_CLK:
        User_rtc_show();
        break;

    case KEY_CLK_SET:
        User_rtc_set();
        break;

    case KEY_MUTE:
        if(User_mute_flag == 0)
        {
            User_mute_flag = 1;
            USER_MUTE_ENABLE();
        }
        else
        {
            User_mute_flag = 0;
            USER_MUTE_DISABLE();
        }
        bsp_app_mute_status();//blesend
        UI_REFLASH_WINDOW(1);
        break;

    case KEY_SYSTEM_RESET:
        cpu_reset();
        break;

    case BLE_VOL_SET:   //BLE直接设置系统音量
        User_menu_set_volume(key_value);
        break;
    case BLE_EQ_SET:    //BLE直接设置预设EQ值
        User_eq_set(key_value);
        UI_SHOW_MENU(MENU_SET_EQ, 3000, User_get_eq_mode(), NULL);
        break;
    case BLE_RGB_SET:
        user_set_rgb_mode(key_value);
        break;

    default:
        ui_key_msg_post(key_event);
#ifdef CONFIG_BOARD_AC695X_SOUNDCARD
        soundcard_key_event_deal(key_event);
#endif
        break;

    }
#if (SMART_BOX_EN)
    extern int smartbox_key_event_deal(u8 key_event, int ret);
    ret = smartbox_key_event_deal(key_event, ret);
#endif
    return ret;
}

int app_power_user_event_handler(struct device_event *dev)
{
#if(TCFG_SYS_LVD_EN == 1)
    switch (dev->event) {
    case POWER_EVENT_POWER_WARNING:
        ui_update_status(STATUS_LOWPOWER);
        tone_play_by_path(tone_table[IDEX_TONE_LOW_POWER], 1);
        /* return 0; */
    }
#endif
    return app_power_event_handler(dev);
}

extern bool User_sd0_in_to_music_task;
static void app_common_device_event_handler(struct sys_event *event)
{
    int ret = 0;
    const char *logo = NULL;
    const char *usb_msg = NULL;
    u8 app  = 0xff ;
    u8 alarm_flag = 0;
    switch ((u32)event->arg) {
#if TCFG_CHARGE_ENABLE
    case DEVICE_EVENT_FROM_CHARGE:
        app_charge_event_handler(&event->u.dev);
        break;
#endif//TCFG_CHARGE_ENABLE

#if TCFG_ONLINE_ENABLE
    case DEVICE_EVENT_FROM_CI_UART:
        ci_data_rx_handler(CI_UART);
        break;

#if TCFG_USER_TWS_ENABLE
    case DEVICE_EVENT_FROM_CI_TWS:
        ci_data_rx_handler(CI_TWS);
        break;
#endif//TCFG_USER_TWS_ENABLE
#endif//TCFG_ONLINE_ENABLE

    case DEVICE_EVENT_FROM_POWER:
        app_power_user_event_handler(&event->u.dev);
        break;

#if TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE
    case DEVICE_EVENT_CHARGE_STORE:
        app_chargestore_event_handler(&event->u.chargestore);
        break;
#endif//TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE

#if(TCFG_CHARGE_BOX_ENABLE)
    case DEVICE_EVENT_FROM_CHARGEBOX:
        charge_box_ctrl_event_handler(&event->u.chargebox);
        break;
#endif


    case DEVICE_EVENT_FROM_OTG:
        ///先分析OTG设备类型
        usb_msg = (const char *)event->u.dev.value;
        if (usb_msg[0] == 's') {
            ///是从机
#if TCFG_PC_ENABLE
            ret = pc_device_event_handler(event);
            if (ret == true) {
                app = APP_PC_TASK;
            }
#endif
            break;
        } else if (usb_msg[0] == 'h') {
            ///是主机, 统一于SD卡等响应主机处理，这里不break
        } else {
            log_e("unknow otg devcie !!!\n");
            break;
        }
    case DRIVER_EVENT_FROM_SD0:
    case DRIVER_EVENT_FROM_SD1:
    case DRIVER_EVENT_FROM_SD2:
#if TCFG_APP_MUSIC_EN
        //UI_SHOW_MENU(MENU_USER_INFO, 1500, (int)event->arg, NULL);  ///显示设备信息

        ret = dev_status_event_filter(event);///解码设备上下线， 设备挂载等处理
        if (ret == true) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                ///设备上线， 非解码模式切换到解码模式播放
                if (app_get_curr_task() != APP_MUSIC_TASK) {
                    app = APP_MUSIC_TASK;

                    ///2020-12-7 修复其他模式插入卡时跳转到music会优先播放u盘
                    if((u32)event->arg == DRIVER_EVENT_FROM_SD0
                       || (u32)event->arg == DRIVER_EVENT_FROM_SD1
                       || (u32)event->arg == DRIVER_EVENT_FROM_SD2)
                        User_sd0_in_to_music_task = 1;
                }
            }
        }
#endif
        break;

#if TCFG_APP_LINEIN_EN
    case DEVICE_EVENT_FROM_LINEIN:
        ret = linein_device_event_handler(event);
        if (ret == true) {
            app = APP_LINEIN_TASK;
        }
        break;
#endif//TCFG_APP_LINEIN_EN

#if TCFG_APP_RTC_EN
    case DEVICE_EVENT_FROM_ALM:
        ret = alarm_sys_event_handler(event);
        if (ret == true) {
            alarm_flag = 1;
            app = APP_RTC_TASK;
        }
        break;
#endif//TCFG_APP_RTC_EN

    default:
        /* printf("unknow SYS_DEVICE_EVENT!!, %x\n", (u32)event->arg); */
        break;
    }

#if (SMART_BOX_EN)
    extern void smartbox_update_dev_state(u32 event);
    smartbox_update_dev_state((u32)event->arg);
#endif

    if (app != 0xff) {
        if ((true != app_check_curr_task(APP_PC_TASK)) || alarm_flag) {

            //PC 不响应因为设备上线引发的模式切换
#if TWFG_APP_POWERON_IGNORE_DEV
            if ((timer_get_ms() - app_var.start_time) > TWFG_APP_POWERON_IGNORE_DEV)
#endif//TWFG_APP_POWERON_IGNORE_DEV
            {
                if(app_get_curr_task() <= APP_POWEROFF_TASK)    ///2020-11-25 关机时插入设备不开机
                {
                    User_sd0_in_to_music_task = 0;
                    return;
                }

#if (TCFG_CHARGE_ENABLE && (!TCFG_CHARGE_POWERON_ENABLE))
                extern u8 get_charge_online_flag(void);
                if (get_charge_online_flag()) {

                } else
#endif
                {

                    printf(">>>>>>>>>>>>>%s %d \n", __FUNCTION__, __LINE__);
                    app_task_switch_to(app);
                }
            }
        }
    }
}


///公共事件处理， 各自模式没有处理的事件， 会统一在这里处理
void app_default_event_deal(struct sys_event *event)
{
    int ret;
    SYS_EVENT_HANDLER_SPECIFIC(event);
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        /*默认公共设备事件处理*/
        /* printf(">>>>>>>>>>>>>%s %d \n", __FUNCTION__, __LINE__); */
        app_common_device_event_handler(event);
        break;
#if TCFG_APP_BT_EN
    case SYS_BT_EVENT:
        if (true != app_check_curr_task(APP_BT_TASK)) {
            /*默认公共BT事件处理*/
            bt_background_event_handler(event);
        }
        break;
#endif
    case SYS_KEY_EVENT:
        app_common_key_msg_deal(event);
        break;
    default:
        printf("unknow event\n");
        break;
    }
}


#if 0
extern int key_event_remap(struct sys_event *e);
extern const u16 bt_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
u8 app_common_key_var_2_event(u32 key_var)
{
    u8 key_event = 0;
    u8 key_value = 0;
    struct sys_event e = {0};
#if TCFG_ADKEY_ENABLE
    for (; key_value < KEY_AD_NUM_MAX; key_value++) {
        for (key_event = 0; key_event < KEY_EVENT_MAX; key_event++) {
            if (bt_key_ad_table[key_value][key_event] == key_var) {
                e.type = SYS_KEY_EVENT;
                e.u.key.type = KEY_DRIVER_TYPE_AD;
                e.u.key.event = key_event;
                e.u.key.value = key_value;
                /* e.u.key.tmr = timer_get_ms(); */
                e.arg  = (void *)DEVICE_EVENT_FROM_KEY;
                /* printf("key2event:%d %d %d\n", key_var, key_value, key_event); */
                if (key_event_remap(&e)) {
                    sys_event_notify(&e);
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

#else

u8 app_common_key_var_2_event(u32 key_var)
{
    struct sys_event e = {0};
    e.type = SYS_KEY_EVENT;
    e.u.key.type = KEY_DRIVER_TYPE_SOFTKEY;
    e.u.key.event = key_var;
    e.arg  = (void *)DEVICE_EVENT_FROM_KEY;
    sys_event_notify(&e);
    return true;
}
#endif

///+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void User_menu_set_volume(s16 volume)
{
	User_set_volume(volume);
    UI_SHOW_MENU(MENU_MAIN_VOL, 3000, volume, NULL);
}

static void User_menu_volume_dir(u8 dir)
{
    if(dir)
    {
#if 0
        if (!tone_get_status()) {
            app_audio_volume_up(1);
        }else{
            return;
        }
        if (app_audio_get_volume(APP_AUDIO_CURRENT_STATE) == app_audio_get_max_volume()) {
            if (tone_get_status() == 0) {
#if TCFG_MAX_VOL_PROMPT
                tone_play_by_path(tone_table[IDEX_TONE_MAX_VOL], 0);
#endif
            }
        }
#else
        app_audio_volume_up(1);
#endif
    }
    else
    {
        app_audio_volume_down(1);
    }
}

s8 User_volume_value = 30;
void User_menu_gain_dir(u8 dir)
{
    u8 cur_menu = UI_GET_CURR_MENU();
    s32 volume = 0;

    if(cur_menu == MENU_BASS)
    {
#if TCFG_EQ_ENABLE
        volume = User_eq_bass_dir(dir);
#endif
    }
    else if(cur_menu == MENU_TREBLE)
    {
#if TCFG_EQ_ENABLE
        volume = User_eq_treble_dir(dir);
#endif
    }
    else if(cur_menu >= MENU_EQ_FREQ_1 && cur_menu <= MENU_EQ_FREQ_7)
    {
        volume = dsp_module_set_eq_section_dir(cur_menu-MENU_EQ_FREQ_1, dir);
    }
    else if(cur_menu == MENU_BAL)
    {
        volume = User_bal_volume_dir(dir);
    }
    else if(cur_menu == MENU_FAD)
    {
        volume = User_fad_volume_dir(dir);
    }
    else if(cur_menu == MENU_LOUD)
    {
#if TCFG_EQ_ENABLE
        volume = User_eq_loud_change();
#endif
    }
    else if(cur_menu == MENU_IFX)
    {
        volume = dsp_module_set_ifx_dir(dir);
    }
    else if(cur_menu == MENU_SET_EQ)
    {
#if TCFG_EQ_ENABLE
        volume = User_eq_dir(dir);
#endif
    }
    else if(cur_menu == MENU_RTC_SET_HOUR)
    {
        User_rtc_set_hour_dir(dir);
        return;
    }
    else if(cur_menu == MENU_RTC_SET_MIN)
    {
        User_rtc_set_min_dir(dir);
        return;
    }
    else
    {
        User_menu_volume_dir(dir);
        volume = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
        User_volume_value = volume;
        cur_menu = MENU_MAIN_VOL;
    }
    UI_SHOW_MENU(cur_menu, 3000, volume, NULL);
}

void User_menu_change(u8 mode)
{
    u8 cur_menu = UI_GET_CURR_MENU();
    s32 volume = 0;

    switch(cur_menu)
    {
        default:
            cur_menu = MENU_MAIN_VOL;
        case MENU_MAIN_VOL:
            if(User_get_eq_mode() != EQ_MODE_CUSTOM)
                cur_menu = MENU_BAL;
            else
            {
                if(app_var.volmcu_type == 0)
                    cur_menu = MENU_BASS;
                if(app_var.volmcu_type == 1)
                    cur_menu = MENU_EQ_FREQ_1;
            }
            break;
        case MENU_BASS:      cur_menu = MENU_TREBLE;    break;
        case MENU_TREBLE:    cur_menu = MENU_BAL;       break;
        case MENU_EQ_FREQ_1: cur_menu = MENU_EQ_FREQ_2; break;
        case MENU_EQ_FREQ_2: cur_menu = MENU_EQ_FREQ_3; break;
        case MENU_EQ_FREQ_3: cur_menu = MENU_EQ_FREQ_4; break;
        case MENU_EQ_FREQ_4: cur_menu = MENU_EQ_FREQ_5; break;
        case MENU_EQ_FREQ_5: cur_menu = MENU_EQ_FREQ_6; break;
        case MENU_EQ_FREQ_6: cur_menu = MENU_EQ_FREQ_7; break;
        case MENU_EQ_FREQ_7: cur_menu = MENU_BAL;       break;
        case MENU_BAL:       cur_menu = MENU_FAD;       break;
        case MENU_FAD:
            if(User_get_eq_mode() != EQ_MODE_CUSTOM)
            {
                if(app_var.volmcu_type == 0)
                    cur_menu = MENU_SET_EQ;
                else if(app_var.volmcu_type == 1)
                    cur_menu = MENU_IFX;
            }
            else
            {
                if(app_var.volmcu_type == 0)
                    cur_menu = MENU_LOUD;
                else if(app_var.volmcu_type == 1)
                    cur_menu = MENU_IFX;
            }
            break;
        case MENU_LOUD:      cur_menu = MENU_SET_EQ;    break;
        case MENU_IFX:       cur_menu = MENU_SET_EQ;    break;
        case MENU_SET_EQ:    cur_menu = MENU_MAIN_VOL;  break;
    }
    switch(cur_menu)
    {
        case MENU_MAIN_VOL:
            volume = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
            break;
#if TCFG_EQ_ENABLE
        case MENU_BASS:
            volume = User_get_bass_gain();
            break;
        case MENU_TREBLE:
            volume = User_get_treble_gain();
            break;
#endif
        case MENU_EQ_FREQ_1:
        case MENU_EQ_FREQ_2:
        case MENU_EQ_FREQ_3:
        case MENU_EQ_FREQ_4:
        case MENU_EQ_FREQ_5:
        case MENU_EQ_FREQ_6:
        case MENU_EQ_FREQ_7:
            volume = dsp_module_get_eq_section_gain(cur_menu-MENU_EQ_FREQ_1);
            break;
        case MENU_BAL:
            volume = User_get_bal_volume();
            break;
        case MENU_FAD:
            volume = User_get_fad_volume();
            break;
        case MENU_IFX:
            volume = dsp_module_get_ifx_mode();
            break;
#if TCFG_EQ_ENABLE
        case MENU_LOUD:
            volume = User_get_loud_sta();
            break;
        case MENU_SET_EQ:
            volume = User_get_eq_mode();
            break;
#endif
        default:
            return;
    }
    if(mode != 0)
    {
        cur_menu = mode;
    }
    UI_SHOW_MENU(cur_menu, 3000, volume, NULL);
}

///2020-12-4 按键直接切换菜单里的内容,表现为: key键切换eq时旋转编码器是调音量,以前是按键切换eq时旋转编码器仍在调eq
void User_menu_change_by_key(u8 app_menu, s32 arg)
{
    UI_SHOW_MENU(app_menu, 3000, arg, User_menu_refresh_by_key);

    menu_refresh_flag = 1;
}

///2020-12-4 当menu_refresh_flag为1时,旋转编码器会重置会音量调节,实现按键和编码分离
u8 User_menu_refresh_by_key(void)
{
    if(menu_refresh_flag)
    {
        menu_refresh_flag = 0;
        return MENU_MAIN;
    }
    return 0;
}

extern u8 knob_direction;
void User_scan_knob_deal(void)
{
    if(knob_direction == 1)
    {
        knob_direction = 0;
        User_menu_gain_dir(1);
    }
    else if(knob_direction == 2)
    {
        knob_direction = 0;
        User_menu_gain_dir(0);
    }
}

#define UI_RTC_SHOW_5S 5000

extern void User_rtc_timeout(void);
static void User_rtc_show(void)
{
    u8 cur_menu = UI_GET_CURR_MENU();
    if(cur_menu == MENU_MAIN)
    {
        UI_SHOW_MENU(MENU_RTC_SHOW, UI_RTC_SHOW_5S, 0, NULL);
    }
    else if(cur_menu == MENU_RTC_SHOW)
    {
        UI_REFLASH_WINDOW(1);
    }
    else if(cur_menu == MENU_RTC_SET_HOUR)
    {
        UI_SHOW_MENU(MENU_RTC_SET_MIN, UI_RTC_SHOW_5S, 0, User_rtc_timeout);
    }
    else if(cur_menu == MENU_RTC_SET_MIN)
    {
        UI_SHOW_MENU(MENU_RTC_SHOW, UI_RTC_SHOW_5S, 0, NULL);
        //UI_SHOW_MENU(MENU_RTC_SET_HOUR, UI_RTC_SHOW_5S, 0, User_rtc_timeout);
    }
}

static void User_rtc_set(void)
{
    u8 cur_menu = UI_GET_CURR_MENU();
    if(cur_menu == MENU_MAIN)
    {
        UI_SHOW_MENU(MENU_RTC_SHOW, UI_RTC_SHOW_5S, 0, NULL);
    }
    else if(cur_menu == MENU_RTC_SHOW)
    {
        UI_SHOW_MENU(MENU_RTC_SET_HOUR, UI_RTC_SHOW_5S, 0, User_rtc_timeout);
    }
    else if(cur_menu == MENU_RTC_SET_HOUR || cur_menu == MENU_RTC_SET_MIN)
    {
        UI_SHOW_MENU(MENU_RTC_SHOW, UI_RTC_SHOW_5S, 0, NULL);
    }
}

static void User_rtc_set_hour_dir(u8 dir)
{
    if(dir == 0)
        dir = 2;
    UI_SHOW_MENU(MENU_RTC_SET_HOUR, UI_RTC_SHOW_5S, dir, User_rtc_timeout);
}

static void User_rtc_set_min_dir(u8 dir)
{
    if(dir == 0)
        dir = 2;
    UI_SHOW_MENU(MENU_RTC_SET_MIN, UI_RTC_SHOW_5S, dir, User_rtc_timeout);
}

///2020-11-24 功放解mute
void User_pa_unmute(void)
{
    User_pa_unmute_timeid = 0;
    USER_MUTE_DISABLE();
    bsp_app_mute_status();//blesend
}

///2020-11-25 自定义新dev信息用于ACC开关U盘SD蓝牙
u8 User_acc_sta = ACC_EVENT_IDLE;
static u8 User_acc_in = 0;  ///0:OUT, 1:IN

AT_VOLATILE_RAM_CODE
void User_update_acc_det(u8 in)
{
    User_acc_in = in;
}

u8 User_get_acc_det(void)
{
    return User_acc_in;
}
