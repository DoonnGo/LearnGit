#include "key_driver.h"
#include "adkey.h"
#include "gpio.h"
#include "system/event.h"
#include "app_config.h"

///2020-11-11
#if USER_AD_KNOB_ENABLE

void ad_knob_init(void)
{
    adc_add_sample_ch(USER_AD_KNOB_AD_CHANNEL);          //注意：初始化AD_KEY之前，先初始化ADC

    gpio_set_die(USER_AD_KNOB_PORT, 0);
    gpio_set_direction(USER_AD_KNOB_PORT, 1);
    gpio_set_pull_down(USER_AD_KNOB_PORT, 0);
#if USER_AD_KNOB_EXTERN_UP_ENABLE
    gpio_set_pull_up(USER_AD_KNOB_PORT, 0);
#else
    gpio_set_pull_up(USER_AD_KNOB_PORT, 1);
#endif
}

#endif
