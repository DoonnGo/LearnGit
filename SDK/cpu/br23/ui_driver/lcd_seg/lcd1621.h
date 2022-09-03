#ifndef _LCD1621_H_
#define _LCD1621_H_
extern u8 lcd1621_sendbuf[16];

void lcd1621_value_set(void);
void lcd1621_init(void);


void lcd1621_icon_update(void);
void lcd1621_flash_icon(void);

#endif
