#ifndef __STREAM_ENTRY_H__
#define __STREAM_ENTRY_H__

#include "system/includes.h"
#include "media/includes.h"

// struct __stream_entry;
struct __stream_entry {
    void *data_priv;
    int (*data_callback)(void *priv, s16 *data, u16 len);
    struct audio_stream_entry entry;
};

struct __stream_entry *stream_entry_open(void *priv, int (*data_callback)(void *priv, s16 *data, u16 len));
void stream_entry_close(struct __stream_entry **hdl);
void stream_entry_resume(struct __stream_entry *hdl);

#endif// __STREAM_ENTRY_H__

