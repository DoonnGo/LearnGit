#include "system/includes.h"
#include "app_main.h"
#include "app_task.h"
#include "music/music.h"
#include "fm/fm_manage.h"
#include "key_event_deal.h"
#include "audio_config.h"
#include "ble_module.h"
#include "rgb_module.h"
extern int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type);

#define TIANYU_MOTO_APP     0

static app_notify_buf[20] = {0};
//************************************************************//
//send
#if TIANYU_MOTO_APP
AT(.text.app)
void bsp_app_music_sta_all(void)
{
    app_notify_buf[0] = 0x04;
    app_notify_buf[1] = 0x01;

    app_notify_buf[2] = f_msc.file_num>>8;
    app_notify_buf[3] = (u8)f_msc.file_num;

    app_notify_buf[4] = f_msc.file_total>>8;
    app_notify_buf[5] = (u8)f_msc.file_total;

	u16 sec;
	sec = f_msc.curtime.min * 60 + f_msc.curtime.sec;
	app_notify_buf[6] = sec>>8;
	app_notify_buf[7] = sec&0xff;

	sec = f_msc.alltime.min * 60 + f_msc.alltime.sec;
	app_notify_buf[8] = sec>>8;
	app_notify_buf[9] = sec&0xff;

    app_notify_buf[10] = f_msc.pause?0:1;

    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 11, ATT_OP_AUTO_READ_CCC);
}

AT(.text.app)
void bsp_app_password_success(void)
{
    app_notify_buf[0] = 0x07;
    app_notify_buf[1] = 0x01;
    app_notify_buf[2] = 0x01;
    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
}
AT(.text.app)
void bsp_app_password_fair(void)
{
    app_notify_buf[0] = 0x07;
    app_notify_buf[1] = 0x01;
    app_notify_buf[2] = 0x00;
    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
}
AT(.text.app)
void bsp_app_password_change_success(void)
{
    app_notify_buf[0] = 0x07;
    app_notify_buf[1] = 0x02;
    app_notify_buf[2] = 0x01;
    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
}
AT(.text.app)
void bsp_app_password_change_fair(void)
{
    app_notify_buf[0] = 0x07;
    app_notify_buf[1] = 0x02;
    app_notify_buf[2] = 0x00;
    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
}
#endif

//Music播放流程:[NewMusic] => SendNum =(delay500Ms)> Time => MuteSta => Playsta => MusicName
//Music_sta: Play <=> Pause
//Music模式播放状态:1:Play,2:Pause
void bsp_app_play_sta(u8 status)
{
#if !TIANYU_MOTO_APP
    app_notify_buf[0] = 0x01;
    app_notify_buf[1] = 0x02;
    app_notify_buf[2] = status;
    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//set clk
//在app发来修改clk指令后返回，表示成功修改
void bsp_app_clock(void)
{
#if !TIANYU_MOTO_APP
    app_notify_buf[0] = 0x06;
    app_notify_buf[1] = 0x03;
    app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 2, ATT_OP_AUTO_READ_CCC);
#endif
}

//Music播放流程:[NewMusic] => SendNum =(delay500Ms)> Time => MuteSta => Playsta => MusicName
//同步
//当前曲目返回
void bsp_app_music_num(void)
{
#if !TIANYU_MOTO_APP
	u16 file_num = music_player_get_file_cur();
	app_notify_buf[0] = 0x03;
	app_notify_buf[1] = 0x05;
	app_notify_buf[2] = file_num>>8;
	app_notify_buf[3] = file_num&0xff;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 4, ATT_OP_AUTO_READ_CCC);
#endif
}

//Music播放流程:[NewMusic] => SendNum =(delay500Ms)> Time => MuteSta => Playsta => MusicName
//Music_sta: Pause => Play
//返回当前播放时间点和歌曲时间总长
void bsp_app_music_time(void)
{
#if !TIANYU_MOTO_APP
	u16 cur_time = music_player_get_dec_cur_time();

	app_notify_buf[0] = 0x04;
	app_notify_buf[1] = 0x05;

	app_notify_buf[2] = cur_time>>8;
	app_notify_buf[3] = cur_time&0xff;

	cur_time = music_player_get_dec_total_time();
	app_notify_buf[4] = cur_time>>8;
	app_notify_buf[5] = cur_time&0xff;

	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 6, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//playmode切换时
//[MusicEnter] => Task => Playmode
//返回Music模式的播放循环模式
void bsp_app_music_playmode(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x04;
	app_notify_buf[1] = 0x06;
	app_notify_buf[2] = sys_cb.play_mode;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//Music播放流程:[NewMusic] => SendNum =(delay500Ms)> Time => MuteSta => Playsta => MusicName
//返回当前歌曲名(文件名)
void bsp_app_music_name(void)
{
#if TIANYU_MOTO_APP
	u8 len = 0, str_len, ptr = 0;
	u8 remain = 1;

	str_len = f_msc.fname_len;

	app_notify_buf[0] = 0x04;
	app_notify_buf[1] = 0x02;
	app_notify_buf[2] = 0x02;
	app_notify_buf[3] = 0x01;

	while (remain)
	{
		if(str_len > 16)
		{
			len = 16;
			remain = 1;
			str_len -= 16;
		}
		else
		{
			len = str_len;
			remain = 0;
			app_notify_buf[3] = 0x00;
		}
		memcpy(&app_notify_buf[4], &f_msc.fname[ptr], len);
		ptr += len;
		app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, len+4, ATT_OP_AUTO_READ_CCC);
	}
#else
	u8 len = 0, str_len, ptr = 0;
	u8 remain = 1;

	str_len = music_file_name_len;
	app_notify_buf[0] = 0x0c;
	app_notify_buf[1] = 0x01;
	app_notify_buf[2] = 0x00;
	app_notify_buf[3] = 0x01;

	while (remain)
	{
		if(str_len > 16)
		{
			len = 16;
			remain = 1;
			str_len -= 16;
		}
		else
		{
			len = str_len;
			remain = 0;
			app_notify_buf[3] = 0x00;
		}
		memcpy(&app_notify_buf[4], &music_file_name[ptr], len);
		ptr += len;
		app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, len+4, ATT_OP_AUTO_READ_CCC);
	}
#endif
}

//同步
//进入新模式时和Music模式切换设备时
//返回当前模式
void bsp_app_workmode(void)
{
#if TIANYU_MOTO_APP
	app_notify_buf[0] = 0x00;
	app_notify_buf[1] = 0x01;
	switch(func_cb.sta)
	{
		case FUNC_PWROFF:
			app_notify_buf[2] = 0x05;
		break;

		case FUNC_MUSIC:
            app_notify_buf[2] = 0x04;
			if(f_msc.cur_dev==DEV_UDISK)
				app_notify_buf[3] = 0x01;
			else
				app_notify_buf[3] = 0x00;
		break;

		case FUNC_FMRX:
			app_notify_buf[2] = 0x01;
		break;

		case FUNC_BT:
			app_notify_buf[2] = 0x03;
		break;

		case FUNC_AUX:
			app_notify_buf[2] = 0x02;
		break;

		case FUNC_EXSPIFLASH_MUSIC:
            app_notify_buf[2] = 0x06;
            app_notify_buf[3] = 0x00;
            if(sys_cb.RFWarnSta == 2 || sys_cb.RFWarnSta == 4)
            {
                app_notify_buf[3] = 0x01;
            }
        break;
	}
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 4, ATT_OP_AUTO_READ_CCC);
#else
	app_notify_buf[0] = 0x8;
	switch(app_get_curr_task())
	{
		case APP_POWEROFF_TASK:
			app_notify_buf[1] = 0x0;
		break;

		case APP_MUSIC_TASK:
        	if (0 == strcmp("udisk0", music_player_get_dev_cur()))
				app_notify_buf[1] = 0x2;
			else
				app_notify_buf[1] = 0x3;
		break;

		case APP_FM_TASK:
			app_notify_buf[1] = 0x4;
		break;

		case APP_BT_TASK:
			app_notify_buf[1] = 0x5;
		break;

		case APP_LINEIN_TASK:
			app_notify_buf[1] = 0x6;
		break;
	}
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 2, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//Mute&Unmute
//进入新模式Unmute并返回
//Fm模式停台出声时
//Music播放流程:[NewMusic] => SendNum =(delay500Ms)> Time => MuteSta => Playsta => MusicName
//返回当前Mute状态
void bsp_app_mute_status(void)
{
#if TIANYU_MOTO_APP
    extern void bsp_app_fm_freq(void);
    if(func_cb.sta == FUNC_FMRX)
    {
        bsp_app_fm_freq();
    }
#else
	extern u8 User_mute_flag = 0;
	app_notify_buf[0] = 0x09;
	app_notify_buf[1] = 0x01;

	if(User_mute_flag)
	{
		app_notify_buf[2] = 0x01;
	}
	else
	{
		app_notify_buf[2] = 0x00;
	}
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变bass值
//返回当前bass值
void bsp_app_bas_status(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0a;
	app_notify_buf[1] = 0x01;
	app_notify_buf[2] = sys_cb.bas_value;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变treble值
//返回当前treble值
void bsp_app_tre_status(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0a;
	app_notify_buf[1] = 0x02;
	app_notify_buf[2] = sys_cb.tre_value;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变bal值
//返回当前bal值
void bsp_app_bal_status(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0a;
	app_notify_buf[1] = 0x03;
	app_notify_buf[2] = app_var.bal_vol+7;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变fad值
//返回当前fad值
void bsp_app_fad_status(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0a;
	app_notify_buf[1] = 0x04;
	app_notify_buf[2] = app_var.fad_vol+7;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变eq值
//返回当前eq值
void bsp_app_eq_type(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0b;
	app_notify_buf[1] = app_var.eq_mode;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 2, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变loud值
//返回当前loud值
void bsp_app_loud_type(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x04;
	app_notify_buf[1] = 0x04;
	app_notify_buf[2] = sys_cb.loud_type;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变vol值
//返回当前vol值
void bsp_app_vol(void)
{
#if TIANYU_MOTO_APP
    u16 Volume = sys_cb.vol * 16 / VOL_MAX;
    app_notify_buf[0] = 0x00;
	app_notify_buf[1] = 0x02;
	app_notify_buf[2] = Volume;
	//app_notify_buf[3] = VOL_MAX;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#else
	app_notify_buf[0] = 0x04;
	app_notify_buf[1] = 0x03;
	app_notify_buf[2] = sys_cb.vol;
	app_notify_buf[3] = VOL_MAX;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 4, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变freq值
//返回当前freq值
void bsp_app_fm_freq(void)
{
#if TIANYU_MOTO_APP
    app_notify_buf[0] = 0x01;
    app_notify_buf[1] = sys_cb.mute?0:1;

	app_notify_buf[2] = fmrx_cb.freq>>8;
	app_notify_buf[3] = fmrx_cb.freq&0xff;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 4, ATT_OP_AUTO_READ_CCC);
#else
	u16 freq = fm_manage_get_fre();
	app_notify_buf[0] = 0x0d;
	app_notify_buf[1] = 0x01;
	app_notify_buf[2] = freq>>8;
	app_notify_buf[3] = freq&0xff;
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 4, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变band值
//进入Fm模式时
//返回当前band值
void bsp_app_fm_band_channel(u8 band)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0d;
	app_notify_buf[1] = 0x02;

	app_notify_buf[2] = band;
	extern u16 User_fm_channel_buf[18];
	memcpy(&app_notify_buf[3], (u8*)&User_fm_channel_buf[(band-1)*6], 12);
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 15, ATT_OP_AUTO_READ_CCC);
#endif

}

//同步
//USB&SD在插入拔出时，BT在连接和断开时
//返回当前设备状态:USB,SD,BT
void bsp_app_device_online(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x09;
	app_notify_buf[1] = 0x02;

	if(dev_manager_online_check_by_logo("udisk0", 1))
		app_notify_buf[2] = 0x01;
	else
		app_notify_buf[2] = 0x00;

	if(dev_manager_online_check_by_logo("sd0", 1))
		app_notify_buf[3] = 0x01;
	else
		app_notify_buf[3] = 0x00;

	if(get_bt_connect_status() >= BT_STATUS_CONNECTING)
		app_notify_buf[4] = 0x01;
	else
		app_notify_buf[4] = 0x00;

	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 5, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//改变rgb值
//返回当前rgb值
void bsp_app_rgb_status(void)
{
#if !TIANYU_MOTO_APP
	app_notify_buf[0] = 0x0E;
	if(app_var.rgb_drive_type)
	{
		app_notify_buf[1] = 0x01;//rgb
		app_notify_buf[2] = app_var.rgb_mode;
	}
	else
	{
		app_notify_buf[1] = 0x00;
		app_notify_buf[2] = 0x00;
	}
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 3, ATT_OP_AUTO_READ_CCC);
#endif
}

//同步
//校验?
void bsp_app_protocol(void)
{
#if !TIANYU_MOTO_APP
#if 0
	app_notify_buf[0] = 0x0f;
	app_notify_buf[1] = 0x01;
	app_notify_buf[2] = 'H';
	app_notify_buf[3] = 'E';
	app_notify_buf[4] = 'X';
	app_notify_buf[5] = 'I';
	app_notify_buf[6] = 'N';
	app_notify_buf[7] = 'G';
#else
	app_notify_buf[0] = 0x0f;
	app_notify_buf[1] = 0x01;
	app_notify_buf[2] = 'T';
	app_notify_buf[3] = 'I';
	app_notify_buf[4] = 'A';
	app_notify_buf[5] = 'N';
	app_notify_buf[6] = 'Y';
	app_notify_buf[7] = 'U';
#endif
	app_send_user_data(ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE, app_notify_buf, 8, ATT_OP_AUTO_READ_CCC);
#endif
}

static u8 update_list_num = 0;
void bsp_app_info(void)
{
	switch(update_list_num)
	{
		case 51:
			bsp_app_protocol();
		break;

		case 52:
			bsp_app_workmode();
			bsp_app_rgb_status();
			break;

		case 53:
			bsp_app_vol();
			bsp_app_device_online();
			break;

		case 54:
			bsp_app_mute_status();
			break;

		case 55:
			bsp_app_bas_status();
			break;

		case 56:
			bsp_app_tre_status();
			break;

		case 57:
			bsp_app_bal_status();
			break;

		case 58:
			bsp_app_fad_status();
			break;

		case 59:
			bsp_app_eq_type();
			bsp_app_loud_type();
			break;

		case 60:
			if(app_check_curr_task(APP_MUSIC_TASK))
			{
				bsp_app_music_num();
				bsp_app_music_playmode();

#if TIANYU_MOTO_APP
				bsp_app_music_sta_all();
#else
				bsp_app_music_time();
				bsp_app_mute_status();
				bsp_app_play_sta(music_player_get_play_status()==FILE_DEC_STATUS_PLAY? 2 : 1);
#endif
				bsp_app_music_name();
			}
			else if(app_check_curr_task(APP_FM_TASK))
			{
				bsp_app_fm_freq();
				bsp_app_fm_band_channel(fm_get_cur_band());
			}
			break;
	}
	update_list_num++;
	if(update_list_num>60)
	{
		update_list_num = 0;
		return;
	}
	sys_timeout_add(NULL, bsp_app_info, 50);
}

//***********************************************//
//***********************************************//
//read
void bsp_app_01_proc(u8 *ptr)
{
#if TIANYU_MOTO_APP
    extern void fmrx_set_freq_APP(void);
    if(func_cb.sta == FUNC_FMRX && ptr[1] == 1)
    {
        fmrx_cb.freq = ptr[2];
        fmrx_cb.freq <<= 8;
        fmrx_cb.freq |= ptr[3];
        fmrx_set_freq_APP();
    }
#else
	u16 msg = KEY_NULL;
	switch(ptr[1])
	{
		case 0x01:
			msg = KEY_POWEROFF;
		break;

		case 0x02:
			msg = KEY_MUSIC_PP;
		break;

		case 0x03:
			//msg = KEY_UPDATE_INFO;
			sys_timeout_add(NULL, bsp_app_info, 50);
			update_list_num = 51;
//			app_cb.update_info = 1;
//			app_cb.update_list_num = 50;
		break;
	}
	if(msg != KEY_NULL)
	{
		app_task_put_key_msg(msg, 0);
	}
#endif
}

void bsp_app_02_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	u16 msg = KEY_NULL;
	switch(ptr[1])
	{
		case 0x01:
			msg = KEY_FM_CHANNEL_1;
		break;
		case 0x02:
			msg = KEY_FM_CHANNEL_2;
		break;
		case 0x03:
			msg = KEY_FM_CHANNEL_3;
		break;
		case 0x04:
			msg = KEY_FM_CHANNEL_4;
		break;
		case 0x05:
			msg = KEY_FM_CHANNEL_5;
		break;
		case 0x06:
			msg = KEY_FM_CHANNEL_6;
		break;
		default:
            return;
	}
    app_task_put_key_msg(msg, 0);
#endif
}

void bsp_app_03_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	u16 msg = KEY_NULL;
	u16 value = 0;
	switch(ptr[1])
	{
		case 0x01:
            msg = KEY_MUSIC_PREV;
		break;

		case 0x02:
            msg = KEY_MUSIC_NEXT;
		break;

		case 0x03:
			msg = KEY_MUSIC_PREV_10;
		break;

		case 0x04:
			msg = KEY_MUSIC_NEXT_10;
		break;

		//case 0x05:
		//	msg = KEY_MUSIC_PLAYE_BY_DEV_FILENUM;
		//	value = (ptr[2]<<8)|ptr[3];
		//break;
	}

	if(msg != KEY_NULL)
	{
		app_task_put_key_msg(msg, value);
	}
#endif
}

void bsp_app_04_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	u16 msg = KEY_NULL;
	int value = 0;
	switch(ptr[1])
	{
		case 0x03:
			value = ptr[2];
			msg = BLE_VOL_SET;
			break;

		case 0x04:
			msg = KEY_LOUD;
			break;
/*
		case 0x06:
			msg = BLE_PLAY_MODE;
			break;*/
	}
	app_task_put_key_msg(msg, value);
#endif
}

void bsp_app_06_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	u16 msg = KEY_NULL;
	int value[3] = {0};
	switch(ptr[1])
	{
		case 0x01:
			msg = KEY_CLK;
			break;

		case 0x02:
			if((ptr[2]<24)&&(ptr[3]<60)&&(ptr[4]<60))
			{
				value[0] = ptr[2];
				value[1] = ptr[3];
				value[2] = ptr[4];
				msg = KEY_CLK_SET;
				bsp_app_clock();
			}
			break;
		default:
			return;
	}
	app_task_put_key_msg(msg, &value);
#endif
}

void bsp_app_07_proc(u8 *ptr)
{
#if TIANYU_MOTO_APP
    if(ptr[1] == 1)         //校验密码
    {
        if(sys_cb.APP_Password[0] == ptr[2]
           && sys_cb.APP_Password[1] == ptr[3]
           && sys_cb.APP_Password[2] == ptr[4]
           && sys_cb.APP_Password[3] == ptr[5]
           )
        {
            sys_cb.APP_Success = 1;
            bsp_app_password_success();//发送成功指仿
        }
        else
        {
            sys_cb.APP_Success = 0;
            bsp_app_password_fair();//发送失败指仿
        }
    }
    else if(ptr[1] == 2)    //修改密码
    {
        if(sys_cb.APP_Success == 1)
        {
            sys_cb.APP_Password[0] = ptr[2];
            sys_cb.APP_Password[1] = ptr[3];
            sys_cb.APP_Password[2] = ptr[4];
            sys_cb.APP_Password[3] = ptr[5];
            param_APP_Password_write();
            param_sync();
            bsp_app_password_change_success();
        }
        else
        {
            bsp_app_password_change_fair();
        }
    }
    else if(ptr[1] == 4)    //同步
    {
        //发送全部信息回去APP
        app_cb.update_info = 1;
        app_cb.update_list_num = 50;
    }
#else
	u16 msg = KEY_NULL;
	switch(ptr[1])
	{
		case 0x01:
			msg = KEY_FM_BAND;
		break;

		case 0x02:
			msg = KEY_FM_SCAN_ALL_DOWN;
		break;

		case 0x04:
			msg = KEY_FM_AUTO_PLAY;
		break;
	}
	app_task_put_key_msg(msg, 0);
#endif
}

void bsp_app_08_proc(u8 cmd)
{
#if !TIANYU_MOTO_APP
	u16 msg = KEY_NULL;
	switch(cmd)
	{
		case 0x00:
			msg = KEY_POWEROFF;
			break;

		case 0x01:
			msg = KEY_MUTE;
			break;
/*
		case 0x02:
			if(dev_is_online(DEV_UDISK))
			{
				if(func_cb.sta==FUNC_MUSIC)
				{
					if(f_msc.cur_dev != DEV_UDISK)
					{
						msg_enqueue(KU_USB_SD);
					}
				}
				else
				{
					func_cb.sta=FUNC_MUSIC;
					sys_cb.cur_dev = DEV_UDISK;
				}
			}
			break;

		case 0x03:
			if(dev_is_online(DEV_SDCARD))
			{
				if(func_cb.sta==FUNC_MUSIC)
				{
					if(f_msc.cur_dev != DEV_SDCARD)
					{
						msg_enqueue(KU_USB_SD);
					}
				}
				else
				{
					func_cb.sta=FUNC_MUSIC;
					sys_cb.cur_dev = DEV_SDCARD;
				}
			}
			break;
*/
		case 0x04:
			msg = KEY_CHANGE_TO_FM;
			break;

		case 0x05:
			msg = KEY_CHANGE_TO_BT;
			break;

		case 0x06:
			msg = KEY_CHANGE_TO_LINEIN;
			break;
	}
	app_task_put_key_msg(msg, 0);
#endif
}

void bsp_app_09_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	if(ptr[1]==0x01)
	{
		app_task_put_key_msg(KEY_MUTE, 0);
	}
#endif
}

void bsp_app_0a_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	u16 msg = KEY_NULL;
	switch (ptr[1])
	{
/*
		case 0x01:
			if((ptr[2]<=14)&&(sys_cb.eq_mode==0))
			{
				sys_cb.bas_value = ptr[2];
				bas_value_set();
				gui_box_show_bas();
			}
			break;

		case 0x02:
			if((ptr[2]<=14)&&(sys_cb.eq_mode==0))
			{
				sys_cb.tre_value = ptr[2];
				tre_value_set();
				gui_box_show_tre();
			}
			break;
*/
		case 0x03:
			if(ptr[2]<=14)
			{
				aucar_cb.bal_level = ptr[2];
				bsp_aucar_bal_set();
				gui_box_show_bal();
			}
			break;

		case 0x04:
			if(ptr[2]<=14)
			{
				aucar_cb.fad_level= ptr[2];
				bsp_aucar_fad_set();
				gui_box_show_fad();
			}
			break;
	}
#endif
}

void bsp_app_0b_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	if(ptr[1] <= 5)
	{
		app_task_put_key_msg(BLE_EQ_SET, ptr[1]);
	}
#endif
}

void bsp_app_0d_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	if(!app_check_curr_task(APP_FM_TASK))
		return;
/*
	if(ptr[1]==0x01)
	{
		u16 freq;
		freq = (ptr[2]<<8)|ptr[3];
		if((freq>=FM_FREQ_MIN)&&(freq<=FM_FREQ_MAX))
		{
			if(fmrx_cb.sta==FMRX_PLAY)
			{
				fmrx_cb.freq = freq;
				fmrx_cb.sta = FMRX_SEEKING_HALF_STOP;
			}
		}
	}
	else if(ptr[1]==0x03)
	{
		u8 msg;
		msg = KEY_M0|ptr[2];
		msg_enqueue(msg|KEY_LONG);
	}*/
#endif
}

void bsp_app_0e_proc(u8 *ptr)
{
#if !TIANYU_MOTO_APP
	if((ptr[2]>0)&&(ptr[2]<=8))
	{
		app_task_put_key_msg(BLE_RGB_SET, ptr[2]);
	}
#endif
}

void bsp_custom_app_process(u8 *ptr, u8 len)
{
	switch (ptr[0])
	{
#if TIANYU_MOTO_APP
		case 0x00:
			bsp_app_00_proc(ptr);
			break;
#endif
		case 0x01:
            bsp_app_01_proc(ptr);
			break;

		case 0x02:
			bsp_app_02_proc(ptr);
			break;

		case 0x03:
			bsp_app_03_proc(ptr);
			break;

		case 0x04:
			bsp_app_04_proc(ptr);
			break;

		case 0x06:
			bsp_app_06_proc(ptr);
			break;

		case 0x07:
			bsp_app_07_proc(ptr);
			break;

		case 0x08:
			bsp_app_08_proc(ptr);
			break;

		case 0x09:
			bsp_app_09_proc(ptr);
			break;

		case 0x0a:
			bsp_app_0a_proc(ptr);
			break;

		case 0x0b:
			bsp_app_0b_proc(ptr);
			break;

		case 0x0d:
			bsp_app_0d_proc(ptr);
			break;

		case 0x0e:
			bsp_app_0e_proc(ptr);
	}
}
