#ifndef _FM_SEVER__H_
#define _FM_SEVER__H_


void fm_volume_pp(void);//播放暂停
void fm_scan_down();//半自动收台
void fm_scan_up();//半自动收台
void fm_scan_all();//全自动收音
void fm_delete_freq();//删当前频点
void fm_prev_station();//上下台
void fm_next_station();//上下台
void fm_volume_up();//声音调节
void fm_volume_down();//声音调整
void fm_next_freq();//上下频点
void fm_prev_freq();//上下频点
void fm_scan_stop(void);//停止搜索，停止后会自动跳到最后一个台

void fm_set_band_channel(u8 channel);   ///2020-11-24 切台1~6
void fm_save_band_channel(u8 channel);  ///2020-11-24 存台1~6
void fm_next_band(void);                ///2020-11-24 切band
void fm_prev_step_long(void);           ///2020-11-24 长按步进(检测到长按时)
void fm_prev_step_hold(void);           ///2020-11-24 长按步进(按住时)
void fm_prev_step_longup(void);         ///2020-11-24 长按步进(松手时)
void fm_next_step_long(void);           ///2020-11-24 长按步进(检测到长按时)
void fm_next_step_hold(void);           ///2020-11-24 长按步进(按住时)
void fm_next_step_longup(void);         ///2020-11-24 长按步进(松手时)
void fm_auto_play_all_channel();        ///2020-12-4  自动浏览全电台
void fm_auto_play_select(void);         ///2020-12-4  停止浏览并选择电台
u8 fm_get_cur_band(void);               ///2020-12-20 获取当前band值

void fm_api_init();//资源申请，读取vm
void fm_api_release();//释放







#endif
