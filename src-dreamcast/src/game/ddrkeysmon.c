#include <arch/timer.h>
#include <dc/maple.h>
#include <kos/thread.h>

#include "game/ddrkeymon.h"

#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"


typedef struct {
    bool is_visible;
    GamepadButtons button_flags;
    uint32_t button_count;
    GamepadButtons* button_array;
} Bind;

struct DDRKeymon_s {
    DDRKeysFIFO ddrkeys_fifo;
    uint64_t start_timestamp;
    Bind* strum_binds;
    int32_t strum_binds_size;
    kthread_t* thd;
    volatile bool thd_running;
    Gamepad gamepad;
};


static void ddrkeymon_internal_append_key(DDRKeymon ddrkeymon, uint64_t timestamp, int32_t strum_id, int32_t button_id, GamepadButtons holding);
static void ddrkeymon_internal_process_key(DDRKeymon ddrkeymon, uint64_t timestamp, GamepadButtons old_buttons, GamepadButtons new_buttons);
static void* ddrkeymon_internal_thd(void* param);
static inline uint64_t ddrkeymon_internal_get_timestamp(float64 offset);


DDRKeymon ddrkeymon_init(Gamepad gamepad, GamepadButtons* strum_binds, int32_t strum_binds_size) {
    DDRKeymon ddrkeymon = malloc_chk(sizeof(struct DDRKeymon_s));
    malloc_assert(ddrkeymon, DDRKeymon);

    *ddrkeymon = (struct DDRKeymon_s){
        .ddrkeys_fifo = (DDRKeysFIFO){
            .queue = {},
            .available = 0,
        },

        .start_timestamp = 0,

        .strum_binds = malloc_for_array(Bind, strum_binds_size),
        .strum_binds_size = strum_binds_size,

        .thd = NULL,
        .thd_running = false,
        .gamepad = gamepad
    };

    for (int32_t i = 0; i < strum_binds_size; i++) {
        int count = math2d_bitcount(strum_binds[i]);
        ddrkeymon->strum_binds[i] = (Bind){
            .is_visible = true,
            .button_flags = strum_binds[i],
            .button_count = (uint32_t)count,
            .button_array = malloc_for_array(GamepadButtons, count)
        };

        for (int32_t j = 0, k = 0; k < count; j++) {
            GamepadButtons flag = strum_binds[i] & (0x01 << j);
            if (flag == 0x00) continue;
            ddrkeymon->strum_binds[i].button_array[k++] = flag;
        }
    }

    return ddrkeymon;
}

void ddrkeymon_destroy(DDRKeymon* ddrkeymon_ptr) {
    if (!ddrkeymon_ptr || !*ddrkeymon_ptr) return;

    DDRKeymon ddrkeymon = *ddrkeymon_ptr;

    // note: "ddrkeys_fifo.queue" is a fixed array do not dispose

    // stop the monitor thread if running
    ddrkeymon_stop(ddrkeymon);

    for (int32_t i = 0; i < ddrkeymon->strum_binds_size; i++) {
        free_chk(ddrkeymon->strum_binds[i].button_array);
    }

    free_chk(ddrkeymon->strum_binds);

    free_chk(ddrkeymon);
    *ddrkeymon_ptr = NULL;
}


DDRKeysFIFO* ddrkeymon_get_fifo(DDRKeymon ddrkeymon) {
    return &ddrkeymon->ddrkeys_fifo;
}

void ddrkeymon_purge(DDRKeysFIFO* ddrkeys_fifo) {
    ddrkeymon_purge2(ddrkeys_fifo, false);
}

void ddrkeymon_purge2(DDRKeysFIFO* ddrkeys_fifo, bool force_drop_first) {
    if (ddrkeys_fifo->available < 1) return;

    int32_t available = 0;

    for (int32_t i = force_drop_first ? 1 : 0; i < ddrkeys_fifo->available; i++) {
        if (ddrkeys_fifo->queue[i].discard) continue;

        if (i == available) {
            available++;
            continue;
        }

        ddrkeys_fifo->queue[i] = ddrkeys_fifo->queue[available]; // clone struct
        available++;

#ifdef DEBUG
        // debugging only
        ddrkeys_fifo->queue[i].in_song_timestamp = -2.0;
        ddrkeys_fifo->queue[i].strum_id = -2;
        ddrkeys_fifo->queue[i].button = 0x00;
#endif
    }

    ddrkeys_fifo->available = available;
}

void ddrkeymon_clear(DDRKeymon ddrkeymon) {
    for (int32_t i = 0; i < ddrkeymon->ddrkeys_fifo.available; i++)
        ddrkeymon->ddrkeys_fifo.queue[i].discard = true;
    ddrkeymon->ddrkeys_fifo.available = 0;
}

void ddrkeymon_start(DDRKeymon ddrkeymon, float64 offset_timestamp) {
    if (ddrkeymon->thd) return;

    ddrkeymon->thd_running = true;
    ddrkeymon->start_timestamp = ddrkeymon_internal_get_timestamp(offset_timestamp);
    ddrkeymon->thd = thd_create(0, ddrkeymon_internal_thd, ddrkeymon);

#ifdef DEBUG
    assert(ddrkeymon->thd);
#endif

    thd_set_prio(ddrkeymon->thd, PRIO_DEFAULT - 1);
}

void ddrkeymon_stop(DDRKeymon ddrkeymon) {
    if (!ddrkeymon->thd) return;

    ddrkeymon->thd_running = false;
    thd_join(ddrkeymon->thd, NULL);
    ddrkeymon->thd = NULL;

    ddrkeymon_clear(ddrkeymon);
}

float64 ddrkeymon_peek_timestamp(DDRKeymon ddrkeymon) {
    return timer_ms_gettime64() - ddrkeymon->start_timestamp;
}

void ddrkeymon_resync(DDRKeymon ddrkeymon, float64 offset_timestamp) {
    ddrkeymon->start_timestamp = ddrkeymon_internal_get_timestamp(offset_timestamp);
}


static void ddrkeymon_internal_append_key(DDRKeymon ddrkeymon, uint64_t timestamp, int32_t strum_id, int32_t button_id, GamepadButtons holding) {
    DDRKey* queue = ddrkeymon->ddrkeys_fifo.queue;
    int32_t available = ddrkeymon->ddrkeys_fifo.available;

    // offset correction
    timestamp -= ddrkeymon->start_timestamp;

    for (int32_t i = available - 1; i >= 0; i--) {
        if (queue[i].discard) continue;
        if (queue[i].strum_id != strum_id) continue;
        if (queue[i].button != button_id) continue;

        // can not exists repeated events
        if (queue[i].holding != holding) break;

        // non-monotomous key event, this happens if the queue overflows
        // which under normal conditions this never should happen
        queue[i].in_song_timestamp = timestamp;
        return;
    }

    if (available >= DDRKEYMON_FIFO_LENGTH) {
        // imminent overflow, drop first key
        logger_warn("ddrkeymon_append_key() queue overflow, ¿are you checking the queue?");
        ddrkeymon_purge2(&ddrkeymon->ddrkeys_fifo, true);

        if (available >= DDRKEYMON_FIFO_LENGTH) {
            logger_error("ddrkeymon_append_key() queue overflow");
            assert(available < DDRKEYMON_FIFO_LENGTH);
        }
    }

    // the queue is healthy, append the key
    queue[available].in_song_timestamp = timestamp;
    queue[available].strum_id = strum_id;
    queue[available].holding = holding;
    queue[available].discard = false;
    queue[available].strum_invisible = !ddrkeymon->strum_binds[strum_id].is_visible;
    queue[available].button = button_id;

    ddrkeymon->ddrkeys_fifo.available = available + 1;
}

static void ddrkeymon_internal_process_key(DDRKeymon ddrkeymon, uint64_t timestamp, GamepadButtons old_buttons, GamepadButtons new_buttons) {
    for (int32_t i = 0; i < ddrkeymon->strum_binds_size; i++) {
        GamepadButtons buttons_flags = ddrkeymon->strum_binds[i].button_flags;
        GamepadButtons old_state = old_buttons & buttons_flags;
        GamepadButtons new_state = new_buttons & buttons_flags;

        if (old_state == new_state) continue;

        for (int32_t j = 0; j < ddrkeymon->strum_binds[i].button_count; j++) {
            GamepadButtons button_flag = ddrkeymon->strum_binds[i].button_array[j];
            GamepadButtons button_state_old = old_state & button_flag;
            GamepadButtons button_state_new = new_state & button_flag;

            if (button_state_old == button_state_new) continue;

            ddrkeymon_internal_append_key(ddrkeymon, timestamp, i, button_flag, button_state_new);
        }
    }
}

static void* ddrkeymon_internal_thd(void* param) {
    DDRKeymon ddrkeymon = (DDRKeymon)param;
    maple_driver_t* cont_dvr = NULL;


    // find the "controller_dvr"
    {
        maple_driver_t* dvr;
        LIST_FOREACH(dvr, &maple_state.driver_list, drv_list) {
            if (dvr->functions == MAPLE_FUNC_CONTROLLER) {
                cont_dvr = dvr;
                break;
            }
        }
    }

    assert_msg(cont_dvr, "'Controller Driver' not registered");

    while (ddrkeymon->thd_running) {
        uint64_t timestamp = timer_ms_gettime64();
        GamepadButtonsChanges ret = gamepad_direct_state_update(ddrkeymon->gamepad);

        if (ret.previous != UINT32_MAX && ret.current != UINT32_MAX) {
            ddrkeymon_internal_process_key(ddrkeymon, timestamp, ret.previous, ret.current);
        }

        //
        // this callback is normally called every vblank (~16.6ms), but force
        // to call it every ~5ms. Note: still called every vblank
        //
        cont_dvr->periodic(cont_dvr);

        thd_sleep(5);
    }

    return NULL;
}

static inline uint64_t ddrkeymon_internal_get_timestamp(float64 offset) {
    uint64_t timestamp = timer_ms_gettime64();

    if (offset < 0.0)
        timestamp -= (uint64_t)(-offset);
    else
        timestamp += (uint64_t)offset;

    return timestamp;
}
