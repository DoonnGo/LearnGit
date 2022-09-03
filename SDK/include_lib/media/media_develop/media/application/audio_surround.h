
#ifndef _AUDIO_SURROUND_API_H_
#define _AUDIO_SURROUND_API_H_
#include "surround/effect_sur_api.h"
// #include "audio_config.h"

typedef struct _surround_update_parm {
    int surround_type;//音效类型
    int rotatestep;//旋转速度
    int damping;//高频衰减速度
    int feedback;//整体衰减速度
    int roomsize;//空间大小
} surround_update_parm;

typedef struct _surround_open_parm {
    u8 channel;
    u8 surround_effect_type;//默认的环绕音效类型
} surround_open_parm;


typedef struct _surround_hdl {
    SUR_FUNC_API *ops;
    void *work_buf;
    OS_MUTEX mutex;
    u8 surround_en: 1;
    u8 update: 1;
    u8 nch_update: 1;

    surround_open_parm parm;

    struct audio_stream_entry entry;	// 音频流入口

} surround_hdl;

/*cmd*/
enum {
    EFFECT_3D_PANORAMA = 0,             //3d全景
    EFFECT_3D_ROTATES,                  //3d环绕
    EFFECT_FLOATING_VOICE,              //流动人声
    EFFECT_GLORY_OF_KINGS,              //王者荣耀
    EFFECT_FOUR_SENSION_BATTLEFIELD,    //四季战场

    EFFECT_OFF,
};
/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_open  环绕音效打开
   @param    *_parm: 环绕音效始化参数，详见结构体surround_open_parm
   @return   句柄
   @note
*/
/*----------------------------------------------------------------------------*/
surround_hdl *audio_surround_open(surround_open_parm *parm);

/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_parm_update 环绕音效参数更新
   @param    cmd:EFFECT_3D,EFFECT_3D_ROTATE,EFFECT_FLOATING_VOICE,EFFECT_GLORY_OF_KINGS,EFFECT_FOUR_SENSION_BATTLEFIELD
   @param    *_parm:参数指针,NULL则使用默认德参数，否则传入自定义参数
   @return   0：成功  -1: 失败
   @note    对耳时，左右声道效果，须设置保持一致
*/
/*----------------------------------------------------------------------------*/
int audio_surround_parm_update(surround_hdl *_hdl, u32 cmd, surround_update_parm *_parm);

/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_close 环绕音效关闭处理
   @param    _hdl:句柄
   @return  0:成功  -1：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_surround_close(surround_hdl *_hdl);

/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_run 环绕音效处理
   @param    _hdl:句柄
   @param    data:需要处理的数据
   @param    len:数据长度
   @return  0:成功  -1：失败
   @note    无数据流节点时，直接使用改接口进行环绕音效的处理
*/
/*----------------------------------------------------------------------------*/
int audio_surround_run(surround_hdl *_hdl, void *data, u32 len);


/*----------------------------------------------------------------------------*/
/**@brief   audio_surround_switch_nch  通道切换
   @param    _hdl:句柄
   @param    nch:通道数 2 4 或者EFFECT_CH_L EFFECT_CH_R
   @return  0:成功  -1：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_surround_switch_nch(surround_hdl *_hdl, int nch);
#endif
