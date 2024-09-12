#include "externals/sndbridge/sourcehandle.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch/arch.h>
#include <kos/fs.h>
#include <kos/mutex.h>
#include <kos/thread.h>

#include "condevt.h"
#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"


#ifndef _arch_dreamcast
#error "BufferedFileHandle is only available in the dreamcast"
#endif


#define BUFFERED_FILEHANDLE_THREADED_INSTANCES 5     // Note: fs_iso9660 has an hard-limit of 8
#define BUFFERED_FILEHANDLE_BUFFER_SIZE (128 * 1024) // 128KiB
#define BUFFERED_FILEHANDLE_THRESHOLD (32 * 1024)    // 32KiB
#define BUFFERED_FILEHANDLE_SEEK_REFILL (8 * 1024)   // 8KiB
#define BUFFERED_FILEHANDLE_WAIT_TIME 500            // 500ms
#define BUFFERED_FILEHANDLE_WAIT_TIME_SHORT 50       // 50ms

typedef struct BufferedFileHandle BufferedFileHandle;

// note: pointers here are volatile, not the array itself
static BufferedFileHandle* volatile buffered_filehandles[BUFFERED_FILEHANDLE_THREADED_INSTANCES] = {0};
static mutex_t buffered_filehandles_mutex = MUTEX_INITIALIZER;
static condevt_t buffered_filehandles_condevt = CONDENVT_INITIALIZER;
static kthread_t* buffered_filehandles_thread = NULL;

static void filehandle_buffered_refill(BufferedFileHandle* hnd, ssize_t maximum);

typedef struct BufferedFileHandle {
    SourceHandle handle;
    char* native_path;
    off_t resume_offset;
    volatile file_t file_hnd;
    off_t file_offset;
    size_t file_length;
    volatile bool eof_reached;
    volatile size_t buffer_used;
    volatile size_t buffer_offset;
    mutex_t mutex;
    uint8_t buffer[BUFFERED_FILEHANDLE_BUFFER_SIZE];
} BufferedFileHandle;
static void BufferedFileHandle_Resume(BufferedFileHandle* buffered_handle) {
    if (buffered_handle->file_hnd == FILEHND_INVALID) {
        buffered_handle->file_hnd = fs_open(buffered_handle->native_path, O_RDONLY);
        if (buffered_handle->file_hnd != FILEHND_INVALID) {
            fs_seek(buffered_handle->file_hnd, buffered_handle->resume_offset, SEEK_SET);
        }
    }
}
static int32_t BufferedFileHandle_Read(SourceHandle* handle, void* output, int32_t request_count) {
    if (request_count == 0) return 0;
    if (!output || request_count < 0) return -1;

    BufferedFileHandle* buffered_handle = (BufferedFileHandle*)handle;

    mutex_lock(&buffered_handle->mutex);

    BufferedFileHandle_Resume(buffered_handle);

    size_t buffer_used = buffered_handle->buffer_used;
    bool eof_reached = buffered_handle->eof_reached;
    if (buffer_used < 1 && eof_reached) {
        mutex_unlock(&buffered_handle->mutex);
        return 0;
    }

    ssize_t readed = 0;
    uint8_t* ptr = output;
    size_t buffer_offset = buffered_handle->buffer_offset;

    if (buffer_used > 0) {
        // copy from internal buffer
        size_t amount = buffer_used < request_count ? buffer_used : (size_t)request_count;
        memcpy(ptr, &buffered_handle->buffer[buffer_offset], amount);

        ptr += amount;
        buffer_offset += amount;
        buffer_used -= amount;
        request_count -= (int32_t)amount;
        readed += (ssize_t)amount;

        if (buffer_used < 1) {
            buffer_offset = buffer_used = 0;
        }
    }

    if (request_count > 0 && !eof_reached) {
        // not enough data in the internal buffer, read directly from the file
        file_t file_hnd = buffered_handle->file_hnd;
        ssize_t ret;

        while ((ret = fs_read(file_hnd, ptr, (size_t)request_count)) > 0) {
            readed += ret;
            ptr += ret;
            request_count -= ret;
            if (request_count < 1) break;
        }

        if (ret == 0) {
            // avoid refilling the buffer, end-of-file reached
            buffered_handle->eof_reached = true;
        }
    }

    buffered_handle->buffer_offset = buffer_offset;
    buffered_handle->buffer_used = buffer_used;
    buffered_handle->file_offset += readed;

    mutex_unlock(&buffered_handle->mutex);

    return (int32_t)readed;
}
static int32_t BufferedFileHandle_Seek(SourceHandle* handle, int64_t offset, int whence) {
    BufferedFileHandle* buffered_handle = (BufferedFileHandle*)handle;

    mutex_lock(&buffered_handle->mutex);

    BufferedFileHandle_Resume(buffered_handle);

    file_t file_hnd = buffered_handle->file_hnd;
    off_t file_offset = buffered_handle->file_offset;
    size_t file_length = buffered_handle->file_length;
    off_t ret = -1;
    off_t new_file_offset = -1;

    switch (whence) {
        case SEEK_SET:
            if (offset < 0 || offset > file_length) {
                errno = EINVAL;
                goto L_return;
            }
            new_file_offset = (off_t)offset;
            break;
        case SEEK_CUR:
            if (offset < 0 && ((uint32_t)-offset) > file_offset) {
                errno = EINVAL;
                goto L_return;
            }
            new_file_offset = file_offset + (off_t)offset;
            break;
        case SEEK_END:
            if (offset < 0 && ((uint32)-offset) > file_length) {
                errno = EINVAL;
                goto L_return;
            }
            new_file_offset = (off_t)offset + (off_t)file_length;
            break;
    }

    if (new_file_offset == file_offset) {
        // nothing to do
        ret = 0;
        goto L_return;
    }

    ret = fs_seek(file_hnd, new_file_offset, SEEK_SET);

    // discard internal buffer data, require various checks to seek buffered data
    buffered_handle->buffer_offset = 0;
    buffered_handle->buffer_used = 0;
    buffered_handle->file_offset = ret;

#ifdef DEBUG
    assert(ret == new_file_offset);
#endif

    // check if eof was reached ¿is this step optional?
    bool eof_reached = file_length == (size_t)-1 ? false : (ret >= file_length);
    buffered_handle->eof_reached = eof_reached;

    if (ret >= 0 && !eof_reached) {
        filehandle_buffered_refill(buffered_handle, BUFFERED_FILEHANDLE_SEEK_REFILL);
    }

L_return:
    mutex_unlock(&buffered_handle->mutex);
    return ret < 0 ? (int32_t)ret : 0;
}
static int64_t BufferedFileHandle_Tell(SourceHandle* handle) {
    BufferedFileHandle* buffered_handle = (BufferedFileHandle*)handle;
    return (int64_t)buffered_handle->file_offset;
}
static int64_t BufferedFileHandle_Length(SourceHandle* handle) {
    BufferedFileHandle* buffered_handle = (BufferedFileHandle*)handle;

    return (int64_t)buffered_handle->file_length;
}
static void BufferedFileHandle_Destroy(SourceHandle* handle) {
    if (!handle) return;
    BufferedFileHandle* buffered_handle = (BufferedFileHandle*)handle;

    mutex_lock(&buffered_filehandles_mutex);
    for (size_t i = 0; i < BUFFERED_FILEHANDLE_THREADED_INSTANCES; i++) {
        if (buffered_filehandles[i] == buffered_handle) {
            buffered_filehandles[i] = NULL;
            break;
        }
    }

    if (buffered_handle->file_hnd != FILEHND_INVALID) {
        fs_close(buffered_handle->file_hnd);
    }

    buffered_handle->file_hnd = FILEHND_INVALID;
    buffered_handle->buffer_offset = buffered_handle->buffer_used = 0;
    buffered_handle->eof_reached = true;

    mutex_destroy(&buffered_handle->mutex);

    free_chk(buffered_handle->native_path);
    free_chk(buffered_handle);

    mutex_unlock(&buffered_filehandles_mutex);
}
static void BufferedFileHandle_Suspend(SourceHandle* handle) {
    BufferedFileHandle* buffered_handle = (BufferedFileHandle*)handle;

    mutex_lock(&buffered_filehandles_mutex);

    if (buffered_handle->file_hnd != FILEHND_INVALID) {
        buffered_handle->resume_offset = fs_tell(buffered_handle->file_hnd);
        fs_close(buffered_handle->file_hnd);
        buffered_handle->file_hnd = FILEHND_INVALID;
    }

    mutex_unlock(&buffered_filehandles_mutex);
}


static void filehandle_buffered_refill(BufferedFileHandle* hnd, ssize_t maximum) {
    file_t file_hnd = hnd->file_hnd;

    if (file_hnd == FILEHND_INVALID) {
        // this instance is suspended, nothing to do
        return;
    }

    size_t buffer_used = hnd->buffer_used;
    ssize_t available = maximum - (ssize_t)buffer_used;
    uint8_t* ptr = hnd->buffer;

    if (buffer_used > 0) {
        size_t buffer_offset = hnd->buffer_offset;
        memmove(ptr, &hnd->buffer[buffer_offset], buffer_used);
        ptr += buffer_used;
    }

    while (available > 0) {
        ssize_t ret = fs_read(file_hnd, ptr, (size_t)available);
        if (ret == 0) {
            // EOF readed
            hnd->eof_reached = true;
            break;
        } else if (ret < 0) {
            // an error ocurred
            break;
        }

        ptr += ret;
        buffer_used += (size_t)ret;
        available -= ret;
    }

    hnd->buffer_offset = 0;
    hnd->buffer_used = buffer_used;
}

static void* filehandle_buffered_poll(void* param) {
    (void)param;

    unsigned int wait_time = BUFFERED_FILEHANDLE_WAIT_TIME;

    while (true) {
        thd_sleep(wait_time);
        wait_time = BUFFERED_FILEHANDLE_WAIT_TIME;

        size_t available = 0;
        bool already_pass = false;

        for (size_t i = 0; i < BUFFERED_FILEHANDLE_THREADED_INSTANCES; i++) {
            BufferedFileHandle* hnd = buffered_filehandles[i];
            if (!hnd) continue;

            if (mutex_trylock(&buffered_filehandles_mutex) == -1) {
                // mutex still in use
                if (already_pass) {
                    // try again later in 50ms
                    wait_time = 50;
                    available++;
                    break;
                }

                thd_pass();

                already_pass = true;
                i--;
                continue;
            }

            /*if (buffered_filehandles[i] != hnd) {
                // disposed while adquiring the mutex
                goto L_unlock_mutex;
            }*/

            available++;

            if (hnd->eof_reached) {
                // nothing to do
                goto L_unlock_mutex;
            }

            size_t buffer_used = hnd->buffer_used;

            bool is_away_from_eof;
            off_t file_offset = hnd->file_offset;
            if (file_offset >= 0 && file_offset < hnd->file_length) {
                size_t file_remaining = hnd->file_length - (size_t)file_offset;
                size_t buffer_available = BUFFERED_FILEHANDLE_BUFFER_SIZE - buffer_used;
                is_away_from_eof = file_remaining > buffer_available;
            } else {
                is_away_from_eof = true;
            }

            if (buffer_used >= BUFFERED_FILEHANDLE_THRESHOLD && is_away_from_eof) {
                // wait a bit more
                wait_time = BUFFERED_FILEHANDLE_WAIT_TIME_SHORT;
                goto L_unlock_mutex;
            }

            mutex_lock(&hnd->mutex);
            filehandle_buffered_refill(hnd, BUFFERED_FILEHANDLE_BUFFER_SIZE);
            mutex_unlock(&hnd->mutex);

        L_unlock_mutex:
            mutex_unlock(&buffered_filehandles_mutex);
        }

        if (available < 1) {
            // block the thread until a new BufferedFileHandle is created
            condevt_wait(&buffered_filehandles_condevt);
        }
    }

    return NULL;
}

static void filehandle_buffered_run_thread_async() {
    if (buffered_filehandles_thread != NULL) {
        condevt_signal(&buffered_filehandles_condevt);
        return;
    }

    buffered_filehandles_thread = thd_create(true, filehandle_buffered_poll, NULL);
    assert(buffered_filehandles_thread);
}


SourceHandle* filehandle_init5(const char* native_path) {
    mutex_lock(&buffered_filehandles_mutex);

    // check if can create a buffered file
    ssize_t index = -1;
    for (ssize_t i = 0; i < BUFFERED_FILEHANDLE_THREADED_INSTANCES; i++) {
        if (buffered_filehandles[i] == NULL) {
            index = i;
            break;
        }
    }

    SourceHandle* hnd = NULL;

    if (index < 0) {
        // all slots are in use
        goto L_fallback_return;
    }

    file_t file_hnd = fs_open(native_path, O_RDONLY);
    if (file_hnd == FILEHND_INVALID) {
        // this never should happen
        logger_error("filehandle_init5() failed to open %s", native_path);
        mutex_unlock(&buffered_filehandles_mutex);
        return NULL;
    }

    size_t file_length = fs_total(file_hnd);
    if (file_length == (size_t)-1) {
        fs_close(file_hnd);
        logger_warn("filehandle_init5() failed to adquire the length %s", native_path);
        goto L_fallback_return;
    }

    hnd = malloc_chk(sizeof(BufferedFileHandle));
    if (!hnd) {
        logger_warn("filehandle_init5() failed creation for %s", native_path);
        goto L_fallback_return;
    }

    BufferedFileHandle* bfhnd = (BufferedFileHandle*)hnd;
    bfhnd->handle = (SourceHandle){
        .read = BufferedFileHandle_Read,
        .seek = BufferedFileHandle_Seek,
        .tell = BufferedFileHandle_Tell,
        .length = BufferedFileHandle_Length,
        .destroy = BufferedFileHandle_Destroy,
        .suspend = BufferedFileHandle_Suspend
    };
    bfhnd->native_path = string_duplicate(native_path);
    bfhnd->resume_offset = 0;
    bfhnd->file_hnd = file_hnd;
    bfhnd->file_offset = 0;
    bfhnd->file_length = file_length;
    bfhnd->eof_reached = false;
    bfhnd->buffer_used = 0;
    bfhnd->buffer_offset = 0;

    mutex_init(&bfhnd->mutex, MUTEX_TYPE_NORMAL);

    buffered_filehandles[index] = bfhnd;

    // prefill
    filehandle_buffered_refill(bfhnd, BUFFERED_FILEHANDLE_SEEK_REFILL);

#ifdef DEBUG
    logger_info("filehandle_init5() Successfully created an instance for %s", native_path);
#endif

    mutex_unlock(&buffered_filehandles_mutex);

    // enable async buffers polling if not running
    filehandle_buffered_run_thread_async();
    return hnd;

L_fallback_return:
    hnd = filehandle_init1(native_path, false);
    mutex_unlock(&buffered_filehandles_mutex);
    return hnd;
}
