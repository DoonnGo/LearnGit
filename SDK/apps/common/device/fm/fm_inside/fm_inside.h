#ifndef _FM_INSIDE_H_
#define _FM_INSIDE_H_

#if(TCFG_FM_INSIDE_ENABLE == ENABLE)

#if DAC2IIS_EN
#define FM_DAC_OUT_SAMPLERATE  44100L
#else
#define FM_DAC_OUT_SAMPLERATE  44100L
#endif

/************************************************************
*                           FM调试说明
*真台少：                     假台多： 				      叠台多：
*减小  FMSCAN_CNR(主)         加大  FMSCAN_CNR(主)        减小：FMSCAN_AGC
*减小  FMSCAN_P_DIFFER(主)    加大  FMSCAN_P_DIFFER(主)
*加大  FMSCAN_N_DIFFER(次)    减小  FMSCAN_N_DIFFER(次)
*
*注意：不要插串口测试搜台数
*************************************************************/

#define FMSCAN_SEEK_CNT_MIN  450 //最小过零点数 400左右
#define FMSCAN_SEEK_CNT_MAX  600 //最大过零点数 600左右
#define FMSCAN_960_CNR       34  //谐波96M的基础cnr 30~40
#define FMSCAN_1080_CNR      34  //谐波108M的基础cnr 30~40
#define FMSCAN_AGC 			 -55 //AGC阈值  -55左右
#define FMSCAN_ADD_DIFFER 	 -67 //低于此值增加noise differ, -67左右

#define FMSCAN_CNR           5   //cnr  1以上
#define FMSCAN_P_DIFFER		 5   //power differ  1以上
#define FMSCAN_N_DIFFER   	 8   //noise differ  8左右

#define FM_IF                3   //0,1.875; 1,2.143; 2,1.5; 3,cnr低的中频听台

#endif

#endif // _FM_INSIDE_H_

