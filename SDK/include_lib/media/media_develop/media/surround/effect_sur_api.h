#ifndef effectSUR_api_h__
#define effectSUR_api_h__

/*单声道输入时，channel设置*/
enum {
    EFFECT_CH_L = 0x10,                 //单声道输入，输出左声道
    EFFECT_CH_R = 0x20,                 //单声道输入，输出右声道
};

enum {
    EFFECT_3D_TYPE0 = 0x01,
    EFFECT_3D_TYPE1 = 0x02,            //这2个2选1  ：如果都置上，优先用EFFECT_3D_TYPE1
    EFFECT_3D_LRDRIFT = 0x04,
    EFFECT_3D_ROTATE = 0X08,           //这2个2选1 : 如果都置上，优先用EFFECT_3D_ROTATE
};


typedef struct __SUR_FUNC_API_ {
    unsigned int (*need_buf)(int flag);
    unsigned int (*open)(unsigned int *ptr, int effectflag, int nch);
    unsigned int (*init)(unsigned int *ptr, int rotatestep, int damping, int feedback, int roomsize);
    unsigned int (*run)(unsigned int *ptr, short *inbuf, int len);             // len是对点
    unsigned int (*switch_effect)(unsigned int *ptr, int effectflag, int rotatestep, int damping, int feedback, int roomsize);//新增的切换音效参数接口
    unsigned int (*switch_nch)(unsigned int *ptr, int nch);//新增的声道修改接口
} SUR_FUNC_API;


extern SUR_FUNC_API *get_sur_func_api();




#endif // reverb_api_h__
