#ifndef _AUDIO_CONFIG_H_
#define _AUDIO_CONFIG_H_

#include "generic/typedef.h"
#include "board_config.h"

#define AUDIO_MIXER_LEN			(128 * 4 * 2)
#define AUDIO_SYNTHESIS_LEN             (128 * 4 * 2)

#if BT_SUPPORT_MUSIC_VOL_SYNC
#define TCFG_MAX_VOL_PROMPT						 0
#else
#define TCFG_MAX_VOL_PROMPT						 1
#endif

/*
 *该配置适用于没有音量按键的产品，防止打开音量同步之后
 *连接支持音量同步的设备，将音量调小过后，连接不支持音
 *量同步的设备，音量没有恢复，导致音量小的问题
 *默认是没有音量同步的，将音量设置到最大值，可以在vol_sync.c
 *该宏里面修改相应的设置。
 */
#define TCFG_VOL_RESET_WHEN_NO_SUPPORT_VOL_SYNC	 0	//不支持音量同步的设备默认最大音量

/*
 *省电容mic偏置电压自动调整(因为校准需要时间，所以有不同的方式)
 *1、烧完程序（完全更新，包括配置区）开机校准一次
 *2、上电复位的时候都校准,即断电重新上电就会校准是否有偏差(默认)
 *3、每次开机都校准，不管有没有断过电，即校准流程每次都跑
 */
#define MC_BIAS_ADJUST_DISABLE		0	//省电容mic偏置校准关闭
#define MC_BIAS_ADJUST_ONE			1	//省电容mic偏置只校准一次（跟dac trim一样）
#define MC_BIAS_ADJUST_POWER_ON		2	//省电容mic偏置每次上电复位都校准(Power_On_Reset)
#define MC_BIAS_ADJUST_ALWAYS		3	//省电容mic偏置每次开机都校准(包括上电复位和其他复位)
#define TCFG_MC_BIAS_AUTO_ADJUST	MC_BIAS_ADJUST_POWER_ON
#define TCFG_MC_CONVERGE_TRACE		0	//省电容mic收敛值跟踪

#if TCFG_USER_BLE_ENABLE
#define TCFG_AEC_SIMPLEX			0  //通话单工模式配置
#else
#define TCFG_AEC_SIMPLEX			0  //通话单工模式配置
#endif

#define TCFG_ESCO_PLC				1  //通话丢包修复

#define TCFG_ESCO_LIMITER			1  	//通话近端底噪/限幅器

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_LR_DIFF || \
     TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_DUAL_LR_DIFF)
#define MAX_ANA_VOL             (21)
#else
#define MAX_ANA_VOL             (15)
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/

#define MAX_COM_VOL             (22)    // 具体数值应小于联合音量等级的数组大小 (combined_vol_list)
#define MAX_DIG_VOL             (USER_UI_MAX_VOL)

#if ((SYS_VOL_TYPE == VOL_TYPE_DIGITAL) || (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW))
#define SYS_MAX_VOL             MAX_DIG_VOL
#elif (SYS_VOL_TYPE == VOL_TYPE_ANALOG)
#define SYS_MAX_VOL             MAX_ANA_VOL
#elif (SYS_VOL_TYPE == VOL_TYPE_AD)
#define SYS_MAX_VOL             MAX_COM_VOL
#else
#error "SYS_VOL_TYPE define error"
#endif


#define SYS_DEFAULT_VOL         	30  //(SYS_MAX_VOL/2)
#define SYS_DEFAULT_TONE_VOL    	45 //(SYS_MAX_VOL)
#define SYS_DEFAULT_SIN_VOL     	30

#define APP_AUDIO_STATE_IDLE        0
#define APP_AUDIO_STATE_MUSIC       1
#define APP_AUDIO_STATE_CALL        2
#define APP_AUDIO_STATE_WTONE       3
#define APP_AUDIO_CURRENT_STATE     4

#define AUDIO_OUTPUT_ONLY_DAC \
    (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC || AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT || AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DONGLE)

#ifdef TCFG_IIS_ENABLE
#define AUDIO_OUTPUT_ONLY_IIS \
    (TCFG_IIS_ENABLE && TCFG_IIS_OUTPUT_EN && AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_IIS)

#define AUDIO_OUTPUT_DAC_AND_IIS \
    (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC_IIS && TCFG_IIS_ENABLE && TCFG_IIS_OUTPUT_EN)
#else
#define AUDIO_OUTPUT_ONLY_IIS   0

#define AUDIO_OUTPUT_DAC_AND_IIS (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC_IIS)

#endif

#define AUDIO_OUTPUT_INCLUDE_IIS (AUDIO_OUTPUT_ONLY_IIS || AUDIO_OUTPUT_DAC_AND_IIS)
#define AUDIO_OUTPUT_INCLUDE_DAC (AUDIO_OUTPUT_ONLY_DAC || AUDIO_OUTPUT_DAC_AND_IIS)

u8 get_max_sys_vol(void);
u8 get_tone_vol(void);

void app_audio_output_init(void);
void app_audio_output_sync_buff_init(void *sync_buff, int len);
int app_audio_output_channel_set(u8 output);
int app_audio_output_channel_get(void);
int app_audio_output_mode_set(u8 output);
int app_audio_output_mode_get(void);
int app_audio_output_write(void *buf, int len);
int app_audio_output_samplerate_select(u32 sample_rate, u8 high);
int app_audio_output_samplerate_set(int sample_rate);
int app_audio_output_samplerate_get(void);
int app_audio_output_start(void);
int app_audio_output_stop(void);
int app_audio_output_reset(u32 msecs);
int app_audio_output_state_get(void);
int app_audio_output_get_cur_buf_points(void);
void app_audio_output_ch_mute(u8 ch, u8 mute);
int app_audio_output_ch_analog_gain_set(u8 ch, u8 again);
s8 app_audio_get_volume(u8 state);
void app_audio_set_volume(u8 state, s8 volume, u8 fade);
void app_audio_volume_up(u8 value);
void app_audio_volume_down(u8 value);
void app_audio_state_switch(u8 state, s16 max_volume);
void app_audio_mute(u8 value);
s16 app_audio_get_max_volume(void);
void app_audio_state_switch(u8 state, s16 max_volume);
void app_audio_state_exit(u8 state);
u8 app_audio_get_state(void);
void volume_up_down_direct(s8 value);
void app_audio_set_mix_volume(u8 front_volume, u8 back_volume);
void app_audio_volume_init(void);
void app_audio_set_digital_volume(s16 volume);
void dac_trim_hook(u8 pos);

int audio_output_buf_time(void);
int audio_output_dev_is_working(void);
int audio_output_sync_start(void);
int audio_output_sync_stop(void);

void app_set_sys_vol(s16 vol_l, s16  vol_r);
void app_set_max_vol(s16 vol);

u32 phone_call_eq_open();
int eq_mode_sw();
int mic_test_start();
int mic_test_stop();

void dac_power_on(void);
void dac_power_off(void);


///2020-11-12
void User_bal_volume_set(s8 volume);
s8 User_get_bal_volume(void);
s8 User_bal_volume_dir(u8 dir);

void User_fad_volume_set(s8 volume);
s8 User_get_fad_volume(void);
s8 User_fad_volume_dir(u8 dir);

void User_set_volume(s16 volume);
#endif/*_AUDIO_CONFIG_H_*/
