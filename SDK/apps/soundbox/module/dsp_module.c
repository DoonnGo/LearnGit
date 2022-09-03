#include "common/app_common.h"
#include "app_task.h"
#include "app_main.h"
//#include "key_event_deal.h"
#include "music/music.h"
//#include "pc/pc.h"
//#include "record/record.h"
//#include "linein/linein.h"
//#include "fm/fm.h"
//#include "btstack/avctp_user.h"
//#include "app_power_manage.h"
//#include "app_chargestore.h"
//#include "usb/otg.h"
//#include "usb/host/usb_host.h"
//#include <stdlib.h>
//#include "bt/bt_tws.h"
//#include "audio_config.h"
//#include "common/power_off.h"
//#include "common/user_msg.h"
#include "audio_config.h"
//#include "audio_enc.h"
//#include "ui/ui_api.h"
//#include "fm_emitter/fm_emitter_manage.h"
//#include "common/fm_emitter_led7_ui.h"
//#if TCFG_CHARGE_ENABLE
//#include "app_charge.h"
//#endif
//#include "dev_multiplex_api.h"
//#include "chgbox_ctrl.h"
//#include "device/chargebox.h"
//#include "app_online_cfg.h"
//#include "soundcard/soundcard.h"
//#include "smartbox_bt_manage.h"
//#include "bt.h"
//#include "common/dev_status.h"
//#include "tone_player.h"
//#include "ui_manage.h"
//#include "soundbox.h"
//#include "audio_recorder_mix.h"
//#include "application/eq_config.h"

///2020-12-17 DSP模块通信
#define UART_DEV_USAGE_TEST_SEL         2       //uart_dev.c api接口使用方法选择
//  选择1  串口中断回调函数推送事件，由事件响应函数接收串口数据
//  选择2  由task接收串口数据

#define UART_DEV_TEST_MULTI_BYTE        0       //uart_dev.c 读写多个字节api / 读写1个字节api 选择

static u8 uart_cbuf[512] __attribute__((aligned(4)));
static u8 uart_rxbuf[512] __attribute__((aligned(4)));
const uart_bus_t *uart_bus = NULL;

static void my_put_u8hex(u8 dat)
{
    u8 tmp;
    tmp = dat / 16;
    if (tmp < 10) {
        putchar(tmp + '0');
    } else {
        putchar(tmp - 10 + 'A');
    }
    tmp = dat % 16;
    if (tmp < 10) {
        putchar(tmp + '0');
    } else {
        putchar(tmp - 10 + 'A');
    }
    putchar(0x20);
}

static void uart_event_handler(struct sys_event *e)
{
    const uart_bus_t *uart_bus;
    u32 uart_rxcnt = 0;

    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            /* printf("uart event: DEVICE_EVENT_FROM_UART_RX_OVERFLOW\n"); */
            uart_bus = (const uart_bus_t *)e->u.dev.value;
            uart_rxcnt = uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
            if (uart_rxcnt) {
                printf("get_buffer:\n");
                for (int i = 0; i < uart_rxcnt; i++) {
                    my_put_u8hex(uart_rxbuf[i]);
                    if (i % 16 == 15) {
                        putchar('\n');
                    }
                }
                if (uart_rxcnt % 16) {
                    putchar('\n');
                }
                uart_bus->write(uart_rxbuf, uart_rxcnt);
            }
            printf("uart out\n");
        }
    }
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            /* printf("uart event:DEVICE_EVENT_FROM_UART_RX_OUTTIME\n"); */
            uart_bus = (const uart_bus_t *)e->u.dev.value;
            uart_rxcnt = uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
            if (uart_rxcnt) {
                printf("get_buffer:\n");
                for (int i = 0; i < uart_rxcnt; i++) {
                    my_put_u8hex(uart_rxbuf[i]);
                    if (i % 16 == 15) {
                        putchar('\n');
                    }
                }
                if (uart_rxcnt % 16) {
                    putchar('\n');
                }
                uart_bus->write(uart_rxbuf, uart_rxcnt);
            }
            printf("uart out\n");
        }
    }
}
//SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_event_handler, 0);

///处理缓存
static void uart_u_task(void *arg)
{
    const uart_bus_t *uart_bus = arg;
    int ret;
    u32 uart_rxcnt = 0;

    printf("uart_u_task start\n");
    while (1) {
#if !UART_DEV_TEST_MULTI_BYTE
        //uart_bus->getbyte()在尚未收到串口数据时会pend信号量，挂起task，直到UART_RX_PND或UART_RX_OT_PND中断发生，post信号量，唤醒task
        ret = uart_bus->getbyte(&uart_rxbuf[0], 0);
        if (ret) {
            uart_rxcnt = 1;
            printf("get_byte: %02x\n", uart_rxbuf[0]);
            uart_bus->putbyte(uart_rxbuf[0]);
        }
#else
        //uart_bus->read()在尚未收到串口数据时会pend信号量，挂起task，直到UART_RX_PND或UART_RX_OT_PND中断发生，post信号量，唤醒task
        uart_rxcnt = uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
        if (uart_rxcnt) {
            printf("get_buffer:\n");
            for (int i = 0; i < uart_rxcnt; i++) {
                my_put_u8hex(uart_rxbuf[i]);
                if (i % 16 == 15) {
                    putchar('\n');
                }
            }
            if (uart_rxcnt % 16) {
                putchar('\n');
            }
            //uart_bus->write(uart_rxbuf, uart_rxcnt);
        }
#endif
    }
}

///Uart 中断
static void uart_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("uart_rx_isr\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart_rx_ot_isr\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
}

u8 dsp_uart_init(void)
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = IO_PORTC_07;//IO_PORT_DP;//
    u_arg.rx_pin = -1;//IO_PORTA_02;
    u_arg.rx_cbuf = uart_cbuf;
    u_arg.rx_cbuf_size = 512;
    u_arg.frame_length = 32;
    u_arg.rx_timeout = 100;
    u_arg.isr_cbfun = uart_isr_hook;
    u_arg.baud = 9600;
    u_arg.is_9bit = 0;

    uart_bus = uart_dev_open(&u_arg);
    if (uart_bus != NULL) {
        printf("uart_dev_open() success\n");
#if (UART_DEV_USAGE_TEST_SEL == 2)
        os_task_create(uart_u_task, (void *)uart_bus, 31, 512, 0, "uart_u_task");
#endif
        return 1;
    }
    return 0;
}

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///DSP_Module Control

///校验计算
u16 Get_CRC16(u8 *pBuf, u8 num)
{
	u8 i, j;
	u16 wcrc = 0xffff;
	for(i=0; i<num; i++)
	{
		wcrc ^=(u16)(pBuf[i]);
		for(j=0; j<8; j++)
		{
			if(wcrc&1)
			{
				wcrc>>=1;
				wcrc ^=0xa001;
			}
			else
			{
				wcrc>>=1;
			}
		}
	}
	return wcrc;
}

///Uart TX 写入
void ap3852_send_cmd(u8 *txBuf, u8 len)
{
    uart_bus->write(txBuf, 8);
}

///设置ifx效果
u8 dsp_module_set_ifx(u8 mode)
{
    if(mode>5)
        return app_var.ifx_mode;

	u16 crc_tmp;
	u8 uart_tx_buff[8] = {0};
    uart_tx_buff[0] = 0x01;
    uart_tx_buff[1] = 0x04;
    uart_tx_buff[5] = mode;

	crc_tmp = Get_CRC16(uart_tx_buff,6);
	uart_tx_buff[6] = (crc_tmp)&0xff;
	uart_tx_buff[7] = (crc_tmp>>8)&0xff;

	ap3852_send_cmd(uart_tx_buff,8);

    return app_var.ifx_mode;
}

u8 dsp_module_set_ifx_dir(u8 dir)
{
    if(dir)
    {
        app_var.ifx_mode++;
        if(app_var.ifx_mode >= 6)
            app_var.ifx_mode = 0;
    }
    else
    {
        if(app_var.ifx_mode)
            app_var.ifx_mode--;
        else
            app_var.ifx_mode = 5;
    }
    return dsp_module_set_ifx(app_var.ifx_mode);
}

u8 dsp_module_get_ifx_mode(void)
{
    return app_var.ifx_mode;
}

///设置7段eq:"60", "330", "630", "1K", "3K", "6K", "10K"
void dsp_module_set_eq_section(u8 section,s8 level)
{
	const u16 eq_gain[14+1]=
    {
        200,//-12db
        210,//-11db
        220,//-10db
        240,//-8db
        260,//-6db
        280,//-4db
        300,//-2db
        320,//0db
        340,//2db
        360,//4db
        380,//6db
        400,//8db
        420,//10db
        430,//11db
        440,//12db
    };
	u16 gain = eq_gain[level+7];

	u8 uart_tx_buff[8] = {0};
	uart_tx_buff[0] = 0x06;
	uart_tx_buff[1] = (gain>>8)&0xff;
	uart_tx_buff[2] = gain&0xff;
	uart_tx_buff[3] = 10;//Q
	uart_tx_buff[4] = 1;//类型
	uart_tx_buff[5] = section;

	u16 crc_tmp = Get_CRC16(uart_tx_buff,6);
	uart_tx_buff[6] = (crc_tmp)&0xff;
	uart_tx_buff[7] = (crc_tmp>>8)&0xff;

	ap3852_send_cmd(uart_tx_buff,8);
}

///+-对应段的EQ增益
s8 dsp_module_set_eq_section_dir(u8 section, u8 dir)
{
    if(dir)
    {
        if(app_var.eq_freq[section] < 7)
            app_var.eq_freq[section]++;
    }
    else
    {
        if(app_var.eq_freq[section] > -7)
            app_var.eq_freq[section]--;
    }

    dsp_module_set_eq_section(section, app_var.eq_freq[section]);
    return app_var.eq_freq[section];
}

///获取段增益值
s8 dsp_module_get_eq_section_gain(u8 section)
{
    return app_var.eq_freq[section];
}

///设置预设EQ值
void dsp_module_set_eq(u8 num)
{
	u16 crc_tmp;
	u8 uart_tx_buff[8] = {0};
    uart_tx_buff[0] = 0x04;
    uart_tx_buff[1] = 0x04;
    uart_tx_buff[5] = num;

	crc_tmp = Get_CRC16(uart_tx_buff,6);
	uart_tx_buff[6] = (crc_tmp)&0xff;
	uart_tx_buff[7] = (crc_tmp>>8)&0xff;

	ap3852_send_cmd(uart_tx_buff,8);
}

///设置4声道输出音量
void dsp_module_channel_set_vol(u8 channel, u8 vol) ///channel:1~4
{
    if((!channel || channel > 4) || vol > get_max_sys_vol())    //只允许1~4声道, 且音量不允许超过最大值
        return;

    u16 gain = 4096 / get_max_sys_vol() * vol;
    if(vol == get_max_sys_vol())
        gain = 4096;

	u16 crc_tmp;
	u8 uart_tx_buff[8] = {0};
    uart_tx_buff[0] = 0x02;
    uart_tx_buff[1] = 0x04;
    uart_tx_buff[2] = 0x00;
    uart_tx_buff[3] = channel+1;
	uart_tx_buff[4] = (gain>>8)&0xff;
	uart_tx_buff[5] = gain&0xff;

	crc_tmp = Get_CRC16(uart_tx_buff,6);
	uart_tx_buff[6] = (crc_tmp)&0xff;
	uart_tx_buff[7] = (crc_tmp>>8)&0xff;

	ap3852_send_cmd(uart_tx_buff,8);
}

///检测板上是否有DSP模块
u8 is_dsp_module_online(void)
{
    u8 ret = 0;
    dsp_uart_init();  //uart:1:成功打开,0:打开失败
    ret = 1;
    return ret;
}
