#include "key_event_deal.h"
#include "key_driver.h"
#include "app_config.h"
#include "board_config.h"
#include "app_task.h"

#ifdef CONFIG_BOARD_AC695X_DEMO
///2020-11-13 阻值从低到高排列
enum{
#if CONFIG_CLIENT_DG || CONFIG_CLIENT_GX
    KEY_BOARD_SYSRESET = 0,
    KEY_BOARD_SEL,
    KEY_BOARD_PREV,
    KEY_BOARD_NEXT,
    KEY_BOARD_CLK,
    KEY_BOARD_MUTE,
    KEY_BOARD_BAND_AMS,
    KEY_BOARD_AMS,
    KEY_BOARD_EQ,
    KEY_BOARD_M1,
    KEY_BOARD_M2,
    KEY_BOARD_M3,
    KEY_BOARD_M4,
    KEY_BOARD_M5,
    KEY_BOARD_M6,
    KEY_BOARD_POWER,
    KEY_BOARD_MODE,
#else
    KEY_BOARD_SYSRESET = 0,
    KEY_BOARD_NONE1,
    KEY_BOARD_NONE2,
    KEY_BOARD_POWER_MODE,
    KEY_BOARD_SEL,
    KEY_BOARD_PREV,
    KEY_BOARD_NEXT,
    KEY_BOARD_CLK,
    KEY_BOARD_BAND_AMS,
    KEY_BOARD_MUTE,
    KEY_BOARD_NONE10,
    KEY_BOARD_M1,
    KEY_BOARD_M2,
    KEY_BOARD_M3,
    KEY_BOARD_M4,
    KEY_BOARD_M5,
    KEY_BOARD_M6,
#endif
};
/***********************************************************
 *				bt 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_BT_EN
const u16 bt_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
//    [KEY_BOARD_POWER_MODE] = {
//        KEY_MODE_HANG_UP,   KEY_POWEROFF,
//    },
    [KEY_BOARD_POWER] = {
        KEY_POWEROFF,
    },
    [KEY_BOARD_MODE] = {
        KEY_MODE_HANG_UP,
    },
    [KEY_BOARD_PREV] = {
        KEY_MUSIC_PREV,
    },
    [KEY_BOARD_MUTE] = {
        KEY_MUTE,
    },
    [KEY_BOARD_NEXT] = {
        KEY_MUSIC_NEXT,
    },
    [KEY_BOARD_SEL] = {
        KEY_MENU_CHANGE,    KEY_OPEN_SIRI,
    },
    [KEY_BOARD_CLK] = {
        KEY_CLK,            KEY_CLK_SET,
    },
    [KEY_BOARD_BAND_AMS] = {
        KEY_CALL_ANSWER,    KEY_CALL_LAST_NO
    },
    [KEY_BOARD_AMS] = {
        KEY_NULL
    },
    [KEY_BOARD_M1] = {
        KEY_MUSIC_PP,
    },
    [KEY_BOARD_M2] = {
        KEY_NULL,
    },
    [KEY_BOARD_M3] = {
        KEY_NULL,
    },
    [KEY_BOARD_M4] = {
        KEY_NULL,
    },
    [KEY_BOARD_M5] = {
        KEY_NULL,
    },
    [KEY_BOARD_M6] = {
        KEY_NULL,
    },
    [KEY_BOARD_SYSRESET] = {
        KEY_NULL,       KEY_SYSTEM_RESET,
    },
    [KEY_BOARD_EQ] = {
        KEY_EQ_MODE,    KEY_LOUD,
    },
///NONE KEY
//    [KEY_BOARD_NONE1] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE2] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE10] = {
//        KEY_NULL,
//    },
};
#endif

/***********************************************************
 *				fm 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_FM_EN
const u16 fm_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
//    [KEY_BOARD_POWER_MODE] = {
//        KEY_CHANGE_MODE, KEY_POWEROFF,
//    },
    [KEY_BOARD_POWER] = {
        KEY_POWEROFF,
    },
    [KEY_BOARD_MODE] = {
        KEY_CHANGE_MODE,
    },
    [KEY_BOARD_PREV] = {
        KEY_FM_SCAN_UP, KEY_FM_PREV_STEP_LONG,KEY_FM_PREV_STEP_HOLD,KEY_FM_PREV_STEP_LONGUP,
    },
    [KEY_BOARD_MUTE] = {
        KEY_MUTE,
    },
    [KEY_BOARD_NEXT] = {
        KEY_FM_SCAN_DOWN,KEY_FM_NEXT_STEP_LONG,KEY_FM_NEXT_STEP_HOLD,KEY_FM_NEXT_STEP_LONGUP,
    },
    [KEY_BOARD_SEL] = {
        KEY_MENU_CHANGE,
    },
    [KEY_BOARD_CLK] = {
        KEY_CLK,        KEY_CLK_SET,
    },
    [KEY_BOARD_BAND_AMS] = {
        KEY_FM_BAND,    KEY_FM_SCAN_ALL_DOWN,
    },
    [KEY_BOARD_AMS] = {
        KEY_FM_AUTO_PLAY,KEY_FM_SCAN_ALL_DOWN,
    },
    [KEY_BOARD_M1] = {
        KEY_FM_CHANNEL_1,KEY_FM_SCHANNEL_1,
    },
    [KEY_BOARD_M2] = {
        KEY_FM_CHANNEL_2,KEY_FM_SCHANNEL_2,
    },
    [KEY_BOARD_M3] = {
        KEY_FM_CHANNEL_3,KEY_FM_SCHANNEL_3,
    },
    [KEY_BOARD_M4] = {
        KEY_FM_CHANNEL_4,KEY_FM_SCHANNEL_4,
    },
    [KEY_BOARD_M5] = {
        KEY_FM_CHANNEL_5,KEY_FM_SCHANNEL_5,
    },
    [KEY_BOARD_M6] = {
        KEY_FM_CHANNEL_6,KEY_FM_SCHANNEL_6,
    },
    [KEY_BOARD_SYSRESET] = {
        KEY_NULL,       KEY_SYSTEM_RESET,
    },
    [KEY_BOARD_EQ] = {
        KEY_EQ_MODE,    KEY_LOUD,
    },
///NONE KEY
//    [KEY_BOARD_NONE1] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE2] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE10] = {
//        KEY_NULL,
//    },
};
#endif

/***********************************************************
 *				linein 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_LINEIN_EN
const u16 linein_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
//    [KEY_BOARD_POWER_MODE] = {
//        KEY_CHANGE_MODE, KEY_POWEROFF,
//    },
    [KEY_BOARD_POWER] = {
        KEY_POWEROFF,
    },
    [KEY_BOARD_MODE] = {
        KEY_CHANGE_MODE,
    },
    [KEY_BOARD_PREV] = {
        KEY_NULL,
    },
    [KEY_BOARD_MUTE] = {
        KEY_MUTE,
    },
    [KEY_BOARD_NEXT] = {
        KEY_NULL,
    },
    [KEY_BOARD_SEL] = {
        KEY_MENU_CHANGE,
    },
    [KEY_BOARD_CLK] = {
        KEY_CLK,        KEY_CLK_SET,
    },
    [KEY_BOARD_BAND_AMS] = {
        KEY_NULL,
    },
    [KEY_BOARD_AMS] = {
        KEY_NULL
    },
    [KEY_BOARD_M1] = {
        KEY_NULL,
    },
    [KEY_BOARD_M2] = {
        KEY_NULL,
    },
    [KEY_BOARD_M3] = {
        KEY_NULL,
    },
    [KEY_BOARD_M4] = {
        KEY_NULL,
    },
    [KEY_BOARD_M5] = {
        KEY_NULL,
    },
    [KEY_BOARD_M6] = {
        KEY_NULL,
    },
    [KEY_BOARD_SYSRESET] = {
        KEY_NULL,       KEY_SYSTEM_RESET,
    },
    [KEY_BOARD_EQ] = {
        KEY_EQ_MODE,    KEY_LOUD,
    },
///NONE KEY
//    [KEY_BOARD_NONE1] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE2] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE10] = {
//        KEY_NULL,
//    },
};
#endif

/***********************************************************
 *				music 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_MUSIC_EN
const u16 music_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
//    [KEY_BOARD_POWER_MODE] = {
//        KEY_CHANGE_MODE, KEY_POWEROFF,
//    },
    [KEY_BOARD_POWER] = {
        KEY_POWEROFF,
    },
    [KEY_BOARD_MODE] = {
        KEY_MUSIC_CHANGE_MODE,
    },
    [KEY_BOARD_PREV] = {
        KEY_MUSIC_PREV, KEY_MUSIC_FR,	KEY_MUSIC_FR,
    },
    [KEY_BOARD_MUTE] = {
        KEY_MUTE,
    },
    [KEY_BOARD_NEXT] = {
        KEY_MUSIC_NEXT, KEY_MUSIC_FF,	KEY_MUSIC_FF,
    },
    [KEY_BOARD_SEL] = {
        KEY_MENU_CHANGE,
    },
    [KEY_BOARD_CLK] = {
        KEY_CLK,        KEY_CLK_SET,
    },
    [KEY_BOARD_BAND_AMS] = {
        KEY_MUSIC_CHANGE_DEV, KEY_MUSIC_ID3,
    },
    [KEY_BOARD_AMS] = {
        KEY_NULL
    },
    [KEY_BOARD_M1] = {
        KEY_MUSIC_PP,
    },
    [KEY_BOARD_M2] = {
        KEY_MUSIC_INT,
    },
    [KEY_BOARD_M3] = {
        KEY_MUSIC_RPT,
    },
    [KEY_BOARD_M4] = {
        KEY_MUSIC_RDM,
    },
    [KEY_BOARD_M5] = {
        KEY_MUSIC_PREV_10,  KEY_MUSIC_PLAYE_PREV_FOLDER,
    },
    [KEY_BOARD_M6] = {
        KEY_MUSIC_NEXT_10,  KEY_MUSIC_PLAYE_NEXT_FOLDER,
    },
    [KEY_BOARD_SYSRESET] = {
        KEY_NULL,       KEY_SYSTEM_RESET,
    },
    [KEY_BOARD_EQ] = {
        KEY_EQ_MODE,    KEY_LOUD,
    },
///NONE KEY
//    [KEY_BOARD_NONE1] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE2] = {
//        KEY_NULL,
//    },
//    [KEY_BOARD_NONE10] = {
//        KEY_NULL,
//    },
};
#endif

/***********************************************************
 *				pc 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_PC_EN
const u16 pc_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_CHANGE_MODE, KEY_POWEROFF,		KEY_POWEROFF_HOLD,	KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_MUSIC_PREV,		KEY_VOL_DOWN,	KEY_VOL_DOWN,	KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [2] = {
        KEY_MUSIC_PP,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [3] = {
        KEY_MUSIC_NEXT,		KEY_VOL_UP,		KEY_VOL_UP,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_REVERB_OPEN,	KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				record 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_RECORD_EN
const u16 record_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_CHANGE_MODE, KEY_POWEROFF,		KEY_POWEROFF_HOLD,	KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,			KEY_VOL_DOWN,	KEY_VOL_DOWN,	KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [2] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [3] = {
        KEY_NULL,			KEY_VOL_UP,		KEY_VOL_UP,		KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_ENC_START,			KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				rtc 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_RTC_EN
const u16 rtc_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [3] = {
        KEY_CHANGE_MODE, KEY_POWEROFF,		KEY_POWEROFF_HOLD,	KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_RTC_DOWN,		KEY_RTC_DOWN,	KEY_RTC_DOWN,	KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_RTC_SW_POS,		KEY_RTC_SW,		KEY_NULL,		KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [0] = {
        KEY_RTC_UP,			KEY_RTC_UP,		KEY_RTC_UP,		KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_REVERB_OPEN,				KEY_NULL
    },
    [5] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [6] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				spdif 模式的 adkey table
 ***********************************************************/
#if TCFG_APP_SPDIF_EN
const u16 spdif_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_CHANGE_MODE, KEY_POWEROFF,		KEY_POWEROFF_HOLD,	KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [2] = {
        KEY_MUSIC_PP,		KEY_POWEROFF,	KEY_POWEROFF_HOLD,	KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [3] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [4] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL,		KEY_SPDIF_SW_SOURCE,	KEY_NULL
    },
    [5] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL,		KEY_NULL,				KEY_NULL
    },
    [6] = {
        KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL,		KEY_SPDIF_SW_SOURCE,	KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,			KEY_NULL,			KEY_NULL,		KEY_NULL,			KEY_NULL
    },
};
#endif

///2020-11-13
#if 1//USER_CAR_AUDIO_POWEROFF_EN
/***********************************************************
 *				poweroff 模式的 adkey table
 ***********************************************************/
const u16 poweroff_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
//    [KEY_BOARD_POWER_MODE] = {
//        KEY_POWER_ON,
//    },
    [KEY_BOARD_POWER] = {
        KEY_POWER_ON,
    },
    [KEY_BOARD_SYSRESET] = {
        KEY_NULL,       KEY_SYSTEM_RESET,
    },
};
#endif

/***********************************************************
 *				idle 模式的 adkey table
 ***********************************************************/
const u16 idle_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
//    [KEY_BOARD_POWER_MODE] = {
//        KEY_NULL,
//    },
    [KEY_BOARD_SYSRESET] = {
        KEY_NULL,       KEY_SYSTEM_RESET,
    },
};
#endif
