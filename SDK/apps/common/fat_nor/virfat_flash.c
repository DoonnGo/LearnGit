#include "system/includes.h"
/* #include "jlfat/tff.h" */
#include "app_config.h"
#include "fs/virfat_flash.h"
/* #include "fs/fs.h" */


void  fat_copy(void *pDest, void *pSrc, u32 copyLen)
{
    while (copyLen--) {
        ((u8 *)pDest)[copyLen] = ((u8 *)pSrc)[copyLen];
    }
}

static u16 ld_word(u8 *p)
{
    return (u16)p[1] << 8 | p[0];
}

static u32 ld_dword(u8 *p)
{
    return (u32)p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0];
}

//fat16
static const unsigned char bpb_data[512] = {
    0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x08, 0x08, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x20, 0xF8, 0x08, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0xEE, 0x8D, 0xF2, 0x2C, 0x4E, 0x4F, 0x20, 0x4E, 0x41,
    0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x36, 0x20, 0x20, 0x20, 0x33, 0xC9,
    0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E, 0xD9, 0xB8, 0x00, 0x20, 0x8E, 0xC0, 0xFC, 0xBD, 0x00, 0x7C,
    0x38, 0x4E, 0x24, 0x7D, 0x24, 0x8B, 0xC1, 0x99, 0xE8, 0x3C, 0x01, 0x72, 0x1C, 0x83, 0xEB, 0x3A,
    0x66, 0xA1, 0x1C, 0x7C, 0x26, 0x66, 0x3B, 0x07, 0x26, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA,
    0x02, 0x88, 0x56, 0x02, 0x80, 0xC3, 0x10, 0x73, 0xEB, 0x33, 0xC9, 0x8A, 0x46, 0x10, 0x98, 0xF7,
    0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E, 0x03, 0x46, 0x0E, 0x13, 0xD1, 0x8B, 0x76, 0x11,
    0x60, 0x89, 0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8, 0x20, 0x00, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 0x03,
    0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11, 0x4E, 0xFE, 0x61, 0xBF, 0x00, 0x00, 0xE8, 0xE6,
    0x00, 0x72, 0x39, 0x26, 0x38, 0x2D, 0x74, 0x17, 0x60, 0xB1, 0x0B, 0xBE, 0xA1, 0x7D, 0xF3, 0xA6,
    0x61, 0x74, 0x32, 0x4E, 0x74, 0x09, 0x83, 0xC7, 0x20, 0x3B, 0xFB, 0x72, 0xE6, 0xEB, 0xDC, 0xA0,
    0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98, 0x40, 0x74, 0x0C, 0x48, 0x74, 0x13, 0xB4, 0x0E,
    0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0, 0xFD, 0x7D, 0xEB, 0xE6, 0xA0, 0xFC, 0x7D, 0xEB,
    0xE1, 0xCD, 0x16, 0xCD, 0x19, 0x26, 0x8B, 0x55, 0x1A, 0x52, 0xB0, 0x01, 0xBB, 0x00, 0x00, 0xE8,
    0x3B, 0x00, 0x72, 0xE8, 0x5B, 0x8A, 0x56, 0x24, 0xBE, 0x0B, 0x7C, 0x8B, 0xFC, 0xC7, 0x46, 0xF0,
    0x3D, 0x7D, 0xC7, 0x46, 0xF4, 0x29, 0x7D, 0x8C, 0xD9, 0x89, 0x4E, 0xF2, 0x89, 0x4E, 0xF6, 0xC6,
    0x06, 0x96, 0x7D, 0xCB, 0xEA, 0x03, 0x00, 0x00, 0x20, 0x0F, 0xB6, 0xC8, 0x66, 0x8B, 0x46, 0xF8,
    0x66, 0x03, 0x46, 0x1C, 0x66, 0x8B, 0xD0, 0x66, 0xC1, 0xEA, 0x10, 0xEB, 0x5E, 0x0F, 0xB6, 0xC8,
    0x4A, 0x4A, 0x8A, 0x46, 0x0D, 0x32, 0xE4, 0xF7, 0xE2, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xEB,
    0x4A, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A, 0x10, 0x91, 0x8B, 0x46, 0x18, 0x96, 0x92, 0x33,
    0xD2, 0xF7, 0xF6, 0x91, 0xF7, 0xF6, 0x42, 0x87, 0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8,
    0xC0, 0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42,
    0x8B, 0xF4, 0x8A, 0x56, 0x24, 0xCD, 0x13, 0x61, 0x61, 0x72, 0x0B, 0x40, 0x75, 0x01, 0x42, 0x03,
    0x5E, 0x0B, 0x49, 0x75, 0x06, 0xF8, 0xC3, 0x41, 0xBB, 0x00, 0x00, 0x60, 0x66, 0x6A, 0x00, 0xEB,
    0xB0, 0x42, 0x4F, 0x4F, 0x54, 0x4D, 0x47, 0x52, 0x20, 0x20, 0x20, 0x20, 0x0D, 0x0A, 0x52, 0x65,
    0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x73, 0x20, 0x6F, 0x72, 0x20, 0x6F, 0x74,
    0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x2E, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73,
    0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20,
    0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61,
    0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCB, 0xD8, 0x55, 0xAA
};

#if 0
static const unsigned char bpb_data[512] = {
    0xEB, 0x58, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x08, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xF0, 0x3A, 0x00, 0x5B, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x29, 0x5C, 0xA2, 0x9B, 0x7A, 0x4E, 0x4F, 0x20, 0x4E, 0x41, 0x4D, 0x45, 0x20, 0x20,
    0x20, 0x20, 0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20, 0x33, 0xC9, 0x8E, 0xD1, 0xBC, 0xF4,
    0x7B, 0x8E, 0xC1, 0x8E, 0xD9, 0xBD, 0x00, 0x7C, 0x88, 0x56, 0x40, 0x88, 0x4E, 0x02, 0x8A, 0x56,
    0x40, 0xB4, 0x41, 0xBB, 0xAA, 0x55, 0xCD, 0x13, 0x72, 0x10, 0x81, 0xFB, 0x55, 0xAA, 0x75, 0x0A,
    0xF6, 0xC1, 0x01, 0x74, 0x05, 0xFE, 0x46, 0x02, 0xEB, 0x2D, 0x8A, 0x56, 0x40, 0xB4, 0x08, 0xCD,
    0x13, 0x73, 0x05, 0xB9, 0xFF, 0xFF, 0x8A, 0xF1, 0x66, 0x0F, 0xB6, 0xC6, 0x40, 0x66, 0x0F, 0xB6,
    0xD1, 0x80, 0xE2, 0x3F, 0xF7, 0xE2, 0x86, 0xCD, 0xC0, 0xED, 0x06, 0x41, 0x66, 0x0F, 0xB7, 0xC9,
    0x66, 0xF7, 0xE1, 0x66, 0x89, 0x46, 0xF8, 0x83, 0x7E, 0x16, 0x00, 0x75, 0x39, 0x83, 0x7E, 0x2A,
    0x00, 0x77, 0x33, 0x66, 0x8B, 0x46, 0x1C, 0x66, 0x83, 0xC0, 0x0C, 0xBB, 0x00, 0x80, 0xB9, 0x01,
    0x00, 0xE8, 0x2C, 0x00, 0xE9, 0xA8, 0x03, 0xA1, 0xF8, 0x7D, 0x80, 0xC4, 0x7C, 0x8B, 0xF0, 0xAC,
    0x84, 0xC0, 0x74, 0x17, 0x3C, 0xFF, 0x74, 0x09, 0xB4, 0x0E, 0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB,
    0xEE, 0xA1, 0xFA, 0x7D, 0xEB, 0xE4, 0xA1, 0x7D, 0x80, 0xEB, 0xDF, 0x98, 0xCD, 0x16, 0xCD, 0x19,
    0x66, 0x60, 0x80, 0x7E, 0x02, 0x00, 0x0F, 0x84, 0x20, 0x00, 0x66, 0x6A, 0x00, 0x66, 0x50, 0x06,
    0x53, 0x66, 0x68, 0x10, 0x00, 0x01, 0x00, 0xB4, 0x42, 0x8A, 0x56, 0x40, 0x8B, 0xF4, 0xCD, 0x13,
    0x66, 0x58, 0x66, 0x58, 0x66, 0x58, 0x66, 0x58, 0xEB, 0x33, 0x66, 0x3B, 0x46, 0xF8, 0x72, 0x03,
    0xF9, 0xEB, 0x2A, 0x66, 0x33, 0xD2, 0x66, 0x0F, 0xB7, 0x4E, 0x18, 0x66, 0xF7, 0xF1, 0xFE, 0xC2,
    0x8A, 0xCA, 0x66, 0x8B, 0xD0, 0x66, 0xC1, 0xEA, 0x10, 0xF7, 0x76, 0x1A, 0x86, 0xD6, 0x8A, 0x56,
    0x40, 0x8A, 0xE8, 0xC0, 0xE4, 0x06, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0xCD, 0x13, 0x66, 0x61, 0x0F,
    0x82, 0x74, 0xFF, 0x81, 0xC3, 0x00, 0x02, 0x66, 0x40, 0x49, 0x75, 0x94, 0xC3, 0x42, 0x4F, 0x4F,
    0x54, 0x4D, 0x47, 0x52, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A, 0x44, 0x69,
    0x73, 0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73,
    0x20, 0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74,
    0x61, 0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0x01, 0xB9, 0x01, 0x00, 0x00, 0x55, 0xAA
};
#endif

#define VIR_FILE_SIZE (1024*1024*300)
static dir_entry_t dir_entry[] = {
    /* {"BT-MUSICWAV", VIR_FILE_SIZE}, */

    {"BT_DISK1MP3", VIR_FILE_SIZE},

    /* {"BT-MUSICWAV", VIR_FILE_SIZE}, */

    {"BT_DISK2MP3", VIR_FILE_SIZE},

    /* {"BT-MUSICWAV", VIR_FILE_SIZE}, */

    {"BT_DISK3MP3", VIR_FILE_SIZE},
};

static virfat_flash_t virfat_flash;
static u8 curr_file = 0xff;
volatile char vir_file_type = 0;
u32 change_flag = 0;
static void *fd;



static u32 get_first_one(u32 n)
{
    u32 pos = 0;
    for (pos = 0; pos < 32; pos++) {
        if (n & BIT(pos)) {
            return pos;
        }
    }
    return 0xff;

}

static bool virfat_flash_mount(virfat_flash_t *disk_handler, u8 *buf)
{
    u32 fatsize;
    u32 maxclust;
    u32 sects_fat;
    u32 n_rootdir;

    /*??????FAT?????????*/
    fatsize = ld_word(&buf[BPB_FATSz16]);										/* Number of sectors per FAT */
    if (!fatsize) {
        fatsize = ld_dword(&buf[BPB_FATSz32]);
    }

    disk_handler->fatsize = fatsize * 512; //fat????????? byte.
    disk_handler->fatbase		=	ld_word(&buf[BPB_RsvdSecCnt]);							/* FAT start sector (lba) */
    /* disk_handler->fatbase2      = disk_handler->fatbase + fatsize; */

    fatsize *= buf[BPB_NumFATs];

    disk_handler->csize		=	get_first_one(buf[BPB_SecPerClus]);									/* Number of sectors per cluster */

    n_rootdir	=	ld_word(&buf[BPB_RootEntCnt]);										/* Nmuber of root directory entries */

    disk_handler->database		=	(disk_handler->fatbase + fatsize + n_rootdir / 16);	/* Data start sector (lba) */


    /*???????????????*/
    disk_handler->capacity = ld_word(&buf[BPB_TotSec16]);														/* Number of sectors on the file system */
    if (!disk_handler->capacity) {
        disk_handler->capacity = ld_dword(&buf[BPB_TotSec32]);
    }
    /* r_printf(">>>[test]:cap = 0x%x, csize = %d\n",disk_handler->capacity, disk_handler->csize); */
    maxclust = (disk_handler->capacity - ld_word(&buf[BPB_RsvdSecCnt]) - fatsize -
                n_rootdir / 16) >> disk_handler->csize;
    disk_handler->max_clust = maxclust + 2;
    /* r_printf(">>>[test]:maxclust = %d\n", maxclust); */

    disk_handler->fs_type = (maxclust >= MIN_FAT32) ? FS_FAT32 : ((maxclust >= MIN_FAT16) ? FS_FAT16 : FS_FAT12);
    /* r_printf(">>>[test]:fs_type = 0x%x\n", disk_handler->fs_type); */

    /* Root directory start cluster */
    if (disk_handler->fs_type == FS_FAT32) {
        disk_handler->dirbase = disk_handler->database;
        disk_handler->database += 1 << disk_handler->csize;
    } else {
        disk_handler->dirbase = disk_handler->fatbase + fatsize;
    }

    disk_handler->clustSize = disk_handler->csize + 9;

    return TRUE;
}

static void vir_read_reserved_area(u8 *buf)
{
    memset(buf, 0, 512);
}


static void vir_read_BPB(u8 *buf)
{
    /* u8 bpb_jl_head[] = "JLBT_DISK"; */

    memset(buf, 0, 512);
    memcpy(buf, bpb_data, sizeof(bpb_data));

    /* memcpy(buf + sizeof(bpb_data), bpb_jl_head, sizeof(bpb_jl_head)); */

    /* buf[510] = 0x55; */
    /* buf[511] = 0xAA; */
}

static u32 virfat_write(void *buf, u32 addr, u32 len)
{
    u32 wlen;
    static u32 clen = 0;
    if ((clen % 4096) == 0) {
        dev_ioctl(fd, IOCTL_ERASE_SECTOR, addr);
        clen = 0;
    }
    wlen = dev_bulk_write(fd, buf, addr, len);
    clen += len;
    return wlen;
}

static u32 virfat_flash_read(void *buf, u32 addr_sec, u32 len)
{
    /* y_printf("[r:%d],,", addr_sec); */
    u32 real_addrsec = 0;
    u32 rlen;
    if (addr_sec == 0) {
        vir_read_BPB(buf);
        y_printf("fatbase:%d \n", virfat_flash.fatbase);
        y_printf("dirbase:%d \n", virfat_flash.dirbase);
        y_printf("database:%d \n", virfat_flash.database);
        return len;
    } else if (addr_sec < virfat_flash.fatbase) {
        vir_read_reserved_area(buf);
        return len;
    } else if (addr_sec < virfat_flash.dirbase) {
        real_addrsec = addr_sec - virfat_flash.fatbase + 0; // 0???flash??????fat??????????????????
    } else if (addr_sec < virfat_flash.database) {
        real_addrsec = addr_sec - virfat_flash.dirbase + (4 * 1024) / 512; // 4K???flash??????????????????????????????
    } else {
        real_addrsec = addr_sec - virfat_flash.database + 8 * 1024 / 512; // 8K???flash??????????????????????????????
    }
    /* r_printf(">>>[test]:real-add= %d\n",real_addrsec); */
    rlen = dev_bulk_read(fd, buf, real_addrsec * 512, len);
    if (rlen != len) {
        log_e("read error!!!!!!!!!!!!!");
    }
    return rlen;
}


static u32 virfat_flash_write(void *buf, u32 addr_sec, u32 len)
{
    /* r_printf("[w:%d],,", addr); */
    u8 *buf_temp = (u8 *)malloc(4096);
    u32 wlen;
    u32 real_addrsec = 0;
    if (addr_sec == 0) {
        return len;
    } else if (addr_sec < virfat_flash.fatbase) {
        return len;
    } else if (addr_sec < virfat_flash.dirbase) {
        real_addrsec = addr_sec - virfat_flash.fatbase + 0; // 0???flash??????fat??????????????????
        dev_bulk_read(fd, buf_temp, real_addrsec * 512, 4096);
        dev_ioctl(fd, IOCTL_ERASE_SECTOR, real_addrsec * 512);
        memcpy(buf_temp, buf, len);
        wlen = dev_bulk_write(fd, buf_temp, real_addrsec * 512, 4096);
    } else if (addr_sec < virfat_flash.database) {
        real_addrsec = addr_sec - virfat_flash.dirbase + 4 * 1024 / 512; // 4K???flash??????????????????????????????
        dev_bulk_read(fd, buf_temp, real_addrsec * 512, 4096);
        dev_ioctl(fd, IOCTL_ERASE_SECTOR, real_addrsec * 512);
        memcpy(buf_temp, buf, len);
        wlen = dev_bulk_write(fd, buf_temp, real_addrsec * 512, 4096);
    } else {
        real_addrsec = addr_sec - virfat_flash.database + 8 * 1024 / 512; // 8K???flash??????????????????????????????
        wlen = virfat_write(buf, real_addrsec * 512, len);
        /* wlen =dev_bulk_write(fd, buf, real_addrsec * 512, len); */
    }
    free(buf_temp);
    buf_temp = NULL;
    if (wlen != len) {
        log_e("read error!!!!!!!!!!!!!");
    }
    return wlen;
}

u32 virfat_flash_erase(int cmd, u32 arg)
{
    return dev_ioctl(fd, cmd, arg);
}
static u32 virfat_flash_capacity()
{
    return virfat_flash.capacity;
}

static u32 virfat_flash_available()
{
    return 1;
}


u8 virfat_flash_init(void *buf_512)
{
    vir_read_BPB(buf_512);

    virfat_flash_mount(&virfat_flash, buf_512);


    /* y_printf("fatbase:%d \n", virfat_flash.fatbase); */
    /* y_printf("dirbase:%d \n", virfat_flash.dirbase); */
    /* y_printf("database:%d \n", virfat_flash.database); */
    /* y_printf("clustSize:%d \n", virfat_flash.clustSize); */

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////device api//////////////////////////////////////////////////////////

static struct device virfat_flash_dev;
static int flash_virfat_init(const struct dev_node *node, void *arg)
{
    u8 buf[512] = {0};
    memcpy(&virfat_flash.re_devname, arg, strlen(arg));
    /* r_printf(">>>[test]:init name  = %s\n", virfat_flash.re_devname); */
    virfat_flash_init(buf);
    return 0;
}

static int flash_virfat_open(const char *name, struct device **device, void *arg)
{
    /* y_printf(">>>[test]:dev_name = %s\n", virfat_flash.re_devname); */
    fd = dev_open((const char *)&virfat_flash.re_devname, arg);
    *device = &virfat_flash_dev;
    return 0;
}
int flash_virfat_read(struct device *device, void *buf, u32 len, u32 addr)
{
    /* y_printf(">>>[test]:r???addr = %d ,len = %d\n", addr, len); */
    void *_buf = buf;
    u32 _len = len;
    u32 _lba = addr;
    while (_len) {
        virfat_flash_read(_buf, _lba, 512);
        //printf_buf(_buf, 512);
        _len -= 1;
        _lba += 1;
        _buf += 512;
    }
    return len;

    /* return virfat_flash_read(buf, addr, len); */
}
static int flash_virfat_write(struct device *device, void *buf, u32 len, u32 addr)
{

    /* r_printf(">>>[test]:w???addr = %d ,len = %d\n", addr, len); */
    /* return virfat_flash_write(buf, addr, len); */
    void *_buf = buf;
    u32 _len = len;
    u32 _lba = addr;
    while (_len) {
        virfat_flash_write(_buf, _lba, 512);
        //printf_buf(_buf, 512);
        _len -= 1;
        _lba += 1;
        _buf += 512;
    }
    return len;

}

static bool flash_virfat_online(const struct dev_node *node)
{
    return 1;
}

static int flash_virfat_close(struct device *device)
{
    dev_close(device);
    return 0;
}
static int flash_virfat_ioctrl(struct device *device, u32 cmd, u32 arg)
{
    int err = 0;
    switch (cmd) {
    case IOCTL_GET_STATUS:
        *(u32 *)arg = 1;
        break;
    case IOCTL_GET_ID:
        dev_ioctl(device, IOCTL_GET_ID, arg);
        /* *((u32 *)arg) = 0x12345678; */
        break;
    case IOCTL_GET_CAPACITY:
        *((u32 *)arg) = virfat_flash_capacity();
        break;
    case IOCTL_GET_BLOCK_SIZE:
        *((u32 *)arg) = 512;
        break;
    case IOCTL_FLUSH:
        break;
    case IOCTL_CMD_RESUME:
        break;
    case IOCTL_CMD_SUSPEND:
        break;
    case IOCTL_SET_ASYNC_MODE:
        break;
    default:
        err = -EINVAL;
        break;
    }
    return err;
}

const struct device_operations virfat_flash_dev_ops = {
    .init = flash_virfat_init,
    .online = flash_virfat_online,
    .open = flash_virfat_open,
    .read = flash_virfat_read,
    .write = flash_virfat_write,
    .ioctl = flash_virfat_ioctrl,
    .close = flash_virfat_close,
};

