#include "system/includes.h"
#include "common/app_common.h"
#include "app_main.h"
#include "rgb_module.h"
#include "ble_module.h"

enum{
    RGB_OFF = 0,    //关闭
    RGB_RED,        //红色
    RGB_GREEN,      //绿色
    RGB_BLUE,       //蓝色
    RGB_YELLOW,     //黄色
    RGB_CYAN,       //青色
    RGB_PURPLE,     //紫色
    RGB_WHITE,      //白色
    RGB_AUTO,       //跳变
    RGB_STEP,       //渐变
};

enum{
    RGB_OFFLINE = 0,        //没有检测到RGB
    RGB_DRIVE_DIRECT,       //直驱RGB
    RGB_DRIVE_3LINES_MCU,   //3线控制RGB单片机
}

#define RGB_DIRECT_INIT()   do{}while(0)
#define IS_R_ONLINE         (0)
#define RGB_RED_ON()        do{}while(0)
#define RGB_RED_OFF()       do{}while(0)
#define RGB_GREEN_ON()      do{}while(0)
#define RGB_GREEN_OFF()     do{}while(0)
#define RGB_BLUE_ON()       do{}while(0)
#define RGB_BLUE_OFF()      do{}while(0)

static u16 user_rgb_timer_id = 0;

//仅变换颜色处理
static void user_change_rgb(u8 mode)
{
    switch(mode)
    {
        case RGB_OFF:
            RGB_RED_OFF();
            RGB_GREEN_OFF();
            RGB_BLUE_OFF();
        break;
        case RGB_RED:
            RGB_RED_ON();
            RGB_GREEN_OFF();
            RGB_BLUE_OFF();
        break;
        case RGB_GREEN:
            RGB_RED_OFF();
            RGB_GREEN_ON();
            RGB_BLUE_OFF();
        break;
        case RGB_BLUE:
            RGB_RED_OFF();
            RGB_GREEN_OFF();
            RGB_BLUE_ON();
        break;
        case RGB_YELLOW:
            RGB_RED_ON();
            RGB_GREEN_ON();
            RGB_BLUE_OFF();
        break;
        case RGB_CYAN:
            RGB_RED_OFF();
            RGB_GREEN_ON();
            RGB_BLUE_ON();
        break;
        case RGB_PURPLE:
            RGB_RED_ON();
            RGB_GREEN_OFF();
            RGB_BLUE_ON();
        break;
        case RGB_WHITE:
            RGB_RED_ON();
            RGB_GREEN_ON();
            RGB_BLUE_ON();
        break;
        default: return;
    }
}

//RGB跳变5S定时器，一直在后台跑，只有在设置到跳变模式时才切换颜色
static void user_rgb_timer(void)
{
    if(app_var.rgb_drive_type == 0)
    {
        return;
    }
    if(app_var.rgb_mode == RGB_AUTO)
    {
        app_var.rgb_auto_mode++;
        if(app_var.rgb_auto_mode >= RGB_AUTO)
            app_var.rgb_auto_mode = RGB_RED;
        user_change_rgb(app_var.rgb_auto_mode);
    }
    user_rgb_timer_id = sys_timeout_add(NULL, user_rgb_timer, 5000);
}

//颜色跳转设置
u8 user_set_rgb_mode(u8 mode)
{
    if(mode > RGB_AUTO)
        return 0;
    if(app_var.rgb_drive_type == RGB_OFFLINE)
    {
        return 0;
    }
    else if(app_var.rgb_drive_type == RGB_DRIVE_DIRECT)
    {
        user_change_rgb(mode);
        if(mode)
            app_var.rgb_mode = mode;
        if(mode == RGB_AUTO)
    }
    else if(app_var.rgb_drive_type == RGB_DRIVE_3LINES_MCU)
    {

    }
    bsp_app_rgb_status();
    return mode;
}

//dir切换设置
u8 user_set_rgb_mode_dir(u8 dir)
{
    if(app_var.rgb_drive_type == RGB_OFFLINE)
    {
        return 0;
    }
    if(dir)
    {
        app_var.rgb_mode++;
        if(app_var.rgb_mode > RGB_AUTO)
            app_var.rgb_mode = RGB_RED;
    }
    else
    {
        if(app_var.rgb_mode > RGB_RED)
            app_var.rgb_mode--;
        else
            app_var.rgb_mode = RGB_AUTO;
    }
    return user_set_rgb_mode(app_var.rgb_mode);
}

//rgb颜色获取
u8 user_get_rgb_mode(void)
{
    return app_var.rgb_mode;
}

//rgb在线检测
u8 user_rgb_online(void)
{
#if USER_UI_LCDSEG_ENABLE
    //检测到LCDSEG2为低电平时为online
    JL_PORTA->DIE &=~BIT(11);
    JL_PORTA->DIR |= BIT(11);
    JL_PORTA->PU |= BIT(11);
    if((JL_PORTA->IN & BIT(11)) == 0)
    {
        app_var.rgb_drive_type = RGB_DRIVE_3LINES_MCU;
    }
#elif USER_UI_1621LCD_ENABLE
    //RGB三路IO检测到低电平为online
    //JL_PORT->DIE &= ~BIT();
    //JL_PORT->DIR |= BIT();
    //JL_PORT->PU |= BIT();
    //app_var.rgb_drive_type = RGB_DRIVE_DIRECT;
    //RGB_DIRECT_INIT();
#endif
/*
    if(app_var.rgb_drive_type)
    {
        app_var.rgb_auto_mode = REB_RED;
        user_rgb_timer_id = sys_timeout_add(NULL, user_rgb_timer, 5000);
    }
*/
    app_var.rgb_auto_mode = REB_RED;
    return app_var.rgb_drive_type;
}

//RGB关闭
void user_rgb_suspend(void)
{
    if(app_var.rgb_drive_type == RGB_OFFLINE)
    {
        return;
    }
    user_change_rgb(RGB_OFF);
    sys_timeout_del(user_rgb_timer_id);
}

//RGB重新开启
void user_rgb_resume(void)
{
    if(app_var.rgb_drive_type == RGB_OFFLINE)
    {
        return;
    }
    user_change_rgb(app_var.rgb_mode==RGB_AUTO? app_var.rgb_auto_mode : app_var.rgb_mode);
    user_rgb_timer_id = sys_timeout_add(NULL, user_rgb_timer, 5000);
}