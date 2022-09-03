/*********************************************************************************************
    *   Filename        : ble_api.h

    *   Description     :

    *   Author          : mx

    *   Email           : lmx@zh-jieli.com

    *   Last modifiled  : 2020-07-01 16:36

    *   Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef __BLE_API_H__
#define __BLE_API_H__


#include "typedef.h"
#include "btstack/btstack_typedef.h"


///***注意：该文件的枚举与库编译密切相关，主要是给用户提供调用所用。用户不能自己在中间添加值。*/
////----user (command) codes----////

/**
 * @brief hci connection handle type
 */

typedef enum {
    /*
     */
    BLE_CMD_ADV_ENABLE  = 1,
    BLE_CMD_ADV_PARAM,
    BLE_CMD_ADV_DATA,
    BLE_CMD_RSP_DATA,
    BLE_CMD_DISCONNECT,
    BLE_CMD_REGIEST_THREAD,
    BLE_CMD_ATT_SEND_INIT,
    BLE_CMD_ATT_MTU_SIZE,
    BLE_CMD_ATT_VAILD_LEN,
    BLE_CMD_ATT_SEND_DATA,
    BLE_CMD_REQ_CONN_PARAM_UPDATE,

    BLE_CMD_SCAN_ENABLE,
    BLE_CMD_SCAN_PARAM,
    BLE_CMD_STACK_EXIT,
    BLE_CMD_CREATE_CONN,
    BLE_CMD_CREATE_CONN_CANCEL,

    BLE_CMD_ADV_PARAM_EXT,
    BLE_CMD_SEND_TEST_KEY_NUM,
    BLE_CMD_LATENCY_HOLD_CNT,
    BLE_CMD_SET_DATA_LENGTH,
    BLE_CMD_SET_HCI_CFG,
    BLE_CMD_SCAN_ENABLE2,

    //< ble5
    BLE_CMD_EXT_ADV_PARAM = 0x40,
    BLE_CMD_EXT_ADV_DATA,
    BLE_CMD_EXT_RSP_DATA,
    BLE_CMD_EXT_ADV_ENABLE,
    BLE_CMD_SET_PHY,
    BLE_CMD_EXT_SCAN_PARAM,
    BLE_CMD_EXT_SCAN_ENABLE,
    BLE_CMD_EXT_CREATE_CONN,
    BLE_CMD_PERIODIC_ADV_PARAM,
    BLE_CMD_PERIODIC_ADV_DATA,
    BLE_CMD_PERIODIC_ADV_ENABLE,
    BLE_CMD_PERIODIC_ADV_CREAT_SYNC,

    //client
    BLE_CMD_SEARCH_PROFILE = 0x80,
    BLE_CMD_WRITE_CCC,
    BLE_CMD_ONNN_PARAM_UPDATA,
} ble_cmd_type_e;

typedef enum {
    BLE_CMD_RET_SUCESS =  0, //执行成功
    BLE_CMD_RET_BUSY = -100, //命令处理忙
    BLE_CMD_PARAM_OVERFLOW,  //传数溢出
    BLE_CMD_OPT_FAIL,        //操作失败
    BLE_BUFFER_FULL,         //缓存满了
    BLE_BUFFER_ERROR,        //缓存出错
    BLE_CMD_PARAM_ERROR,     //传参出错
    BLE_CMD_STACK_NOT_RUN,   //协议栈没有运行
    BLE_CMD_CCC_FAIL,        //没有使能通知，导致NOTIFY或INDICATE发送失败，
} ble_cmd_ret_e;

//--------------------------------------------
ble_cmd_ret_e ble_user_cmd_prepare(ble_cmd_type_e cmd, int argc, ...);

struct conn_update_param_t {
    u16 interval_min;  //(unit:0.625ms)
    u16 interval_max;  //(unit:0.625ms)
    u16 latency;       //(unit: interval)
    u16 timeout;       //(unit:10ms)
};

typedef enum {
    PFL_SERVER_UUID16 = 1, //指定16bit UUID搜索方式
    PFL_SERVER_UUID128,    //指定128bit UUID搜索方式
    PFL_SERVER_ALL,        //搜索所有的UUID
} search_profile_type_e;


//------
struct create_conn_param_t {
    u16 conn_interval;        //0--public address,1--random address
    u16 conn_latency;         //(unit: interval)
    u16 supervision_timeout;  //(unit:10ms)
    u8 peer_address_type;     //0--public address,1--random address
    u8 peer_address[6];
} _GNU_PACKED_;

typedef struct {
    u8   event_type;    //0--ADV_IND,1--ADV_DIRECT_IND,2--ADV_SCAN_IND,3--ADV_NONCONN_IND,4--SCAN_RSP
    u8   address_type;  //0--public address,1--random address
    u8   address[6];    //peer_address
    s8   rssi;          //range:-127 ~128 dbm
    u8   length;        //广播包长度
    u8   data[0];       //广播包内容
} adv_report_t;

typedef struct {
    //base info
    u8   type;       //< See <btstack/hci_cmds.h> SM_...
    u8   size;
    u16  con_handle; //connection 's handle, >0
    u8   addr_type;  //0--public address,1--random address
    u8   address[6];
    //extend info
    u8   data[4];
} sm_just_event_t;

//BLE_CMD_SET_HCI_CFG
typedef enum {
    HCI_CFG_OWN_ADDRESS_TYPE =  0, //
    HCI_CFG_ADV_FILTER_POLICY, //
    HCI_CFG_SCAN_FILTER_POLICY, //
    HCI_CFG_INITIATOR_FILTER_POLICY, //
    //add here
} hci_cfg_par_e;


//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------

/**********************************************************************************
   @function gatt client 角色 初始化
   @return
 *********************************************************************************/
void gatt_client_init(void);

/**********************************************************************************
   @function 注册gatt client 角色,事件回调处理
   @param [in] handler      事件处理函数
   @return
 *********************************************************************************/
void gatt_client_register_packet_handler(btstack_packet_handler_t handler);

/**********************************************************************************
   @function 初始化配对表
   @param [in]
   @return
 *********************************************************************************/
void le_device_db_init(void);

/**********************************************************************************
   @function 注册passkey 输入回调
   @param [in]
   @return
 *********************************************************************************/
void reset_PK_cb_register(void (*reset_pk)(u32 *));

/**********************************************************************************
   @function 设置ble的蓝牙public地址
   @param [in] addr
   @return 0--success ,非0--fail
   note:可以结合接口 ble_op_set_own_address_type 配置选择地址类型
 *********************************************************************************/
int le_controller_set_mac(void *addr);

/**********************************************************************************
   @function 获取ble的蓝牙public地址
   @param [out] addr
   @return 0--success ,非0--fail
 *********************************************************************************/
int le_controller_get_mac(void *addr);

/**********************************************************************************
   @function 初始化ble的蓝牙random地址
   @param [in] addr
   @return 0--success ,非0--fail
   note:可以结合接口 ble_op_set_own_address_type 配置选择地址类型
 *********************************************************************************/
int le_controller_set_random_mac(void *addr);

/**********************************************************************************
   @function 获取ble的蓝牙random地址
   @param [out] addr
   @return 0--success ,非0--fail
 *********************************************************************************/
int le_controller_get_random_mac(void *addr);

/**********************************************************************************
   @function 配置GATT 角色,default server
   @param [in] role   0--server, or 1--client
   @return
 *********************************************************************************/
void ble_stack_gatt_role(u8 role);//0--server,1--client

/**********************************************************************************
   @function client 连接后初始化
   @param [in] handle   连接 con_handle,range：>0
   @param [in] buffer 配置缓存地址
   @param [in] buffer_size 缓存大小
   @return
 *********************************************************************************/
void user_client_init(u16 handle, u8 *buffer, u16 buffer_size);

/**********************************************************************************
   @function 获取链路对方的信号强度
   @param [in] con_handle 连接handle, range：>0
   @return  rssi 强度， range:-127 ~128 dbm
 *********************************************************************************/
s8 ble_vendor_get_peer_rssi(u16 conn_handle);

/**********************************************************************************
   @function 设置 搜索结束
   @param [in]
   @return
 *********************************************************************************/
void user_client_set_search_complete(void);

/**********************************************************************************
   @function 提供生成 ble对应的类型地址
   @param [out] address
   @param [in] type
   1--STATIC_DEVICE_ADDR
   2--NON_RESOLVABLE_PRIVATE_ADDR
   3--RESOLVABLE_PRIVATE_ADDR
   @return
 *********************************************************************************/
void ble_vendor_address_generate(u8 *address, u8 type);

/**********************************************************************************
   @function 根据提供的edr地址生成对应ble地址
   @param [out] ble_address
   @param [in]  edr_address
   @return
 *********************************************************************************/
void lib_make_ble_address(u8 *ble_address, u8 *edr_address);

/**********************************************************************************
   @function 配置设备的地址类型，默认为 0--public address
   @param [in] address_type   Range: 0x00 to 0x03
   @return  see ble_cmd_ret_e
   !!!注意：设置的时候必须在 设置广播参数 或者扫描参数 或者创建连接参数 前配置好
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_own_address_type(u8 address_type) */
#define ble_op_set_own_address_type(address_type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_OWN_ADDRESS_TYPE,(int)address_type)

/**********************************************************************************
   @function 开关BLE广播， !!!注意：开广播前必现先配置好广播的参数
   @param [in] enable   0 or 1
   @return  see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_adv_enable(int enable) */
#define ble_op_adv_enable(enable)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_ENABLE, 1, (int)enable)

/**********************************************************************************
   @function 配置adv filter policy, default 0
   @param [in] type   Range: 0x00 to 0x03
   @return  see ble_cmd_ret_e
   !!!注意：设置的时候必须在 设置广播参数 前配置好
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_filter_policy(u8 type) */
#define ble_op_set_adv_filter_policy(type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_ADV_FILTER_POLICY,type)

/**********************************************************************************

   @function 配置广播参数，!!!注意：设置的时候必现在广播关闭的状态下
   @param [in] adv_interval     广播周期，Range: 0x0020 to 0x4000 (unit: 0.625ms)
   @param [in] adv_type         广播类型，Range: 0x00 to 0x04
   @param [in] adv_channel      广播类型通道，range:Range: 0x01 to 0x07
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_param(u16 adv_interval,u8 adv_type) */
#define ble_op_set_adv_param(adv_interval,adv_type,adv_channel)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_PARAM, 3, (int)adv_interval, (int)adv_type, (int)adv_channel)


/**********************************************************************************
   @function 配置广播参数，!!!注意：设置的时候必现在广播关闭的状态下
   @param [in] adv_interval     广播周期，Range: 0x0020 to 0x4000 (unit: 0.625ms)
   @param [in] adv_type         广播类型，Range: 0x00 to 0x04
   @param [in] adv_channel      广播类型通道，range:Range: 0x01 to 0x07
   @param [in] peer_info        (全局变量地址)，定向广播时，填入对方的地址信息，
                                peer_info[0]--地址类型range 0~1，peer_info[1~7]--地址内容
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_param_ext(u16 adv_interval,u8 adv_type,u8 adv_channel,const u8 *peer_info) */
#define ble_op_set_adv_param_ext(adv_interval,adv_type,adv_channel,peer_info)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_PARAM_EXT, 4, (int)adv_interval, (int)adv_type, (int)adv_channel, (void*)peer_info)


/**********************************************************************************

   @function 配置广播 Advertising Data内容，!!!注意：设置的时候必现在广播关闭的状态下
   @param [in] adv_len         adv 数据包长度，Range: 0x00 to 0x1f
   @param [in] adv_data        (全局变量地址)，adv数据包地址
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_adv_data(u8 adv_len,const *u8 adv_data) */
#define ble_op_set_adv_data(adv_len,adv_data)     \
	ble_user_cmd_prepare(BLE_CMD_ADV_DATA, 2, (int)adv_len, (void*)adv_data)


/**********************************************************************************

   @function 配置广播 Scan Response Data内容，!!!注意：设置的时候必现在广播关闭的状态下
   @param [in] rsp_len         rsp 数据包长度，Range: 0x00 to 0x1f
   @param [in] rsp_data        (全局变量地址)，rsp数据包地址
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_rsp_data(u8 rsp_len,const *u8 rsp_data) */
#define ble_op_set_rsp_data(rsp_len,rsp_data)     \
	ble_user_cmd_prepare(BLE_CMD_RSP_DATA, 2, (int)rsp_len, (void*)rsp_data)


/**********************************************************************************

   @function 断开 ble 连接
   @param [in] con_handle         连接handle，range：>0
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_disconnect(u16 con_handle) */
#define ble_op_disconnect(con_handle)     \
	ble_user_cmd_prepare(BLE_CMD_DISCONNECT, 1, (int)con_handle)

/**********************************************************************************

   @function 配置ATT发送模块RAM大小
   @param [in] con_handle        连接 con_handle,range：>0
   @param [in] att_ram_addr      传入ATT发送模块ram地址，地址按4字节对齐
   @param [in] att_ram_size      传入ATT发送模块ram大小
   @param [in] att_payload_size  发送ATT包payload的最大长度 <= MTU，range：20 to MTU size
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_att_send_init(u16 con_handle,u8 *att_ram_addr,int att_ram_size,int att_payload_size) */
#define ble_op_att_send_init(con_handle,att_ram_addr,att_ram_size,att_payload_size)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, con_handle,att_ram_addr,att_ram_size,att_payload_size)


/**********************************************************************************

   @function 根据对方的接收MTU大小，配置本地可发送的MTU的大小
   @param [in]mtu  对方mtu payload的大小
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_att_set_send_mtu(u16 mtu) */
#define ble_op_att_set_send_mtu(mtu)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_MTU_SIZE, 1, mtu);



/**********************************************************************************

   @function 获取ATT发送模块，cbuffer 可写入数据的长度
   @param [out] remain_size_ptr   输出可写入长度值
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_att_get_remain(int *remain_size_ptr) */
#define ble_op_att_get_remain(remain_size_ptr)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_VAILD_LEN, 1, remain_size_ptr)

/**********************************************************************************

   @function   ATT操作handle发送数据
   @param [in] att_handle  att 操handle
   @param [in] data  数据地址
   @param [in] len   数据长度  <= cbuffer 可写入的长度
   @param [in] att_op_type   see  att_op_type_e (att.h)
   @return    see ble_cmd_ret_e
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_att_send_data(u16 att_handle,u8 *data,u16 len, att_op_type_e att_op_type) */
#define ble_op_att_send_data(att_handle,data,len,att_op_type)     \
	ble_user_cmd_prepare(BLE_CMD_ATT_SEND_DATA, 4, att_handle, data, len, att_op_type)


/**********************************************************************************

   @function   请求更新连接参数
   @param [in] con_handle 连接 con_handle,range：>0
   @param [in] con_param (全局变量地址)，连接参数
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_conn_param_request(u16 con_handle,const struct conn_update_param_t *con_param) */
#define ble_op_conn_param_request(con_handle,con_param)     \
	ble_user_cmd_prepare(BLE_CMD_REQ_CONN_PARAM_UPDATE, 2, con_handle, (void*)con_param)

/**********************************************************************************

   @function  发起data length 交换
   @param [in] tx_octets
   @param [in] tx_time
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_data_length(u16 con_handle,u16 tx_octets,u16 tx_time) */
#define ble_op_set_data_length(con_handle,tx_octets,tx_time)     \
	ble_user_cmd_prepare(BLE_CMD_SET_DATA_LENGTH, 3, con_handle, tx_octets, tx_time)


/**********************************************************************************

   @function  ble5.0 配置 广播参数
   @param [in] param  广播参数
   @param [in] param_len  参数长度
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_ext_adv_param(u8 *param,u16 param_len) */
#define ble_op_set_ext_adv_param(param,param_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_ADV_PARAM, 2, param, param_len)

/**********************************************************************************

   @function ble5.0 配置 adv 数据包
   @param [in]data  数据包内容
   @param [in]data_len 数据包长度
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_ext_adv_data(u8 *data,u16 data_len) */
#define ble_op_set_ext_adv_data(data,data_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_ADV_DATA, 2, data, data_len)


/**********************************************************************************

   @function ble5.0 配置 respond 数据包
   @param [in]data  数据包内容
   @param [in]data_len 数据包长度
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_ext_rsp_data(u8 *data,u16 data_len) */
#define ble_op_set_ext_rsp_data(data,data_len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_RSP_DATA, 2, data, data_len)



/**********************************************************************************

   @function ble5.0 开关广播
   @param [in] cmd  命令信息
   @param [in] cmd_len 命令长度
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_ext_adv_enable(u8 *cmd,u16 cmd_le) */
#define ble_op_set_ext_adv_enable(cmd,len)     \
	ble_user_cmd_prepare(BLE_CMD_EXT_ADV_ENABLE, 2, cmd, cmd_len)


/**********************************************************************************

   @function ble5.0 配置phy
   @param [in]con_handle 连接 con_handle,range：>0
   @param [in]all_phys
   @param [in]tx_phy
   @param [in]rx_phy
   @param [in]phy_options
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_ext_phy(u16 con_handle,u16 all_phys,u16 tx_phy,u16 rx_phy,u16 phy_options) */
#define ble_op_set_ext_phy(con_handle,all_phys,tx_phy,rx_phy,phy_options)     \
    ble_user_cmd_prepare(BLE_CMD_SET_PHY, 5, con_handle, all_phys, tx_phy, rx_phy, phy_options)


/**********************************************************************************

   @function  ble5.0 配置 主机scan 参数
   @param [in]param    参数内容
   @param [in]param_len 参数长度
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_ext_scan_param(u8 *param,u16 param_le) */
#define ble_op_set_ext_scan_param(param,param_len)     \
        ble_user_cmd_prepare(BLE_CMD_EXT_SCAN_PARAM, 2, param, param_len)


/**********************************************************************************

   @function  ble5.0 配置 主机scan 开关
   @param [in] cmd  命令信息
   @param [in] cmd_len 命令长度
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_ext_scan_enable(u8 *cmd,u16 cmd_le) */
#define ble_op_ext_scan_enable(cmd,cmd_len)     \
        ble_user_cmd_prepare(BLE_CMD_EXT_SCAN_ENABLE, 2, cmd, cmd_len)


/**********************************************************************************

   @function  ble5.0 配置 主机创建连接监听
   @param [in]conn_param 连接参数信息
   @param [in]param_len 参数长度
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_ext_create_conn(u8 *conn_param,u16 param_len_len) */
#define ble_op_ext_create_conn(conn_param,param_len)     \
    ble_user_cmd_prepare(BLE_CMD_EXT_CREATE_CONN, 2, conn_param, param_len)


/**********************************************************************************

   @function   忽略进入latency 的次数。!!!注意：让设备不进入latency模式，加快响应速度，但会耗电
   @param [in] con_handle 连接 con_handle,range：>0
   @param [in] skip_interval  忽略的interval的次数
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_latency_skip(u16 con_handle,u16 skip_interval) */
#define ble_op_latency_skip(con_handle,skip_interval)     \
	ble_user_cmd_prepare(BLE_CMD_LATENCY_HOLD_CNT, 2, con_handle, skip_interval)

/**********************************************************************************

   @function   测试盒识别按键测试
   @param [in] con_handle 连接 con_handle,range：>0
   @param [in] key_num  按键
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_test_key_num(u16 con_handle,u8 key_num) */
#define ble_op_test_key_num(con_handle,key_num)     \
	ble_user_cmd_prepare(BLE_CMD_SEND_TEST_KEY_NUM, 2, con_handle, key_num)

/**********************************************************************************

   @function   退出ble协议栈
   @param [in] control : 0--退出stack(default),1--退出stack + controller
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_stack_exit(u8 control) */
#define ble_op_stack_exit(control)     \
	ble_user_cmd_prepare(BLE_CMD_STACK_EXIT, 1, control)

/**********************************************************************************

   @function   挂载协议栈线程调用
   @param [in] thread_callback  回调函数
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_regist_thread_call(void (*thread_callback)(void)) */
#define ble_op_regist_thread_call(thread_callback)     \
	ble_user_cmd_prepare(BLE_CMD_REGIEST_THREAD, 1, thread_callback)

/**********************************************************************************

   @function
   @param [in]
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\



/**********************************************************************************

   @function  开关BLE搜索扫描，!!!注意：开搜索前必现先配置好搜索的参数
   @param [in] enable   0 or 1
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
   note:filter_duplicate 默认 为 1
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_scan_enable(u8 enable) */
#define ble_op_scan_enable(enable)     \
	ble_user_cmd_prepare(BLE_CMD_SCAN_ENABLE, 1, enable)

/**********************************************************************************

   @function  开关BLE搜索扫描，!!!注意：开搜索前必现先配置好搜索的参数
   @param [in] enable   0 or 1
   @param [in] filter_duplicate  0 or 1
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
   note:filter_duplicate 默认 为 1
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_scan_enable2(u8 enable,u8 filter_duplicate) */
#define ble_op_scan_enable2(enable,filter_duplicate)     \
	ble_user_cmd_prepare(BLE_CMD_SCAN_ENABLE2, 2, enable,filter_duplicate)

/**********************************************************************************
   @function 配置scan filter policy,default 0
   @param [in] type   Range: 0x00 to 0x03
   @return  see ble_cmd_ret_e
   !!!注意：设置的时候必须在 设置扫描参数 前配置好
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_scan_filter_policy(u8 type) */
#define ble_op_set_scan_filter_policy(type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_SCAN_FILTER_POLICY,type)

/**********************************************************************************

   @function   配置搜索参数
   @param [in] scan_type       搜索类型 Range: 0x00 to 0x01     (unit: 0.625ms)
   @param [in] scan_interval   搜索周期 Range: 0x0004 to 0x4000 (unit: 0.625ms)   >= scan_window
   @param [in] scan_window     搜索窗口 Range: 0x0004 to 0x4000 (unit: 0.625ms),  <= scan_interval
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_set_scan_param(u8 scan_type,u16 scan_interval,u16 scan_window) */
#define ble_op_set_scan_param(scan_type,scan_interval,scan_window)     \
	ble_user_cmd_prepare(BLE_CMD_SCAN_PARAM, 3, scan_type, scan_interval, scan_window)

/**********************************************************************************
   @function 配置creat filter policy,default 0
   @param [in] type   Range: 0x00 to 0x03
   @return  see ble_cmd_ret_e
   !!!注意：设置的时候必须在 设置创建连接参数 前配置好
 *********************************************************************************/
/* ble_cmd_ret_e ble_op_set_create_filter_policy(u8 type) */
#define ble_op_set_create_filter_policy(type)     \
	ble_user_cmd_prepare(BLE_CMD_SET_HCI_CFG, 2,HCI_CFG_INITIATOR_FILTER_POLICY,type)

/**********************************************************************************

   @function  建立BLE创建连接监听
   @param [in] create_conn_param 连接参数
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_create_connection(struct create_conn_param_t * create_conn_param) */
#define ble_op_create_connection(create_conn_param)     \
	ble_user_cmd_prepare(BLE_CMD_CREATE_CONN, 1, create_conn_param)

/**********************************************************************************

   @function   取消BLE连接监听
   @param [in]
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_create_connection_cancel(void) */
#define ble_op_create_connection_cancel()     \
	ble_user_cmd_prepare(BLE_CMD_CREATE_CONN_CANCEL,0)

/**********************************************************************************

   @function   ble 主机搜索所有服务
   @param [in]
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_search_profile_all(void) */
#define ble_op_search_profile_all()     \
	ble_user_cmd_prepare(BLE_CMD_SEARCH_PROFILE, 2, PFL_SERVER_ALL, 0)

/**********************************************************************************

   @function   ble 主机搜索指定UUID16服务
   @param [in] uuid16   uuid
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_search_profile_uuid16(u16 uuid16) */
#define ble_op_search_profile_uuid16(uuid16)     \
	ble_user_cmd_prepare(BLE_CMD_SEARCH_PROFILE, 2, PFL_SERVER_UUID16, uuid16)

/**********************************************************************************

   @function   ble 主机搜索指定UUID128服务
   @param [in] uuid16   uuid128
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_search_profile_uuid128(const u8 *uuid128_pt) */
#define ble_op_search_profile_uuid128(uuid128_pt)     \
	ble_user_cmd_prepare(BLE_CMD_SEARCH_PROFILE, 2, PFL_SERVER_UUID128, uuid128_pt)

/**********************************************************************************

   @function   ble 主机 更新连接参数
   @param [in] con_handle 连接 con_handle,range：>0
   @param [in] con_param (全局变量地址)，连接参数
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\
/* ble_cmd_ret_e ble_op_conn_param_update(u16 con_handle,struct conn_update_param_t *con_param) */
#define ble_op_conn_param_update(con_handle,con_param)     \
	ble_user_cmd_prepare(BLE_CMD_ONNN_PARAM_UPDATA, 2, con_handle, con_param)


/**********************************************************************************

   @function
   @param [in]
   @param [in]
   @param [in]
   @param [in]
   @return    see ble_cmd_ret_e
 *********************************************************************************/\


/**********************************************************************************

   @function   ble 从机 配置配对表(可以不设置使用sdk默认值)
   @param [in] pair_devices_count 记录配对设备  range: 0~10,默认10 ,若配置为0,不支持配对
   @param [in] is_allow_cover  是否允许循环覆盖记录  1 or 0,默认1
   @param [in]
   @param [in]
   @return    true or false
	note: 上电调用配置,若配置的个数跟之前不一样，默认清楚所有的配对表数据
 *********************************************************************************/\
void ble_list_config_reset(u8 pair_devices_count, u8 is_allow_cover);

/**********************************************************************************

   @function   ble 从机 清空配对表
   @param [in]
   @param [in]
   @return    true or false
	note:
 *********************************************************************************/\
bool ble_list_clear_all(void);

/**********************************************************************************

   @function   ble 从机 检测连接地址是否在已配对表中
   @param [in] conn_addr        对方地址6bytes
   @param [in] conn_addr_type   对方地址类型range: 0~1
   @param [in]
   @param [in]
   @return    true or false
	note:
 *********************************************************************************/\
bool ble_list_check_addr_is_exist(u8 *conn_addr, u8 conn_addr_type);


/**********************************************************************************

   @function   ble 从机 把设备从配对表中删除
   @param [in] conn_addr        对方连接地址6bytes
   @param [in] conn_addr_type   对方连接地址类型range: 0~1
   @param [in]
   @param [in]
   @return    true or false
	note:
 *********************************************************************************/\
bool ble_list_delete_device(u8 *conn_addr, u8 conn_addr_type);


/**********************************************************************************

   @function   ble 从机 获取配对表中最后连接设备的 id_address (public address)
   @param [out] id_addr
   @param [in]
   @param [in]
   @return    true or false
	note:
 *********************************************************************************/\
bool ble_list_get_last_id_addr(u8 *id_addr);


/**********************************************************************************

   @function   ble 从机 获取已配对设备连接地址对应的 id_address (public address)
   @param [in] conn_addr        对方连接地址6bytes
   @param [in] conn_addr_type   对方连接地址类型range: 0~1
   @param [out] id_addr
   @param [in]
   @param [in]
   @return    true or false
	note:
 *********************************************************************************/\
bool ble_list_get_id_addr(u8 *conn_addr, u8 conn_addr_type, u8 *id_addr);



































#endif

