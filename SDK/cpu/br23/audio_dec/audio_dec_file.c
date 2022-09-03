/*
 ****************************************************************
 *File : audio_dec_file.c
 *Note :
 *
 ****************************************************************
 */
//////////////////////////////////////////////////////////////////////////////
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "effectrs_sync.h"
#include "app_config.h"
#include "audio_config.h"
#include "audio_dec.h"
#include "app_config.h"
#include "app_main.h"
#include "classic/tws_api.h"

#include "music/music_decrypt.h"
#include "music/music_id3.h"
#include "pitchshifter/pitchshifter_api.h"
#include "mono2stereo/reverb_mono2stero_api.h"
#include "audio_enc.h"
#include "clock_cfg.h"
#include "application/audio_pitch.h"
#include "application/audio_eq_drc_apply.h"
#include "audio_base.h"
#include "channel_switch.h"


#if TCFG_APP_MUSIC_EN


#define FILE_DEC_PICK_EN			1 // 本地解码拆包转发

#if (!TCFG_DEC2TWS_ENABLE)
#undef FILE_DEC_PICK_EN
#define FILE_DEC_PICK_EN			0
#endif

#ifndef BREAKPOINT_DATA_LEN
#define BREAKPOINT_DATA_LEN			32
#endif

const int FILE_DEC_ONCE_OUT_NUM	= ((512 * 4) * 2);	// 一次最多输出长度。避免多解码叠加时卡住其他解码太长时间


//////////////////////////////////////////////////////////////////////////////

struct dec_type {
    u32 type;	// 解码类型
    u32 clk;	// 解码时钟
};

const struct dec_type  dec_clk_tb[] = {
    {AUDIO_CODING_MP3,  DEC_MP3_CLK},
    {AUDIO_CODING_WAV,  DEC_WAV_CLK},
    {AUDIO_CODING_G729, DEC_G729_CLK},
    {AUDIO_CODING_G726, DEC_G726_CLK},
    {AUDIO_CODING_PCM,  DEC_PCM_CLK},
    {AUDIO_CODING_MTY,  DEC_MTY_CLK},
    {AUDIO_CODING_WMA,  DEC_WMA_CLK},

    {AUDIO_CODING_APE,  DEC_APE_CLK},
    {AUDIO_CODING_FLAC, DEC_FLAC_CLK},
    {AUDIO_CODING_DTS,  DEC_DTS_CLK},
    {AUDIO_CODING_M4A,  DEC_M4A_CLK},
    {AUDIO_CODING_ALAC,  DEC_ALAC_CLK},
    {AUDIO_CODING_MIDI, DEC_MIDI_CLK},

    {AUDIO_CODING_MP3 | AUDIO_CODING_STU_PICK,  DEC_MP3PICK_CLK},
    {AUDIO_CODING_WMA | AUDIO_CODING_STU_PICK,  DEC_WMAPICK_CLK},
    {AUDIO_CODING_M4A | AUDIO_CODING_STU_PICK,  DEC_M4APICK_CLK},
};

//////////////////////////////////////////////////////////////////////////////

struct file_dec_hdl *file_dec = NULL;	// 文件解码句柄
u8 file_dec_start_pause = 0;	// 启动解码后但不马上开始播放


//////////////////////////////////////////////////////////////////////////////

void *file_eq_drc_open(u16 sample_rate, u8 ch_num);
void file_eq_drc_close(struct audio_eq_drc *eq_drc);
void *file_rl_rr_eq_drc_open(u16 sample_rate, u8 ch_num);
void file_rl_rr_eq_drc_close(struct audio_eq_drc *eq_drc);

extern void put_u16hex(u16 dat);

extern int tws_api_get_tws_state();
extern void local_tws_sync_no_check_data_buf(u8 no_check);

int file_dec_repeat_set(u8 repeat_num);

//////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------------*/
/**@brief    获取文件解码hdl
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void *get_file_dec_hdl()
{
    return file_dec;
}
/*----------------------------------------------------------------------------*/
/**@brief    解码时钟添加
   @param    type: 解码类型
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void dec_clock_add(u32 type)
{
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(dec_clk_tb); i++) {
        if (type == dec_clk_tb[i].type) {
            clock_add(dec_clk_tb[i].clk);
            return;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    解码时钟移除
   @param    type: 解码类型
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void dec_clock_remove(u32 type)
{
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(dec_clk_tb); i++) {
        if (type == dec_clk_tb[i].type) {
            clock_remove(dec_clk_tb[i].clk);
            return;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    读取文件数据
   @param    *decoder: 解码器句柄
   @param    *buf: 数据
   @param    len: 数据长度
   @return   >=0：读到的数据长度
   @return   <0：错误
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_fread(struct audio_decoder *decoder, void *buf, u32 len)
{
    struct file_decoder *file_dec = container_of(decoder, struct file_decoder, decoder);
    struct file_dec_hdl *dec = container_of(file_dec, struct file_dec_hdl, file_dec);
    int rlen;
#if TCFG_DEC_DECRYPT_ENABLE
    u32 addr;
    addr = fpos(dec->file);
    rlen = fread(dec->file, buf, len);
    if (rlen && (rlen <= len)) {
        cryptanalysis_buff(&dec->mply_cipher, buf, addr, rlen);
    }
#else
    rlen = fread(dec->file, buf, len);
#endif
    if (rlen > len) {
        // putchar('r');
        rlen = 0;
        dec->read_err = 1;
    } else {
        // putchar('R');
        dec->read_err = 0;
    }
    return rlen;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件指针定位
   @param    *decoder: 解码器句柄
   @param    offset: 定位偏移
   @param    seek_mode: 定位类型
   @return   0：成功
   @return   非0：错误
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_fseek(struct audio_decoder *decoder, u32 offset, int seek_mode)
{
    struct file_decoder *file_dec = container_of(decoder, struct file_decoder, decoder);
    struct file_dec_hdl *dec = container_of(file_dec, struct file_dec_hdl, file_dec);
    return fseek(dec->file, offset, seek_mode);
}

/*----------------------------------------------------------------------------*/
/**@brief    读取文件长度
   @param    *decoder: 解码器句柄
   @return   文件长度
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_flen(struct audio_decoder *decoder)
{
    struct file_decoder *file_dec = container_of(decoder, struct file_decoder, decoder);
    struct file_dec_hdl *dec = container_of(file_dec, struct file_dec_hdl, file_dec);
    int len = 0;
    len = flen(dec->file);
    return len;
}

static const u32 file_input_coding_more[] = {
#if TCFG_DEC_MP3_ENABLE
    AUDIO_CODING_MP3,
#endif
    0,
};

static const struct audio_dec_input file_input = {
    .coding_type = 0
#if TCFG_DEC_WMA_ENABLE
    | AUDIO_CODING_WMA
#endif
#if TCFG_DEC_WAV_ENABLE
    | AUDIO_CODING_WAV
#endif
#if TCFG_DEC_FLAC_ENABLE
    | AUDIO_CODING_FLAC
#endif
#if TCFG_DEC_APE_ENABLE
    | AUDIO_CODING_APE
#endif
#if TCFG_DEC_M4A_ENABLE
    | AUDIO_CODING_M4A
#endif
#if TCFG_DEC_ALAC_ENABLE
    | AUDIO_CODING_ALAC
#endif
#if TCFG_DEC_AMR_ENABLE
    | AUDIO_CODING_AMR
#endif
#if TCFG_DEC_DTS_ENABLE
    | AUDIO_CODING_DTS
#endif
#if TCFG_DEC_G726_ENABLE
    | AUDIO_CODING_G726
#endif
#if TCFG_DEC_MIDI_ENABLE
    | AUDIO_CODING_MIDI
#endif
    ,
    .p_more_coding_type = (u32 *)file_input_coding_more,
    .data_type   = AUDIO_INPUT_FILE,
    .ops = {
        .file = {
            .fread = file_fread,
            .fseek = file_fseek,
            .flen  = file_flen,
        }
    }
};

/*----------------------------------------------------------------------------*/
/**@brief    文件解码释放
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void file_dec_release(void)
{
    struct file_dec_hdl *dec = file_dec;

#if TCFG_DEC_ID3_V1_ENABLE
    if (dec->p_mp3_id3_v1) {
        id3_obj_post(&dec->p_mp3_id3_v1);
    }
#endif
#if TCFG_DEC_ID3_V2_ENABLE
    if (dec->p_mp3_id3_v2) {
        id3_obj_post(&dec->p_mp3_id3_v2);
    }
#endif

    audio_decoder_task_del_wait(&decode_task, &dec->wait);

    if (dec->file_dec.decoder.fmt.coding_type) {
        dec_clock_remove(dec->file_dec.decoder.fmt.coding_type);
    }

    local_irq_disable();
    if (file_dec->dec_bp) {
        free(file_dec->dec_bp);
        file_dec->dec_bp = NULL;
    }
    free(file_dec);
    file_dec = NULL;
    local_irq_enable();
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码事件处理
   @param    *decoder: 解码器句柄
   @param    argc: 参数个数
   @param    *argv: 参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void file_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        log_i("AUDIO_DEC_EVENT_END\n");
        if (!file_dec) {
            log_i("file_dec handle err ");
            break;
        }

        if (file_dec->id != argv[1]) {
            log_w("file_dec id err : 0x%x, 0x%x \n", file_dec->id, argv[1]);
            break;
        }

        // 有回调，让上层close，避免close后上层读不到断点等
        if (file_dec->evt_cb) {
            /* file_dec->evt_cb(file_dec->evt_priv, argc, argv); */
            int msg[2];
            msg[0] = argv[0];
            msg[1] = file_dec->read_err;
            /* log_i("read err0:%d ", file_dec->read_err); */
            file_dec->evt_cb(file_dec->evt_priv, 2, msg);
        } else {
            file_dec_close();
        }
        //audio_decoder_resume_all(&decode_task);
        break;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码数据流激活
   @param    *p: 私有句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void file_dec_out_stream_resume(void *p)
{
    struct file_dec_hdl *dec = p;
    audio_decoder_resume(&dec->file_dec.decoder);
}


/*----------------------------------------------------------------------------*/
/**@brief    文件解码开始
   @param
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_dec_start(void)
{
    int err;
    struct file_dec_hdl *dec = file_dec;
    struct audio_mixer *p_mixer = &mixer;
    u8 pcm_enc_flag = 0;

    if (!dec) {
        return -EINVAL;
    }

    log_i("file_dec_start: in\n");

#if FILE_DEC_PICK_EN
    if (localtws_check_enable() == true) {
        // 使用拆包方式传输
        err = file_decoder_open(&dec->file_dec, &file_input, &decode_task,
                                dec->bp, 1);
        if (err == 0) {
            dec->pick_flag = 1;
            dec->file_dec.dec_no_out_sound = 1;
            // 同步不需要检查buf变化
            local_tws_sync_no_check_data_buf(1);
            // 打开localtws推送
            localtws_push_open();
            // 启动localtws功能
            localtws_start(&dec->file_dec.decoder.fmt);
            // 关闭资源等待。最终会在localtws解码处等待
            audio_decoder_task_del_wait(&decode_task, &dec->wait);
            goto __open_ok;
        }
    }
#endif

    // 打开file解码器
    err = file_decoder_open(&dec->file_dec, &file_input, &decode_task,
                            dec->bp, 0);
    if (err) {
        goto __err1;
    }

__open_ok:
    file_decoder_set_event_handler(&dec->file_dec, file_dec_event_handler, dec->id);

    // 获取id3
    if (dec->file_dec.decoder.dec_ops->coding_type == AUDIO_CODING_MP3) {
#if TCFG_DEC_ID3_V1_ENABLE
        if (dec->p_mp3_id3_v1) {
            id3_obj_post(&dec->p_mp3_id3_v1);
        }
        dec->p_mp3_id3_v1 = id3_v1_obj_get(dec->file);
#endif
#if TCFG_DEC_ID3_V2_ENABLE
        if (dec->p_mp3_id3_v2) {
            id3_obj_post(&dec->p_mp3_id3_v2);
        }
        dec->p_mp3_id3_v2 = id3_v2_obj_get(dec->file);
#endif
    }


#if TCFG_PCM_ENC2TWS_ENABLE
    if (dec->file_dec.dec_no_out_sound == 0) {
        // localtws
        struct audio_fmt enc_f;
        memcpy(&enc_f, &dec->file_dec.decoder.fmt, sizeof(struct audio_fmt));
        enc_f.coding_type = AUDIO_CODING_SBC;
        enc_f.channel = dec->file_dec.output_ch_num;
        int ret = localtws_enc_api_open(&enc_f, 0);
        if (ret == true) {
            dec->file_dec.dec_no_out_sound = 1;
            // 重定向mixer
            p_mixer = &g_localtws.mixer;
            // 关闭资源等待。最终会在localtws解码处等待
            audio_decoder_task_del_wait(&decode_task, &dec->wait);
            // 重新设置解码输出声道
            file_decoder_set_output_channel(&dec->file_dec);
        }
    }
#endif

#if TCFG_SPEED_PITCH_ENABLE
    static	PS69_CONTEXT_CONF pitch_param;
    pitch_param.pitchV = 32768;//32767 是原始音调  >32768是音调变高，《32768 音调变低，建议范围20000 - 50000
    pitch_param.speedV = 40;//>80变快,<80 变慢，建议范围30-130
    pitch_param.sr = dec->file_dec.sample_rate ;
    pitch_param.chn = dec->file_dec.output_ch_num ;
    dec->p_pitchspeed_hdl = open_pitchspeed(&pitch_param, NULL);
#endif
    if (!dec->file_dec.dec_no_out_sound) {
        audio_mode_main_dec_open(AUDIO_MODE_MAIN_STATE_DEC_FILE);
    }
    // 设置叠加功能
    audio_mixer_ch_open_head(&dec->mix_ch, p_mixer);
    audio_mixer_ch_set_src(&dec->mix_ch, 1, 0);


    dec_clock_add(dec->file_dec.decoder.dec_ops->coding_type);

    /* #if TCFG_DEC_MIDI_ENABLE */
    /* // midi功能设置 */
    /* if (dec->file_dec.decoder.dec_ops->coding_type == AUDIO_CODING_MIDI) { */
    /* extern int midi_init(void *info); */
    /* memset(&dec->midi_init_info_val, 0, sizeof(MIDI_INIT_STRUCT)); */
    /* int ret = midi_init(&dec->midi_init_info_val); */
    /* if (ret) { */
    /* goto __err3; */
    /* } */
    /* audio_decoder_ioctrl(&dec->file_dec.decoder, CMD_INIT_CONFIG, &dec->midi_init_info_val); */
    /* } */
    /* #endif */
    if (dec->stream_handler && (dec->pick_flag == 0)) {
        dec->stream_handler(dec->stream_priv, FILE_DEC_STREAM_OPEN, dec);
        goto __stream_set_end;
    }
    dec->eq_drc = file_eq_drc_open(dec->file_dec.sample_rate, dec->file_dec.output_ch_num);

#if TCFG_EQ_DIVIDE_ENABLE
    dec->eq_drc_rl_rr = file_rl_rr_eq_drc_open(dec->file_dec.sample_rate, dec->file_dec.output_ch_num);
    if (dec->eq_drc_rl_rr) {
        audio_vocal_tract_open(&dec->vocal_tract, AUDIO_SYNTHESIS_LEN);
        audio_vocal_tract_synthesis_open(&dec->synthesis_ch_fl_fr, &dec->vocal_tract, FL_FR);
        audio_vocal_tract_synthesis_open(&dec->synthesis_ch_rl_rr, &dec->vocal_tract, RL_RR);
    } else {
        dec->ch_switch = channel_switch_open(AUDIO_CH_QUAD, AUDIO_SYNTHESIS_LEN / 2);
    }
#endif
    // 数据流串联
    struct audio_stream_entry *entries[8] = {NULL};
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &dec->file_dec.decoder.entry;

#if FILE_DEC_PICK_EN
    if (dec->pick_flag) {
        // 拆包直接输出到localtws推送，中间不经过任何处理
        entries[entry_cnt++] = &g_localtws.push.entry;
    } else
#endif
    {
#if TCFG_SPEED_PITCH_ENABLE
        if (dec->p_pitchspeed_hdl) {
            entries[entry_cnt++] = &dec->p_pitchspeed_hdl->entry;
        }
#endif

#if TCFG_EQ_ENABLE && TCFG_MUSIC_MODE_EQ_ENABLE
        if (dec->eq_drc) {
            entries[entry_cnt++] = &dec->eq_drc->entry;
        }
#endif
#if TCFG_EQ_DIVIDE_ENABLE
        if (dec->eq_drc_rl_rr) {
            entries[entry_cnt++] = &dec->synthesis_ch_fl_fr.entry;
        } else {
            if (dec->ch_switch) {
                entries[entry_cnt++] = &dec->ch_switch->entry;
            }
        }
#endif

        entries[entry_cnt++] = &dec->mix_ch.entry;
    }
    dec->stream = audio_stream_open(dec, file_dec_out_stream_resume);
    audio_stream_add_list(dec->stream, entries, entry_cnt);

#if TCFG_EQ_DIVIDE_ENABLE
    if (dec->eq_drc_rl_rr) { //接在eq_drc的上一个节点
        audio_stream_add_entry(entries[entry_cnt - 4], &dec->eq_drc_rl_rr->entry);
        audio_stream_add_entry(&dec->eq_drc_rl_rr->entry, &dec->synthesis_ch_rl_rr.entry);
    }
#endif

__stream_set_end:
    log_i("total_time : %d \n", dec->file_dec.dec_total_time);

#if FILE_DEC_REPEAT_EN
    file_dec_repeat_set(3);
#endif

    audio_output_set_start_volume(APP_AUDIO_STATE_MUSIC);

    // 文件打开就暂停
    if (file_dec_start_pause) {
        log_i("file_dec_start_pause\n");
        file_dec_start_pause = 0;
        dec->file_dec.status = FILE_DEC_STATUS_PAUSE;
        return 0;
    }

    // 设置时钟
    clock_set_cur();

    dec->file_dec.status = FILE_DEC_STATUS_PLAY;
    err = audio_decoder_start(&dec->file_dec.decoder);
    if (err) {
        goto __err3;
    }
    return 0;

__err3:
    dec->file_dec.status = 0;
#if TCFG_SPEED_PITCH_ENABLE
    if (dec->p_pitchspeed_hdl) {
        close_pitchspeed(dec->p_pitchspeed_hdl);
    }
#endif
    file_eq_drc_close(dec->eq_drc);
#if TCFG_EQ_DIVIDE_ENABLE
    file_rl_rr_eq_drc_close(dec->eq_drc_rl_rr);
    audio_vocal_tract_synthesis_close(&dec->synthesis_ch_fl_fr);
    audio_vocal_tract_synthesis_close(&dec->synthesis_ch_rl_rr);
    audio_vocal_tract_close(&dec->vocal_tract);
    channel_switch_close(&dec->ch_switch);
#endif
    audio_mixer_ch_close(&dec->mix_ch);
#if TCFG_PCM_ENC2TWS_ENABLE
    if (file_dec->file_dec.dec_no_out_sound) {
        file_dec->file_dec.dec_no_out_sound = 0;
        localtws_enc_api_close();
    }
#endif
    if (dec->stream_handler) {
        dec->stream_handler(dec->stream_priv, FILE_DEC_STREAM_CLOSE, dec);
    }
    if (dec->stream) {
        audio_stream_close(dec->stream);
        dec->stream = NULL;
    }

    file_decoder_close(&dec->file_dec);
__err1:
    file_dec_release();
    // 设置时钟
    clock_set_cur();
    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码资源等待
   @param    *wait: 句柄
   @param    event: 事件
   @return   0：成功
   @note     用于多解码打断处理
*/
/*----------------------------------------------------------------------------*/
static int file_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    log_i("file_wait_res_handler, event:%d, status:%d ", event, file_dec->file_dec.status);
    if (event == AUDIO_RES_GET) {
        // 启动解码
        if (file_dec->file_dec.status == 0) {
            err = file_dec_start();
        } else if (file_dec->file_dec.tmp_pause) {
            file_dec->file_dec.tmp_pause = 0;

            audio_output_set_start_volume(APP_AUDIO_STATE_MUSIC);
            if (file_dec->file_dec.status == FILE_DEC_STATUS_PLAY) {
                err = audio_decoder_start(&file_dec->file_dec.decoder);
                if (!file_dec->pick_flag) {
                    audio_mixer_ch_pause(&file_dec->mix_ch, 0);
                }
            }
        }
    } else if (event == AUDIO_RES_PUT) {
        // 被打断
        if (file_dec->file_dec.status) {
            if (file_dec->file_dec.status == FILE_DEC_STATUS_PLAY || \
                file_dec->file_dec.status == FILE_DEC_STATUS_PAUSE) {
                if (!file_dec->pick_flag) {
                    audio_mixer_ch_pause(&file_dec->mix_ch, 1);
                }
                err = audio_decoder_pause(&file_dec->file_dec.decoder);
                /* os_time_dly(2); */
                /* audio_output_stop(); */

            }
            file_dec->file_dec.tmp_pause = 1;
        }
    }

    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    file解码pp处理
   @param    play: 1-播放。0-暂停
   @return
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
static void file_dec_pp_ctrl(u8 play)
{
    if (!file_dec) {
        return ;
    }
    if (play) {
        // 播放前处理
        clock_pause_play(0);
        if (!file_dec->pick_flag) {
            audio_mixer_ch_pause(&file_dec->mix_ch, 0);
        }
    } else {
        // 暂停后处理
#if TCFG_DEC2TWS_ENABLE
        if (file_dec->file_dec.dec_no_out_sound) {
            // 发送一个暂停命令，避免从机收数超时进入stop
            localtws_dec_pause();
        }
#endif
        if (!file_dec->pick_flag) {
            audio_mixer_ch_pause(&file_dec->mix_ch, 1);
            //audio_decoder_resume_all(&decode_task);
        }
        if (audio_mixer_get_active_ch_num(&mixer) == 0) {
            clock_pause_play(1);
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    创建一个文件解码
   @param    *priv: 事件回调私有参数
   @param    *handler: 事件回调句柄
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_create(void *priv, void (*handler)(void *, int argc, int *argv))
{
    struct file_dec_hdl *dec;
    if (file_dec) {
        file_dec_close();
    }

    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }

    file_dec = dec;
    file_dec->evt_cb = handler;
    file_dec->evt_priv = priv;

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    设置解码数据流设置回调接口
   @param    *dec: 解码句柄
   @param    *stream_handler: 数据流设置回调
   @param    *stream_priv: 数据流设置回调私有句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_dec_set_stream_set_hdl(struct file_dec_hdl *dec,
                                 void (*stream_handler)(void *priv, int event, struct file_dec_hdl *),
                                 void *stream_priv)
{
    if (dec) {
        dec->stream_handler = stream_handler;
        dec->stream_priv = stream_priv;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    打开文件解码
   @param    *file: 文件句柄
   @param    *bp: 断点信息
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_open(void *file, struct audio_dec_breakpoint *bp)
{
    int err;
    struct file_dec_hdl *dec = file_dec;

    log_i("file_dec_open: in, 0x%x, bp:0x%x \n", file, bp);

    if ((!dec) || (!file)) {
        return -EPERM;
    }
    dec->file = file;
    dec->bp = bp;
    dec->id = rand32();

    dec->file_dec.ch_type = AUDIO_CH_MAX;
    dec->file_dec.output_ch_num = audio_output_channel_num();
    dec->file_dec.output_ch_type = audio_output_channel_type();

#if TCFG_DEC_DECRYPT_ENABLE
    cipher_init(&dec->mply_cipher, TCFG_DEC_DECRYPT_KEY);
    cipher_check_decode_file(&dec->mply_cipher, file);
#endif

#if TCFG_DEC2TWS_ENABLE
    // 设置localtws重播接口
    localtws_globle_set_dec_restart(file_dec_push_restart);
#endif

    dec->wait.priority = 1;
    dec->wait.preemption = 0;
    dec->wait.snatch_same_prio = 1;
    dec->wait.handler = file_wait_res_handler;
    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);

    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    关闭文件解码
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_dec_close(void)
{
    if (!file_dec) {
        return;
    }

    if (file_dec->file_dec.status) {
        file_dec->file_dec.status = 0;
        audio_mixer_ch_pause(&file_dec->mix_ch, 1);
        file_decoder_close(&file_dec->file_dec);
#if TCFG_SPEED_PITCH_ENABLE
        if (file_dec->p_pitchspeed_hdl) {
            close_pitchspeed(file_dec->p_pitchspeed_hdl);
        }
#endif
        file_eq_drc_close(file_dec->eq_drc);
#if TCFG_EQ_DIVIDE_ENABLE
        file_rl_rr_eq_drc_close(file_dec->eq_drc_rl_rr);
        audio_vocal_tract_synthesis_close(&file_dec->synthesis_ch_fl_fr);
        audio_vocal_tract_synthesis_close(&file_dec->synthesis_ch_rl_rr);
        audio_vocal_tract_close(&file_dec->vocal_tract);
        channel_switch_close(&file_dec->ch_switch);
#endif
        audio_mixer_ch_close(&file_dec->mix_ch);
#if TCFG_PCM_ENC2TWS_ENABLE
        if (file_dec->file_dec.dec_no_out_sound) {
            file_dec->file_dec.dec_no_out_sound = 0;
            localtws_enc_api_close();
        }
#endif
        if (file_dec->stream_handler) {
            file_dec->stream_handler(file_dec->stream_priv, FILE_DEC_STREAM_CLOSE, file_dec);
        }
        if (file_dec->stream) {
            audio_stream_close(file_dec->stream);
            file_dec->stream = NULL;
        }
    }

    file_dec_release();

    clock_set_cur();
    log_i("file_dec_close: exit\n");
}

/*----------------------------------------------------------------------------*/
/**@brief    获取file_dec句柄
   @param
   @return   file_dec句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct file_decoder *file_dec_get_file_decoder_hdl(void)
{
    if (file_dec) {
        return &file_dec->file_dec;
    }
    return NULL;
}

/*----------------------------------------------------------------------------*/
/**@brief    获取file_dec状态
   @param
   @return   解码状态
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_get_status(void)
{
    struct file_decoder *dec = file_dec_get_file_decoder_hdl();
    if (dec) {
        return dec->status;
    }
    return FILE_DEC_STATUS_STOP;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码重新开始
   @param    id: 文件解码id
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_restart(int id)
{
    if ((!file_dec) || (id != file_dec->id)) {
        return -1;
    }
    if (file_dec->bp == NULL) {
        if (file_dec->dec_bp == NULL) {
            file_dec->dec_bp = zalloc(sizeof(struct audio_dec_breakpoint) + BREAKPOINT_DATA_LEN);
            ASSERT(file_dec->dec_bp);
            file_dec->dec_bp->data_len = BREAKPOINT_DATA_LEN;
        }
        file_dec->bp = file_dec->dec_bp;
    }
    if (file_dec->file_dec.status && file_dec->bp) {
        audio_decoder_get_breakpoint(&file_dec->file_dec.decoder, file_dec->bp);
    }

    void *file = file_dec->file;
    void *bp = file_dec->bp;
    void *evt_cb = file_dec->evt_cb;
    void *evt_priv = file_dec->evt_priv;
    int err;
    void *dec_bp = file_dec->dec_bp; // 先保存一下，避免close被释放
    file_dec->dec_bp = NULL;

    file_dec_close();
    err = file_dec_create(evt_priv, evt_cb);
    if (!err) {
        file_dec->dec_bp = dec_bp; // 还原回去
        err = file_dec_open(file, bp);
    } else {
        if (dec_bp) {
            free(dec_bp); // 失败，释放
        }
    }
    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    推送文件解码重新开始命令
   @param
   @return   true：成功
   @return   false：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_push_restart(void)
{
    if (!file_dec) {
        return false;
    }
    int argv[3];
    argv[0] = (int)file_dec_restart;
    argv[1] = 1;
    argv[2] = (int)file_dec->id;
    os_taskq_post_type(os_current_task(), Q_CALLBACK, ARRAY_SIZE(argv), argv);
    return true;
}

#if FILE_DEC_REPEAT_EN
/*----------------------------------------------------------------------------*/
/**@brief    循环播放回调接口
   @param    *priv: 私有参数
   @return   0：循环播放
   @return   非0：结束循环
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_dec_repeat_cb(void *priv)
{
    struct file_dec_hdl *dec = priv;
    y_printf("file_dec_repeat_cb\n");
    if (dec->repeat_num) {
        dec->repeat_num--;
    } else {
        y_printf("file_dec_repeat_cb end\n");
        return -1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    设置循环播放次数
   @param    repeat_num: 循环次数
   @return   true：成功
   @return   false：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_repeat_set(u8 repeat_num)
{
    struct file_dec_hdl *dec = file_dec;
    if (!dec || !dec->file_dec.decoder.dec_ops) {
        return false;
    }
    switch (dec->file_dec.decoder.dec_ops->coding_type) {
    case AUDIO_CODING_MP3:
    case AUDIO_CODING_WAV: {
        dec->repeat_num = repeat_num;
        struct audio_repeat_mode_param rep = {0};
        rep.flag = 1; //使能
        rep.headcut_frame = 2; //依据需求砍掉前面几帧，仅mp3格式有效
        rep.tailcut_frame = 2; //依据需求砍掉后面几帧，仅mp3格式有效
        rep.repeat_callback = file_dec_repeat_cb;
        rep.callback_priv = dec;
        rep.repair_buf = &dec->repair_buf;
        audio_decoder_ioctrl(&dec->file_dec.decoder, AUDIO_IOCTRL_CMD_REPEAT_PLAY, &rep);
    }
    return true;
    }
    return false;
}
#endif

#endif /*TCFG_APP_MUSIC_EN*/


/*----------------------------------------------------------------------------*/
/**@brief    file decoder pp处理
   @param    *dec: file解码句柄
   @param    play: 1-播放。0-暂停
   @return
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
void file_decoder_pp_ctrl(struct file_decoder *dec, u8 play)
{
#if TCFG_APP_MUSIC_EN
    if (file_dec && (&file_dec->file_dec == dec)) {
        file_dec_pp_ctrl(play);
    }
#endif /*TCFG_APP_MUSIC_EN*/
}



/*----------------------------------------------------------------------------*/
/**@brief    音乐模式 eq drc 打开
   @param    sample_rate:采样率
   @param    ch_num:通道个数
   @return   句柄
   @note
*/
/*----------------------------------------------------------------------------*/
void *file_eq_drc_open(u16 sample_rate, u8 ch_num)
{

#if TCFG_EQ_ENABLE

    struct audio_eq_drc *eq_drc = NULL;
    struct audio_eq_drc_parm effect_parm = {0};
#if TCFG_MUSIC_MODE_EQ_ENABLE
    effect_parm.eq_en = 1;

#if TCFG_DRC_ENABLE
#if TCFG_MUSIC_MODE_DRC_ENABLE
    effect_parm.drc_en = 1;
    effect_parm.drc_cb = drc_get_filter_info;
#endif
#endif

    if (effect_parm.eq_en) {
        effect_parm.async_en = 1;
        effect_parm.out_32bit = 1;
        effect_parm.online_en = 1;
        effect_parm.mode_en = 1;
    }

    effect_parm.eq_name = song_eq_mode;
#if TCFG_EQ_DIVIDE_ENABLE
    effect_parm.divide_en = 1;
#endif

    effect_parm.ch_num = ch_num;
    effect_parm.sr = sample_rate;
    effect_parm.eq_cb = eq_get_filter_info;
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
/**@brief    音乐模式 eq drc 关闭
   @param    句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_eq_drc_close(struct audio_eq_drc *eq_drc)
{
#if TCFG_EQ_ENABLE
#if TCFG_MUSIC_MODE_EQ_ENABLE
    if (eq_drc) {
        audio_eq_drc_close(eq_drc);
        eq_drc = NULL;
        clock_remove(EQ_CLK);
#if TCFG_DRC_ENABLE
#if TCFG_MUSIC_MODE_DRC_ENABLE
        clock_remove(EQ_DRC_CLK);
#endif
#endif
    }
#endif
#endif
    return;
}

/*----------------------------------------------------------------------------*/
/**@brief    音乐模式 RL RR 通道eq drc 打开
   @param    sample_rate:采样率
   @param    ch_num:通道个数
   @return   句柄
   @note
*/
/*----------------------------------------------------------------------------*/
void *file_rl_rr_eq_drc_open(u16 sample_rate, u8 ch_num)
{

#if TCFG_EQ_ENABLE

    struct audio_eq_drc *eq_drc = NULL;
    struct audio_eq_drc_parm effect_parm = {0};
#if TCFG_MUSIC_MODE_EQ_ENABLE
    effect_parm.eq_en = 1;

#if TCFG_DRC_ENABLE
#if TCFG_MUSIC_MODE_DRC_ENABLE
    effect_parm.drc_en = 1;
    effect_parm.drc_cb = drc_get_filter_info;
#endif
#endif


    if (effect_parm.eq_en) {
        effect_parm.async_en = 1;
        effect_parm.out_32bit = 1;
        effect_parm.online_en = 1;
        effect_parm.mode_en = 1;
    }

#if TCFG_EQ_DIVIDE_ENABLE
    effect_parm.divide_en = 1;
    effect_parm.eq_name = rl_eq_mode;
    /* #else */
    /* effect_parm.eq_name = fr_eq_mode; */
#endif


    effect_parm.ch_num = ch_num;
    effect_parm.sr = sample_rate;
    effect_parm.eq_cb = eq_get_filter_info;
    log_i("ch_num %d\n,sr %d\n", ch_num, sample_rate);
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
/**@brief    音乐模式 RL RR 通道eq drc 关闭
   @param    句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_rl_rr_eq_drc_close(struct audio_eq_drc *eq_drc)
{
#if TCFG_EQ_ENABLE
#if TCFG_MUSIC_MODE_EQ_ENABLE
    if (eq_drc) {
        audio_eq_drc_close(eq_drc);
        eq_drc = NULL;
        clock_remove(EQ_CLK);
#if TCFG_DRC_ENABLE
#if TCFG_MUSIC_MODE_DRC_ENABLE
        clock_remove(EQ_DRC_CLK);
#endif
#endif
    }
#endif
#endif
    return;
}

/*------------------------------------------------------------------------------*/
/**@brief   音乐模式ID3获取
   @param
   @return
   @note
*/
/*------------------------------------------------------------------------------*/
#if TCFG_DEC_ID3_V1_ENABLE
void *music_id3_get(void)
{
    struct file_dec_hdl *dec = file_dec;
    if (!dec || !dec->file_dec.decoder.dec_ops) {
        return NULL;
    }
    return dec->p_mp3_id3_v1->id3_buf;
}
#endif

