#include "ui/includes.h"
#include "timer.h"
#include "asm/crc16.h"
#include "ui/lcd_spi/lcd_drive.h"
#include "ascii.h"
#include "font/font_textout.h"
#include "res/rle.h"
#include "res/resfile.h"
#include "app_config.h"
#include "dev_manager.h"

#if TCFG_SPI_LCD_ENABLE

#define UI_DEBUG 0
/* #define UI_BUF_CALC */

#if (UI_DEBUG == 1)

#define UI_PUTS puts
#define UI_PRINTF printf

#else

#define UI_PUTS(...)
#define UI_PRINTF(...)

#endif

#define _RGB565(r,g,b)  (u16)((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3))
#define UI_RGB565(c)  \
        _RGB565((c>>16)&0xff,(c>>8)&0xff,c&0xff)

#define TEXT_MONO_CLR 0x555aaa
#define TEXT_MONO_INV 0xaaa555
#define RECT_MONO_CLR 0x555aaa
#define BGC_MONO_SET  0x555aaa


struct fb_map_user {
    u16 xoffset;
    u16 yoffset;
    u16 width;
    u16 height;
    u8  *baddr;
    u8  *yaddr;
    u8  *uaddr;
    u8  *vaddr;
    u8 transp;
    u8 format;
};

struct fb_var_screeninfo {
    u16 s_xoffset;            //显示区域x坐标
    u16 s_yoffset;            //显示区域y坐标
    u16 s_xres;               //显示区域宽度
    u16 s_yres;               //显示区域高度
    u16 v_xoffset;      //屏幕的虚拟x坐标
    u16 v_yoffset;      //屏幕的虚拟y坐标
    u16 v_xres;         //屏幕的虚拟宽度
    u16 v_yres;         //屏幕的虚拟高度
    u16 rotate;
};

struct window_head {
    u32 offset;
    u32 len;
    u32 ptr_table_offset;
    u16 ptr_table_len;
    u16 crc_data;
    u16 crc_table;
    u16 crc_head;
};

struct ui_file_head {
    u8  res[16];
    u8 type;
    u8 window_num;
    u16 prop_len;
    u8 rotate;
    u8 rev[3];
};


static u32 ui_rotate = false;
static u32 ui_hori_mirror = false;
static u32 ui_vert_mirror = false;
static int malloc_cnt = 0;
static FILE *ui_file = NULL;
static int ui_file_len = 0;

static int open_resource_file();

static const struct ui_platform_api br23_platform_api;

struct ui_priv {
    struct ui_platform_api *api;
    struct lcd_interface *lcd;
    int window_offset;
    struct lcd_info info;
};
static struct ui_priv priv ALIGNED(4);
#define __this (&priv)

#ifdef UI_BUF_CALC
struct buffer {
    struct list_head list;
    u8 *buf;
    int size;
};
struct buffer buffer_used = {0};
#endif

void *br23_malloc(int size)
{
    void *buf;
    malloc_cnt++;
    buf = (void *)malloc(size);

    /* printf("platform_malloc : 0x%x, %d\n", buf, size); */
#ifdef UI_BUF_CALC
    struct buffer *new = (struct buffer *)malloc(sizeof(struct buffer));
    new->buf = buf;
    new->size = size;
    list_add_tail(new, &buffer_used);
    printf("platform_malloc : 0x%x, %d\n", buf, size);

    struct buffer *p;
    int buffer_used_total = 0;
    list_for_each_entry(p, &buffer_used.list, list) {
        buffer_used_total += p->size;
    }
    printf("used buffer size:%d\n\n", buffer_used_total);
#endif

    return buf;
}

void br23_free(void *buf)
{

    /* printf("platform_free : 0x%x\n",buf); */
    free(buf);
    malloc_cnt--;

#ifdef UI_BUF_CALC
    struct buffer *p, *n;
    list_for_each_entry_safe(p, n, &buffer_used.list, list) {
        if (p->buf == buf) {
            printf("platform_free : 0x%x, %d\n", p->buf, p->size);
            __list_del_entry(p);
            free(p);
        }
    }

    int buffer_used_total = 0;
    list_for_each_entry(p, &buffer_used.list, list) {
        buffer_used_total += p->size;
    }
    printf("used buffer size:%d\n\n", buffer_used_total);
#endif
}

int ui_platform_ok()
{
    return (malloc_cnt == 0);
}

static void draw_rect_range_check(struct rect *r, struct fb_map_user *map)
{
    if (r->left < map->xoffset) {
        r->left = map->xoffset;
    }
    if (r->left > (map->xoffset + map->width)) {
        r->left = map->xoffset + map->width;
    }
    if ((r->left + r->width) > (map->xoffset + map->width)) {
        r->width = map->xoffset + map->width - r->left;
    }
    if (r->top < map->yoffset) {
        r->top = map->yoffset;
    }
    if (r->top > (map->yoffset + map->height)) {
        r->top = map->yoffset + map->height;
    }
    if ((r->top + r->height) > (map->yoffset + map->height)) {
        r->height = map->yoffset + map->height - r->top;
    }

    ASSERT(r->left >= map->xoffset);
    ASSERT(r->top  >= map->yoffset);
    ASSERT((r->left + r->width) <= (map->xoffset + map->width));
    ASSERT((r->top + r->height) <= (map->yoffset + map->height));
}


/* 透明色: 16bits 0x55aa      0101 0xxx 1011 01xx 0101 0xxx
 *         24bits 0x50b450    0101 0000 1011 0100 0101 0000 , 80 180 80
 * */
void __font_pix_copy(struct draw_context *dc, int format, struct fb_map_user *map, u8 *pix, struct rect *draw, int x, int y,
                     int height, int width, int color)
{

    int i, j, h;
    u16 osd_color;
    u32 size;

    osd_color = (format == DC_DATA_FORMAT_OSD8) || (format == DC_DATA_FORMAT_OSD8A) ? color & 0xff : color & 0xffff ;

    for (j = 0; j < (height + 7) / 8; j++) { /* 纵向8像素为1字节 */
        for (i = 0; i < width; i++) {
            if (((i + x) >= draw->left)
                && ((i + x) <= (draw->left + draw->width - 1))) { /* x在绘制区域，要绘制 */
                u8 pixel = pix[j * width + i];
                int hh = height - (j * 8);
                if (hh > 8) {
                    hh = 8;
                }
                for (h = 0; h < hh; h++) {
                    if (((y + j * 8 + h) >= draw->top)
                        && ((y + j * 8 + h) <= (draw->top + draw->height - 1))) { /* y在绘制区域，要绘制 */
                        u16 clr = pixel & BIT(h) ? osd_color : 0;
                        if (clr) {
                            if (platform_api->draw_point) {
                                platform_api->draw_point(dc, x + i, y + j * 8 + h, clr);
                            }
                        }
                    }
                } /* endof for h */
            }
        }/* endof for i */
    }/* endof for j */
}


static int image_str_size_check(int page_num, const char *txt, int *width, int *height)
{

    u16 id = ((u8)txt[1] << 8) | (u8)txt[0];
    u16 cnt = 0;
    struct image_file file;
    int w = 0, h = 0;

    while (id != 0x00ff) {
        if (open_image_by_id(&file, id, page_num) != 0) {
            return -EFAULT;
        }
        w += file.width;
        cnt += 2;
        id = ((u8)txt[cnt + 1] << 8) | (u8)txt[cnt];
    }
    h = file.height;
    *width = w;
    *height = h;
    return 0;
}

void platform_putchar(struct font_info *info, u8 *pixel, u16 width, u16 height, u16 x, u16 y)
{
    __font_pix_copy(info->dc, info->disp.format,
                    (struct fb_map_user *)info->disp.map,
                    pixel,
                    (struct rect *)info->disp.rect,
                    x,
                    y,
                    height,
                    width,
                    info->disp.color);
}


struct file_dev {
    const char *logo;
    const char *root_path;
};


struct file_browser {
    int show_mode;
    struct rect r;
    struct vfscan *fscan;
    /* struct server *server; */
    struct ui_file_browser bro;
    struct file_dev dev;//支持三个设备
};


static int check_file_ext(const char *ext_table, const char *ext)
{
    const char *str;

    for (str = ext_table; *str != '\0'; str += 3) {
        if (0 == ASCII_StrCmpNoCase(ext, str, 3)) {
            return true;
        }
    }
    return false;
}

static const u8 MUSIC_SCAN_PARAM[] = "-t"
                                     "MP1MP2MP3"
                                     " -sn -d"
                                     ;




static int platform_file_get_dev_total()
{
    return dev_manager_get_total(1);
}



static void platform_file_browser_get_dev_info(struct ui_file_browser *_bro, u8 index)
{
    struct file_browser *bro = container_of(_bro, struct file_browser, bro);
    struct file_dev *file_dev;//支持三个设备


    if (!bro) {
        return;
    }
    struct __dev *dev = dev_manager_find_by_index(index, 0);//storage_dev_find_by_index(index);
    if (dev) {
        file_dev = &bro->dev;
        file_dev->logo = dev_manager_get_logo(dev);//获取设备logo
        file_dev->root_path = dev_manager_get_root_path_by_logo(dev);//获取设备路径
    }
}



static struct ui_file_browser *platform_file_browser_open(struct rect *r,
        const char *path, const char *ftype, int show_mode)
{
    int err;
    struct file_browser *bro;
    struct __dev *dev = 0;
    bro = (struct file_browser *)malloc(sizeof(*bro));
    if (!bro) {
        return NULL;
    }
    bro->bro.file_number = 0;
    bro->show_mode = show_mode;

    if (!path) {
        dev = dev_manager_find_active(0);///storage_dev_last();//获取最后一个设备的路径
        if (!dev) {
            free(bro);
            return NULL;
        }
        path = dev_manager_get_root_path_by_logo(dev);//dev->root_path;
    }

    if (!ftype) {
        ftype = MUSIC_SCAN_PARAM;
    }

    bro->fscan = fscan(path, ftype, 9);
    bro->bro.dev_num =  dev_manager_get_total(1);//获取在线设备总数

    if (bro->fscan) {
        bro->bro.file_number = bro->fscan->file_number;
        if (bro->bro.file_number == 0) {
            return &bro->bro;
        }
    }

    if (r) {
        memcpy(&bro->r, r, sizeof(struct rect));
    }

    return &bro->bro;

__err:
    fscan_release(bro->fscan);
    free(bro);
    return NULL;
}



static void platform_file_browser_close(struct ui_file_browser *_bro)
{
    struct file_browser *bro = container_of(_bro, struct file_browser, bro);

    if (!bro) {
        return;
    }
    if (bro->fscan) {
        fscan_release(bro->fscan);
    }
    free(bro);
}

static int platform_get_file_attrs(struct ui_file_browser *_bro,
                                   struct ui_file_attrs *attrs)
{
    int i, j;
    struct vfs_attr attr;
    struct file_browser *bro = container_of(_bro, struct file_browser, bro);

    if (!bro->fscan) {
        return -ENOENT;
    }

    FILE *file = fselect(bro->fscan, FSEL_BY_NUMBER, attrs->file_num + 1);
    if (!file) {
        return -ENOENT;
    }

    attrs->format = "ascii";

    fget_attrs(file, &attrs->attr);
    /* printf(" attr = %x, fsize =  %x,sclust = %x\n",attrs->attr.attr,attrs->attr.fsize,attrs->attr.sclust); */

    struct sys_time *time;
    time =  &(attrs->attr.crt_time);
    /* printf("y =%d  m =%d d = %d,h = %d ,m = %d ,s =%d\n",time->year,time->month,time->day,time->hour,time->min,time->sec);  */

    time =  &(attrs->attr.wrt_time);
    /* printf("y =%d  m =%d d = %d,h = %d ,m = %d ,s =%d\n",time->year,time->month,time->day,time->hour,time->min,time->sec);  */


    int len = fget_name(file, (u8 *)attrs->fname, 16);//长文件获取有问题
    if (len < 0) {
        fclose(file);
        return -EINVAL;
    }

    for (i = 0; i < len; i++) {
        if ((u8)attrs->fname[i] >= 0x80) {
            attrs->format = "uft16";
            goto _next;
        }
    }


    /* ASCII_ToUpper(attrs->fname, strlen(attrs->fname)); */

_next:

#if 0//文件系统接口不完善，临时解决
    for (i = 0; i < len; i++) {
        if (attrs->fname[i] == '.') {
            break;
        }
    }

    if (i == len) {
        attrs->ftype = UI_FTYPE_DIR;
    } else {
        char *ext = attrs->fname + i + 1;

        if (check_file_ext("JPG", ext)) {
            attrs->ftype = UI_FTYPE_IMAGE;
        } else if (check_file_ext("MOVAVI", ext)) {
            attrs->ftype = UI_FTYPE_VIDEO;
        } else if (check_file_ext("MP3WMAWAV", ext)) {
            attrs->ftype = UI_FTYPE_AUDIO;
        } else {
            attrs->ftype = UI_FTYPE_UNKNOW;
        }
    }
#else
    /* printf("name = %d %d \n",strlen(attrs->fname),len); */
    /* put_buf(attrs->fname,len); */

    if (attrs->attr.attr & F_ATTR_DIR) {
        attrs->ftype = UI_FTYPE_DIR;
    } else {
        char *ext = attrs->fname + strlen(attrs->fname) - 3;
        if (check_file_ext("JPG", ext)) {
            attrs->ftype = UI_FTYPE_IMAGE;
        } else if (check_file_ext("MOVAVI", ext)) {
            attrs->ftype = UI_FTYPE_VIDEO;
        } else if (check_file_ext("MP3WMAWAV", ext)) {
            attrs->ftype = UI_FTYPE_AUDIO;
        } else {
            attrs->ftype = UI_FTYPE_UNKNOW;
        }

    }

#endif

    fclose(file);

    return 0;
}

static int platform_set_file_attrs(struct ui_file_browser *_bro,
                                   struct ui_file_attrs *attrs)
{
    int attr = 0;
    struct file_browser *bro = container_of(_bro, struct file_browser, bro);

    if (!bro->fscan) {
        return -ENOENT;
    }

    FILE *file = fselect(bro->fscan, FSEL_BY_NUMBER, attrs->file_num + 1);
    if (!file) {
        return -EINVAL;
    }

    fget_attr(file, &attr);
    if (attrs->attr.attr & F_ATTR_RO) {
        attr |= F_ATTR_RO;
    } else {
        attr &= ~F_ATTR_RO;
    }
    fset_attr(file, attr);

    fclose(file);

    return 0;
}

static void *platform_open_file(struct ui_file_browser *_bro,
                                struct ui_file_attrs *attrs)
{
    struct file_browser *bro = container_of(_bro, struct file_browser, bro);

    if (!bro->fscan) {
        return NULL;
    }

    return fselect(bro->fscan, FSEL_BY_NUMBER, attrs->file_num + 1);
}

static int platform_delete_file(struct ui_file_browser *_bro,
                                struct ui_file_attrs *attrs)
{
    struct file_browser *bro = container_of(_bro, struct file_browser, bro);

    if (!bro->fscan) {
        return -EINVAL;
    }

    FILE *file = fselect(bro->fscan, FSEL_BY_NUMBER, attrs->file_num + 1);
    if (!file) {
        return -EFAULT;
    }
    fdelete(file);

    return 0;
}

void test_browser()
{
    struct ui_file_browser *browser = NULL;
    static struct ui_file_attrs attrs = {0};
    if (!browser) {
        browser = platform_file_browser_open(NULL, NULL, MUSIC_SCAN_PARAM, 0);
    }

    if (browser) {
        printf("file num = %d \n", browser->file_number);
        platform_get_file_attrs(browser, &attrs);
        printf("format =%s name =%s type = %x \n", attrs.format, attrs.fname, attrs.ftype);
        platform_delete_file(browser, &attrs);
        attrs.file_num ++;
        if (attrs.file_num >= browser->file_number) {
            attrs.file_num = 0;
        }
    }
}

static void *br23_set_timer(void *priv, void (*callback)(void *), u32 msec)
{
    return (void *)sys_timer_add(priv, callback, msec);
}

static int br23_del_timer(void *fd)
{
    if (fd) {
        sys_timer_del((int)fd);
    }

    return 0;
}

u32 __attribute__((weak)) set_retry_cnt()
{
    return 10;
}

AT_UI_RAM
static void *br23_load_widget_info(void *_head, u8 page)
{
    struct ui_file_head head ALIGNED(4);
    static union ui_control_info info ALIGNED(4) = {0};
    static const int rotate[] = {0, 90, 180, 270};
    static int last_page = -1;
    int head_len;
    int retry = 10;

    if (page != (u8) - 1) {
        struct ui_style style;
        int load_style = 0;

        if (last_page == -1) { //第一次上电未初始化
            if (page == 0) { //表盘界面
                style.file = RES_PATH"watch.sty";
            } else {//其他界面
                style.file = RES_PATH"JL.sty";
            }
            load_style = 1;
        } else if ((page == 0) && (last_page > 0)) { //其他 -> 表盘
            style.file = RES_PATH"watch.sty";
            load_style = 1;
        } else if ((page > 0) && (last_page == 0)) {//表盘 -> 其他
            style.file = RES_PATH"JL.sty";
            load_style = 1;
        }

        if (load_style && platform_api->load_style) {
            platform_api->load_style(&style);
            load_style = 0;
        }

        last_page = page;
        res_fseek(ui_file, 0, SEEK_SET);
        res_fread(ui_file, &head, sizeof(struct ui_file_head));
        ui_rotate = rotate[head.rotate];
        ui_core_set_rotate(ui_rotate);
        switch (head.rotate) {
        case 1: /* 旋转90度 */
            ui_hori_mirror = true;
            ui_vert_mirror = false;
            break;
        case 3:/* 旋转270度 */
            ui_hori_mirror = false;
            ui_vert_mirror = true;
            break;
        default:
            ui_hori_mirror = false;
            ui_vert_mirror = false;
            break;
        }

        res_fseek(ui_file, sizeof(struct ui_file_head) + sizeof(struct window_head)*page, SEEK_SET);
        res_fread(ui_file, &__this->window_offset, sizeof(__this->window_offset));
    }

    ASSERT((u32)_head <= ui_file_len, ",_head invalid! _head : 0x%x ui_file_len : 0x%x\n", (u32)_head, ui_file_len);

    do {
        res_fseek(ui_file, __this->window_offset + (u32)_head, SEEK_SET);

        if ((u32)_head == 0) {
            res_fread(ui_file, &info, sizeof(struct window_info));
        } else {
            res_fread(ui_file, &info.head, sizeof(struct ui_ctrl_info_head));
            if (info.head.len > 800) {
                printf("info.head.len = %d\n", info.head.len);
            }
            head_len = info.head.len - sizeof(struct ui_ctrl_info_head);
            if ((head_len > 0) && (head_len <= sizeof(union ui_control_info))) {
                res_fread(ui_file, &((u8 *)&info)[sizeof(struct ui_ctrl_info_head)], head_len);
            } else {
                printf("head_len = %d\n", head_len);
                printf("read widget info err, retry %d!\n", retry);
                if (retry == 0) {
                    printf("read widget info fail!\n");
                    ASSERT(0);
                }
            }
        }
    } while (retry--);

    return &info;
}

AT_UI_RAM
static void *br23_load_css(void *_css)
{
    static struct element_css1 css ALIGNED(4) = {0};

    ASSERT((u32)_css <= ui_file_len, ", _css invalid! _css : 0x%x , ui_file_len : 0x%x\n", _css, ui_file_len);

    res_fseek(ui_file, __this->window_offset + (u32)_css, SEEK_SET);
    res_fread(ui_file, &css, sizeof(struct element_css1));

    return &css;
}

AT_UI_RAM
static void *br23_load_image_list(void *_list)
{
    u16 image[32] ALIGNED(4);
    static struct ui_image_list_t list ALIGNED(4) = {0};
    int retry = 10;

    if ((u32)_list == 0) {
        return NULL;
    }

    ASSERT((u32)_list <= ui_file_len, ", _list invalid! _list : 0x%x, ui_file_len : 0x%x\n", (u32)_list, ui_file_len);

    do {
        memset(&list, 0x00, sizeof(struct ui_image_list));
        res_fseek(ui_file, __this->window_offset + (u32)_list, SEEK_SET);
        res_fread(ui_file, &list.num, sizeof(list.num));
        if (list.num == 0) {
            printf("list.num : %d\n", list.num);
            return NULL;
        }
        if (list.num < 32) {
            res_fread(ui_file, image, list.num * sizeof(list.image[0]));
            memcpy(list.image, image, list.num * sizeof(list.image[0]));
        } else {
            printf("list.num = %d\n", list.num);
            printf("load_image_list error,retry %d!\n", retry);
            if (retry == 0) {
                return NULL;
            }
        }
    } while (retry--);

    return &list;
}


AT_UI_RAM
static void *br23_load_text_list(void *__list)
{
    u8 buf[4 * 2] ALIGNED(4);
    static struct ui_text_list_t _list ALIGNED(4) = {0};
    struct ui_text_list_t *list;
    int retry = 10;
    int i;

    if ((u32)__list == 0) {
        return NULL;
    }

    ASSERT((u32)__list <= ui_file_len, ", __list invalid! _list : 0x%x, ui_file_len : 0x%x\n", (u32)__list, ui_file_len);

    list = &_list;
    do {
        memset(list, 0x00, sizeof(struct ui_text_list_t));
        res_fseek(ui_file, __this->window_offset + (u32)__list, SEEK_SET);
        res_fread(ui_file, &list->num, sizeof(list->num));
        if (list->num == 0) {
            return NULL;
        }
        if (list->num <= 4) {
            res_fread(ui_file, buf, list->num * 2);
            for (i = 0; i < list->num; i++) {
                ASSERT(buf[2 * i] < 0x100);
                list->str[i] = buf[2 * i];
            }
        } else {
            printf("list->num = %d\n", list->num);
            printf("load_text_list error, retry %d!\n", retry);
            if (retry == 0) {
                return NULL;
            }
        }
    } while (retry--);
    return list;
}



static void *br23_load_window(int id)
{
    u8 *ui;
    int i;
    u32 *ptr;
    u16 *ptr_table;
    struct ui_file_head head ALIGNED(4);
    struct window_head window ALIGNED(4);
    int len = sizeof(struct ui_file_head);
    int retry;
    static const int rotate[] = {0, 90, 180, 270};


    if (!ui_file) {
        printf("ui_file : 0x%x\n", ui_file);
        return NULL;
    }
    ui_platform_ok();

    for (retry = 0; retry < set_retry_cnt(); retry++) {
        res_fseek(ui_file, 0, SEEK_SET);
        res_fread(ui_file, &head, len);

        if (id >= head.window_num) {
            return NULL;
        }

        res_fseek(ui_file, sizeof(struct window_head)*id, SEEK_CUR);
        res_fread(ui_file, &window, sizeof(struct window_head));

        u16 crc = CRC16(&window, (u32) & (((struct window_head *)0)->crc_data));
        if (crc == window.crc_head) {
            ui_rotate = rotate[head.rotate];
            ui_core_set_rotate(ui_rotate);
            switch (head.rotate) {
            case 1: /* 旋转90度 */
                ui_hori_mirror = true;
                ui_vert_mirror = false;
                break;
            case 3:/* 旋转270度 */
                ui_hori_mirror = false;
                ui_vert_mirror = true;
                break;
            default:
                ui_hori_mirror = false;
                ui_vert_mirror = false;
                break;
            }
            goto __read_data;
        }
    }

    return NULL;

__read_data:
    ui = (u8 *)__this->api->malloc(window.len);
    if (!ui) {
        return NULL;
    }
    for (retry = 0; retry < set_retry_cnt(); retry++) {
        res_fseek(ui_file, window.offset, SEEK_SET);
        res_fread(ui_file, ui, window.len);

        u16 crc = CRC16(ui, window.len);
        if (crc == window.crc_data) {
            goto __read_table;
        }
    }

    __this->api->free(ui);
    return NULL;

__read_table:
    ptr_table = (u16 *)__this->api->malloc(window.ptr_table_len);
    if (!ptr_table) {
        __this->api->free(ui);
        return NULL;
    }
    for (retry = 0; retry < set_retry_cnt(); retry++) {
        res_fseek(ui_file, window.ptr_table_offset, SEEK_SET);
        res_fread(ui_file, ptr_table, window.ptr_table_len);

        u16 crc = CRC16(ptr_table, window.ptr_table_len);
        if (crc == window.crc_table) {
            u16 *offset = ptr_table;
            for (i = 0; i < window.ptr_table_len; i += 2) {
                ptr = (u32 *)(ui + *offset++);
                if (*ptr != 0) {
                    *ptr += (u32)ui;
                }
            }
            __this->api->free(ptr_table);
            return ui;
        }
    }

    __this->api->free(ui);
    __this->api->free(ptr_table);

    return NULL;
}

static void br23_unload_window(void *ui)
{
    if (ui) {
        __this->api->free(ui);
    }
}


static int br23_load_style(struct ui_style *style)
{
    int err;
    int i, j;
    int len;
    struct vfscan *fs;
    char name[64];
    char style_name[16];
    static char cur_style = 0xff;


    if (!style->file && cur_style == 0) {
        return 0;
    }

    if (ui_file) {
        fclose(ui_file);
    }

    if (style->file == NULL) {
        cur_style = 0;
        err = open_resource_file();
        if (err) {
            return -EINVAL;
        }
#if 0
        fs = fscan("mnt/spiflash/res", "-t*.sty");
        if (!fs) {
            printf("open mnt/spiflash/res fail!\n");
            return -EFAULT;
        }
        ui_file = fselect(fs, FSEL_FIRST_FILE, 0);
        if (!ui_file) {
            fscan_release(fs);
            return -ENOENT;
        }
        len = fget_name(ui_file, (u8 *)name, 16);
        if (len) {
            style_name[len - 4] = 0;
            memcpy(style_name, name, len - 4);
            ui_core_set_style(style_name);
        }

        fscan_release(fs);
#else
        ui_file = res_fopen(RES_PATH"JL.sty", "r");
        if (!ui_file) {
            return -ENOENT;
        }
        ui_file_len = res_flen(ui_file);
        len = 6;
        strcpy(style_name, "JL.sty");
        if (len) {
            style_name[len - 4] = 0;
            ui_core_set_style(style_name);
        }
#endif
    } else {
        cur_style = 1;
        ui_file = res_fopen(style->file, "r");
        printf("open stylefile %s\n", style->file);
        if (!ui_file) {
            return -EINVAL;
        }

        ui_file_len = res_flen(ui_file);
        for (i = 0; style->file[i] != '.'; i++) {
            name[i] = style->file[i];
        }
        name[i++] = '.';
        name[i++] = 'r';
        name[i++] = 'e';
        name[i++] = 's';
        name[i] = '\0';
        open_resfile(name);
        printf("open resfile %s\n", name);

        name[--i] = 'r';
        name[--i] = 't';
        name[--i] = 's';
        open_str_file(name);
        printf("open strfile %s\n", name);

        name[i++] = 'a';
        name[i++] = 's';
        name[i++] = 'i';
        font_ascii_init(name);
        printf("open asciifile %s\n", name);

        for (i = strlen(style->file) - 5; i >= 0; i--) {
            if (style->file[i] == '/') {
                break;
            }
        }

        for (i++, j = 0; style->file[i] != '\0'; i++) {
            if (style->file[i] == '.') {
                name[j] = '\0';
                break;
            }
            name[j++] = style->file[i];
        }
        ASCII_ToUpper(name, j);
        err = ui_core_set_style(name);
        if (err) {
            printf("style_err: %s\n", name);
        }
    }

    return 0;

__err2:
    close_resfile();
__err1:
    fclose(ui_file);

    return err;
}

static int br23_open_draw_context(struct draw_context *dc)
{
    dc->buf_num = 1;
    if (__this->lcd->buffer_malloc) {
        u8 *buf;
        u32 len;
        __this->lcd->buffer_malloc(&buf, &len);
        dc->buf0 = buf;
        dc->buf1 = &buf[len / 2];
        dc->len  = len / 2;
        dc->buf  = dc->buf0;
        /* __this->lcd->buffer_malloc(&dc->buf, &dc->len); */
    }

    if (__this->lcd->get_screen_info) {
        __this->lcd->get_screen_info(&__this->info);
        dc->width = __this->info.width;
        dc->height = __this->info.height;
        dc->col_align = __this->info.col_align;
        dc->row_align = __this->info.row_align;
        dc->lines = dc->len / dc->width / 2;
        if (dc->lines > (dc->height / 10)) {
            dc->lines = dc->height / 10;
        }
        printf("dc->width : %d, dc->lines : %d\n", dc->width, dc->lines);
    }
    switch (__this->info.color_format) {
    case LCD_COLOR_RGB565:
        if (dc->data_format != DC_DATA_FORMAT_OSD16) {
            ASSERT(0, "The color format of layer don't match the lcd driver,page %d please select OSD16!", dc->page);
        }
        break;
    case LCD_COLOR_MONO:
        if (dc->data_format != DC_DATA_FORMAT_MONO) {
            ASSERT(0, "The color format of layer don't match the lcd driver,page %d please select OSD1!", dc->page);
        }
        break;
    }
    if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        dc->fbuf_len = __this->info.width * (3 + 3 * dc->lines) + 0x80;
        dc->fbuf = (u8 *)__this->api->malloc(dc->fbuf_len);
    } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
        dc->fbuf_len = __this->info.width * 2;
        dc->fbuf = (u8 *)__this->api->malloc(dc->fbuf_len);
    }

    return 0;
}

static int br23_get_draw_context(struct draw_context *dc)
{
    if (dc->buf == dc->buf0) {
        dc->buf = dc->buf1;
    } else {
        dc->buf = dc->buf0;
    }

    dc->disp.left  = dc->need_draw.left;
    dc->disp.width = dc->need_draw.width;
    if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        int lines = dc->len / dc->need_draw.width / 2;

        if ((dc->disp.top == 0) && (dc->disp.height == 0)) {
            dc->disp.top   = dc->need_draw.top;
            dc->disp.height = lines > dc->need_draw.height ? dc->need_draw.height : lines;
        } else {
            dc->disp.top   = dc->disp.top + dc->disp.height;
            dc->disp.height = lines > (dc->need_draw.top + dc->need_draw.height - dc->disp.top) ?
                              (dc->need_draw.top + dc->need_draw.height - dc->disp.top) : lines;
        }
        dc->disp.height = dc->disp.height / dc->row_align * dc->row_align;
    } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
        dc->disp.top = dc->need_draw.top;
        dc->disp.height = dc->need_draw.height;
    }

    return 0;
}

static int br23_put_draw_context(struct draw_context *dc)
{
    if (__this->lcd->set_draw_area) {
        __this->lcd->set_draw_area(dc->disp.left, dc->disp.left + dc->disp.width - 1,
                                   dc->disp.top, dc->disp.top + dc->disp.height - 1);
    }

    u8 wait = ((dc->need_draw.top + dc->need_draw.height) == (dc->disp.top + dc->disp.height)) ? 1 : 0;
    if (__this->lcd->draw) {
        if (dc->data_format == DC_DATA_FORMAT_OSD16) {
            __this->lcd->draw(dc->buf, dc->disp.height * dc->disp.width * 2, wait);
        } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
            __this->lcd->draw(dc->buf, __this->info.width * __this->info.height / 8, wait);
        }
    }
    return 0;
}


static int br23_set_draw_context(struct draw_context *dc)
{
    return 0;
}

static int br23_close_draw_context(struct draw_context *dc)
{
    if (__this->lcd->buffer_free) {
        __this->lcd->buffer_free(dc->buf);
    }
    if (dc->fbuf) {
        __this->api->free(dc->fbuf);
        dc->fbuf = NULL;
        dc->fbuf_len = 0;
    }

    return 0;
}

static int br23_invert_rect(struct draw_context *dc, u32 acolor)
{
    int i;
    int len;
    int w, h;
    int color = acolor & 0xffff;

    if (dc->data_format == DC_DATA_FORMAT_MONO) {
        color |= BIT(31);
        for (h = 0; h < dc->draw.height; h++) {
            for (w = 0; w < dc->draw.width; w++) {
                if (platform_api->draw_point) {
                    platform_api->draw_point(dc, dc->draw.left + w, dc->draw.top + h, color);
                }
            }
        }
    }
    return 0;
}

static int br23_fill_rect(struct draw_context *dc, u32 acolor)
{
    int i;
    int w, h;
    u16 color = acolor & 0xffff;

    if (dc->data_format == DC_DATA_FORMAT_MONO) {
        color = (color == UI_RGB565(BGC_MONO_SET)) ? 0xffff : 0x55aa;

        for (h = 0; h < dc->draw.height; h++) {
            for (w = 0; w < dc->draw.width; w++) {
                if (platform_api->draw_point) {
                    platform_api->draw_point(dc, dc->draw.left + w, dc->draw.top + h, color);
                }
            }
        }
    } else {
        u16 color16 = (color >> 8) | ((color & 0xff) << 8);
        u32 color32 = (color16 << 16) | color16;

        h = 0;
        u32 *p32 = (u32 *)&dc->buf[(dc->draw.top + h - dc->disp.top) * dc->disp.width * 2 + (dc->draw.left - dc->disp.left) * 2];
        u32 *_p32 = p32;
        u32 len = dc->draw.width * 2;
        if ((u32)p32 % 4) {
            u16 *p16 = (u16 *)p32;
            *p16++ = color16;
            p32 = (u32 *)p16;
            len -= 2;
            ASSERT((u32)p32 % 4 == 0);
        }

        u32 count = len / 4;
        while (count--) {
            *p32++ = color32;
        }
        count = (len % 4) / 2;
        u16 *p16 = (u16 *)p32;
        while (count--) {
            *p16++ = color16;
        }

        for (h = 1; h < dc->draw.height; h++) {
            u32 *__p32 = (u32 *)&dc->buf[(dc->draw.top + h - dc->disp.top) * dc->disp.width * 2 + (dc->draw.left - dc->disp.left) * 2];
            memcpy(__p32, _p32, dc->draw.width * 2);
        }
    }

    return 0;
}

static inline void __draw_vertical_line(struct draw_context *dc, int x, int y, int width, int height, int color, int format)
{
    int i, j;
    struct rect r = {0};
    struct rect disp = {0};

    disp.left  = x;
    disp.top   = y;
    disp.width = width;
    disp.height = height;
    if (!get_rect_cover(&dc->draw, &disp, &r)) {
        return;
    }

    switch (format) {
    case DC_DATA_FORMAT_OSD16:
        for (i = 0; i < r.width; i++) {
            for (j = 0; j < r.height; j++) {
                if (platform_api->draw_point) {
                    platform_api->draw_point(dc, r.left + i, r.top + j, color);
                }
            }
        }
        break;
    case DC_DATA_FORMAT_MONO:
        for (i = 0; i < r.width; i++) {
            for (j = 0; j < r.height; j++) {
                if (platform_api->draw_point) {
                    platform_api->draw_point(dc, r.left + i, r.top + j, color);
                }
            }
        }
        break;

    }
}

static inline void __draw_line(struct draw_context *dc, int x, int y, int width, int height, int color, int format)
{
    int i, j;
    struct rect r = {0};
    struct rect disp = {0};

    disp.left  = x;
    disp.top   = y;
    disp.width = width;
    disp.height = height;
    if (!get_rect_cover(&dc->draw, &disp, &r)) {
        return;
    }

    switch (format) {
    case DC_DATA_FORMAT_OSD16:
        for (i = 0; i < r.height; i++) {
            for (j = 0; j < r.width; j++) {
                if (platform_api->draw_point) {
                    platform_api->draw_point(dc, r.left + j, r.top + i, color);
                }
            }
        }
        break;
    case DC_DATA_FORMAT_MONO:
        for (i = 0; i < r.height; i++) {
            for (j = 0; j < r.width; j++) {
                if (platform_api->draw_point) {
                    platform_api->draw_point(dc, r.left + j, r.top + i, color);
                }
            }
        }
        break;
    }
}

static int br23_draw_rect(struct draw_context *dc, struct css_border *border)
{
    int err;
    int offset;
    int color = border->color & 0xffff;

    /* draw_rect_range_check(&dc->draw, map); */
    /* draw_rect_range_check(&dc->rect, map); */

    if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        color = border->color & 0xffff;
    } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
        color = (color != UI_RGB565(RECT_MONO_CLR)) ? (color ? color : 0xffff) : 0x55aa;
    }

    if (border->left) {
        if (dc->rect.left >= dc->draw.left &&
            dc->rect.left <= rect_right(&dc->draw)) {
            __draw_vertical_line(dc, dc->draw.left, dc->draw.top,
                                 border->left, dc->draw.height, color, dc->data_format);
        }
    }
    if (border->right) {
        if (rect_right(&dc->rect) >= dc->draw.left &&
            rect_right(&dc->rect) <= rect_right(&dc->draw)) {
            __draw_vertical_line(dc, dc->draw.left + dc->draw.width - border->right, dc->draw.top,
                                 border->right, dc->draw.height, color, dc->data_format);
        }
    }
    if (border->top) {
        if (dc->rect.top >= dc->draw.top &&
            dc->rect.top <= rect_bottom(&dc->draw)) {
            __draw_line(dc, dc->draw.left, dc->draw.top,
                        dc->draw.width, border->top, color, dc->data_format);
        }
    }
    if (border->bottom) {
        if (rect_bottom(&dc->rect) >= dc->draw.top &&
            rect_bottom(&dc->rect) <= rect_bottom(&dc->draw)) {
            __draw_line(dc, dc->draw.left, dc->draw.top + dc->draw.height - border->bottom,
                        dc->draw.width, border->bottom, color, dc->data_format);
        }
    }

    return 0;
}

__attribute__((always_inline_when_const_args))
AT_UI_RAM
static u16 get_mixed_pixel(u16 backcolor, u16 color, u8 alpha)
{
    u16 mixed_color;
    u8 r0, g0, b0;
    u8 r1, g1, b1;
    u8 r2, g2, b2;

    if (alpha == 0) {
        return backcolor;
    }

    r0 = ((backcolor >> 11) & 0x1f) << 3;
    g0 = ((backcolor >> 5) & 0x3f) << 2;
    b0 = ((backcolor >> 0) & 0x1f) << 3;

    r1 = ((color >> 11) & 0x1f) << 3;
    g1 = ((color >> 5) & 0x3f) << 2;
    b1 = ((color >> 0) & 0x1f) << 3;

    r2 = (alpha * r1 + (255 - alpha) * r0) / 255;
    g2 = (alpha * g1 + (255 - alpha) * g0) / 255;
    b2 = (alpha * b1 + (255 - alpha) * b0) / 255;

    mixed_color = ((r2 >> 3) << 11) | ((g2 >> 2) << 5) | (b2 >> 3);

    return (mixed_color >> 8) | (mixed_color & 0xff) << 8;
}

static int br23_read_image_info(struct draw_context *dc, u32 id, u8 page, struct ui_image_attrs *attrs)
{
    struct image_file file;

    if (((u16) - 1 == id) || ((u32) - 1 == id)) {
        return -1;
    }

    int err = open_image_by_id(&file, id, dc->page);
    if (err) {
        return -EFAULT;
    }
    attrs->width = file.width;
    attrs->height = file.height;

    return 0;
}

AT_UI_RAM
int line_update(u8 *mask, u16 y, u16 width)
{
    int i;
    if (!mask) {
        return true;
    }
    for (i = 0; i < (width + 7) / 8; i++) {
        if (mask[y * ((width + 7) / 8) + i]) {
            return true;
        }
    }
    return false;
}


AT(.ui_ram)
static int br23_draw_image(struct draw_context *dc, u32 src, u8 quadrant, u8 *mask)
{
    u8 *pixelbuf;
    u8 *temp_pixelbuf;
    u8 *alphabuf;
    u8 *temp_alphabuf;
    struct rect draw_r;
    struct rect r = {0};
    struct rect disp = {0};
    struct image_file file;
    int h, hh, w;
    int buf_offset;

    if (((u16) - 1 == src) || ((u32) - 1 == src)) {
        return -1;
    }

    draw_r.left   = dc->draw.left;
    draw_r.top    = dc->draw.top;
    draw_r.width  = dc->draw.width;
    draw_r.height = dc->draw.height;

    /* UI_PRINTF("image draw %d, %d, %d, %d\n", dc->draw.left, dc->draw.top, dc->draw.width, dc->draw.height); */
    /* UI_PRINTF("image rect %d, %d, %d, %d\n", dc->rect.left, dc->rect.top, dc->rect.width, dc->rect.height); */

    int err = open_image_by_id(&file, src, dc->page);
    if (err) {
        return -EFAULT;
    }

    int x = dc->rect.left;
    int y = dc->rect.top;

    if (dc->align == UI_ALIGN_CENTER) {
        x += (dc->rect.width / 2 - file.width / 2);
        y += (dc->rect.height / 2 - file.height / 2);
    } else if (dc->align == UI_ALIGN_RIGHT) {
        x += dc->rect.width - file.width;
    }

    int temp_pixelbuf_len = dc->width * 2 * dc->lines + 0x40 - 8;
    int temp_alphabuf_len = dc->width * dc->lines + 0x40 - 8;
    int align;

    if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        buf_offset = 0;
        pixelbuf = &dc->fbuf[buf_offset];//2 bytes * line
        buf_offset += dc->width * 2;
        buf_offset = (buf_offset + 3) / 4 * 4;
        alphabuf = &dc->fbuf[buf_offset];//1 bytes * line
        buf_offset += dc->width;
        buf_offset = (buf_offset + 3) / 4 * 4;
        temp_pixelbuf = &dc->fbuf[buf_offset];
        buf_offset += temp_pixelbuf_len;
        buf_offset = (buf_offset + 3) / 4 * 4;
        temp_alphabuf = &dc->fbuf[buf_offset];
    } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
        pixelbuf = dc->fbuf;
    } else {
        ASSERT(0);
    }

    disp.left   = x;
    disp.top    = y;
    disp.width  = file.width;
    disp.height = file.height;

    if (dc->data_format == DC_DATA_FORMAT_MONO) {
        if (get_rect_cover(&draw_r, &disp, &r)) {
            int _offset = -1;
            for (h = 0; h < r.height; h++) {
                if (file.compress == 0) {
                    int offset = (r.top + h - disp.top) / 8 * file.width + (r.left - disp.left);
                    if (_offset != offset) {
                        if (br23_read_image_data(&file, pixelbuf, r.width, offset) != r.width) {
                            return -EFAULT;
                        }
                        _offset = offset;
                    }
                } else {
                    ASSERT(0, "the compress mode not support!");
                }

                for (w = 0; w < r.width; w++) {
                    u8 color = (pixelbuf[w] & BIT((r.top + h - disp.top) % 8)) ? 1 : 0;
                    if (color) {
                        if (platform_api->draw_point) {
                            platform_api->draw_point(dc, r.left + w, r.top + h, color);
                        }
                    }
                }
            }
        }
    } else if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        if (get_rect_cover(&draw_r, &disp, &r)) {
            u32 alpha_addr = 0;
            br23_read_image_data(&file, &alpha_addr, sizeof(alpha_addr), 0);

            for (h = 0; h < r.height;) {
                int rh = r.top + h - disp.top;
                int rw = r.left - disp.left;
                int vh = rh;
                int vw = rw;
                if (quadrant == 0) {
                    ;
                } else if (quadrant == 1) {
                    vh = file.height - rh - 1;
                } else if (quadrant == 2) {
                    vh = file.height - rh - 1;
                    vw = file.width - rw - r.width;
                } else {
                    vw = file.width - rw - r.width;
                }
                ASSERT(vw >= 0);
                ASSERT(vh >= 0);

                struct rle_line *line;
                struct rle_line *alpha_line;
                u8 *ptemp;
                u8 *alpha_ptemp;
                int lines;

                if (file.compress == 0) {
                    int remain = (r.height - h) > (file.height - vh) ? (file.height - vh) : (r.height - h);
                    int offset = 4 + vh * file.width * 2 + vw * 2;

                    br23_read_image_data(&file, &alpha_addr, sizeof(alpha_addr), 0);
                    if (!alpha_addr) {
                        lines = dc->fbuf_len / file.width / 2;
                    } else {
                        lines = dc->fbuf_len / file.width / 3;
                    }
                    lines = (lines > remain) ? remain : lines;
                    if ((quadrant == 1) || (quadrant == 2)) {
                        lines = 1;
                    }
                    pixelbuf = dc->fbuf;
                    alphabuf = &dc->fbuf[(lines * file.width * 2 + 3) / 4 * 4];

                    if (br23_read_image_data(&file, pixelbuf, file.width * 2 * lines, offset) != file.width * 2 * lines) {
                        return -EFAULT;
                    }
                    if (alpha_addr) {
                        offset = alpha_addr + vh * file.width + vw;
                        if (br23_read_image_data(&file, alphabuf, file.width * lines, offset) != file.width * lines) {
                            return -EFAULT;
                        }
                    }
                } else if (file.compress == 1) {
                    int remain = (r.height - h) > (file.height - vh) ? (file.height - vh) : (r.height - h);
                    int headlen = sizeof(struct rle_header) + (remain * 2 + 3) / 4 * 4;

                    line = (struct rle_line *)temp_pixelbuf;
                    ptemp = &temp_pixelbuf[headlen];
                    memset(line, 0x00, sizeof(struct rle_line));

                    br23_read_image_data(&file, ptemp, sizeof(struct rle_header)*remain, 4 + vh * sizeof(struct rle_header));

                    int i;
                    struct rle_header *rle = (struct rle_header *)ptemp;
                    int total_len = 0;
                    for (i = 0; i < remain; i++) {
                        if (i == 0) {
                            line->addr = rle[i].addr;
                            line->len[i] = rle[i].len;
                        } else {
                            line->len[i] = rle[i].len;
                        }
                        if ((total_len + rle[i].len) > (temp_pixelbuf_len - headlen)) {
                            break;
                        }
                        total_len += rle[i].len;
                        line->num ++;
                        if ((quadrant == 1) || (quadrant == 2)) {
                            break;
                        }
                    }
                    br23_read_image_data(&file, ptemp, total_len, 4 + line->addr);

                    if (alpha_addr) {
                        int headlen = sizeof(struct rle_header) + (line->num * 2 + 3) / 4 * 4;
                        alpha_ptemp = &temp_alphabuf[headlen];
                        br23_read_image_data(&file, alpha_ptemp, sizeof(struct rle_header)*line->num, alpha_addr + vh * sizeof(struct rle_header));

                        struct rle_header *rle = (struct rle_header *)alpha_ptemp;
                        alpha_line = (struct rle_line *)temp_alphabuf;
                        memset(alpha_line, 0x00, sizeof(struct rle_line));
                        int total_len = 0;
                        for (i = 0; i < line->num; i++) {
                            if (i == 0) {
                                alpha_line->addr = rle[i].addr;
                                alpha_line->len[i] = rle[i].len;
                            } else {
                                alpha_line->len[i] = rle[i].len;
                            }
                            if ((total_len + rle[i].len) > (temp_alphabuf_len - headlen)) {
                                break;
                            }
                            total_len += rle[i].len;
                            alpha_line->num ++;
                        }

                        br23_read_image_data(&file, alpha_ptemp, total_len, alpha_addr + alpha_line->addr);
                    }
                } else {
                    ASSERT(0, "the compress mode not support!");
                }

                u8 *p0 = ptemp;
                u8 *p1 = alpha_ptemp;
                int line_num;
                if (file.compress == 0) {
                    /* line_num = 1; */
                    line_num = lines;
                } else {
                    if (alpha_addr) {
                        line_num = (line->num > alpha_line->num) ? alpha_line->num : line->num;
                    } else {
                        line_num = line->num;
                    }
                }

                for (hh = 0; hh < line_num; hh++, h++) {
                    if (file.compress == 1) {
                        if (line_update(mask, r.top + h - dc->disp.top, dc->disp.width)) {
                            Rle_Decode(p0, line->len[hh], pixelbuf, file.width * 2, vw * 2, r.width * 2);
                            p0 += line->len[hh];
                            if (alpha_addr) {
                                Rle_Decode(p1, alpha_line->len[hh], alphabuf, file.width, vw, r.width);
                                p1 += alpha_line->len[hh];
                            }
                        } else {
                            p0 += line->len[hh];
                            p1 += alpha_line->len[hh];
                            continue;
                        }
                    }

                    u16 *pdisp = (u16 *)dc->buf;
                    u16 *pixelbuf16 = (u16 *)pixelbuf;

                    if (!alpha_addr) {
                        u16 x0 = r.left + 0/* vww */;
                        u16 y0 = r.top + h;
                        int offset = (y0 - dc->disp.top) * dc->disp.width + (x0 - dc->disp.left);
                        /* if ((offset * 2 + 1) < dc->len) { */
                        /* pdisp[offset] = pixel; */
                        /* } */
                        memcpy(&pdisp[offset], pixelbuf16, r.width * 2);
                        if (file.compress == 0) {
                            pixelbuf += file.width * 2;
                        }
                        continue;
                    }

                    for (w = 0; w < r.width; w++) {
                        u16 color, pixel;
                        u8  alpha = alpha_addr ? alphabuf[w] : 255;

                        pixel = color = pixelbuf16[w];
                        if (color) {
                            if (platform_api->draw_point) {
                                int vww = w;
                                if ((quadrant == 2) || (quadrant == 3)) {
                                    vww = r.width - w - 1;
                                }
                                u16 x0 = r.left + vww;
                                u16 y0 = r.top + h;

                                if (alpha < 255) {
                                    u16 backcolor = platform_api->read_point(dc, x0, y0);
                                    pixel = get_mixed_pixel(backcolor >> 8 | (backcolor & 0xff) << 8, color>>8 | (color & 0xff) << 8, alpha);
                                }

                                if (mask) {
                                    int yy = y0 - dc->disp.top;
                                    int xx = x0 - dc->disp.left;
                                    if (yy >= dc->disp.height) {
                                        continue;
                                    }
                                    if (xx >= dc->disp.width) {
                                        continue;
                                    }
                                    if (mask[yy * ((dc->disp.width + 7) / 8) + xx / 8] & BIT(xx % 8)) {
                                        int offset = (y0 - dc->disp.top) * dc->disp.width + (x0 - dc->disp.left);
                                        if ((offset * 2 + 1) < dc->len) {
                                            pdisp[offset] = pixel;
                                        }
                                    }
                                } else {
                                    int offset = (y0 - dc->disp.top) * dc->disp.width + (x0 - dc->disp.left);
                                    if ((offset * 2 + 1) < dc->len) {
                                        pdisp[offset] = pixel;
                                    }
                                }
                            }
                        }
                    }

                    if (file.compress == 0) {
                        pixelbuf += file.width * 2;
                        alphabuf += file.width;
                    }
                }
            }
        }
    }

    return 0;
}

static int br23_show_text(struct draw_context *dc, struct ui_text_attrs *text)
{
    struct rect draw_r;
    struct rect r = {0};
    struct rect disp = {0};
    struct image_file file;


    /* 控件从绝对x,y 转成相对图层的x,y */
    int x = dc->rect.left;
    int y = dc->rect.top;

    /* 绘制区域从绝对x,y 转成相对图层的x,y */
    draw_r.left   = dc->draw.left;
    draw_r.top    = dc->draw.top;
    draw_r.width  = dc->draw.width;
    draw_r.height = dc->draw.height;

    if (text->format && !strcmp(text->format, "text")) {
        static struct font_info *info = NULL;
        static int language = 0;
        if (!info || (language != ui_language_get())) {
            language = ui_language_get();
            if (info) {
                font_close(info);
            }
            info = font_open(NULL, language);
            ASSERT(info, "font_open fail!");
        }

        if (info && (FT_ERROR_NONE == (info->sta & (~FT_ERROR_NOTABFILE)))) {
            info->disp.map    = 0;
            info->disp.rect   = &draw_r;
            info->disp.format = dc->data_format;
            if (dc->data_format == DC_DATA_FORMAT_OSD16) {
                info->disp.color  = text->color;
            } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
                if (text->color == UI_RGB565(TEXT_MONO_INV)) {
                    info->disp.color = 0x55aa;//清显示
                } else {
                    info->disp.color = (text->color != UI_RGB565(TEXT_MONO_CLR)) ? (text->color ? text->color : 0xffff) : 0x55aa;
                }
            }
            info->dc = dc;

            info->text_width  = draw_r.width;
            info->text_height = draw_r.height;
            info->flags       = text->flags;
            /* info->offset      = text->offset; */
            int roll = 0;//需要滚动
            int multi_line = 0;
            /* FONT_SHOW_MULTI_LINE */
            if (text->encode == FONT_ENCODE_ANSI) {
                int width = font_text_width(info, (u8 *)text->str, text->strlen);
                int height;


                if (info->ascpixel.size) {
                    height = info->ascpixel.size;
                } else if (info->pixel.size) {
                    height = info->pixel.size;
                } else {
                    ASSERT(0, "can't get the height of font.");
                }

                if (width > dc->rect.width) {
                    width = dc->rect.width;
                    roll = 1;
                    multi_line = 1;
                }


                if (text->flags & FONT_SHOW_MULTI_LINE) {
                    height += multi_line * height;
                }

                if (height > dc->rect.height) {
                    height = dc->rect.height;
                }

                y += (dc->rect.height / 2 - height / 2);

                if (dc->align == UI_ALIGN_CENTER) {
                    x += (dc->rect.width / 2 - width / 2);
                } else if (dc->align == UI_ALIGN_RIGHT) {
                    x += (dc->rect.width - width);
                }
                info->x = x;
                info->y = y;
                int len = font_textout(info, (u8 *)(text->str + roll * text->offset * 2), text->strlen - roll * text->offset * 2, x, y);
                ASSERT(len <= 255);
                text->displen = len;
            } else if (text->encode == FONT_ENCODE_UNICODE) {
                if (FT_ERROR_NONE == (info->sta & FT_ERROR_NOTABFILE)) {
                    if (text->endian == FONT_ENDIAN_BIG) {
                        info->bigendian = true;
                    } else {
                        info->bigendian = false;
                    }
                    int width = font_textw_width(info, (u8 *)text->str, text->strlen);
                    int height;

                    if (info->ascpixel.size) {
                        height = info->ascpixel.size;
                    } else if (info->pixel.size) {
                        height = info->pixel.size;
                    } else {
                        ASSERT(0, "can't get the height of font.");
                    }

                    if (width > dc->rect.width) {
                        width = dc->rect.width;
                        roll = 1;
                        multi_line = 1;
                    }

                    if (text->flags & FONT_SHOW_MULTI_LINE) {
                        height += multi_line * height;
                    }


                    if (height > dc->rect.height) {
                        height = dc->rect.height;
                    }

                    y += (dc->rect.height / 2 - height / 2);
                    if (dc->align == UI_ALIGN_CENTER) {
                        x += (dc->rect.width / 2 - width / 2);
                    } else if (dc->align == UI_ALIGN_RIGHT) {
                        x += (dc->rect.width - width);
                    }

                    info->x = x;
                    info->y = y;
                    int len = font_textout_unicode(info, (u8 *)(text->str + roll * text->offset * 2), text->strlen - roll * text->offset * 2, x, y);
                    ASSERT(len <= 255);
                    text->displen = len;
                }
            } else {
                int width = font_textu_width(info, (u8 *)text->str, text->strlen);
                int height;

                if (info->ascpixel.size) {
                    height = info->ascpixel.size;
                } else if (info->pixel.size) {
                    height = info->pixel.size;
                } else {
                    ASSERT(0, "can't get the height of font.");
                }

                if (width > dc->rect.width) {
                    width = dc->rect.width;
                }
                if (height > dc->rect.height) {
                    height = dc->rect.height;
                }

                y += (dc->rect.height / 2 - height / 2);
                if (dc->align == UI_ALIGN_CENTER) {
                    x += (dc->rect.width / 2 - width / 2);
                } else if (dc->align == UI_ALIGN_RIGHT) {
                    x += (dc->rect.width - width);
                }

                info->x = x;
                info->y = y;
                int len = font_textout_utf8(info, (u8 *)text->str, text->strlen, x, y);
                ASSERT(len <= 255);
                text->displen = len;
            }
        }
    } else if (text->format && !strcmp(text->format, "ascii")) {
        char *str;
        u32 w_sum;
        if (!text->str) {
            return 0;
        }
        if ((u8)text->str[0] == 0xff) {
            return 0;
        }

        if (dc->align == UI_ALIGN_CENTER) {
            w_sum = font_ascii_width_check(text->str);
            x += (dc->rect.width / 2 - w_sum / 2);
        } else if (dc->align == UI_ALIGN_RIGHT) {
            w_sum = font_ascii_width_check(text->str);
            x += (dc->rect.width - w_sum);
        }

        str = text->str;
        while (*str) {
            u8 *pixbuf = dc->fbuf;
            int width;
            int height;
            int color;
            font_ascii_get_pix(*str, pixbuf, dc->fbuf_len, &height, &width);
            if (dc->data_format == DC_DATA_FORMAT_OSD16) {
                color  = text->color;
            } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
                if (text->color == UI_RGB565(TEXT_MONO_INV)) {
                    color = 0x55aa;//清显示
                } else {
                    color = (text->color != UI_RGB565(TEXT_MONO_CLR)) ? (text->color ? text->color : 0xffff) : 0x55aa;
                }
            }
            __font_pix_copy(dc, dc->data_format, 0, pixbuf, &draw_r, x, y, height, width, color);
            x += width;
            str++;
        }
    } else if (text->format && !strcmp(text->format, "strpic")) {
        u16 id = (u8)text->str[0];
        u8 *pixbuf;
        int w;
        int h;

        if (id == 0xffff) {
            return 0;
        }

        if (open_string_pic(&file, id)) {
            return 0;
        }

        y += (dc->rect.height / 2 - file.height / 2);
        if (dc->align == UI_ALIGN_CENTER) {
            x += (dc->rect.width / 2 - file.width / 2);
        } else if (dc->align == UI_ALIGN_RIGHT) {
            x += (dc->rect.width - file.width);
        }

        pixbuf = dc->fbuf;
        if (!pixbuf) {
            return -ENOMEM;
        }

        disp.left   = x;
        disp.top    = y;
        disp.width  = file.width;
        disp.height = file.height;

        if (get_rect_cover(&draw_r, &disp, &r)) {
            if ((dc->data_format == DC_DATA_FORMAT_MONO) && (text->color == UI_RGB565(TEXT_MONO_INV))) {
                if (__this->api->fill_rect) {
                    __this->api->fill_rect(dc, UI_RGB565(BGC_MONO_SET));
                }
            }
            for (h = 0; h < file.height; h += 8) {
                if (file.compress == 0) {
                    int offset = (h / 8) * file.width;
                    if (br23_read_str_data(&file, pixbuf, file.width, offset) != file.width) {
                        return -EFAULT;
                    }
                } else {
                    ASSERT(0, "the compress mode not support!");
                }
                int color;
                if (dc->data_format == DC_DATA_FORMAT_OSD16) {
                    color  = text->color;
                } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
                    if (text->color == UI_RGB565(TEXT_MONO_INV)) {
                        color = 0x55aa;//清显示
                    } else {
                        color = (text->color != UI_RGB565(TEXT_MONO_CLR)) ? (text->color ? text->color : 0xffff) : 0x55aa;
                    }
                }
                __font_pix_copy(dc, dc->data_format, 0, pixbuf, &r, x, y + h / 8 * 8, 8, file.width, color);
            }
        }
    } else if (text->format && !strcmp(text->format, "image")) {
        u8 *pixelbuf;
        u8 *temp_pixelbuf;
        u8 *alphabuf;
        u8 *temp_alphabuf;
        u16 cnt = 0;
        u16 id = ((u8)text->str[1] << 8) | (u8)text->str[0];
        u32 w, h;
        int ww, hh;


        if (image_str_size_check(dc->page, text->str, &ww, &hh) != 0) {
            return -EFAULT;
        }
        if (dc->align == UI_ALIGN_CENTER) {
            x += (dc->rect.width / 2 - ww / 2);
        } else if (dc->align == UI_ALIGN_RIGHT) {
            x += (dc->rect.width - ww);
        }
        y += (dc->rect.height / 2 - hh / 2);
        while ((id != 0x00ff) && (id != 0xffff)) {
            if (open_image_by_id(&file, id, dc->page) != 0) {
                return -EFAULT;
            }

            disp.left   = x;
            disp.top    = y;
            disp.width  = file.width;
            disp.height = file.height;

            if (dc->data_format == DC_DATA_FORMAT_MONO) {
                pixelbuf = dc->fbuf;
                if (get_rect_cover(&draw_r, &disp, &r)) {
                    int _offset = -1;
                    for (h = 0; h < r.height; h++) {
                        if (file.compress == 0) {
                            int offset = (r.top + h - disp.top) / 8 * file.width + (r.left - disp.left);
                            if (_offset != offset) {
                                if (br23_read_image_data(&file, pixelbuf, r.width, offset) != r.width) {
                                    return -EFAULT;
                                }
                                _offset = offset;
                            }
                        } else {
                            ASSERT(0, "the compress mode not support!");
                        }
                        for (w = 0; w < r.width; w++) {
                            u8 color = (pixelbuf[w] & BIT((r.top + h - disp.top) % 8)) ? 1 : 0;
                            if (color) {
                                if (platform_api->draw_point) {
                                    u16 text_color;
                                    if (text->color != 0xffffff) {
                                        text_color = (text->color != UI_RGB565(TEXT_MONO_CLR)) ? (text->color ? text->color : 0xffff) : 0x55aa;
                                    } else {
                                        text_color = color;
                                    }
                                    platform_api->draw_point(dc, r.left + w, r.top + h, text_color);
                                }
                            }
                        }
                    }
                }
            } else if (dc->data_format == DC_DATA_FORMAT_OSD16) {
                int temp_pixelbuf_len = dc->width * 2 * dc->lines + 0x40 - 8;
                int temp_alphabuf_len = dc->width * dc->lines + 0x40 - 8;
                int buf_offset;

                buf_offset = 0;
                pixelbuf = &dc->fbuf[buf_offset];//2 bytes * line
                buf_offset += dc->width * 2;
                buf_offset = (buf_offset + 3) / 4 * 4;
                alphabuf = &dc->fbuf[buf_offset];//1 bytes * line
                buf_offset += dc->width;
                buf_offset = (buf_offset + 3) / 4 * 4;
                temp_pixelbuf = &dc->fbuf[buf_offset];
                buf_offset += temp_pixelbuf_len;
                buf_offset = (buf_offset + 3) / 4 * 4;
                temp_alphabuf = &dc->fbuf[buf_offset];

                u32 alpha_addr = 0;
                if (get_rect_cover(&draw_r, &disp, &r)) {
                    for (h = 0; h < r.height;) {
                        int vh = r.top + h - disp.top;
                        int vw = r.left - disp.left;

                        struct rle_line *line;
                        struct rle_line *alpha_line;
                        u8 *ptemp;
                        u8 *alpha_ptemp;

                        if (file.compress == 0) {
                            int offset = 4 + vh * file.width * 2 + vw * 2;
                            if (br23_read_image_data(&file, pixelbuf, r.width * 2, offset) != r.width * 2) {
                                return -EFAULT;
                            }
                            br23_read_image_data(&file, &alpha_addr, sizeof(alpha_addr), 0);
                            if (alpha_addr) {
                                offset = alpha_addr + vh * file.width + vw;
                                br23_read_image_data(&file, alphabuf, r.width, offset);
                            }
                        } else if (file.compress == 1) {
                            int remain = (r.height - h) > (file.height - vh) ? (file.height - vh) : (r.height - h);
                            int headlen = sizeof(struct rle_header) + (remain * 2 + 3) / 4 * 4;

                            line = (struct rle_line *)temp_pixelbuf;
                            ptemp = &temp_pixelbuf[headlen];
                            memset(line, 0x00, sizeof(struct rle_line));

                            int rle_header_len = sizeof(struct rle_header) * remain;
                            br23_read_image_data(&file, ptemp, rle_header_len, 4 + vh * sizeof(struct rle_header));

                            int i;
                            struct rle_header *rle = (struct rle_header *)ptemp;
                            int total_len = 0;
                            for (i = 0; i < remain; i++) {
                                if (i == 0) {
                                    line->addr = rle[i].addr;
                                    line->len[i] = rle[i].len;
                                } else {
                                    line->len[i] = rle[i].len;
                                }
                                if ((total_len + rle[i].len) > (temp_pixelbuf_len - headlen)) {
                                    break;
                                }
                                total_len += rle[i].len;
                                line->num ++;
                            }

                            br23_read_image_data(&file, ptemp, total_len, 4 + line->addr);

                            br23_read_image_data(&file, &alpha_addr, sizeof(alpha_addr), 0);
                            if (alpha_addr) {
                                int headlen = sizeof(struct rle_header) + (line->num * 2 + 3) / 4 * 4;
                                alpha_ptemp = &temp_alphabuf[headlen];
                                br23_read_image_data(&file, alpha_ptemp, sizeof(struct rle_header)*line->num, alpha_addr + vh * sizeof(struct rle_header));

                                struct rle_header *rle = (struct rle_header *)alpha_ptemp;
                                alpha_line = (struct rle_line *)temp_alphabuf;
                                memset(alpha_line, 0x00, sizeof(struct rle_line));
                                int total_len = 0;
                                for (i = 0; i < line->num; i++) {
                                    if (i == 0) {
                                        alpha_line->addr = rle[i].addr;
                                        alpha_line->len[i] = rle[i].len;
                                    } else {
                                        alpha_line->len[i] = rle[i].len;
                                    }
                                    if ((total_len + rle[i].len) > (temp_alphabuf_len - headlen)) {
                                        break;
                                    }
                                    total_len += rle[i].len;
                                    alpha_line->num ++;
                                }

                                br23_read_image_data(&file, alpha_ptemp, total_len, alpha_addr + alpha_line->addr);
                            }
                        } else {
                            ASSERT(0, "the compress mode not support!");
                        }

                        u8 *p0 = ptemp;
                        u8 *p1 = alpha_ptemp;
                        int line_num;
                        if (file.compress == 0) {
                            line_num = 1;
                        } else {
                            line_num = (line->num > alpha_line->num) ? alpha_line->num : line->num;
                        }

                        for (hh = 0; hh < line_num; hh++, h++) {
                            if (file.compress == 1) {
                                Rle_Decode(p0, line->len[hh], pixelbuf, file.width * 2, vw * 2, r.width * 2);
                                p0 += line->len[hh];
                                Rle_Decode(p1, alpha_line->len[hh], alphabuf, file.width, vw, r.width);
                                p1 += alpha_line->len[hh];
                            }
                            u16 *pdisp = (u16 *)dc->buf;
                            u16 *pixelbuf16 = (u16 *)pixelbuf;
                            for (w = 0; w < r.width; w++) {
                                u16 color, pixel;
                                u8  alpha = alpha_addr ? alphabuf[w] : 255;

                                pixel = color = pixelbuf16[w];
                                if (color) {
                                    if (platform_api->draw_point) {
                                        int vww = w;
                                        u16 x0 = r.left + vww;
                                        u16 y0 = r.top + h;

                                        if (alpha < 255) {
                                            u16 backcolor = platform_api->read_point(dc, x0, y0);
                                            pixel = get_mixed_pixel(backcolor >> 8 | (backcolor & 0xff) << 8, color>>8 | (color & 0xff) << 8, alpha);
                                        }

                                        int offset = (y0 - dc->disp.top) * dc->disp.width + (x0 - dc->disp.left);
                                        if ((offset * 2 + 1) < dc->len) {
                                            pdisp[offset] = pixel;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            x += file.width;
            cnt += 2;
            id = ((u8)text->str[cnt + 1] << 8) | (u8)text->str[cnt];
        }
    }

    return 0;
}
AT_UI_RAM
u32 br23_read_point(struct draw_context *dc, u16 x, u16 y)
{
    u32 pixel;
    u16 *pdisp = dc->buf;
    if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        int offset = (y - dc->disp.top) * dc->disp.width + (x - dc->disp.left);
        ASSERT((offset * 2 + 1) < dc->len, "dc->len:%d", dc->len);
        if ((offset * 2 + 1) >= dc->len) {
            return -1;
        }

        pixel = pdisp[offset];//(dc->buf[offset * 2] << 8) | dc->buf[offset * 2 + 1];
    } else {
        ASSERT(0);
    }

    return pixel;
}

__attribute__((always_inline_when_const_args))
AT_UI_RAM
int br23_draw_point(struct draw_context *dc, u16 x, u16 y, u32 pixel)
{
    if (dc->data_format == DC_DATA_FORMAT_OSD16) {
        int offset = (y - dc->disp.top) * dc->disp.width + (x - dc->disp.left);

        /* ASSERT((offset * 2 + 1) < dc->len, "dc->len:%d", dc->len); */
        if ((offset * 2 + 1) >= dc->len) {
            return -1;
        }

        dc->buf[offset * 2    ] = pixel >> 8;
        dc->buf[offset * 2 + 1] = pixel;
    } else if (dc->data_format == DC_DATA_FORMAT_MONO) {
        /* ASSERT(x < __this->info.width); */
        /* ASSERT(y < __this->info.height); */
        if ((x >= __this->info.width) || (y >= __this->info.height)) {
            return -1;
        }

        if (pixel & BIT(31)) {
            dc->buf[y / 8 * __this->info.width + x] ^= BIT(y % 8);
        } else if (pixel == 0x55aa) {
            dc->buf[y / 8 * __this->info.width + x] &= ~BIT(y % 8);
        } else if (pixel) {
            dc->buf[y / 8 * __this->info.width + x] |= BIT(y % 8);
        } else {
            dc->buf[y / 8 * __this->info.width + x] &= ~BIT(y % 8);
        }
    }

    return 0;
}

int br23_open_device(struct draw_context *dc, const char *device)
{
    return 0;
}

int br23_close_device(int fd)
{
    return 0;
}

static const struct ui_platform_api br23_platform_api = {
    .malloc             = br23_malloc,
    .free               = br23_free,

    .load_style         = br23_load_style,
    .load_window        = br23_load_window,
    .unload_window      = br23_unload_window,

    .open_draw_context  = br23_open_draw_context,
    .get_draw_context   = br23_get_draw_context,
    .put_draw_context   = br23_put_draw_context,
    .set_draw_context   = br23_set_draw_context,
    .close_draw_context = br23_close_draw_context,

    .load_widget_info   = br23_load_widget_info,
    .load_css           = br23_load_css,
    .load_image_list    = br23_load_image_list,
    .load_text_list     = br23_load_text_list,

    .fill_rect          = br23_fill_rect,
    .draw_rect          = br23_draw_rect,
    .draw_image         = br23_draw_image,
    .show_text          = br23_show_text,
    .read_point         = br23_read_point,
    .draw_point         = br23_draw_point,
    .invert_rect        = br23_invert_rect,

    .read_image_info    = br23_read_image_info,

    .open_device        = br23_open_device,
    .close_device       = br23_close_device,

    .set_timer          = br23_set_timer,
    .del_timer          = br23_del_timer,

    .file_browser_open  = NULL,
    .get_file_attrs     = NULL,
    .set_file_attrs     = NULL,
    .show_file_preview  = NULL,
    .move_file_preview  = NULL,
    .clear_file_preview = NULL,
    .flush_file_preview = NULL,
    .open_file          = NULL,
    .delete_file        = NULL,
    .file_browser_close = NULL,
};




static int open_resource_file()
{
    int ret;

    printf("open_resouece_file...\n");

    ret = open_resfile(RES_PATH"menu.res");
    if (ret) {
        return -EINVAL;
    }
    ret = open_str_file(RES_PATH"str.res");
    if (ret) {
        return -EINVAL;
    }
    ret = font_ascii_init(RES_PATH"ascii.res");
    if (ret) {
        return -EINVAL;
    }
    return 0;
}

int __attribute__((weak)) lcd_get_scrennifo(struct fb_var_screeninfo *info)
{
    info->s_xoffset = 0;
    info->s_yoffset = 0;
    info->s_xres = 240;
    info->s_yres = 240;

    return 0;
}

int ui_platform_init(void *lcd)
{
    struct rect rect;
    struct lcd_info info = {0};

#ifdef UI_BUF_CALC
    INIT_LIST_HEAD(&buffer_used.list);
#endif

    __this->api = &br23_platform_api;
    ASSERT(__this->api->open_draw_context);
    ASSERT(__this->api->get_draw_context);
    ASSERT(__this->api->put_draw_context);
    ASSERT(__this->api->set_draw_context);
    ASSERT(__this->api->close_draw_context);


    __this->lcd = lcd_get_hdl();
    ASSERT(__this->lcd);
    ASSERT(__this->lcd->init);
    ASSERT(__this->lcd->get_screen_info);
    ASSERT(__this->lcd->buffer_malloc);
    ASSERT(__this->lcd->buffer_free);
    ASSERT(__this->lcd->draw);
    ASSERT(__this->lcd->set_draw_area);

    if (__this->lcd->init) {
        __this->lcd->init(lcd);
    }

    if (__this->lcd->backlight_ctrl) {
        __this->lcd->backlight_ctrl(true);
    }

    if (__this->lcd->get_screen_info) {
        __this->lcd->get_screen_info(&info);
    }
    rect.left   = 0;
    rect.top    = 0;
    rect.width  = info.width;
    rect.height = info.height;

    printf("ui_platform_init :: [%d,%d,%d,%d]\n", rect.left, rect.top, rect.width, rect.height);

    ui_core_init(__this->api, &rect);

    return 0;
}



int ui_style_file_version_compare(int version)
{
    int v;
    int len;
    struct ui_file_head head;
    static u8 checked = 0;

    if (checked == 0) {
        if (!ui_file) {
            puts("ui version_compare ui_file null!\n");
            ASSERT(0);
            return 0;
        }
        res_fseek(ui_file, 0, SEEK_SET);
        len = sizeof(struct ui_file_head);
        res_fread(ui_file, &head, len);
        printf("style file version is: 0x%x,UI_VERSION is: 0x%x\n", *(u32 *)(head.res), version);
        if (*(u32 *)head.res != version) {
            puts("style file version is not the same as UI_VERSION !!\n");
            ASSERT(0);
        }
        checked = 1;
    }
    return 0;
}


#endif
