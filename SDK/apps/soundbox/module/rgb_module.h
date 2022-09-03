#ifndef _RGB_MODULE_H_
#define _RGB_MODULE_H_

u8 user_set_rgb_mode(u8 mode);      //颜色跳转设置
u8 user_set_rgb_mode_dir(u8 dir);   //dir切换设置
u8 user_get_rgb_mode(void);         //rgb颜色获取
u8 user_rgb_online(void);           //rgb在线检测
void user_rgb_suspend(void);        //RGB关闭
void user_rgb_resume(void);         //RGB重新开启

#endif