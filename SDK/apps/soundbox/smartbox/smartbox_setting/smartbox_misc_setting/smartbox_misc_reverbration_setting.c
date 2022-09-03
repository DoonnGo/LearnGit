#include "app_config.h"
#include "syscfg_id.h"

#include "smartbox_setting_opt.h"
#include "smartbox_misc_setting.h"
#include "le_smartbox_module.h"

#if (SMART_BOX_EN && SMARTBOX_REVERBERATION_SETTING && TCFG_MIC_EFFECT_ENABLE)
#include "mic_effect.h"

extern void mic_effect_set_echo_delay(u32 delay);
extern u32 mic_effect_get_echo_delay(void);
extern void mic_effect_set_echo_decay(u32 decay);
extern u32 mic_effect_get_echo_decay(void);
extern u8 mic_effect_get_status(void);

#pragma pack(1)
struct t_reverbration {
    u8 state;
    u16 depth_val;
    u16 strength_val;
};
#pragma pack()

static struct t_reverbration reverbration;

static int reverbration_setting_set(u8 *misc_setting, u8 is_conversion)
{
    u32 offset = 0;
    reverbration.state = misc_setting[offset++];

    if (is_conversion) {
        reverbration.depth_val = misc_setting[offset++] << 8 | misc_setting[offset++];
        reverbration.strength_val = misc_setting[offset++] << 8 | misc_setting[offset++];
    } else {
        memcpy((u8 *)&reverbration.depth_val, misc_setting + offset, sizeof(reverbration.depth_val));
        offset += 2;
        memcpy((u8 *)&reverbration.strength_val, misc_setting + offset, sizeof(reverbration.strength_val));
        offset += 2;
    }
    return offset;
}

static int reverbration_setting_get(u8 *misc_setting, u8 is_conversion)
{
    u32 offset = 0;
    misc_setting[offset++] = reverbration.state;
    if (is_conversion) {
        misc_setting[offset++] = (reverbration.depth_val >> 8) & 0xFF;
        misc_setting[offset++] = (reverbration.depth_val) & 0xFF;
        misc_setting[offset++] = (reverbration.strength_val >> 8) & 0xFF;
        misc_setting[offset++] = (reverbration.strength_val) & 0xFF;
    } else {
        memcpy(misc_setting + offset, (u8 *)&reverbration.depth_val, sizeof(reverbration.depth_val));
        offset += 2;
        memcpy(misc_setting + offset, (u8 *)&reverbration.strength_val, sizeof(reverbration.strength_val));
        offset += 2;
    }
    return offset;
}

static int reverbration_write_vm(u8 *misc_setting)
{
    u8 reverbration_state = 0;
    syscfg_read(CFG_RCSP_MISC_REVERB_ON_OFF, &reverbration_state, sizeof(reverbration_state));
    if (reverbration_state != reverbration.state) {
        syscfg_write(CFG_RCSP_MISC_REVERB_ON_OFF, &reverbration.state, sizeof(reverbration.state));
    }
    return 0;
}

static int reverbration_state_update(void)
{
    // 值不相同才设置
    static u8 prev_state = -1;
    static u16 prev_depth_val = -1;
    static u16 prev_strength_val = -1;
    if (-1 == prev_state || reverbration.state != prev_state) {
        if (reverbration.state) {
            mic_effect_start();
        } else {
            mic_effect_stop();
        }
        prev_state = reverbration.state;
    }

    if (0 == reverbration.state) {
        prev_depth_val = -1;
        prev_strength_val = -1;
    }

    if (reverbration.state && (-1 == prev_depth_val || reverbration.depth_val != prev_depth_val)) {
        mic_effect_set_echo_delay(reverbration.depth_val * 2);
        prev_depth_val = reverbration.depth_val;
    }

    if (reverbration.state && (-1 == prev_strength_val || reverbration.strength_val != prev_strength_val)) {
        mic_effect_set_echo_decay(reverbration.strength_val * 70 / 100);
        prev_strength_val = reverbration.strength_val;
    }
    return 0;
}

static int reverbration_custom_setting_init(void)
{
    u8 reverbration_state = 0;
    if (sizeof(reverbration_state) == syscfg_read(CFG_RCSP_MISC_REVERB_ON_OFF, &reverbration_state, sizeof(reverbration_state))) {
        reverbration.state = reverbration_state;
    } else {
        reverbration.state = mic_effect_get_status();
    }
    reverbration.depth_val = mic_effect_get_echo_delay() / 2;
    reverbration.strength_val = mic_effect_get_echo_decay() * 100 / 70;
    return 0;
}

static SMARTBOX_MISC_SETTING_OPT reverbration_setting_opt = {
    .misc_data_len = 5,
    .misc_syscfg_id = CFG_RCSP_MISC_REVERB_ON_OFF,
    .misc_setting_type = MISC_SETTING_REVERBERATION,
    .misc_set_setting = reverbration_setting_set,
    .misc_get_setting = reverbration_setting_get,
    .misc_state_update = reverbration_state_update,
    .misc_custom_setting_init = reverbration_custom_setting_init,
    .misc_custom_write_vm = reverbration_write_vm,
};
REGISTER_APP_MISC_SETTING_OPT(reverbration_setting_opt);

void smartbox_update_reverbrateion_state_by_key(void)
{
    reverbration.state = !reverbration.state;
    reverbration_write_vm(NULL);
    reverbration_state_update();
}

void smartbox_close_reverbrateion_state_and_update(void)
{
    reverbration.state = 0;
    reverbration_write_vm(NULL);
    reverbration_state_update();
}

#else

void smartbox_update_reverbrateion_state_by_key(void)
{

}

void smartbox_close_reverbrateion_state_and_update(void)
{

}
#endif
