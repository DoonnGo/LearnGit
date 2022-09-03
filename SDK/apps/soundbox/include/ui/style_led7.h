#ifndef UI_STYLE_LED7_H
#define UI_STYLE_LED7_H

#include "ui/ui_common.h"
#include "ui/led7/led7_driver.h"
#include "ui/lcd_seg/lcd_seg3x9_driver.h"



enum ui_menu_main {
    UI_MENU_MAIN_NULL = 0,
    UI_RTC_MENU_MAIN,
    UI_MUSIC_MENU_MAIN,
    UI_AUX_MENU_MAIN,
    UI_BT_MENU_MAIN,
    UI_RECORD_MENU_MAIN,
    UI_FM_MENU_MAIN,
    UI_PC_MENU_MAIN,
    UI_IDLE_MENU_MAIN,
};





enum {

    MENU_POWER_UP = 0x1,
    MENU_WAIT,
    MENU_BT,
    MENU_PC,
    MENU_PC_VOL_UP,
    MENU_PC_VOL_DOWN,
    MENU_AUX,
    MENU_ALM_UP,

    MENU_SHOW_STRING,
    MENU_MAIN_VOL,
    MENU_BASS,
    MENU_TREBLE,
    MENU_EQ_FREQ_1,
    MENU_EQ_FREQ_2,
    MENU_EQ_FREQ_3,
    MENU_EQ_FREQ_4,
    MENU_EQ_FREQ_5,
    MENU_EQ_FREQ_6,
    MENU_EQ_FREQ_7,
    MENU_BAL,
    MENU_FAD,
    MENU_LOUD,
    MENU_IFX,
    MENU_SET_EQ,
    MENU_RTC_SHOW,
    MENU_RTC_SET_HOUR,
    MENU_RTC_SET_MIN,
    MENU_SET_PLAY_MODE,

    MENU_PLAY_TIME,
    MENU_FILENUM,
    MENU_FOLDER_NUM,
    MENU_DEV_LOGO,
    MENU_INPUT_NUMBER,
    MENU_MUSIC_PAUSE,
    MENU_MUSIC_REPEATMODE,
    MENU_MUSIC_ID3,

    MENU_FM_MAIN,
    MENU_FM_DISP_FRE,
    MENU_FM_SET_FRE,
    MENU_FM_STATION,
    MENU_IR_FM_SET_FRE,
    MENU_FM_STEP_IN,
    MENU_FM_STEP_OUT,

    MENU_RTC_SET,
    MENU_RTC_PWD,
    MENU_ALM_SET,

    MENU_BT_SEARCH_DEVICE,
    MENU_BT_CONNECT_DEVICE,
    MENU_BT_DEVICE_ADD,
    MENU_BT_DEVICE_NAME,
    MENU_RECODE_MAIN,
    MENU_RECODE_ERR,
    MENU_POWER,
    MENU_LIST_DISPLAY,
    MENU_BT_INPUT_NUMBER,   ///2020-11-28 输入数字拨号


    MENU_LED0,
    MENU_LED1,


    MENU_RECORD,

    MENU_USER_INFO,

    MENU_SEC_REFRESH = 0x80,
    MENU_REFRESH,
    MENU_MAIN = 0xff,
};







//=================================================================================//
//                        			UI 配置数据结构                    			   //
//=================================================================================//
struct ui_dis_api {
    int ui;
    void *(*open)(void *hd);
    void (*ui_main)(void *hd, void *private);
    int (*ui_user)(void *hd, void *private, int menu, int arg);
    void (*close)(void *hd, void *private);
};



typedef struct _LCD_DISP_API {
    void (*clear)(void);
    void (*setXY)(u32 x, u32 y);
    void (*FlashChar)(u32);
    void (*Clear_FlashChar)(u32);
    void (*show_string)(u8 *);
    void (*show_char)(u8);
    void (*show_number)(u8);
    void (*show_icon)(u32);
    void (*flash_icon)(u32);
    void (*clear_icon)(u32);
    void (*show_pic)(u32);
    void (*hide_pic)(u32);
    void (*lock)(u32);
    bool (*flash_500ms)(void);
    u16  (*rtc_showtime_get)(void);
    void (*ignore_next_flash)(void);
    void (*show_fft)(u8);
    void (*show_fft_level)(u8);
} LCD_API;



extern const struct ui_dis_api bt_main;
extern const struct ui_dis_api fm_main;
extern const struct ui_dis_api music_main;
extern const struct ui_dis_api record_main;
extern const struct ui_dis_api rtc_main;
extern const struct ui_dis_api pc_main;
extern const struct ui_dis_api linein_main;
extern const struct ui_dis_api idle_main;




#endif


