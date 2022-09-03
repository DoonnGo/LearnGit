#ifndef _LCDSEG_H_
#define _LCDSEG_H_

#define SEG_A           BIT(0)
#define SEG_B           BIT(1)
#define SEG_C           BIT(2)
#define SEG_D           BIT(3)
#define SEG_E           BIT(4)
#define SEG_F           BIT(5)
#define SEG_G           BIT(6)
#define SEG_H			BIT(7)
#define SEG_I           BIT(8)
#define SEG_J           BIT(9)
#define SEG_K           BIT(10)
#define SEG_L           BIT(11)
#define SEG_M           BIT(12)

#define LCDSEG_NC			0
#define LCDSEG_0        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_1        (SEG_B | SEG_C)
#define LCDSEG_2        (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D)
#define LCDSEG_3        (SEG_A | SEG_B | SEG_G | SEG_C | SEG_D)
#define LCDSEG_4        (SEG_B | SEG_C | SEG_F | SEG_G)
#define LCDSEG_5        (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_6        (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_7        (SEG_A | SEG_B | SEG_C )
#define LCDSEG_8        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_9        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)

#define LCDSEG_A_C        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_B_C        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G| SEG_H)//(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)//
#define LCDSEG_C_C        (SEG_A | SEG_D | SEG_E | SEG_F)
#define LCDSEG_D_C        (SEG_A | SEG_B | SEG_C| SEG_D | SEG_H)//(SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)//
#define LCDSEG_E_C        (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_F_C        (SEG_A | SEG_E | SEG_F | SEG_G)
#define LCDSEG_G_C        (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_H_C        (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_I_C        (SEG_A | SEG_D |SEG_H)//(SEG_B | SEG_C)//
#define LCDSEG_J_C        (SEG_B | SEG_C | SEG_D | SEG_E)
#define LCDSEG_K_C        (SEG_E | SEG_F | SEG_G |SEG_J |SEG_M)//(SEG_E | SEG_F | SEG_G)//
#define LCDSEG_L_C        (SEG_D | SEG_E | SEG_F)
#define LCDSEG_M_C        (SEG_B | SEG_C | SEG_E |SEG_F |SEG_I |SEG_M)//(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//
#define LCDSEG_N_C        (SEG_B | SEG_C | SEG_E | SEG_F |SEG_I |SEG_J)//(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//
#define LCDSEG_O_C        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_P_C        (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define LCDSEG_Q_C        (SEG_A | SEG_B | SEG_C |SEG_D |SEG_E| SEG_F | SEG_J)//(SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)//
#define LCDSEG_R_C        (SEG_A | SEG_B | SEG_E | SEG_F |SEG_G |SEG_J)//(SEG_E | SEG_G)//
#define LCDSEG_S_C        (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_T_C        (SEG_A | SEG_H)//(SEG_D | SEG_E | SEG_F | SEG_G)//
#define LCDSEG_U_C        (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_V_C        (SEG_F | SEG_E |SEG_L | SEG_M)//(SEG_B | SEG_C | SEG_D | SEG_F | SEG_E)//
#define LCDSEG_W_C        (SEG_B | SEG_C | SEG_E |SEG_F |SEG_L |SEG_J)//(SEG_C | SEG_D | SEG_E)//
#define LCDSEG_X_C        (SEG_I | SEG_J | SEG_L | SEG_M)//(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)//
#define LCDSEG_Y_C        (SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_Z_C        (SEG_A | SEG_D | SEG_M | SEG_L)//(SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)//

#define LCDSEG_A        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_B        (SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)//(SEG_A | SEG_B | SEG_C | SEG_D | SEG_G| SEG_H)//
#define LCDSEG_C        (SEG_A | SEG_D | SEG_E | SEG_F)
#define LCDSEG_D        (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)//(SEG_A | SEG_B | SEG_C| SEG_D | SEG_H)//
#define LCDSEG_E        (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_F        (SEG_A | SEG_E | SEG_F | SEG_G)
#define LCDSEG_G        (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_H        (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_I        (SEG_B | SEG_C)//(SEG_A | SEG_D |SEG_H)//
#define LCDSEG_J        (SEG_B | SEG_C | SEG_D | SEG_E)
#define LCDSEG_K        (SEG_E | SEG_F | SEG_G)//(SEG_E | SEG_F | SEG_G |SEG_J |SEG_M)//
#define LCDSEG_L        (SEG_D | SEG_E | SEG_F)
#define LCDSEG_M        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//(SEG_B | SEG_C | SEG_E |SEG_F |SEG_I |SEG_M)//
#define LCDSEG_N        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//(SEG_B | SEG_C | SEG_E | SEG_F |SEG_I |SEG_J)//
#define LCDSEG_O        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_P        (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define LCDSEG_Q        (SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)//(SEG_A | SEG_B | SEG_C |SEG_D |SEG_E| SEG_F | SEG_J)//
#define LCDSEG_R        (SEG_E | SEG_G)//(SEG_A | SEG_B | SEG_E | SEG_F |SEG_G |SEG_J)//
#define LCDSEG_S        (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_T        (SEG_D | SEG_E | SEG_F | SEG_G)//(SEG_A | SEG_H)//
#define LCDSEG_U        (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_V        (SEG_B | SEG_C | SEG_D | SEG_F | SEG_E)//(SEG_F | SEG_E |SEG_L | SEG_M)//
#define LCDSEG_W        (SEG_C | SEG_D | SEG_E)//(SEG_B | SEG_C | SEG_E |SEG_F |SEG_L |SEG_J)//
#define LCDSEG_X        (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)//(SEG_I | SEG_J | SEG_L | SEG_M)//
#define LCDSEG_Y        (SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_Z        (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)//(SEG_A | SEG_D | SEG_M | SEG_L)//

#define LCDSEG_UP_     (SEG_A)
#define LCDSEG_DN_     (SEG_D)
#define LCDSEG_AMP     (SEG_A|SEG_D|SEG_I|SEG_J|SEG_M|SEG_L) // &
#define LCDSEG_APO     (SEG_M)  //'
#define LCDSEG_BRACKET_L   (SEG_M|SEG_J) //(
#define LCDSEG_BRACKET_R   (SEG_I|SEG_L) //)
#define LCDSEG_UNDEF    (SEG_G|SEG_H|SEG_I|SEG_J|SEG_M|SEG_L)//*
#define LCDSEG_PLUS	 	(SEG_G|SEG_H) //+
#define LCDSEG_COMMA   (SEG_L) //,
#define LCDSEG__           (SEG_G) //-
#define LCDSEG_POINT       (SEG_J) //.
#define LCDSEG_SLASH   (SEG_M|SEG_L)   // /

#define LED7_PLAY  		BIT(0) 		//0: 播放图标
#define LED7_PAUSE 		BIT(1) 		//1: 暂停图标
#define LED7_USB   		BIT(2) 		//2: USB图标
#define LED7_SD   		BIT(3) 		//3: SD图标
#define LED7_2POINT 	BIT(4) 		//4: 冒号图标
#define LED7_FM 		BIT(5) 		//5: FM图标
#define LED7_DOT 		BIT(6) 		//6: 小数点图标
#define LED7_MP3 		BIT(7) 		//7: MP3图标
#define LED7_REPEAT 	BIT(8) 		//8: REPEAT图标
#define LED7_CHARGE 	BIT(9) 		//9: 充电图标
#define LED7_BT 		BIT(10) 	//10: 充电图标
#define LED7_AUX 		BIT(11)  	//11: AUX图标
#define LED7_WMA 		BIT(12) 	//12: WMA图标

typedef struct{
    u16 seg_buf[6];
    u32  icon_buf[3];
    u32  flash_icon_buf[3];
    u16 pin_A[6];
    u16 pin_B[6];
    u16 pin_C[6];
    u16 flash_500ms;
    u32 rtc_time_ms;
    u8  pos_X;
    u16 flash_char_bit;
    bool lock;
    u8 fft_enble;
    u16 fft_level;

    u16 ignore_next_flash; //忽略闪烁的下一次隐藏
    u16 screen_show_buf[8];
}lcdseg_cb_t;
extern lcdseg_cb_t lcdseg_cb;

enum{
    COM1 = 0,
    COM2,
    COM3,
    COM4,
    COM5,
    COM6,
};

enum{
    SEG1 = 0,
    SEG2,
    SEG3,
    SEG4,
    SEG5,
    SEG6,
    SEG7,
    SEG8,
    SEG9,
    SEG10,
    SEG11,
    SEG12,
    SEG13,
    SEG14,
    SEG15,
    SEG16,
};

void lcdseg_disp(void);
void lcdseg_disp_buff(u16 disp_data,u8 disp_pos);
void lcdseg_disp_buff_hide(u16 disp_data,u8 disp_pos);
void lcdseg_buff2pin(void);
#endif
