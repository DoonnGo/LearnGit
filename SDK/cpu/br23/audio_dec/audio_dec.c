#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "classic/tws_api.h"
#include "classic/hci_lmp.h"
#include "effectrs_sync.h"
#include "application/audio_eq_drc_apply.h"
#include "app_config.h"
#include "audio_config.h"
#include "aec_user.h"
#include "audio_enc.h"
#include "audio_dec.h"
#include "app_main.h"
#include "encode/encode_write.h"
#include "audio_digital_vol.h"
#include "common/Resample_api.h"
/* #include "fm_emitter/fm_emitter_manage.h" */
/* #include "asm/audio_spdif.h" */
#include "clock_cfg.h"
#include "audio_link.h"
#include "btstack/avctp_user.h"
#include "application/audio_output_dac.h"
#include "application/audio_energy_detect.h"
#include "application/audio_dig_vol.h"
#include "audio_sbc_codec.h"
#include "application/audio_equalloudness_eq.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif

#if AUDIO_OUTPUT_AUTOMUTE
void *mix_out_automute_hdl = NULL;
struct list_head *mix_out_automute_entry = NULL;
extern void mix_out_automute_open();
extern void mix_out_automute_close();
#endif

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)
void *bt_digvol_last = NULL;
void *bt_digvol_last_entry = NULL;
#endif


#define AUDIO_CODEC_SUPPORT_SYNC	1 // 同步

#if (RECORDER_MIX_EN)
#define MAX_SRC_NUMBER      		5 // 最大支持src个数
#else
#define MAX_SRC_NUMBER      		3 // 最大支持src个数
#endif/*RECORDER_MIX_EN*/

#define AUDIO_DECODE_TASK_WAKEUP_TIME	0	// 解码定时唤醒 // ms

//////////////////////////////////////////////////////////////////////////////

struct audio_decoder_task 	decode_task = {0};
struct audio_mixer 			mixer = {0};
/*struct audio_stream_dac_out *dac_last = NULL;*/

static u8 audio_dec_inited = 0;

struct audio_eq_drc *mix_eq_drc = NULL;
#if AUDIO_EQUALLOUDNESS_CONFIG
loudness_hdl *loudness;
#endif

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
#if !TCFG_EQ_DIVIDE_ENABLE
struct channel_switch *mix_ch_switch = NULL;//声道变换
#endif
#endif

#if (TCFG_PREVENT_TASK_FILL)
struct prevent_task_fill *prevent_fill = NULL;
#endif

u8  audio_src_hw_filt[SRC_FILT_POINTS * SRC_CHI * 2 * MAX_SRC_NUMBER];
s16 mix_buff[AUDIO_MIXER_LEN / 2] SEC(.dec_mix_buff);
#if (RECORDER_MIX_EN)
struct audio_mixer recorder_mixer = {0};
s16 recorder_mix_buff[AUDIO_MIXER_LEN / 2] SEC(.dec_mix_buff);
#endif/*RECORDER_MIX_EN*/

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC)
#if AUDIO_CODEC_SUPPORT_SYNC
s16 dac_sync_buff[256];
#endif
#endif



extern const int config_mixer_en;

#define AUDIO_DEC_MIXER_EN		config_mixer_en

//////////////////////////////////////////////////////////////////////////////
void *mix_out_eq_drc_open(u16 sample_rate, u8 ch_num);
void mix_out_eq_drc_close(struct audio_eq_drc *eq_drc);

extern void audio_reverb_set_src_by_dac_sync(int in_rate, int out_rate);
extern void audio_linein_set_src_by_dac_sync(int in_rate, int out_rate);
extern void audio_usb_set_src_by_dac_sync(int in_rate, int out_rate);

//////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------------*/
/**@brief   获取dac能量值
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_dac_energy_get(void)
{
#if AUDIO_OUTPUT_AUTOMUTE
    int audio_energy_detect_energy_get(void *_hdl, u8 ch);
    if (mix_out_automute_hdl) {
        return audio_energy_detect_energy_get(mix_out_automute_hdl, BIT(0));
    }

    return (-1);
#else
    return 0;
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    激活所有解码
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_resume_all_decoder(void)
{
    audio_decoder_resume_all(&decode_task);
}

/*----------------------------------------------------------------------------*/
/**@brief    src中断处理
   @param
   @return
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
void audio_src_isr_deal(void)
{
    audio_resume_all_decoder();
}

/*----------------------------------------------------------------------------*/
/**@brief    dac变采样处理（同步）
   @param    in_rate: 输入采样率
   @param    out_rate: 输出采样率
   @return
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
void audio_dac_sync_src_deal(int in_rate, int out_rate)
{
#if TCFG_REVERB_ENABLE
    audio_reverb_set_src_by_dac_sync(in_rate, out_rate);
#endif
#if TCFG_LINEIN_ENABLE
    audio_linein_set_src_by_dac_sync(in_rate, out_rate);
#endif

#if TCFG_APP_PC_EN
#if TCFG_PC_ENABLE
    audio_usb_set_src_by_dac_sync(in_rate, out_rate);
#endif
#endif
}

#if AUDIO_DECODE_TASK_WAKEUP_TIME
#include "timer.h"
/*----------------------------------------------------------------------------*/
/**@brief    解码定时处理
   @param    *priv: 私有参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void audio_decoder_wakeup_timer(void *priv)
{
    //putchar('k');
    audio_resume_all_decoder();
}
/*----------------------------------------------------------------------------*/
/**@brief    添加一个解码预处理
   @param    *task: 解码任务
   @return   0: ok
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_decoder_task_add_probe(struct audio_decoder_task *task)
{
    if (task->wakeup_timer == 0) {
        task->wakeup_timer = sys_hi_timer_add(NULL, audio_decoder_wakeup_timer, AUDIO_DECODE_TASK_WAKEUP_TIME);
        log_i("audio_decoder_task_add_probe:%d\n", task->wakeup_timer);
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    删除一个解码预处理
   @param    *task: 解码任务
   @return   0: ok
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
int audio_decoder_task_del_probe(struct audio_decoder_task *task)
{
    log_i("audio_decoder_task_del_probe\n");
    if (audio_decoder_task_wait_state(task) > 0) {
        /*解码任务列表还有任务*/
        return 0;
    }
    if (task->wakeup_timer) {
        log_i("audio_decoder_task_del_probe:%d\n", task->wakeup_timer);
        sys_hi_timer_del(task->wakeup_timer);
        task->wakeup_timer = 0;
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    重定义唤醒时间
   @param    msecs: 唤醒时间ms
   @return   0: ok
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_decoder_wakeup_modify(int msecs)
{
    if (decode_task.wakeup_timer) {
        sys_hi_timer_modify(decode_task.wakeup_timer, msecs);
    }

    return 0;
}
#endif/*AUDIO_DECODE_TASK_WAKEUP_TIME*/

/*----------------------------------------------------------------------------*/
/**@brief    各模式主解码open
   @param	 state: 参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_mode_main_dec_open(u32 state)
{
#if 0
    // 等待提示音解码完
    tone_dec_wait_stop(200);
    // 等待当前dac中的数据输出完
    os_time_dly(audio_output_buf_time() / 10 + 1);
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    获取输出默认采样率
   @param
   @return   0: 采样率可变
   @return   非0: 固定采样率
   @note
*/
/*----------------------------------------------------------------------------*/
u32 audio_output_nor_rate(void)
{
#if TCFG_IIS_ENABLE
    return TCFG_IIS_OUTPUT_SR;
#endif
#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC)

#if (TCFG_REVERB_ENABLE || TCFG_MIC_EFFECT_ENABLE)
    /* return TCFG_REVERB_SAMPLERATE_DEFUAL; */
#endif
    /* return  app_audio_output_samplerate_select(input_rate, 1); */
#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)

#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_FM)
    return 41667;
#else
    return 44100;
#endif

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    获取输出采样率
   @param    input_rate: 输入采样率
   @return   输出采样率
   @note
*/
/*----------------------------------------------------------------------------*/
u32 audio_output_rate(int input_rate)
{
    u32 out_rate = audio_output_nor_rate();
    if (out_rate) {
        return out_rate;
    }

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)
    if ((bt_user_priv_var.emitter_or_receiver == BT_EMITTER_EN) && (!bt_phone_dec_is_running())) {
        y_printf("+++  \n");
        return audio_sbc_enc_get_rate();
    }
#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DONGLE)
    return DONGLE_OUTPUT_SAMPLE_RATE;
#endif

#if (TCFG_REVERB_ENABLE || TCFG_MIC_EFFECT_ENABLE)
    if (input_rate > 48000) {
        return 48000;
    }
#endif
    //y_printf("+++ 11 \n");
    return  app_audio_output_samplerate_select(input_rate, 1);
}

/*----------------------------------------------------------------------------*/
/**@brief    获取输出通道数
   @param
   @return   输出通道数
   @note
*/
/*----------------------------------------------------------------------------*/
u32 audio_output_channel_num(void)
{
#if ((AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC) || (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT))

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)
    if (bt_user_priv_var.emitter_or_receiver == BT_EMITTER_EN) {
        return audio_sbc_enc_get_channel_num();
    }
#endif

    /*根据DAC输出的方式选择输出的声道*/
    u8 dac_connect_mode =  app_audio_output_mode_get();
    if (dac_connect_mode == DAC_OUTPUT_LR || dac_connect_mode == DAC_OUTPUT_DUAL_LR_DIFF) {
        return 2;
    } else if (dac_connect_mode == DAC_OUTPUT_FRONT_LR_REAR_LR) {
        return 2;
    } else {
        return 1;
    }
#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_FM)
    return  2;
#else
    return  2;
#endif
}

u32 audio_output_channel_type(void)
{
#if ((AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC) || (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT))

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)
    if (bt_user_priv_var.emitter_or_receiver == BT_EMITTER_EN) {
        if (audio_sbc_enc_get_channel_num() == 2) {
            return AUDIO_CH_LR;
        }
        return AUDIO_CH_DIFF;
    }
#endif

    /*根据DAC输出的方式选择输出的声道*/
    u8 dac_connect_mode =  app_audio_output_mode_get();
    if (dac_connect_mode == DAC_OUTPUT_LR || dac_connect_mode == DAC_OUTPUT_DUAL_LR_DIFF) {
        return AUDIO_CH_LR;
    } else if (dac_connect_mode == DAC_OUTPUT_FRONT_LR_REAR_LR) {
        return AUDIO_CH_LR;
    } else if (dac_connect_mode == DAC_OUTPUT_MONO_L) {
        return AUDIO_CH_L;
    } else if (dac_connect_mode == DAC_OUTPUT_MONO_R) {
        return AUDIO_CH_R;
    } else {
        return AUDIO_CH_DIFF;
    }
#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_FM)
    return  AUDIO_CH_LR;
#else
    return  AUDIO_CH_LR;
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    设置输出音量状态
   @param    state: 输出音量状态
   @return   0: ok
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_output_set_start_volume(u8 state)
{
    s16 vol_max = get_max_sys_vol();
    if (state == APP_AUDIO_STATE_CALL) {
        vol_max = app_var.aec_dac_gain;
    }
    app_audio_state_switch(state, vol_max);
    return 0;
}


/*----------------------------------------------------------------------------*/
/**@brief    开始音频输出
   @param    sample_rate: 输出采样率
   @param    reset_rate: 更新输出采样率
   @return   0: ok
   @note
*/
/*----------------------------------------------------------------------------*/
u8 audio_output_flag = 0;
int audio_output_start(u32 sample_rate, u8 reset_rate)
{
    if (reset_rate) {
        app_audio_output_samplerate_set(sample_rate);
    }

    if (audio_output_flag) {
        return 0;
    }

    audio_output_flag = 1;
    app_audio_output_start();

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    关闭音频输出
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_output_stop(void)
{
    audio_output_flag = 0;

    app_audio_output_stop();
}

/*----------------------------------------------------------------------------*/
/**@brief    打开一个变采样通道
   @param    *priv: output回调私有句柄
   @param    *output_handler: 变采样输出回调
   @param    *channel: 声道数
   @param    *input_sample_rate: 输入采样率
   @param    *output_sample_rate: 输出采样率
   @return   变采样句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct audio_src_handle *audio_hw_resample_open(void *priv,
        int (*output_handler)(void *, void *, int),
        u8 channel,
        u16 input_sample_rate,
        u16 output_sample_rate)
{
    struct audio_src_handle *hdl;
    hdl = zalloc(sizeof(struct audio_src_handle));
    if (hdl) {
        audio_hw_src_open(hdl, channel, SRC_TYPE_RESAMPLE);
        audio_hw_src_set_rate(hdl, input_sample_rate, output_sample_rate);
        audio_src_set_output_handler(hdl, priv, output_handler);
    }

    return hdl;
}

/*----------------------------------------------------------------------------*/
/**@brief    关闭变采样
   @param    *hdl: 变采样句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_hw_resample_close(struct audio_src_handle *hdl)
{
    if (hdl) {
        audio_hw_src_stop(hdl);
        audio_hw_src_close(hdl);
        free(hdl);
    }
}


/*----------------------------------------------------------------------------*/
/**@brief    mixer事件处理
   @param    *mixer: 句柄
   @param    event: 事件
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void mixer_event_handler(struct audio_mixer *mixer, int event)
{
    switch (event) {
    case MIXER_EVENT_OPEN:
        if (audio_mixer_get_ch_num(mixer) >= 1) {
            clock_add_set(DEC_MIX_CLK);
        }
        break;
    case MIXER_EVENT_CLOSE:
        if (audio_mixer_get_ch_num(mixer) == 0) {
            clock_remove_set(DEC_MIX_CLK);
        }

        {
#if TCFG_REVERB_ENABLE
            reset_reverb_src_out(audio_output_rate(audio_mixer_get_sample_rate(mixer)));
#endif
        }
        break;
    case MIXER_EVENT_SR_CHANGE:
#if 0
        y_printf("sr change:%d \n", mixer->sample_rate);
#endif
        break;
    }

}
/*----------------------------------------------------------------------------*/
/**@brief    检测mixer采样率支持
   @param    *mixer: 句柄
   @param    sr: 采样率
   @return   支持的采样率
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 audio_mixer_check_sr(struct audio_mixer *mixer, u16 sr)
{
    return audio_output_rate(sr);;
}

/*----------------------------------------------------------------------------*/
/**@brief    最后一级stream激活
   @param    *p: 私有参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void audio_last_out_stream_resume(void *p)
{
}

/*----------------------------------------------------------------------------*/
/**@brief    获取系统软件数字音量
   @param    无
   @return   返回系统软件数字音量值
   @note    改回调由用户实现
*/
/*----------------------------------------------------------------------------*/
int vol_get_test()
{
    u8 vol = 0;
    /* vol = audio_dig_vol_get(digvol_last, AUDIO_DIG_VOL_ALL_CH); */
    return vol;
}


/*----------------------------------------------------------------------------*/
/**@brief    音频解码初始化
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
extern struct audio_dac_hdl dac_hdl;
int audio_dec_init()
{
    int err;

    printf("audio_dec_init\n");


    err = audio_decoder_task_create(&decode_task, "audio_dec");

#if (TCFG_PREVENT_TASK_FILL)
    prevent_fill = prevent_task_fill_create("prevent");
    if (prevent_fill) {
        decode_task.prevent_fill = prevent_task_fill_ch_open(prevent_fill, 20);
    }
#endif


    app_audio_output_init();

    /*硬件SRC模块滤波器buffer设置，可根据最大使用数量设置整体buffer*/
    audio_src_base_filt_init(audio_src_hw_filt, sizeof(audio_src_hw_filt));

    if (!AUDIO_DEC_MIXER_EN) {
        goto __mixer_init_end;
    }
    audio_mixer_open(&mixer);
    audio_mixer_set_event_handler(&mixer, mixer_event_handler);
    audio_mixer_set_check_sr_handler(&mixer, audio_mixer_check_sr);
    if (config_mixer_en) {
        /*初始化mix_buf的长度*/
        audio_mixer_set_output_buf(&mixer, mix_buff, sizeof(mix_buff));
    }
    u8 ch_num = audio_output_channel_num();
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
#if TCFG_EQ_DIVIDE_ENABLE
    ch_num = 4;
#endif
#endif
    audio_mixer_set_channel_num(&mixer, ch_num);
    u16 sr = audio_output_nor_rate();
    if (sr) {
        // 固定采样率输出
        audio_mixer_set_sample_rate(&mixer, MIXER_SR_SPEC, sr);
    }
    struct audio_stream_entry *entries[8] = {NULL};

#if !TCFG_EQ_DIVIDE_ENABLE
    /*dac_last = audio_stream_dac_out_open();*/
    mix_eq_drc = mix_out_eq_drc_open(sr, ch_num);
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
    mix_ch_switch = channel_switch_open(AUDIO_CH_QUAD, AUDIO_SYNTHESIS_LEN / 2);
#endif
#endif

#if AUDIO_OUTPUT_AUTOMUTE
    mix_out_automute_open();
#endif
#if AUDIO_EQUALLOUDNESS_CONFIG
    loudness_open_parm parm = {0};
    parm.sr = sr;
    parm.ch_num = ch_num;
    parm.threadhold_vol = LOUDNESS_THREADHOLD_VOL;
    parm.vol_cb = vol_get_test;
    loudness = audio_equal_loudness_open(&parm);
#endif

    // 数据流串联。可以在mixer和last中间添加其他的数据流，比如eq等
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &mixer.entry;

#if AUDIO_EQUALLOUDNESS_CONFIG
    if (loudness) {
        entries[entry_cnt++] = &loudness->loudness->entry;
    }
#endif
#if TCFG_EQ_ENABLE && TCFG_AUDIO_OUT_EQ_ENABLE
    if (mix_eq_drc) {
        entries[entry_cnt++] = &mix_eq_drc->entry;
    }
#endif

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
#if !TCFG_EQ_DIVIDE_ENABLE
    if (mix_ch_switch) {
        entries[entry_cnt++] = &mix_ch_switch->entry;
    }
#endif
#endif

#if AUDIO_OUTPUT_AUTOMUTE
    entries[entry_cnt++] = mix_out_automute_entry;
#endif
    entries[entry_cnt++] = &dac_hdl.entry;

    mixer.stream = audio_stream_open(NULL, audio_last_out_stream_resume);
    audio_stream_add_list(mixer.stream, entries, entry_cnt);

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)
    audio_dig_vol_param bt_digvol_last_param = {
        .vol_start = app_var.music_volume,
        .vol_max = SYS_MAX_VOL,
        .ch_total = 2,
        .fade_en = 1,
        .fade_points_step = 5,
        .fade_gain_step = 10,
        .vol_list = NULL,
    };
    bt_digvol_last = audio_dig_vol_open(&bt_digvol_last_param);
    bt_digvol_last_entry = audio_dig_vol_entry_get(bt_digvol_last);
    audio_stream_add_entry(entries[entry_cnt - 2], bt_digvol_last_entry);

    audio_sbc_emitter_init();
    audio_stream_add_entry(bt_digvol_last_entry, &sbc_emitter.entry);
#endif

__mixer_init_end:

#if (RECORDER_MIX_EN)
    recorder_mix_init(&recorder_mixer, recorder_mix_buff, sizeof(recorder_mix_buff));
#endif//RECORDER_MIX_EN

    app_audio_volume_init();
    audio_output_set_start_volume(APP_AUDIO_STATE_MUSIC);

#if TCFG_IIS_ENABLE
    audio_link_init();
#if TCFG_IIS_OUTPUT_EN
    audio_link_open(TCFG_IIS_OUTPUT_PORT, ALINK_DIR_TX);
#endif
#endif

    audio_dec_inited = 1;
#if TCFG_SPDIF_ENABLE
    spdif_init();
#endif

    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    音频解码初始化判断
   @param
   @return   1: 还没初始化
   @return   0: 已经初始化
   @note
*/
/*----------------------------------------------------------------------------*/
static u8 audio_dec_init_complete()
{
    if (!audio_dec_inited) {
        return 0;
    }

    return 1;
}
REGISTER_LP_TARGET(audio_dec_init_lp_target) = {
    .name = "audio_dec_init",
    .is_idle = audio_dec_init_complete,
};


struct drc_ch high_bass_drc = {0};
static int high_bass_th = 0;
/*----------------------------------------------------------------------------*/
/**@brief    高低音限幅器系数回调
   @param    *drc: 句柄
   @param    *info: 系数结构地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int high_bass_drc_set_filter_info(int th)
{
    /* int th = 0; // -60 ~ 0 db  */
    if (th < -60) {
        th = -60;
    }
    if (th > 0) {
        th = 0;
    }
    local_irq_disable();
    high_bass_th = th;
    if (mix_eq_drc && mix_eq_drc->drc) {
        mix_eq_drc->drc->updata = 1;
    }
    local_irq_enable();
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    高低音限幅器系数回调
   @param    *drc: 句柄
   @param    *info: 系数结构地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int high_bass_drc_get_filter_info(struct audio_drc *drc, struct audio_drc_filter_info *info)
{
    int th = high_bass_th;//-60 ~0db
    int threshold = round(pow(10.0, th / 20.0) * 32768); // 0db:32768, -60db:33

    high_bass_drc.nband = 1;
    high_bass_drc.type = 1;
    high_bass_drc._p.limiter[0].attacktime = 5;
    high_bass_drc._p.limiter[0].releasetime = 300;
    high_bass_drc._p.limiter[0].threshold[0] = threshold;
    high_bass_drc._p.limiter[0].threshold[1] = 32768;

    info->pch = info->R_pch = &high_bass_drc;
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    高低音eq打开
   @param    sample_rate:采样率
   @param    ch_num:通道个数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void *mix_out_eq_drc_open(u16 sample_rate, u8 ch_num)
{

#if TCFG_EQ_ENABLE

    struct audio_eq_drc *eq_drc = NULL;
    struct audio_eq_drc_parm effect_parm = {0};
#if TCFG_AUDIO_OUT_EQ_ENABLE
    effect_parm.high_bass = 1;
    effect_parm.eq_en = 1;

#if TCFG_DRC_ENABLE
#if TCFG_AUDIO_OUT_DRC_ENABLE
    effect_parm.drc_en = 1;
    effect_parm.drc_cb = high_bass_drc_get_filter_info;
#endif
#endif
    log_i("=====sr %d, ch_num %d\n", sample_rate, ch_num);

    if (effect_parm.eq_en) {
        effect_parm.async_en = 1;
        effect_parm.out_32bit = 1;
        effect_parm.online_en = 0;
        effect_parm.mode_en = 0;
    }

    effect_parm.eq_name = song_eq_mode;


    effect_parm.ch_num = ch_num;
    effect_parm.sr = sample_rate;
    printf("ch_num %d\n,sr %d\n", ch_num, sample_rate);
    eq_drc = audio_eq_drc_open(&effect_parm);

    clock_add(EQ_CLK);
    if (effect_parm.drc_en) {
        clock_add(EQ_DRC_CLK);
    }
#endif
    return eq_drc;
#endif

    return NULL;
}
/*----------------------------------------------------------------------------*/
/**@brief    高低音eq关闭
   @param    *eq_drc:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void mix_out_eq_drc_close(struct audio_eq_drc *eq_drc)
{
#if TCFG_EQ_ENABLE
#if TCFG_AUDIO_OUT_EQ_ENABLE
    if (eq_drc) {
        audio_eq_drc_close(eq_drc);
        eq_drc = NULL;
        clock_remove(EQ_CLK);
#if TCFG_DRC_ENABLE
#if TCFG_AUDIO_OUT_DRC_ENABLE
        clock_remove(EQ_DRC_CLK);
#endif
#endif
    }
#endif
#endif

    return;
}

/*----------------------------------------------------------------------------*/
/**@brief    mix out后 做高低音
   @param    cmd:AUDIO_EQ_HIGH 高音 ,AUDIO_EQ_BASS 低音
   @param    hb:中心截止频率与增益,频率写0时，使用默认的125 125khz
   @return
   @note
*/
/*----------------------------------------------------------------------------*/

void mix_out_high_bass(u32 cmd, struct high_bass *hb)
{
    if (mix_eq_drc) {
        audio_eq_drc_parm_update(mix_eq_drc, cmd, (void *)hb);
    }
}
/*----------------------------------------------------------------------------*/
/**@brief    mix out后 是否做高低音处理
   @param    cmd:AUDIO_EQ_HIGH_BASS_DIS
   @param    dis:0 默认做高低音处理，1：不做高低音处理
   @return
   @note    该接口可用于控制某些模式不做高低音
*/
/*----------------------------------------------------------------------------*/
void mix_out_high_bass_dis(u32 cmd, u8 dis)
{
    if (mix_eq_drc) {
        audio_eq_drc_parm_update(mix_eq_drc, cmd, (void *)dis);
    }
}

#if AUDIO_OUTPUT_AUTOMUTE

void audio_mix_out_automute_mute(u8 mute)
{
    printf(">>>>>>>>>>>>>>>>>>>> %s\n", mute ? ("MUTE") : ("UNMUTE"));
}

/* #define AUDIO_E_DET_UNMUTE      (0x00) */
/* #define AUDIO_E_DET_MUTE        (0x01) */
void mix_out_automute_handler(u8 event, u8 ch)
{
    printf(">>>> ch:%d %s\n", ch, event ? ("MUTE") : ("UNMUTE"));
    if (ch == app_audio_output_channel_get()) {
        audio_mix_out_automute_mute(event);
    }
}

void mix_out_automute_skip(u8 skip)
{
    u8 mute = !skip;
    if (mix_out_automute_hdl) {
        audio_energy_detect_skip(mix_out_automute_hdl, 0xFFFF, skip);
        audio_mix_out_automute_mute(mute);
    }
}

void mix_out_automute_open()
{
    if (mix_out_automute_hdl) {
        printf("mix_out_automute is already open !\n");
        return;
    }
    audio_energy_detect_param e_det_param = {0};
    e_det_param.mute_energy = 5;
    e_det_param.unmute_energy = 10;
    e_det_param.mute_time_ms = 1000;
    e_det_param.unmute_time_ms = 50;
    e_det_param.count_cycle_ms = 10;
    e_det_param.sample_rate = 44100;
    e_det_param.event_handler = mix_out_automute_handler;
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
    e_det_param.ch_total = 4;
#else
    e_det_param.ch_total = app_audio_output_channel_get();
#endif
    e_det_param.dcc = 1;
    mix_out_automute_hdl = audio_energy_detect_open(&e_det_param);
    mix_out_automute_entry = audio_energy_detect_entry_get(mix_out_automute_hdl);
}

void mix_out_automute_close()
{
    if (mix_out_automute_hdl) {
        audio_energy_detect_close(mix_out_automute_hdl);
    }
}
#endif  //#if AUDIO_OUTPUT_AUTOMUTE


