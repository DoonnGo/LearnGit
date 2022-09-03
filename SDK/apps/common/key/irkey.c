#include "key_driver.h"
#include "irkey.h"
#include "gpio.h"
#include "asm/irflt.h"
#include "app_config.h"

#if TCFG_IRKEY_ENABLE

u8 ir_get_key_value(void);
//按键驱动扫描参数列表
struct key_driver_para irkey_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 0,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_IR,
    .get_value 		  = ir_get_key_value,
};



const u8 IRTabFF00[] = {
    NKEY_00, NKEY_01, NKEY_02, NKEY_03, NKEY_04, NKEY_05, NKEY_06, IR_06, IR_15, IR_08, NKEY_0A, NKEY_0B, IR_12, IR_11, NKEY_0E, NKEY_0F,
    NKEY_10, NKEY_11, NKEY_12, NKEY_13, NKEY_14, IR_07, IR_09, NKEY_17, IR_13, IR_10, NKEY_1A, NKEY_1B, IR_16, NKEY_1D, NKEY_1E, NKEY_1F,
    NKEY_20, NKEY_21, NKEY_22, NKEY_23, NKEY_24, NKEY_25, NKEY_26, NKEY_27, NKEY_28, NKEY_29, NKEY_2A, NKEY_2B, NKEY_2C, NKEY_2D, NKEY_2E, NKEY_2F,
    NKEY_30, NKEY_31, NKEY_32, NKEY_33, NKEY_34, NKEY_35, NKEY_36, NKEY_37, NKEY_38, NKEY_39, NKEY_3A, NKEY_3B, NKEY_3C, NKEY_3D, NKEY_3E, NKEY_3F,
    IR_04, NKEY_41, IR_18, IR_05, IR_03, IR_00, IR_01, IR_02, NKEY_48, NKEY_49, IR_20, NKEY_4B, NKEY_4C, NKEY_4D, NKEY_4E, NKEY_4F,
    NKEY_50, NKEY_51, IR_19, NKEY_53, NKEY_54, NKEY_55, NKEY_56, NKEY_57, NKEY_58, NKEY_59, IR_17, NKEY_5B, NKEY_5C, NKEY_5D, IR_14, NKEY_5F,
};

#define KEY_IR_TBL_NUM      KEY_IR_NUM_MAX

///2020-11-19  ir_data <-> ir_number
const u8 ir_tbl_FF00[KEY_IR_TBL_NUM] =
{
    0x45, //KEY_POWER,
    0x46, //KEY_MODE,
    0x47, //KEY_MUTE,
    0x44, //KEY_PLAY,
    0x40, //KEY_PREV,
    0x43, //KEY_NEXT,
    0x07, //KEY_EQ,
    0x15, //KEY_VOL_DOWN,
    0x09, //KEY_VOL_UP,
    0x16, //KEY_M0,
    0x19, //KEY_REPEAT,
    0x0D, //KEY_USB_SD,
    0x0C, //KEY_M1,
    0x18, //KEY_M2,
    0x5E, //KEY_M3,
    0x08, //KEY_M4,
    0x1C, //KEY_M5,
    0x5A, //KEY_M6,
    0x42, //KEY_M7,
    0x52, //KEY_M8,
    0x4A, //KEY_M9,
};

const u8 ir_tbl_7F80[KEY_IR_TBL_NUM] =
{
    0x12, //KEY_POWER,
    0x1A, //KEY_MODE,
    0x1E, //KEY_MUTE,
    0x01, //KEY_PLAY,
    0x02, //KEY_PREV,
    0x03, //KEY_NEXT,
    0x04, //KEY_EQ,
    0x05, //KEY_VOL_DOWN,
    0x06, //KEY_VOL_UP,
    0x07, //KEY_M0,
    0x08, //KEY_REPEAT,
    0x09, //KEY_USB_SD,
    0x0A, //KEY_M1,
    0x1B, //KEY_M2,
    0x1F, //KEY_M3,
    0x0C, //KEY_M4
    0x0D, //KEY_M5,
    0x0E, //KEY_M6,
    0x00, //KEY_M7,
    0x0F, //KEY_M8,
    0x19, //KEY_M9,
};

/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 ir_get_key_value(void)
{
    u8 tkey = 0xff;
    tkey = get_irflt_value();
    if (tkey == 0xff) {
        return tkey;
    }
#if 0
    tkey = IRTabFF00[tkey];
#else
    u8 i = 0;
    const u8 *ir_tbl;
    u16 usercode = get_irflt_usercode();
    if(usercode == 0xFF00)
        ir_tbl = ir_tbl_FF00;
    else if(usercode == 0x7F80)
        ir_tbl = ir_tbl_7F80;

    for(; i < KEY_IR_TBL_NUM; i++)
    {
        if(ir_tbl[i] == tkey)
            break;
    }
    if(i >= KEY_IR_TBL_NUM)
        return 0xff;

#if 0   ///2020-11-20两个遥控丝印一致,使用一张表即可
    if(usercode == 0x7F80)
        i += KEY_IR_TBL_NUM;
#endif

    tkey = i;
#endif
    return tkey;
}


/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ir_key_init(void)
*/
/*----------------------------------------------------------------------------*/
int irkey_init(const struct irkey_platform_data *irkey_data)
{
    printf("irkey_init ");

    ir_input_io_sel(irkey_data->port);

    ir_output_timer_sel();

    irflt_config();

    ir_timeout_set();

    return 0;
}

#endif

