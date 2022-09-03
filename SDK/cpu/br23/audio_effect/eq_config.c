
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_online_cfg.h"
#include "online_db/online_db_deal.h"
#include "application/audio_eq_drc_apply.h"
#include "app_main.h"
#include "ble_module.h"

/* #define LOG_TAG     "[APP-EQ]" */
/* #define LOG_ERROR_ENABLE */
/* #define LOG_INFO_ENABLE */
/* #define LOG_DUMP_ENABLE */
/* #include "debug.h" */

const u8 audio_eq_sdk_name[16] 		= "AC695N";
#if TCFG_EQ_DIVIDE_ENABLE
const u8 audio_eq_ver[4] 			= {0, 7, 3, 0};//四声道独立eq版本
#else
const u8 audio_eq_ver[4] 			= {0, 7, 2, 0};//四声道eq使用同一效果、立体声、单声道的eq版本
#endif

#if (TCFG_EQ_ENABLE != 0)

#if !TCFG_USE_EQ_FILE
const struct eq_seg_info eq_tab_normal[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 << 20, (int)(0.7f * (1 << 24))},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif
};

const struct eq_seg_info eq_tab_rock[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    -2 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    2 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -2 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -2 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4 << 20, (int)(0.7f * (1 << 24))},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

};

const struct eq_seg_info eq_tab_pop[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     3 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     1 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   -2 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -4 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -4 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  -2 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   1 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2 << 20, (int)(0.7f * (1 << 24))},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

};

const struct eq_seg_info eq_tab_classic[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     8 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    8 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    0 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   2 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2 << 20, (int)(0.7f * (1 << 24))},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

};

const struct eq_seg_info eq_tab_country[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     -2 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    2 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    2 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4 << 20, (int)(0.7f * (1 << 24))},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif
};

const struct eq_seg_info eq_tab_jazz[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    4 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   4 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   2 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   3 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4 << 20, (int)(0.7f * (1 << 24))},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif


};
struct eq_seg_info eq_tab_custom[] = {
#if 0
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 100/*125*/,   0 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 200/*250*/,   0 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 3000/*2000*/,  0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 << 20, (int)(0.7f * (1 << 24))},
#else
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0 << 20, (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0 << 20, (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0 << 20, (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 << 20, (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 << 20, (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0 << 20, (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 << 20, (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 << 20, (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

#if (EQ_SECTION_MAX > 20)
    {20, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {21, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {22, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {23, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {24, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {25, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {26, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {27, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {28, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {29, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {30, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
    {31, EQ_IIR_TYPE_BAND_PASS, 96000, 0 << 20, (int)(0.7f * (1 << 24))},
#endif

};


const EQ_CFG_SEG *eq_type_tab[EQ_MODE_MAX] = {
    eq_tab_normal, eq_tab_rock, eq_tab_pop, eq_tab_classic, eq_tab_jazz, eq_tab_country, eq_tab_custom
};
#endif

__attribute__((weak)) u32 get_eq_mode_tab(void)
{
#if !TCFG_USE_EQ_FILE
    return (u32)eq_type_tab;
#else
    return 0;
#endif
}

#if (EQ_SECTION_MAX==9)
static const u8 eq_mode_use_idx[] = {
    0,	1,	2,	3,	4,	5,	/*6,*/	7,	8,	9
};
#elif (EQ_SECTION_MAX==8)
static const u8 eq_mode_use_idx[] = {
    0,	/*1,*/	2,	3,	4,	5,	6,	7,	/*8,*/	9
};
#elif (EQ_SECTION_MAX==7)
static const u8 eq_mode_use_idx[] = {
    0,	/*1,*/	2,	3,	4,	5,	/*6,*/	7,	/*8,*/	9
};
#elif (EQ_SECTION_MAX==6)
static const u8 eq_mode_use_idx[] = {
    0,	/*1,*/	2,	3,	4,	/*5,*/	/*6,*/	7,	/*8,*/	9
};
#elif (EQ_SECTION_MAX==5)
static const u8 eq_mode_use_idx[] = {
    /*0,*/	1,	/*2,*/	3,	/*4,*/	5,	/*6,*/	7,	/*8,*/	9
};
#else
static const u8 eq_mode_use_idx[] = {
    0,	1,	2,	3,	4,	5,	6,	7,	8,	9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
#endif
/*
 *通话下行eq系数表
 * */
#if TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE
const struct eq_seg_info phone_eq_tab_normal[] = {
    {0, EQ_IIR_TYPE_HIGH_PASS, 200,   0 << 20, (int)(0.7f  * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0 << 20, (int)(0.7f  * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0 << 20, (int)(0.7f  * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 450,   0 << 20, (int)(0.7f  * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 << 20, (int)(0.7f  * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 << 20, (int)(0.7f  * (1 << 24))},
};
#endif

/*
 *通话上行eq系数表
 * */
const struct eq_seg_info ul_eq_tab_normal[] = {
    {0, EQ_IIR_TYPE_HIGH_PASS, 200,   0 << 20, (int)(0.7f  * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0 << 20, (int)(0.7f  * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0 << 20, (int)(0.7f  * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 450,   0 << 20, (int)(0.7f  * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 << 20, (int)(0.7f  * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 << 20, (int)(0.7f  * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 << 20, (int)(0.7f  * (1 << 24))},
};



#define SONG_SECTION  EQ_SECTION_MAX
#define CALL_SECTION  3//下行段数,小于等于SONG_SECTION
#define UL_SECTION    3//上行段数,小于等于SONG_SECTION
/*
 *下行的宽频和窄频段数需一致，上行的宽频和窄频段数需要一致
 *表的每一项顺序不可修改
 * */
eq_tool_cfg eq_tool_tab[] = {
    {call_eq_mode,	(u8 *)"通话宽频下行EQ", 0x3000, CALL_SECTION, 1, {EQ_ONLINE_CMD_CALL_EQ_SEG, 0}},
    {call_narrow_eq_mode,	(u8 *)"通话窄频下行EQ", 0x3001, CALL_SECTION, 1, {EQ_ONLINE_CMD_CALL_EQ_SEG, 0}},
    {aec_eq_mode,	(u8 *)"通话宽频上行EQ", 0x3002, UL_SECTION,   1, {EQ_ONLINE_CMD_AEC_EQ_SEG,  0}},
    {aec_narrow_eq_mode,	(u8 *)"通话窄频上行EQ", 0x3003, UL_SECTION,   1, {EQ_ONLINE_CMD_AEC_EQ_SEG,  0}},
    {song_eq_mode,	(u8 *)"普通音频EQ", 	0x3004, SONG_SECTION, 2, {EQ_ONLINE_CMD_SONG_EQ_SEG, EQ_ONLINE_CMD_SONG_DRC}},
#ifdef DAC_OUTPUT_FRONT_LR_REAR_LR
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
    {fr_eq_mode,	(u8 *)"FR_EQ",      	0x3005, SONG_SECTION, 2, {EQ_ONLINE_CMD_SONG_EQ_SEG, EQ_ONLINE_CMD_SONG_DRC}},
#if TCFG_EQ_DIVIDE_ENABLE
    {rl_eq_mode,	(u8 *)"RL_EQ",      	0x3006, SONG_SECTION, 2, {EQ_ONLINE_CMD_SONG_EQ_SEG, EQ_ONLINE_CMD_SONG_DRC}},
    {rr_eq_mode,	(u8 *)"RR_EQ",      	0x3007, SONG_SECTION, 2, {EQ_ONLINE_CMD_SONG_EQ_SEG, EQ_ONLINE_CMD_SONG_DRC}},
#endif
#endif
#endif
};

/*----------------------------------------------------------------------------*/
/**@brief    eq 段数更新,需要在eq_init前就准备好
   @param    mode:call_eq_mode\call_narrow_eq_section等模式
   @param    section:段数最大为EQ_SECTION_MAX
   @return
   @note     下行的宽频和窄频段数需一致，上行的宽频和窄频段数需要一致
*/
/*----------------------------------------------------------------------------*/
void set_eq_tool_tab_section(u8 mode, u8 section)
{
#if TCFG_EQ_ONLINE_ENABLE
    eq_tool_tab[mode].section = section;
#endif
}

void drc_default_init(EQ_CFG *eq_cfg, u8 mode)
{
#if TCFG_DRC_ENABLE
    int i = mode;
    if (eq_cfg && eq_cfg->drc) {
        //限幅器的初始值
        int th = 0;//db -60db~0db
        int threshold = round(pow(10.0, th / 20.0) * 32768); // 0db:32768, -60db:33
        eq_cfg->cfg_parm[i].drc_parm.parm.drc.nband = 1;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc.type = 1;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].attacktime = 5;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].releasetime = 500;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].threshold[0] = threshold;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].threshold[1] = 32768;
    }
#endif

}

int eq_init(void)
{
    audio_eq_init();
    eq_adjust_parm parm = {0};
#if TCFG_EQ_ONLINE_ENABLE
    parm.online_en = 1;
#endif
    parm.fade_en = 0;

#if TCFG_USE_EQ_FILE
    parm.file_en = 1;
#endif

#if TCFG_DRC_ENABLE
    parm.drc = 1;
#endif

#if TCFG_USER_TWS_ENABLE
    parm.tws = 1;
#endif

#if APP_ONLINE_DEBUG
    parm.app = 1;
#endif

#if (RCSP_ADV_EN)&&(JL_EARPHONE_APP_EN)&&(TCFG_DRC_ENABLE == 0)
    parm.limit_zero = 1;
#endif

#if TCFG_EQ_DIVIDE_ENABLE
    parm.stero = 1;
    parm.mode_num = 8;
#else

#endif

    if (!parm.stero) {
        parm.mode_num = 5;// 一共有多少个模式
        /* #ifdef DAC_OUTPUT_FRONT_LR_REAR_LR */
        /* #if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR) */
        /* parm.mode_num = 6; */
        /* #endif */
        /* #endif */
    }

#if TCFG_PHONE_EQ_ENABLE
    parm.phone_eq_tab = phone_eq_tab_normal;
    parm.phone_eq_tab_size = ARRAY_SIZE(phone_eq_tab_normal);
#endif
    parm.ul_eq_tab = ul_eq_tab_normal;
    parm.ul_eq_tab_size = ARRAY_SIZE(ul_eq_tab_normal);

    parm.eq_tool_tab = eq_tool_tab;


    parm.eq_mode_use_idx = eq_mode_use_idx;
    parm.eq_type_tab = (void *)get_eq_mode_tab();
    parm.type_num = EQ_MODE_MAX;

    parm.section_max = EQ_SECTION_MAX;

    EQ_CFG *eq_cfg = eq_cfg_open(&parm);
    if (eq_cfg) {
#if APP_ONLINE_DEBUG
        if (eq_cfg->app) {
            app_online_db_register_handle(DB_PKT_TYPE_EQ, eq_app_online_parse);
        }
#endif

        for (int i = 0; i < eq_cfg->mode_num; i++) {
            if (eq_cfg->eq_type == EQ_TYPE_MODE_TAB) {
                set_global_gain(eq_cfg, i, 0);
                drc_default_init(eq_cfg, i);
            }
        }
    }
    return 0;
}
__initcall(eq_init);

///2020-11-7
static u8 eq_read_data_from_vm(u8 syscfg_id, u8 *buf, u8 buf_len)
{
    int len = 0;
    u8 i = 0;

    len = syscfg_read(syscfg_id, buf, buf_len);

    if (len > 0) {
        for (i = 0; i < buf_len; i++) {
            if (buf[i] != 0xff) {
                return (buf_len == len);
            }
        }
    }

    return 0;
}

#define USER_EQ_LOUD_SECTION    2
#define USER_EQ_BASS_SECTION    3
#define USER_EQ_TREBLE_SECTION  6
static u8 app_get_eq_info(s8 *get_eq_info)
{
    u16 i;
    get_eq_info[0] = eq_mode_get_cur();
    u8 data_len = 1;

//    printf("================\n");
//    printf("app_get_eq_info:\n");

    if (10 == EQ_SECTION_MAX) {
        // 10段eq : mode + gain[10byte]
        for (i = 0; i < 10; i++) {
            get_eq_info[data_len] = eq_mode_get_gain(get_eq_info[0], i);
            data_len++;
        }
    } else {
        // 多段eq : mode + num + value(freq[2byte] + gain[1byte])
        get_eq_info[1] = EQ_SECTION_MAX;
//        printf("seg:%d\n", get_eq_info[1]);
        data_len++;
        for (i = 0; i < EQ_SECTION_MAX; i++) {
            if(i == USER_EQ_LOUD_SECTION || i == USER_EQ_BASS_SECTION || i == USER_EQ_TREBLE_SECTION)
                get_eq_info[data_len] = eq_mode_get_gain(get_eq_info[0], i);
//            printf("%d ", get_eq_info[data_len]);
            data_len++;
        }
        get_eq_info[0] |= (1 << 7);
    }
//    printf("\n");
//    printf("================\n");
    return data_len;
}

static u8 g_eq_setting_info[11] = {0};

///eq信息获取 2020-11-7
static void get_eq_setting(u8 *eq_setting)
{
    memcpy(eq_setting, g_eq_setting_info, 11);
}

///eq信息转换 2020-11-7
static void set_eq_setting(s8 *eq_setting)
{
    u8 i;
    u8 eq_setting_mode = eq_setting[0];
    if (eq_setting_mode >> 7) {
        // 多段eq
        for (i = 2; i < eq_setting[1] + 2; i++) {
            if ((eq_setting[i] > 12) && (eq_setting[i] < -12)) {
                eq_setting[i] = 0;
            }
        }
    } else {
        // 10段eq
        for (i = 1; i < 11; i++) {
            if ((eq_setting[i] > 12) && (eq_setting[i] < -12)) {
                eq_setting[i] = 0;
            }
        }
    }
    memcpy(g_eq_setting_info, eq_setting, 11);
}

///将eq信息写入VM 2020-11-7
static void update_eq_vm_value(u8 *eq_setting)
{
    u8 status = *(((u8 *)eq_setting) + 1);

    syscfg_write(CFG_RCSP_ADV_EQ_MODE_SETTING, eq_setting, 1);

    /*自定义修改EQ参数*/
    if (EQ_MODE_CUSTOM == (eq_setting[0] & 0x7F)) {
        if (eq_setting[0] >> 7) {
            // 多段eq
            status = eq_setting[2];
        }
        if (status != 0x7F) {
            syscfg_write(CFG_RCSP_ADV_EQ_DATA_SETTING, &eq_setting[1], 10);
        }
    }

}

///设置eq 2020-11-7
static void eq_setting_info_deal(u8 *eq_info_data)
{
    u8 data;
    u8 status;
    u8 mode;
    if (eq_info_data[0] >> 7) {
        status = eq_info_data[2];
    } else {
        status = *(((u8 *)eq_info_data) + 1);
    }
    mode = eq_info_data[0] & 0x7F;
    if (mode < EQ_MODE_CUSTOM) {
        eq_mode_set(mode);
    } else {
        // 自定义修改EQ参数
        if (EQ_MODE_CUSTOM == mode) {
            if (status != 0x7F)	{
                u8 i;
                for (i = 0; i < EQ_SECTION_MAX; i++) {
                    if (eq_info_data[0] >> 7) {
                        data = eq_info_data[i + 2];
                    } else {
                        data = eq_info_data[i + 1];
                    }
                    eq_mode_set_custom_param(i, (s8)data);
                }
//                printf("EQ:%d,sta:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",mode, eq_info_data[0] >> 7,
//                       eq_info_data[2],eq_info_data[3],eq_info_data[4],eq_info_data[5],eq_info_data[6],
//                       eq_info_data[7],eq_info_data[8],eq_info_data[9],eq_info_data[10],eq_info_data[11]);
            }
            eq_mode_set(mode);
        }
    }
}

///将eq_setting[]里的eq参数设置为实际eq值 2020-11-7
static void deal_eq_setting(s8 *eq_setting, u8 write_vm)
{
    u8 eq_info[11] = {0};
    if (!eq_setting) {
        get_eq_setting(eq_info);
    } else {
        u8 status = *(((u8 *)eq_setting) + 1);
        /*自定义修改EQ参数*/
        if (EQ_MODE_CUSTOM == (eq_setting[0] & 0x7F)) {
            printf("EQ_CUSTOM\n");
            if (eq_setting[0] >> 7) {
                status = eq_setting[2];
            }

            if (status != 0x7F) {
                printf("SET EQ_CUSTOM\n");
                memcpy(eq_info, eq_setting, 11);
                set_eq_setting(eq_info);
            } else {
                g_eq_setting_info[0] = eq_setting[0];
                get_eq_setting(eq_info);
            }
        } else {
            memcpy(eq_info, eq_setting, 11);
            g_eq_setting_info[0] = eq_setting[0];
        }

    }
#if 0   ///2020-12-5 不写入VM,测试排除复位问题
    if (write_vm) {
        update_eq_vm_value(eq_info);
    }
#endif

    eq_setting_info_deal(eq_info);
}

///eq自定义初始化 2020-11-7
void eq_setting_init(void)
{
    s8 eq_setting_vm_info[11] = {0};
    s8 eq_setting_info[10] = {0};
    s8 eq_setting_mode = 0;
    u8 i;
    s8 get_eq_info[22] = {0};

#if 0
    //从VM中获取eq模式及参数
    u8 vm_read_len = eq_read_data_from_vm(CFG_RCSP_ADV_EQ_DATA_SETTING, &eq_setting_info, sizeof(eq_setting_info));

    //获取到有内容，则设置读取的EQ值
    if (vm_read_len) {
        if (eq_read_data_from_vm(CFG_RCSP_ADV_EQ_MODE_SETTING, &eq_setting_mode, sizeof(eq_setting_mode))) {
            eq_setting_vm_info[0] = eq_setting_mode;
            memcpy(&eq_setting_vm_info[1], eq_setting_info, 10);
            set_eq_setting(eq_setting_vm_info);
            deal_eq_setting(NULL, 0);
        }
    }
    else //没有获取到内容则重新初始化并写入VM
#endif
    {
        eq_mode_set(EQ_MODE_NORMAL);
//        app_get_eq_info(get_eq_info);
//        deal_eq_setting(get_eq_info, 1);
    }
}

s8 User_get_eq_mode(void)
{
    return app_var.eq_mode;
}

s8 User_eq_dir(u8 dir)
{
    u8 mode = app_var.eq_mode;
    if(dir)
    {
        if(mode < EQ_MODE_CUSTOM)
            mode++;
        else
            mode = EQ_MODE_NORMAL + 1;
    }
    else
    {
        if(mode > EQ_MODE_NORMAL)
            mode--;
        if(mode == EQ_MODE_NORMAL)
            mode = EQ_MODE_CUSTOM;
    }
    User_eq_set(mode);
    return mode;
}

/*EQ:0:31, 1:62, 2:125, 3:250, 4:500, 5:1000, 6:2000, 7:4000, 8:8000, 9:16000*/

static void __User_eq_section_gain(u8 section, s8 gain)
{
    s8 get_eq_info[22] = {0};
//    if(eq_mode_get_cur() != EQ_MODE_CUSTOM)
//    {
//        eq_mode_set(EQ_MODE_CUSTOM);
//    }
#if 0
    app_get_eq_info(get_eq_info);
    get_eq_info[section + 2] = gain;
    deal_eq_setting(get_eq_info, 1);
#else
    eq_mode_set_custom_param(section, gain);
    eq_mode_set(EQ_MODE_CUSTOM);
#endif
}
#include "audio_dec.h"

static u8 Menu_mode = 0;
static s8 Menu_bass_gain = 0;
static s8 Menu_treble_gain = 0;
static s8 Menu_loud_sta = 0;

void User_eq_bass_gain(u8 gain)
{
    __User_eq_section_gain(USER_EQ_BASS_SECTION, gain);
    bsp_app_bas_status();//blesend
}

s8 User_get_bass_gain(void)
{
    return Menu_bass_gain;
}

s8 User_eq_bass_dir(u8 dir)
{
    if(dir)
    {
        if(Menu_bass_gain < 7)
            Menu_bass_gain++;
    }
    else
    {
        if(Menu_bass_gain > -7)
            Menu_bass_gain--;
    }
//    printf("Bass:%d\n",Menu_bass_gain);
    User_eq_bass_gain(Menu_bass_gain*12/7);

    return Menu_bass_gain;
}

void User_eq_treble_gain(u8 gain)
{
    __User_eq_section_gain(USER_EQ_TREBLE_SECTION, gain);
    __User_eq_section_gain(USER_EQ_TREBLE_SECTION+2, gain);
    bsp_app_tre_status();//blesend
}

s8 User_get_treble_gain(void)
{
    return Menu_treble_gain;
}

s8 User_eq_treble_dir(u8 dir)
{
    if(dir)
    {
        if(Menu_treble_gain < 7)
            Menu_treble_gain++;
    }
    else
    {
        if(Menu_treble_gain > -7)
            Menu_treble_gain--;
    }
    //printf("Treble:%d\n",Menu_treble_gain);
    User_eq_treble_gain(Menu_treble_gain*12/7);

    return Menu_treble_gain;
}

s8 User_get_loud_sta(void)
{
    return Menu_loud_sta;
}

s8 User_eq_loud_change(void)
{
    Menu_loud_sta ^= 1;
    //printf("loudsta:%d\n",Menu_loud_sta);

    __User_eq_section_gain(USER_EQ_LOUD_SECTION, Menu_loud_sta?10:0);

    bsp_app_loud_type();//blesend

    return Menu_loud_sta;
}

void User_eq_set(u8 mode)
{
    s8 get_eq_info[22] = {0};
    if(mode == EQ_MODE_CUSTOM)
    {
        if(app_var.volmcu_type == 0)
            __User_eq_section_gain(USER_EQ_LOUD_SECTION, Menu_loud_sta?10:0);
        else if(app_var.volmcu_type == 1)
            dsp_module_set_eq(0);
    }
    else
    {
        //clr
        if(app_var.volmcu_type == 1)
            dsp_module_set_eq(mode);
    }
    if(app_var.volmcu_type == 0)
        eq_mode_set(mode);

    app_var.eq_mode = mode;
    bsp_app_eq_type();//blesend
}

void eq_setting_init_2(void)
{
    User_eq_set(EQ_MODE_CUSTOM);
}

#endif
