1、app指令补全
    1)模块化：设计成可以一个c文件可以移植的模块
2、整理lcdseg和1621的显示，最好做到模块化
    1)模块化：分别整理到一个独立的c文件上，做到可方便移植
    2)简化程序，重新设计
3、写RGB驱动框架，独立创建c文件来写
    1)颜色分为:Red,Green,Blue,Yellow,Cyan,Purple,White;还有Auto模式,默认Auto模式
    2)每种颜色单独显示接口:user_set_rgb_mode(u8 mode)
    3)RGB直驱与RGBMCU(3-Line[16b4])驱动都要加上
    4)配合设计RGB检测电路：LCDSEG2加3.9K下拉为检测RGBMCU；直驱检测则为直接将RGB的IO口设置为输入上拉检测到低电平则表明该路有三极管，此时认为有RGB直驱电路:
        arr_var.rgb_online = 1;作为打开rgb标志位
    5)app控制、返回、显示

P.S. 设计时尽量用变量来作为开关而不是#define