#include "soundcard/soundcard.h"
#include "soundcard/lamp.h"
#include "soundcard/peripheral.h"
#include "key_event_deal.h"

#if SOUNDCARD_ENABLE

struct __soundcard {
    u8 mode;
    u8 electric;
    u8 pitch;
    u8 dodge;

    u8 mic_status;

    u16 monitor_vol;
    u16 rec_vol;


};

struct __soundcard *soundcard = NULL;
#define __this soundcard;

enum {
    MIC_TYPE_NORMAL = 0x0,
    MIC_TYPE_EAR,
};

enum {
    KEY_SOUNDCARD_MODE_ELECTRIC,
    KEY_SOUNDCARD_MODE_PITCH,
    KEY_SOUNDCARD_MODE_MAGIC,
    KEY_SOUNDCARD_MODE_BOOM,
    KEY_SOUNDCARD_MODE_SHOUTING_WHEAT,
    KEY_SOUNDCARD_MODE_DODGE,

    KEY_SOUNDCARD_MAKE_NOISE0,
    KEY_SOUNDCARD_MAKE_NOISE1,
    KEY_SOUNDCARD_MAKE_NOISE2,
    KEY_SOUNDCARD_MAKE_NOISE3,
    KEY_SOUNDCARD_MAKE_NOISE4,
    KEY_SOUNDCARD_MAKE_NOISE5,
    KEY_SOUNDCARD_MAKE_NOISE6,
    KEY_SOUNDCARD_MAKE_NOISE7,
    KEY_SOUNDCARD_MAKE_NOISE8,
    KEY_SOUNDCARD_MAKE_NOISE9,
    KEY_SOUNDCARD_MAKE_NOISE10,

    KEY_SOUNDCARD_USB_MIC_MUTE_SWICH,
    KEY_NORMAL_MIC_STATUS_UPDATE,
    KEY_EAR_MIC_STATUS_UPDATE,
    KEY_SOUNDCARD_AUX_STATUS_UPDATE,
}

enum {
    SOUNDCARD_MODE_NORMAL = 0x0,
    SOUNDCARD_MODE_ELECTRIC,
    SOUNDCARD_MODE_PITCH,
    SOUNDCARD_MODE_MAGIC,
    SOUNDCARD_MODE_BOOM,
    SOUNDCARD_MODE_SHOUTING_WHEAT,
    //SOUNDCARD_MODE_DODGE,
};

static const u32 electric_tab[] = {
    A_MAJOR,
    Ashop_MAJOR,
    B_MAJOR,
    C_MAJOR,
    Cshop_MAJOR,
    D_MAJOR,
    Dshop_MAJOR,
    E_MAJOR,
    F_MAJOR,
    Fshop_MAJOR,
    G_MAJOR,
    Gshop_MAJOR,
};

static const u16 pitch_tab[] = {
    EFFECT_REVERB_MODE_BOY_TO_GIRL,
    EFFECT_REVERB_MODE_GIRL_TO_BOY,
    EFFECT_REVERB_MODE_GIRL_TO_BOY,
    EFFECT_REVERB_MODE_KIDS,
};

static void soundcard_mic_status_update(void)
{
    if (__this->mic_status == 0) {
        //没有mic在线
        //关闭ear mic
        soundcard_ear_mic_mute(1);
        //关闭混响
        mic_effect_pause(1);
        return ;
    }

    if (__this->mic_status & BIT(MIC_TYPE_NORMAL)) {
        //普通mic在线
        //mute ear mic
        soundcard_ear_mic_mute(1);
    } else if (__this->mic_status & BIT(MIC_TYPE_EAR)) {
        //ear mic在线
        //unmute ear mic
        soundcard_ear_mic_mute(0);
    }
    //释放暂停状态
    mic_effect_pause(0);
}

void soundcard_event_deal(struct sys_event *event)
{
    struct key_event *key = &event->u.key;
    int key_event = event->u.key.event;
    int key_value = event->u.key.value;
    u8 mic_status = 0;

    if (key_event == KEY_NULL) {
        return false;
    }

    switch (key_event) {
    //按键模式操作
    case KEY_SOUNDCARD_MODE_ELECTRIC:
        log_i("KEY_SOUNDCARD_MODE_ELECTRIC\n");
        if (__this->mode != SOUNDCARD_MODE_ELECTRIC) {
            __this->mode = SOUNDCARD_MODE_ELECTRIC;
            __this->electric = 0;
        } else {
            __this->electric++;
            if (__this->electric >= ARRAY_SIZE(electric_tab)) {
                __this->mode = SOUNDCARD_MODE_NORMAL;
                mic_effect_change_mode(EFFECT_REVERB_MODE_NORMAL);
                break;
            }
        }
        //play notice
        soundcard_make_notice_electric(__this->electric);
        //切换模式为电音模式,设置完需要手动配置电音的pitch参数，工具不可配
        mic_effect_change_mode(EFFECT_REVERB_MODE_ELECTRIC);
        PITCH_PARM_SET2 electric_parm = {0};
        //设置电音的大调级数
        electric_parm.formant_shift = electric_tab[__this->electric];
        //电音该参数固定为100
        electric_parm.pitch = 100;
        //因为电音参数是固定的不是通过工具
        mic_effect_pitch_parm_fill(&electric_parm, 1, 0);
        break;
    case KEY_SOUNDCARD_MODE_PITCH:
        log_i("KEY_SOUNDCARD_MODE_PITCH\n");
        if (__this->mode != SOUNDCARD_MODE_PITCH) {
            __this->mode = SOUNDCARD_MODE_PITCH;
            __this->pitch = 0;
        } else {
            __this->pitch++;
            if (__this->pitch >= ARRAY_SIZE(pitch_tab)) {
                __this->mode = SOUNDCARD_MODE_NORMAL;
                mic_effect_change_mode(EFFECT_REVERB_MODE_NORMAL);
                break;
            }
        }
        mic_effect_change_mode(pitch_tab[__this->pitch]);
        break;
    case KEY_SOUNDCARD_MODE_MAGIC:
        log_i("KEY_SOUNDCARD_MODE_MAGIC\n");
        if (__this->mode != SOUNDCARD_MODE_MAGIC) {
            __this->mode = SOUNDCARD_MODE_MAGIC;
            mic_effect_change_mode(EFFECT_REVERB_MODE_MAGIC);
        } else {
            __this->mode = SOUNDCARD_MODE_NORMAL;
            mic_effect_change_mode(EFFECT_REVERB_MODE_NORMAL);
        }
        break;
    case KEY_SOUNDCARD_MODE_BOOM:
        log_i("KEY_SOUNDCARD_MODE_BOOM\n");
        if (__this->mode != SOUNDCARD_MODE_BOOM) {
            __this->mode = SOUNDCARD_MODE_BOOM;
            mic_effect_change_mode(EFFECT_REVERB_MODE_BOOM);
        } else {
            __this->mode = SOUNDCARD_MODE_NORMAL;
            mic_effect_change_mode(EFFECT_REVERB_MODE_NORMAL);
        }
        break;
    case KEY_SOUNDCARD_MODE_SHOUTING_WHEAT:
        log_i("KEY_SOUNDCARD_MODE_SHOUTING_WHEAT\n");
        if (__this->mode != SOUNDCARD_MODE_SHOUTING_WHEAT) {
            __this->mode = SOUNDCARD_MODE_SHOUTING_WHEAT;
            mic_effect_change_mode(EFFECT_REVERB_MODE_SHOUTING_WHEAT);
        } else {
            __this->mode = SOUNDCARD_MODE_NORMAL;
            mic_effect_change_mode(EFFECT_REVERB_MODE_NORMAL);
        }
        break;
    case KEY_SOUNDCARD_MODE_DODGE:
        __this->dodge = !__this->dodge;
        log_i("KEY_SOUNDCARD_MODE_DODGE, %d\n", __this->dodge);
        break;

    case KEY_SOUNDCARD_USB_MIC_MUTE_SWICH:
        log_i("KEY_SOUNDCARD_USB_MIC_MUTE_SWICH\n");
        break;

    //按键提示音响应处理
    case KEY_SOUNDCARD_MAKE_NOISE0:
    case KEY_SOUNDCARD_MAKE_NOISE1:
    case KEY_SOUNDCARD_MAKE_NOISE2:
    case KEY_SOUNDCARD_MAKE_NOISE3:
    case KEY_SOUNDCARD_MAKE_NOISE4:
    case KEY_SOUNDCARD_MAKE_NOISE5:
    case KEY_SOUNDCARD_MAKE_NOISE6:
    case KEY_SOUNDCARD_MAKE_NOISE7:
    case KEY_SOUNDCARD_MAKE_NOISE8:
    case KEY_SOUNDCARD_MAKE_NOISE9:
    case KEY_SOUNDCARD_MAKE_NOISE10:
        log_i("make noise index = %d\n", key_event - KEY_SOUNDCARD_MAKE_NOISE0);
        soundcard_make_some_noise(key_event - KEY_SOUNDCARD_MAKE_NOISE0);
        break;
    case KEY_SLIDE_MIC:
        mic_effect_set_dvol();
        break;
    case KEY_SLIDE_WET_GAIN:
        mic_effect_set_reverb_wet();
        break;
    case KEY_SLIDE_BASS:
        break;
    case KEY_SLIDE_TREBLE:
        break;
    case KEY_SLIDE_RECORD_VOL:
        __this->rec_vol = get_max_sys_vol() * key_value / 30;
        printf("rec_vol = %d\n", __this->rec_vol);
        //设置dac fr fl硬件数字音量
        audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(2) | BIT(3), __this->rec_vol, 1);
        break;
    case KEY_SLIDE_MUSIC_VOL:
        break;
    case KEY_SLIDE_EARPHONE_VOL:
        __this->monitor_vol = get_max_sys_vol() * key_value / 30;
        printf("monitor_vol = %d\n", __this->monitor_vol);
        //设置dac fr fl硬件数字音量
        audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0) | BIT(1), __this->monitor_vol, 1);
        break;

    case KEY_NORMAL_MIC_STATUS_UPDATE:
        if (key_value) {
            __this->mic_status |= BIT(MIC_TYPE_NORMAL);
        } else {
            __this->mic_status &= ~BIT(MIC_TYPE_NORMAL);
        }
        soundcard_mic_status_update();
        break;
    case KEY_EAR_MIC_STATUS_UPDATE:
        if (key_value) {
            __this->mic_status |= BIT(MIC_TYPE_EAR);
        } else {
            __this->mic_status &= ~BIT(MIC_TYPE_EAR);
        }
        soundcard_mic_status_update();
        break;

    case KEY_SOUNDCARD_AUX_STATUS_UPDATE:
        if (key_value) {

        } else {

        }
        break;
    default:
        break;
    }

    log_i("soundcard mode = %d\n", __this->mode);
    log_i("mic_effect mode = %d\n", mic_effect_get_cur_mode());
    log_i("electric = %d\n", __this->electric);
    log_i("pitch = %d\n", __this->pitch);
    log_i("dodge = %d\n", __this->dodge);
}
#endif/*SOUNDCARD_ENABLE*/


