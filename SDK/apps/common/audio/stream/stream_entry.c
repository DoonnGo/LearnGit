#include "stream_entry.h"


static int stream_entry_data_handler(struct audio_stream_entry *entry,
                                     struct audio_data_frame *in,
                                     struct audio_data_frame *out)
{
    struct __stream_entry *hdl = container_of(entry, struct __stream_entry, entry);
    if (in->data_len == 0) {
        return 0;
    }

    if (hdl->data_callback) {
        return hdl->data_callback(hdl->data_priv, in->data, in->data_len);
    }

    return in->data_len;
}

static void stream_entry_output_data_process_len(struct audio_stream_entry *entry,  int len)
{
}

struct __stream_entry *stream_entry_open(void *priv, int (*data_callback)(void *priv, s16 *data, u16 len))
{
    struct __stream_entry *hdl;
    hdl = zalloc(sizeof(struct __stream_entry));
    if (hdl == NULL) {
        return NULL;
    }
    hdl->data_priv = priv;
    hdl->data_callback = data_callback;
    hdl->entry.data_process_len = stream_entry_output_data_process_len;
    hdl->entry.data_handler = stream_entry_data_handler;

    return hdl;
}

void stream_entry_close(struct __stream_entry **hdl)
{
    if (hdl && (*hdl)) {
        struct __stream_entry *stream = *hdl;
        audio_stream_del_entry(&stream->entry);
        local_irq_disable();
        free(stream);
        *hdl = NULL;
        local_irq_enable();
    }
}

void stream_entry_resume(struct __stream_entry *hdl)
{
    if (hdl) {
        audio_stream_resume(&hdl->entry);
    }
}




