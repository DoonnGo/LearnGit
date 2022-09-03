#include "ui/ui_api.h"
#include "music/music_ui.h"
#include "fm_emitter/fm_emitter_manage.h"
#include "audio_dec.h"
#include "audio_dec_file.h"
#include "music/music_player.h"
#include "key_event_deal.h"
//#include "app_msg.h"

#if TCFG_APP_MUSIC_EN
#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_USER_LCDSEG))
/* UI_DIS_VAR *ui_get_dis_var()//获取存储模式参数的结构体 */
/* { */
/* return ui_dis_var; */
/* } */


void ui_music_temp_finsh(u8 menu)//子菜单被打断或者显示超时
{
    switch (menu) {
    default:
        break;
    }
}

static void ui_lcdseg_show_music_time(void *hd, int sencond)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[7] = {0};
    u8 min = 0;
    min = sencond / 60 % 60;
    sencond = sencond % 60;

#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "%02d%02d", min, sencond);
#else
    sprintf((char *)bcd_number, "%03d%01d%02d", music_player_get_file_cur(), min, sencond);
#endif
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->clear_icon(0xffff);
    dis->show_string(bcd_number);
    dis->flash_icon(LCDSEG_2POINT);
    dis->show_icon(LCDSEG_MP3);
    dis->show_icon(LCDSEG_MID_);
    dis->lock(0);

}


static void lcdseg_show_filenumber(void *hd, u16 file_num)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[5] = {0};	    ///<换算结果显示缓存
    itoa4(file_num, (u8 *)bcd_number);

    dis->lock(1);
    dis->clear();
#if CONFIG_CLIENT_DG
    dis->setXY(0, 0);
#else
    dis->setXY(2, 0);
#endif
    if (file_num > 999 && file_num <= 9999) {
        bcd_number[0] = file_num/1000 + '0';
    } else {
        bcd_number[0] = ' ';
    }
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_input_filenumber(void *hd, u16 file_num)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[5] = {0};	    ///<换算结果显示缓存
    itoa4(file_num, (u8 *)bcd_number);

    dis->lock(1);
    dis->clear();
#if CONFIG_CLIENT_DG
    dis->setXY(0, 0);
#else
    dis->setXY(2, 0);
#endif
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_foldernumber(void *hd, u16 folder_num)
{
    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[5] = {0};	    ///<换算结果显示缓存
    itoa4(folder_num, (u8 *)bcd_number);

    dis->lock(1);
    dis->clear();
#if CONFIG_CLIENT_DG
    dis->setXY(0, 0);
#else
    dis->setXY(2, 0);
#endif
    bcd_number[0] = 'F';
    dis->show_string(bcd_number);
    dis->lock(0);
}

static void lcdseg_show_dev(void *hd, u16 dev_num)
{
    LCD_API *dis = (LCD_API *)hd;

    if(dev_num != UI_DEV_SD0 && dev_num != UI_DEV_SD1 && dev_num != UI_DEV_USB)
    {
        return;
    }

#if CONFIG_CLIENT_DG
    const char *ui_dev_name[] = {
        ///               "____"
        [UI_DEV_NONE]   = "ERR",
        [UI_DEV_SD0]    = " SD",
        [UI_DEV_SD1]    = " SD2",
        [UI_DEV_USB]    = "USB",
        [UI_DEV_OTHER]  = "ERR",
    };
#else
    const char *ui_dev_name[] = {
        ///               "______"
        [UI_DEV_NONE]   = "NO DEV",
        [UI_DEV_SD0]    = "  SD",
        [UI_DEV_SD1]    = "  SD2",
        [UI_DEV_USB]    = "  USB",
        [UI_DEV_OTHER]  = "  ERR",
    };
#endif

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string(ui_dev_name[dev_num]);
    dis->lock(0);
}


static void lcdseg_show_pause(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
#if CONFIG_CLIENT_DG
    dis->show_string((u8 *)"PAUS");
#elif CONFIG_CLIENT_GX
    dis->show_string((u8 *)" PAUSE");
#else
    dis->show_string((u8 *)"PAUSE");
#endif
    dis->lock(0);
}

static void lcdseg_show_repeat_mode(void *hd, u32 val)
{
    if (!val) {
        return ;
    }
    u8 mode = (u8)val - 1;

#if CONFIG_CLIENT_DG
    const u8 playmodestr[][5] = {
        "ALL",
        "RPT",
        "Fold",
        "RDM",
        "INT"
    };
#else
    const u8 playmodestr[][5] = {
        " ALL",
        " RPT",
        "Fold",
        " RDM",
        " INT"
    };
#endif

    if (mode >= sizeof(playmodestr) / sizeof(playmodestr[0])) {
        printf("rpt mode display err !!\n");
        return ;
    }

    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
#if CONFIG_CLIENT_DG
    dis->setXY(0, 0);
#else
    dis->setXY(1, 0);
#endif
    dis->show_string((u8 *)playmodestr[mode]);
    dis->lock(0);
}

#if TCFG_DEC_ID3_V1_ENABLE
///2020-11-30 显示ID3
struct id3_v1_hdl {
    char header[3];		// TAG
    char title[30]; 	// 标题
    char artist[30];	// 作者
    char album[30];		// 专辑
    char year[4];		// 出品年代
    char comment[30];	// 备注
    char genre;			// 类型
};

static void lcdseg_show_id3_test(void *hd)
{
    static u8 count = 0;
    struct id3_v1_hdl *hdl = music_id3_get();

    char *id3_buf;
    if(count == 0)
        id3_buf = hdl->title;
    else if(count == 1)
        id3_buf = hdl->artist;
    else if(count == 2)
        id3_buf = hdl->album;
    if(++count >= 3)
        count = 0;

    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[6] = {0};	    ///<换算结果显示缓存

    sprintf((char *)bcd_number, "%-0.5s", id3_buf);

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string(bcd_number);
    dis->lock(0);

}

#if CONFIG_CLIENT_DG
    #define MUSIC_ID3_SHOW_NUM    4     //显示ID3位数
#else
    #define MUSIC_ID3_SHOW_NUM    5     //显示ID3位数
#endif
#define MUSIC_TIME_SHOW_CNT   8     //显示8秒时间后进入ID3显示
static u8 lcdseg_id3_num = 0;       //id3显示内容编号:0:时间; 1:"TITLE"; 2:title内容; 3:"ARTIS"; 4:artist内容; 5:"ALBUM"; 6:album内容;
static u8 lcdseg_id3_cnt = 0;       //内容buf坐标
static u8 lcdseg_id3_time_cnt = 0;  //显示播放时间的计数,当 时间(秒) > (MUSIC_TIME_SHOW_CNT*2) 时,进入ID3显示
static void lcdseg_show_id3_restart(u8 en)  //0:重置计数显示时间,1:重置计数显示ID3
{
    lcdseg_id3_num = en?1:0;
    lcdseg_id3_cnt = 0;
    lcdseg_id3_time_cnt = 0;
}

static void lcdseg_show_id3_main(void *hd)  //ID3显示
{
    struct id3_v1_hdl *hdl = music_id3_get();   //获取ID3内容
    struct id3_v1_hdl id3_info;                 //ID3缓存
    memcpy(&id3_info, hdl, sizeof(id3_info));   //防止突然切歌导致指针内存泄露

    if(hdl == NULL)
    {
        lcdseg_id3_num = 0;
        lcdseg_id3_cnt = 0;
        return;
    }

    char *id3_buf;
#if CONFIG_CLIENT_DG
    const char *id3_TAG[] = {
        "TITL",
        "ARTI",
        "ALBU",
    };
#else
    const char *id3_TAG[] = {
        "TITLE",
        "ARTIS",
        "ALBUM",
    };
#endif
    if(lcdseg_id3_num == 1)                     //选择要显示内容
        id3_buf = id3_TAG[0];
    else if(lcdseg_id3_num == 2)
        id3_buf = id3_info.title;
    else if(lcdseg_id3_num == 3)
        id3_buf = id3_TAG[1];
    else if(lcdseg_id3_num == 4)
        id3_buf = id3_info.artist;
    else if(lcdseg_id3_num == 5)
        id3_buf = id3_TAG[2];
    else if(lcdseg_id3_num == 6)
        id3_buf = id3_info.album;

    LCD_API *dis = (LCD_API *)hd;
    u8 bcd_number[6] = {0};	    ///<换算结果显示缓存
    u8 id3_cnt = lcdseg_id3_cnt;                //屏幕显示坐标
    static u8 delay_count = 0;                  //停留计数:显示 [TAG/滚动到末端停留] 的时间

    if(id3_buf[id3_cnt + MUSIC_ID3_SHOW_NUM] == 0)//滚动到了末端
    {
        if(delay_count == 0)                    //滚动到了末端开始计数3个循环,即 [TAG/滚动到末端停留] 的时间
            delay_count = 3;
        else
        {
            delay_count--;
            if(delay_count == 0)                //[TAG/滚动到末端停留] 的计数结束,重置计数并进入下一项内容的显示
            {
                lcdseg_id3_cnt = 0;
                lcdseg_id3_num++;
                if(lcdseg_id3_num > 6)
                    lcdseg_id3_num = 0;
            }
            return;
        }
    }
    else
        lcdseg_id3_cnt++;

#if CONFIG_CLIENT_DG
    sprintf((char *)bcd_number, "%-0.4s", &id3_buf[id3_cnt]);
#else
    sprintf((char *)bcd_number, "%-0.5s", &id3_buf[id3_cnt]);
#endif
    if(bcd_number[MUSIC_ID3_SHOW_NUM] > 0)
        bcd_number[MUSIC_ID3_SHOW_NUM] = 0;                           ///修正显示缓存,使其只在允许的位数显示

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string(bcd_number);
    dis->lock(0);
}
#endif

static void lcdseg_fm_show_freq(void *hd, u32 arg)
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

static void *ui_open_music(void *hd)
{
    MUSIC_DIS_VAR *ui_music = NULL;
    /* ui_music = (MUSIC_DIS_VAR *)malloc(sizeof(MUSIC_DIS_VAR)); */
    /* if (ui_music == NULL) { */
    /* return NULL; */
    /* } */
    ui_set_auto_reflash(500);//设置主页500ms自动刷新
    return ui_music;
}

static void ui_close_music(void *hd, void *private)
{
    MUSIC_DIS_VAR *ui_music = (MUSIC_DIS_VAR *)private;
    LCD_API *dis = (LCD_API *)hd;
    if (!dis) {
        return ;
    }

    if (ui_music) {
        free(ui_music);
    }
}

void lcdseg_show_icon_online_dev(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;

    dis->lock(1);

    if (dev_manager_online_check_by_logo("udisk0", 1)) {
        dis->show_icon(LCDSEG_USB);
    }
    if (dev_manager_online_check_by_logo("sd0", 1)) {
        dis->show_icon(LCDSEG_SD);
    }

    dis->lock(0);
}

static void lcdseg_show_music_dev(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    char *dev = NULL;

    dis->lock(1);

    /* char *music_play_get_cur_dev(void); */
    /* char *dev = music_play_get_cur_dev(); */
    dev = music_player_get_dev_cur();

    if (dev_manager_online_check_by_logo("udisk0", 1)) {
        dis->show_icon(LCDSEG_USB);
    }
    if (dev_manager_online_check_by_logo("sd0", 1)) {
        dis->show_icon(LCDSEG_SD);
    }

    if (dev) {
        if (!memcmp(dev, "udisk", 5)) {
            dis->flash_icon(LCDSEG_USB);
        } else {
            dis->flash_icon(LCDSEG_SD);
        }
    }

    ///2020-11-14
    int music_type = file_dec_get_decoder_type();
    if(music_type == AUDIO_CODING_MP3)
        dis->show_icon(LCDSEG_MP3);
    else if(music_type == AUDIO_CODING_WAV)
        dis->show_icon(LCDSEG_WAV);
    else if(music_type == AUDIO_CODING_WMA)
        dis->show_icon(LCDSEG_WMA);

    dis->lock(0);
}

static void ui_music_main(void *hd, void *private) //主界面显示
{
    if (!hd) {
        return;
    }
    MUSIC_DIS_VAR *ui_music = (MUSIC_DIS_VAR *)private;
#if TCFG_APP_FM_EMITTER_EN
    if (true == file_dec_is_pause()) {
        lcdseg_show_pause(hd);
    } else {
        u16 fre = fm_emitter_manage_get_fre();
        if (fre != 0) {
            lcdseg_fm_show_freq(hd, fre);
        }
    }
#else
    extern u8 User_mute_flag;
    if(User_mute_flag)
    {
        extern void lcdseg_show_mute(void *hd);
        lcdseg_show_mute(hd);
        return;
    }
    if (true == file_dec_is_play()) {
        int sencond = file_dec_get_cur_time();

#if !TCFG_DEC_ID3_V1_ENABLE
        ui_lcdseg_show_music_time(hd, sencond);
        lcdseg_show_music_dev(hd);
#else
        if(lcdseg_id3_num == 0)
        {
            ui_lcdseg_show_music_time(hd, sencond);
            lcdseg_show_music_dev(hd);

            ///2020-11-30 MUSIC_TIME_SHOW_CNT秒后进入ID3
            if(++lcdseg_id3_time_cnt >= MUSIC_TIME_SHOW_CNT)
            {
                lcdseg_show_id3_restart(1);
            }
        }
        else
        {
            lcdseg_show_id3_main(hd);
        }
#endif

/*------Music INT playmode------------------------------------------------*/
        static int last_sec = 0;
        if(User_music_get_playmode() == 5)  ///2020-11-28 playmode INT
        {
            if(last_sec <= 8 && sencond >= 9)
            {
                app_task_put_key_msg(KEY_MUSIC_PLAYER_AUTO_NEXT, 0);
            }
            last_sec = sencond;
        }
        else if(sencond <= 8)
            last_sec = sencond;
/*------End---------------------------------------------------------------*/
    } else if (file_dec_is_pause()) {
        lcdseg_show_pause(hd);
    } else {
        printf("!!! %s %d\n", __FUNCTION__, __LINE__);

    }

#endif
}


static int ui_music_user(void *hd, void *private, u8 menu, u32 arg)//子界面显示 //返回true不继续传递 ，返回false由common统一处理
{
    int ret = true;
    LCD_API *dis = (LCD_API *)hd;
    MUSIC_DIS_VAR *ui_music = (MUSIC_DIS_VAR *)private;
    if (!dis) {
        return false;
    }
    switch (menu) {
    case MENU_FILENUM:
        lcdseg_show_filenumber(hd, arg);
#if TCFG_DEC_ID3_V1_ENABLE
        lcdseg_show_id3_restart(0); ///2020-11-30 新歌曲清空计数
#endif
        break;
    case MENU_FOLDER_NUM:
        lcdseg_show_foldernumber(hd, arg);
        break;
    case MENU_MUSIC_REPEATMODE:
        lcdseg_show_repeat_mode(hd, arg);
        break;
    case MENU_DEV_LOGO:
        lcdseg_show_dev(hd, arg);
        break;
    case MENU_INPUT_NUMBER:
        lcdseg_show_input_filenumber(hd, arg);
        break;
//    case MENU_MUSIC_ID3:
////        lcdseg_show_id3_test(hd);
//        lcdseg_show_id3_restart(1);
//        break;
    default:
        ret = false;
        break;
    }

    lcdseg_show_icon_online_dev(hd);
    return ret;

}



const struct ui_dis_api music_main = {
    .ui      = UI_MUSIC_MENU_MAIN,
    .open    = ui_open_music,
    .ui_main = ui_music_main,
    .ui_user = ui_music_user,
    .close   = ui_close_music,
};

#endif
#endif
