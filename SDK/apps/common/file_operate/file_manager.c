#include "file_manager.h"
#include "app_config.h"
#include "clock_cfg.h"

FILE *file_manager_select(struct vfscan *fs, int sel_mode, int arg)
{
    FILE *_file = NULL;
    clock_add_set(SCAN_DISK_CLK);
    _file = fselect(fs, sel_mode, arg);
    clock_remove_set(SCAN_DISK_CLK);
    return _file;
}


