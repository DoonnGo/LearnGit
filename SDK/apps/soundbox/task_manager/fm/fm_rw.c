#include "system/includes.h"
#include "syscfg_id.h"
#include "fm/fm.h"
#include "fm/fm_manage.h"
#include "fm/fm_rw.h"
#if TCFG_APP_FM_EN


/*************************************************************
   此文件函数主要是fm模式vm 保存 消息api

   注意：在逻辑操作上，sdk大部分是使用了虚拟频点,即(1,、2、3、4)代替了8750、8760、8770等真实的频率
   转换规则：真实 = REAL_FREQ(虚拟)
             虚拟 = VIRTUAL_FREQ(真实)
**************************************************************/




#define FM_USE_MALLOC_VM 1//使用静态变量优化vm访问
#define FM_USE_VM        0///2020-12-7 FM储存不使用VM

#if FM_USE_MALLOC_VM
FM_INFO *__this_fm_info = NULL;
#endif

///将18个台存到公共数组
u16 User_fm_channel_buf[18];

/*----------------------------------------------------------------------------*/
/*@brief   获取一个byte中有几个位被置一
  @param   byte ：所传进去的byte
  @return  被置一位数
  @note    u8 get_total_mem_channel(void)
 */
/*----------------------------------------------------------------------------*/
static u8 __my_get_one_count(u8 byte)
{
    u8 count = 0 ;
    while (byte) {
        count++ ;
        byte &= (byte - 1) ;
    }
    return count ;
}
/*----------------------------------------------------------------------------*/
/**@brief 获取全部记录的频道
  @param 	无
  @return  频道总数
  @note  u8 get_total_mem_channel(void)
 */
/*----------------------------------------------------------------------------*/
u16 get_total_mem_channel(void)
{
    FM_INFO fm_info = {0};
    fm_read_info(&fm_info);
    return fm_info.total_chanel;
}
/*----------------------------------------------------------------------------*/
/**@brief 根据频点偏移量获取频道
  @param 	输入真实频点
  @return  频道
  @note  u8 get_channel_via_fre(u8 fre)

 */
/*----------------------------------------------------------------------------*/
u16 get_channel_via_fre(u16 fre)
{
    FM_INFO fm_info = {0};
    u16 i, k;
    u16 total = 0;
    fre = VIRTUAL_FREQ(fre);//fre - 874;
    fm_read_info(&fm_info);
    for (i = 0; i < (MEM_FM_LEN); i++) {
        for (k = 0; k < 8; k++) {
            if (fm_info.dat[i] & (BIT(k))) {
                total++;
                if (fre == (i * 8 + k)) {
                    return total;		 //return fre index
                }
            }
        }
    }
    return  0x00;//fm_mode_var->wFreChannel;						    //find none
}

/*----------------------------------------------------------------------------*/
/**@brief 通过频道获取频点
  @param 	channel：频道
  @return  有效的频点偏移量
  @note  u8 get_fre_via_channle(u8 channel)
 */
/*----------------------------------------------------------------------------*/
u16 get_fre_via_channel(u16 channel)
{
    if(channel)
    {
        return VIRTUAL_FREQ(User_fm_channel_buf[channel-1]);
    }
    return 0;

    FM_INFO fm_info = {0};
    u16 i, k;
    u16 total = 0;

    fm_read_info(&fm_info);

    for (i = 0; i < (MEM_FM_LEN); i++) {
        for (k = 0; k < 8; k++) {
            if (fm_info.dat[i] & (BIT(k))) {
                total++;
                if (total == channel) {
                    return i * 8 + k;		 //fre = MIN_FRE + return val
                }
            }
        }
    }

    return 0;							//find none
}


/*----------------------------------------------------------------------------*/
/**@brief 从vm清除所有频点信息
  @param 	无
  @return  无
  @note  void clear_all_fm_point(void)
 */
/*----------------------------------------------------------------------------*/
void clear_all_fm_point(void)
{
    FM_INFO fm_info = {0};
    fm_info.mask = FM_VM_MASK;
    fm_info.curFreq = 1;
    fm_info.curChanel = 0;
    fm_info.total_chanel  = 0;
    fm_save_info(&fm_info);

    User_fm_channel_buf[0] = 8750;
    User_fm_channel_buf[1] = 9000;
    User_fm_channel_buf[2] = 9800;
    User_fm_channel_buf[3] = 10600;
    User_fm_channel_buf[4] = 10800;
    User_fm_channel_buf[5] = 8750;
    User_fm_channel_buf[6] = User_fm_channel_buf[0];
    User_fm_channel_buf[7] = User_fm_channel_buf[1];
    User_fm_channel_buf[8] = User_fm_channel_buf[2];
    User_fm_channel_buf[9] = User_fm_channel_buf[3];
    User_fm_channel_buf[10] = User_fm_channel_buf[4];
    User_fm_channel_buf[11] = User_fm_channel_buf[5];
    User_fm_channel_buf[12] = User_fm_channel_buf[0];
    User_fm_channel_buf[13] = User_fm_channel_buf[1];
    User_fm_channel_buf[14] = User_fm_channel_buf[2];
    User_fm_channel_buf[15] = User_fm_channel_buf[3];
    User_fm_channel_buf[16] = User_fm_channel_buf[4];
    User_fm_channel_buf[17] = User_fm_channel_buf[5];
}

/*----------------------------------------------------------------------------*/
/**@brief 根据频点偏移量保存到相应的频点位变量到vm
  @param 	fre：真实频点
  @return  无
  @note  void save_fm_point(u8 fre)
 */
/*----------------------------------------------------------------------------*/
void save_fm_point(u16 fre, u16 channel)//1-206
{
    ///2020-11-24 先将freq存到channel中
    if(channel)
    {
        User_fm_channel_buf[channel-1] = fre;
    }

    FM_INFO fm_info = {0};
    u16 i, k;
    u16 total = 0;
    fm_read_info(&fm_info);
    fre = VIRTUAL_FREQ(fre);//fre - 874;
    /* fre = fre - 874; */
    i = fre / 8;
    k = fre % 8;

    fm_info.dat[i] |= BIT(k);
    fm_info.curFreq = fre;

    for (i = 0; i < (MEM_FM_LEN); i++) {
        total += __my_get_one_count(fm_info.dat[i]);
    }

    total = total > MAX_CHANNEL ? MAX_CHANNEL : total;
    fm_info.total_chanel = total;

    fm_save_info(&fm_info);
}

/*----------------------------------------------------------------------------*/
/**@brief 删除频道
  @param 	虚拟频点
  @return  无
  @note  void delete_fm_point(u8 fre)
 */
/*----------------------------------------------------------------------------*/
void delete_fm_point(u16 fre)
{
    u16 i, k;
    FM_INFO fm_info = {0};
    fm_read_info(&fm_info);
    i = fre / 8;
    k = fre % 8;
    if (fm_info.dat[i] & BIT(k)) {
        fm_info.dat[i] &= (~BIT(k));
        fm_info.total_chanel--;
        fm_save_info(&fm_info);
    }
}
/*----------------------------------------------------------------------------*/
/**@brief 最近一次操作记录
  @param 	无
  @return  无
  @note  u8 ch_save(void)
 */
/*----------------------------------------------------------------------------*/
void fm_last_ch_save(u16 channel)
{
    FM_INFO fm_info = {0};
    fm_read_info(&fm_info);
    fm_info.curFreq   = 0;
    fm_info.curChanel = channel;
    fm_save_info(&fm_info);
}

/*----------------------------------------------------------------------------*/
/**@brief 最近一次操作记录
  @param 	真实频点
  @return  无
  @note  u8 ch_save(void)
 */
/*----------------------------------------------------------------------------*/
void fm_last_freq_save(u16 freq)
{
    FM_INFO fm_info = {0};
    fm_read_info(&fm_info);
    fm_info.curChanel = 0;
    freq = VIRTUAL_FREQ(freq);//fre - 874;
    fm_info.curFreq   = freq;// - 874;
    fm_save_info(&fm_info);
}



/*----------------------------------------------------------------------------*/
/**@brief  保存信息到fm_buf
  @param  无
  @return 无
  @note  void fm_save_info()
 */
/*----------------------------------------------------------------------------*/
void fm_save_info(FM_INFO *info)
{
#if FM_USE_MALLOC_VM
    if (__this_fm_info) {
        memcpy(__this_fm_info, info, sizeof(FM_INFO));
    }
#endif

#if FM_USE_VM
    syscfg_write(VM_FM_INFO, info, sizeof(FM_INFO));
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  从vm读取信息
  @param  无
  @return 无
  @note  void fm_read_info()
 */
/*----------------------------------------------------------------------------*/
void fm_read_info(FM_INFO *info)
{

#if FM_USE_MALLOC_VM
    if (__this_fm_info) {
        memcpy(info, __this_fm_info, sizeof(FM_INFO));
        return;
    }
#endif

#if FM_USE_VM
    if (sizeof(FM_INFO) != syscfg_read(VM_FM_INFO, info, sizeof(FM_INFO))) {
        r_printf("fm_info is null xx\n");
        memset(info, 0x00, sizeof(FM_INFO));
    }
#endif

#if FM_USE_MALLOC_VM
    if (!__this_fm_info) {
        __this_fm_info = zalloc(sizeof(FM_INFO));
        memcpy(__this_fm_info, info, sizeof(FM_INFO));
    }
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  检查vm信息
  @param  无
  @return 无
  @note  void fm_read_info()
 */
/*----------------------------------------------------------------------------*/
void fm_vm_check(void)
{
    FM_INFO fm_info = {0};
    fm_read_info(&fm_info);
    ///2020-11-24 每次上电都初始化一次
    static u8 first_flag = 1;
    if (fm_info.mask !=  FM_VM_MASK || first_flag)
    {
        first_flag = 0;
        fm_info.mask = FM_VM_MASK;
        fm_info.curFreq = 1;
        fm_info.curChanel = 1;
        fm_info.total_chanel  = 18;
        fm_save_info(&fm_info);

        User_fm_channel_buf[0] = 8750;
        User_fm_channel_buf[1] = 9000;
        User_fm_channel_buf[2] = 9800;
        User_fm_channel_buf[3] = 10600;
        User_fm_channel_buf[4] = 10800;
        User_fm_channel_buf[5] = 8750;
        User_fm_channel_buf[6] = User_fm_channel_buf[0];
        User_fm_channel_buf[7] = User_fm_channel_buf[1];
        User_fm_channel_buf[8] = User_fm_channel_buf[2];
        User_fm_channel_buf[9] = User_fm_channel_buf[3];
        User_fm_channel_buf[10] = User_fm_channel_buf[4];
        User_fm_channel_buf[11] = User_fm_channel_buf[5];
        User_fm_channel_buf[12] = User_fm_channel_buf[0];
        User_fm_channel_buf[13] = User_fm_channel_buf[1];
        User_fm_channel_buf[14] = User_fm_channel_buf[2];
        User_fm_channel_buf[15] = User_fm_channel_buf[3];
        User_fm_channel_buf[16] = User_fm_channel_buf[4];
        User_fm_channel_buf[17] = User_fm_channel_buf[5];
    }
}



#endif

