#include "key_event_deal.h"
#include "key_driver.h"
#include "app_config.h"
#include "board_config.h"
#include "app_task.h"

#ifdef CONFIG_BOARD_AC695X_DEMO
enum{
#if 0
    IR_K_POWER = 0,
    IR_K_MODE,
    IR_K_MUTE,
    IR_K_PLAY,
    IR_K_PREV,
    IR_K_NEXT,
    IR_K_EQ,
    IR_K_VOL_DOWN,
    IR_K_VOL_UP,
    IR_K_M0,
    IR_K_RPT,
    IR_K_USB_SD,
    IR_K_M1,
    IR_K_M2,
    IR_K_M3,
    IR_K_M4,
    IR_K_M5,
    IR_K_M6,
    IR_K_M7,
    IR_K_M8,
    IR_K_M9,
#else
    IR_K_POWER = 0,
    IR_K_MUTE,
    IR_K_MODE,
    IR_K_M1,
    IR_K_M2,
    IR_K_M3,
    IR_K_M4,
    IR_K_M5,
    IR_K_M6,
    IR_K_LOUD,
    IR_K_VOL_UP,
    IR_K_EQ,
    IR_K_PREV,
    IR_K_SEL,
    IR_K_NEXT,
    IR_K_AMS,
    IR_K_VOL_DOWN,
    IR_K_BAND,
    IR_K_STOP,
    IR_K_ST,
    IR_K_CLK,
#endif
};

/***********************************************************
 *				bt 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_BT_EN
const u16 bt_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [IR_K_POWER] =      {KEY_POWEROFF},
    [IR_K_MODE] =       {KEY_CHANGE_MODE},
    [IR_K_MUTE] =       {KEY_MUTE},
//    [IR_K_PLAY] =       {KEY_MUSIC_PP},
    [IR_K_PREV] =       {KEY_MUSIC_PREV},
    [IR_K_NEXT] =       {KEY_MUSIC_NEXT},
    [IR_K_EQ] =         {KEY_EQ_MODE},
    [IR_K_VOL_DOWN] =   {KEY_ENCODER_DOWN,	KEY_ENCODER_DOWN,   KEY_ENCODER_DOWN},
    [IR_K_VOL_UP] =     {KEY_ENCODER_UP,	KEY_ENCODER_UP,		KEY_ENCODER_UP},
//    [IR_K_M0] =         {KEY_IR_NUM_0},
//    [IR_K_RPT] =        {KEY_CALL_ANSWER,   KEY_CALL_LAST_NO},
//    [IR_K_USB_SD] =     {KEY_CALL_HANG_UP},
//    [IR_K_M1] =         {KEY_IR_NUM_1},
//    [IR_K_M2] =         {KEY_IR_NUM_2},
//    [IR_K_M3] =         {KEY_IR_NUM_3},
//    [IR_K_M4] =         {KEY_IR_NUM_4},
//    [IR_K_M5] =         {KEY_IR_NUM_5},
//    [IR_K_M6] =         {KEY_IR_NUM_6},
//    [IR_K_M7] =         {KEY_IR_NUM_7},
//    [IR_K_M8] =         {KEY_IR_NUM_8},
//    [IR_K_M9] =         {KEY_IR_NUM_9},
    [IR_K_M1] =         {KEY_MUSIC_PP},
    [IR_K_LOUD] =       {KEY_LOUD},
    [IR_K_SEL] =        {KEY_MENU_CHANGE},
    [IR_K_CLK] =        {KEY_CLK,           KEY_CLK_SET},
};
#endif

/***********************************************************
 *				fm 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_FM_EN
const u16 fm_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [IR_K_POWER] =      {KEY_POWEROFF},
    [IR_K_MODE] =       {KEY_CHANGE_MODE},
    [IR_K_MUTE] =       {KEY_MUTE},
//    [IR_K_PLAY] =       {KEY_MUSIC_PP},
    [IR_K_PREV] =       {KEY_FM_SCAN_UP,    KEY_FM_PREV_STEP_LONG,KEY_FM_PREV_STEP_HOLD,KEY_FM_PREV_STEP_LONGUP},
    [IR_K_NEXT] =       {KEY_FM_SCAN_DOWN,  KEY_FM_NEXT_STEP_LONG,KEY_FM_NEXT_STEP_HOLD,KEY_FM_NEXT_STEP_LONGUP},
    [IR_K_EQ] =         {KEY_EQ_MODE},
    [IR_K_VOL_DOWN] =   {KEY_ENCODER_DOWN,	KEY_ENCODER_DOWN,   KEY_ENCODER_DOWN},
    [IR_K_VOL_UP] =     {KEY_ENCODER_UP,	KEY_ENCODER_UP,		KEY_ENCODER_UP},
//    [IR_K_M0] =         {KEY_IR_NUM_0},
//    [IR_K_RPT] =        {KEY_MUSIC_CHANGE_REPEAT},
//    [IR_K_USB_SD] =     {KEY_MUSIC_CHANGE_DEV},
    [IR_K_M1] =         {KEY_FM_CHANNEL_1,KEY_FM_SCHANNEL_1},
    [IR_K_M2] =         {KEY_FM_CHANNEL_2,KEY_FM_SCHANNEL_2},
    [IR_K_M3] =         {KEY_FM_CHANNEL_3,KEY_FM_SCHANNEL_3},
    [IR_K_M4] =         {KEY_FM_CHANNEL_4,KEY_FM_SCHANNEL_4},
    [IR_K_M5] =         {KEY_FM_CHANNEL_5,KEY_FM_SCHANNEL_5},
    [IR_K_M6] =         {KEY_FM_CHANNEL_6,KEY_FM_SCHANNEL_6},
//    [IR_K_M7] =         {KEY_IR_NUM_7},
//    [IR_K_M8] =         {KEY_IR_NUM_8},
//    [IR_K_M9] =         {KEY_IR_NUM_9},
    [IR_K_AMS] =        {KEY_FM_AUTO_PLAY,  KEY_FM_SCAN_ALL_DOWN},
    [IR_K_BAND] =       {KEY_FM_BAND},
    [IR_K_LOUD] =       {KEY_LOUD},
    [IR_K_SEL] =        {KEY_MENU_CHANGE},
    [IR_K_CLK] =        {KEY_CLK,           KEY_CLK_SET},
    [IR_K_ST] =         {KEY_FM_ST},
};
#endif

/***********************************************************
 *				linein 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_LINEIN_EN
const u16 linein_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [IR_K_POWER] =      {KEY_POWEROFF},
    [IR_K_MODE] =       {KEY_CHANGE_MODE},
    [IR_K_MUTE] =       {KEY_MUTE},
//    [IR_K_PLAY] =       {KEY_MUSIC_PP},
//    [IR_K_PREV] =       {KEY_MUSIC_PREV,	KEY_MUSIC_FR,	    KEY_MUSIC_FR},
//    [IR_K_NEXT] =       {KEY_MUSIC_NEXT,	KEY_MUSIC_FF,	    KEY_MUSIC_FF},
    [IR_K_EQ] =         {KEY_EQ_MODE},
    [IR_K_VOL_DOWN] =   {KEY_ENCODER_DOWN,	KEY_ENCODER_DOWN,   KEY_ENCODER_DOWN},
    [IR_K_VOL_UP] =     {KEY_ENCODER_UP,	KEY_ENCODER_UP,		KEY_ENCODER_UP},
//    [IR_K_M0] =         {KEY_IR_NUM_0},
//    [IR_K_RPT] =        {KEY_MUSIC_CHANGE_REPEAT},
//    [IR_K_USB_SD] =     {KEY_MUSIC_CHANGE_DEV},
//    [IR_K_M1] =         {KEY_IR_NUM_1},
//    [IR_K_M2] =         {KEY_IR_NUM_2},
//    [IR_K_M3] =         {KEY_IR_NUM_3},
//    [IR_K_M4] =         {KEY_IR_NUM_4},
//    [IR_K_M5] =         {KEY_IR_NUM_5},
//    [IR_K_M6] =         {KEY_IR_NUM_6},
//    [IR_K_M7] =         {KEY_IR_NUM_7},
//    [IR_K_M8] =         {KEY_IR_NUM_8},
//    [IR_K_M9] =         {KEY_IR_NUM_9},
    [IR_K_LOUD] =       {KEY_LOUD},
    [IR_K_SEL] =        {KEY_MENU_CHANGE},
    [IR_K_CLK] =        {KEY_CLK,           KEY_CLK_SET},
};
#endif

/***********************************************************
 *				music 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_MUSIC_EN
const u16 music_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [IR_K_POWER] =      {KEY_POWEROFF},
    [IR_K_MODE] =       {KEY_CHANGE_MODE},
    [IR_K_MUTE] =       {KEY_MUTE},
//    [IR_K_PLAY] =       {KEY_MUSIC_PP},
    [IR_K_PREV] =       {KEY_MUSIC_PREV,	KEY_MUSIC_FR,	    KEY_MUSIC_FR},
    [IR_K_NEXT] =       {KEY_MUSIC_NEXT,	KEY_MUSIC_FF,	    KEY_MUSIC_FF},
    [IR_K_EQ] =         {KEY_EQ_MODE},
    [IR_K_VOL_DOWN] =   {KEY_ENCODER_DOWN,	KEY_ENCODER_DOWN,   KEY_ENCODER_DOWN},
    [IR_K_VOL_UP] =     {KEY_ENCODER_UP,	KEY_ENCODER_UP,		KEY_ENCODER_UP},
//    [IR_K_M0] =         {KEY_IR_NUM_0},
//    [IR_K_RPT] =        {KEY_MUSIC_CHANGE_REPEAT},
//    [IR_K_USB_SD] =     {KEY_MUSIC_CHANGE_DEV},
//    [IR_K_M1] =         {KEY_IR_NUM_1},
//    [IR_K_M2] =         {KEY_IR_NUM_2},
//    [IR_K_M3] =         {KEY_IR_NUM_3},
//    [IR_K_M4] =         {KEY_IR_NUM_4},
//    [IR_K_M5] =         {KEY_IR_NUM_5},
//    [IR_K_M6] =         {KEY_IR_NUM_6},
//    [IR_K_M7] =         {KEY_IR_NUM_7},
//    [IR_K_M8] =         {KEY_IR_NUM_8},
//    [IR_K_M9] =         {KEY_IR_NUM_9},
    [IR_K_M1] =         {KEY_MUSIC_PP},
//    [IR_K_M2] =         {KEY_MUSIC_INT},
    [IR_K_M3] =         {KEY_MUSIC_RPT},
    [IR_K_M4] =         {KEY_MUSIC_RDM},
    [IR_K_M5] =         {KEY_MUSIC_PREV_10},
    [IR_K_M6] =         {KEY_MUSIC_NEXT_10},
    [IR_K_LOUD] =       {KEY_LOUD},
    [IR_K_SEL] =        {KEY_MENU_CHANGE},
    [IR_K_CLK] =        {KEY_CLK,           KEY_CLK_SET},
};
#endif

/***********************************************************
 *				poweroff 模式的 irkey table
 ***********************************************************/
const u16 poweroff_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [IR_K_POWER] =      {KEY_POWER_ON},
};

/***********************************************************
 *				pc 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_PC_EN
const u16 pc_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [0] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [1] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [2] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [3] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [10] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [11] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [12] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [13] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [14] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [15] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [16] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [17] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [18] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [19] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [20] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
};
#endif

/***********************************************************
 *				record 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_RECORD_EN
const u16 record_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [0] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [1] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [2] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [3] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [10] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [11] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [12] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [13] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [14] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [15] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [16] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [17] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [18] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [19] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [20] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
};
#endif

/***********************************************************
 *				rtc 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_RTC_EN
const u16 rtc_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [0] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [1] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [2] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [3] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [10] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [11] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [12] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [13] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [14] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [15] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [16] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [17] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [18] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [19] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [20] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
};
#endif

/***********************************************************
 *				spdif 模式的 irkey table
 ***********************************************************/
#if TCFG_APP_SPDIF_EN
const u16 spdif_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [0] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [1] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [2] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [3] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [10] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [11] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [12] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [13] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [14] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [15] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [16] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [17] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [18] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [19] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [20] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
};
#endif

/***********************************************************
 *				idle 模式的 irkey table
 ***********************************************************/
const u16 idle_key_ir_table[KEY_IR_NUM_MAX][KEY_EVENT_MAX] = {
    [0] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [1] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [2] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [3] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [4] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [5] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [6] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [7] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [8] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [9] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [10] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [11] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [12] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [13] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [14] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [15] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [16] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [17] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [18] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [19] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
    [20] = {
        KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL,		KEY_NULL
    },
};
#endif
