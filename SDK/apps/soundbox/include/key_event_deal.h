#ifndef __KEY_EVENT_DEAL_H__
#define __KEY_EVENT_DEAL_H__

#include "typedef.h"
#include "system/event.h"

enum {
    KEY_POWER_ON = 0x80,//从0x80开始,避免与系统默认事件冲突
    KEY_POWER_ON_HOLD,
    KEY_POWEROFF,
    KEY_POWEROFF_HOLD,
    KEY_BT_DIRECT_INIT,
    KEY_BT_DIRECT_CLOSE,
    KEY_MUSIC_PP,
    KEY_MUSIC_PREV,
    KEY_MUSIC_NEXT,
    KEY_MUSIC_FF,
    KEY_MUSIC_FR,
    KEY_MUSIC_PLAYER_START,
    KEY_MUSIC_PLAYER_END,
    KEY_MUSIC_DEVICE_TONE_END,
    KEY_MUSIC_PLAYER_QUIT,
    KEY_MUSIC_PLAYER_AUTO_NEXT,
    KEY_MUSIC_PLAYER_PLAY_FIRST,
    KEY_MUSIC_PLAYER_PLAY_LAST,
    KEY_MUSIC_CHANGE_REPEAT,
    KEY_MUSIC_CHANGE_DEV,
    KEY_MUSIC_AUTO_NEXT_DEV,
    KEY_MUSIC_CHANGE_DEV_REPEAT,
    KEY_MUSIC_SET_PITCH,
    KEY_MUSIC_SET_SPEED,
    KEY_MUSIC_PLAYE_BY_DEV_FILENUM,
    KEY_MUSIC_PLAYE_BY_DEV_SCLUST,
    KEY_MUSIC_PLAYE_BY_DEV_PATH,
    KEY_MUSIC_DELETE_FILE,
    KEY_MUSIC_PLAYE_NEXT_FOLDER,
    KEY_MUSIC_PLAYE_PREV_FOLDER,
    KEY_MUSIC_PLAYE_REC_FOLDER_SWITCH,
    KEY_MUSIC_ID3,
    KEY_MUSIC_CHANGE_MODE,

    KEY_VOL_UP,
    KEY_VOL_DOWN,
    KEY_CALL_LAST_NO,
    KEY_CALL_HANG_UP,
    KEY_CALL_ANSWER,
    KEY_OPEN_SIRI,
    KEY_HID_CONTROL,
    KEY_LOW_LANTECY,
    KEY_CHANGE_MODE,

    KEY_EQ_MODE,
    KEY_LOUD,
    KEY_THIRD_CLICK,

    KEY_FM_SCAN_ALL,
    KEY_FM_SCAN_ALL_UP,
    KEY_FM_SCAN_ALL_DOWN,
    KEY_FM_PREV_STATION,
    KEY_FM_NEXT_STATION,
    KEY_FM_PREV_FREQ,
    KEY_FM_NEXT_FREQ,
    KEY_FM_SCAN_UP,//半自动搜台
    KEY_FM_SCAN_DOWN,//半自动搜台
    KEY_FM_CHANNEL_1,
    KEY_FM_CHANNEL_2,
    KEY_FM_CHANNEL_3,
    KEY_FM_CHANNEL_4,
    KEY_FM_CHANNEL_5,
    KEY_FM_CHANNEL_6,
    KEY_FM_SCHANNEL_1,
    KEY_FM_SCHANNEL_2,
    KEY_FM_SCHANNEL_3,
    KEY_FM_SCHANNEL_4,
    KEY_FM_SCHANNEL_5,
    KEY_FM_SCHANNEL_6,
    KEY_FM_PREV_STEP_LONG,
    KEY_FM_PREV_STEP_HOLD,
    KEY_FM_PREV_STEP_LONGUP,
    KEY_FM_NEXT_STEP_LONG,
    KEY_FM_NEXT_STEP_HOLD,
    KEY_FM_NEXT_STEP_LONGUP,
    KEY_FM_BAND,
    KEY_FM_AUTO_PLAY,
    KEY_FM_ST,


    KEY_FM_EMITTER_MENU,
    KEY_FM_EMITTER_NEXT_FREQ,
    KEY_FM_EMITTER_PERV_FREQ,

    KEY_RTC_UP,
    KEY_RTC_DOWN,
    KEY_RTC_SW,
    KEY_RTC_SW_POS,

    KEY_SPDIF_SW_SOURCE,

    KEY_BT_EMITTER_SW,
    KEY_BT_EMITTER_PLAY,
    KEY_BT_EMITTER_PAUSE,
    KEY_BT_EMITTER_RECEIVER_SW,

    KEY_SWITCH_PITCH_MODE,
    KEY_ENC_START,
    KEY_REVERB_OPEN,
    KEY_REVERB_DEEPVAL_UP,
    KEY_REVERB_DEEPVAL_DOWN,
    KEY_REVERB_GAIN0_UP,
    KEY_REVERB_GAIN1_UP,
    KEY_REVERB_GAIN2_UP,
    // KEY_REVERB_GAIN_DOWN,

    KEY_TM_GMA_SEND,
    KEY_SEND_SPEECH_START,
    KEY_AI_DEC_SUSPEND,
    KEY_AI_DEC_RESUME,
    KEY_DUEROS_VER,
    KEY_DUEROS_SEND,
    KEY_TWS_DUEROS_RAND_SET,
    KEY_TWS_BLE_SLAVE_SPEECH_START,
    KEY_SPEECH_START_FROM_TWS,
    KEY_SPEECH_STOP_FROM_TWS,
    KEY_TWS_BLE_DUEROS_CONNECT,
    KEY_TWS_BLE_DUEROS_DISCONNECT,

    KEY_TWS_SEARCH_PAIR,
    KEY_TWS_REMOVE_PAIR,
    KEY_TWS_SEARCH_REMOVE_PAIR,
    KEY_TWS_DISCONN,
    KEY_TWS_CONN,

    KEY_BOX_POWER_CLICK,
    KEY_BOX_POWER_LONG,
    KEY_BOX_POWER_HOLD,
    KEY_BOX_POWER_UP,

    KEY_ELECTRIC_MODE,
    KEY_PITCH_MODE,
    KEY_MAGIC_MODE,
    KEY_BOOM_MODE,
    KEY_MIC_PRIORITY_MODE,
    KEY_DODGE_MODE,
    KEY_USB_MIC_CH_SWITCH,
    KEY_TONE_huanhu,
    KEY_TONE_ganga,
    KEY_TONE_qiangsheng,
    KEY_TONE_bishi,
    KEY_TONE_chuchang,
    KEY_TONE_feiwen,
    KEY_TONE_xiaosheng,
    KEY_TONE_zhangsheng,
    KEY_TONE_guanzhu,
    KEY_TONE_momoda,
    KEY_TONE_zeilala,
    KEY_TONE_feichengwurao,
    KEY_KTV_TEST,

    KEY_TEST_DEMO_0,
    KEY_TEST_DEMO_1,

    KEY_IR_NUM_0,  //中间不允许插入
    KEY_IR_NUM_1,
    KEY_IR_NUM_2,
    KEY_IR_NUM_3,
    KEY_IR_NUM_4,
    KEY_IR_NUM_5,
    KEY_IR_NUM_6,
    KEY_IR_NUM_7,
    KEY_IR_NUM_8,
    KEY_IR_NUM_9,//中间不允许插入
    //在这里增加元素
    //
    KEY_HID_MODE_SWITCH,
    KEY_HID_TAKE_PICTURE,
    KEY_LINEIN_START,

    KEY_MENU_VOL_UP,
    KEY_MENU_VOL_DOWN,
    KEY_MENU_BASS_UP,
    KEY_MENU_BASS_DOWN,
    KEY_ENCODER_UP,
    KEY_ENCODER_DOWN,
    KEY_MENU_CHANGE,
    KEY_MODE_HANG_UP,
    KEY_MUSIC_INT,
    KEY_MUSIC_RPT,
    KEY_MUSIC_RDM,
    KEY_MUSIC_PREV_10,
    KEY_MUSIC_NEXT_10,
    KEY_MUSIC_GET_TYPE,
    KEY_CLK,
    KEY_CLK_SET,
    KEY_MUTE,
    KEY_SYSTEM_RESET,

    KEY_CHANGE_TO_FM,
    KEY_CHANGE_TO_LINEIN,
    KEY_CHANGE_TO_BT,
    KEY_CHANGE_TO_MUSIC,
    KEY_CHANGE_TO_MUSIC_USB,
    KEY_CHANGE_TO_MUSIC_SD,

    BLE_VOL_SET,
    BLE_EQ_SET,
    BLE_RGB_SET,

    //不会出现在按键主流程，用于不重要得其他操作
    KEY_MINOR_OPT,

    KEY_NULL = 0xFFFF,

    KEY_MSG_MAX = 0xFFFF,
    //音箱sdk 按键消息已经加大为0xffff
};


enum {
    ONE_KEY_CTL_NEXT_PREV = 1,
    ONE_KEY_CTL_VOL_UP_DOWN,
};


#endif
