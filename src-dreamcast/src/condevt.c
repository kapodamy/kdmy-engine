#include <assert.h>
#include <stdlib.h>

#include "condevt.h"



#ifdef SNDBRIDGE_DREAMCAST_STUB

static inline void calc_timeout(struct timespec* ts, int milliseconds) {
    int sec = milliseconds / 1000;
    milliseconds -= sec * 1000;

    clock_gettime(CLOCK_REALTIME, ts);

    ts->tv_sec += sec;
    ts->tv_nsec += milliseconds * 1000 * 1000;
}


bool condevt_init(condevt_t* condevt, bool manual, bool signal) {
    assert(condevt);

    if (pthread_mutex_init(&condevt->mutex, NULL) != 0) {
        return false;
    }
    if (pthread_cond_init(&condevt->cond, NULL) != 0) {
        pthread_cond_destroy(&condevt->cond);
        pthread_mutex_destroy(&condevt->mutex);
        return false;
    }

    condevt->manual = manual;
    condevt->signal = signal;
    return true;
}

bool condevt_signal(condevt_t* condevt) {
    assert(condevt);

    int ret = pthread_mutex_lock(&condevt->mutex);
    if (ret != 0) return false;

    condevt->signal = true;
    int ret2 = pthread_cond_broadcast(&condevt->cond);

    ret = pthread_mutex_unlock(&condevt->mutex);
    if (ret != 0) return false;

    return ret2 == 0;
}

bool condevt_wait(condevt_t* condevt) {
    assert(condevt);

    int ret = pthread_mutex_lock(&condevt->mutex);
    if (ret != 0) return false;

    int ret2 = 0;
    while (!condevt->signal && ret2 == 0) {
        ret2 = pthread_cond_wait(&condevt->cond, &condevt->mutex);
    }

    if (!condevt->manual) condevt->signal = false;

    ret = pthread_mutex_unlock(&condevt->mutex);
    if (ret != 0) return false;

    return ret2 == 0;
}

bool condevt_wait_timed(condevt_t* condevt, int timeout) {
    assert(condevt);

    struct timespec time;
    calc_timeout(&time, timeout);

    int ret = pthread_mutex_lock(&condevt->mutex);
    if (ret != 0) return false;

    int ret2 = 0;
    calc_timeout(&time, timeout);
    while (!condevt->signal && ret2 == 0) {
        ret2 = pthread_cond_timedwait(&condevt->cond, &condevt->mutex, &time);
    }

    if (!condevt->manual && ret2 != ETIMEDOUT) condevt->signal = false;

    ret = pthread_mutex_unlock(&condevt->mutex);
    if (ret != 0) return false;

    return ret2 == 0;
}

bool condevt_signal_one(condevt_t* condevt) {
    assert(condevt);

    int ret = pthread_mutex_lock(&condevt->mutex);
    if (ret != 0) return false;

    int ret2 = pthread_cond_signal(&condevt->cond);

    ret = pthread_mutex_unlock(&condevt->mutex);
    if (ret != 0) return false;

    return ret2 == 0;
}

void condevt_destroy(condevt_t* condevt) {
    if (!condevt) return;
    pthread_mutex_destroy(&condevt->mutex);
    pthread_cond_signal(&condevt->cond);
    pthread_cond_destroy(&condevt->cond);
}

void condevt_reset(condevt_t* condevt) {
    assert(condevt);
    condevt->signal = false;
}

void condevt_set_manual_reset(condevt_t* condevt, bool manual) {
    assert(condevt);

    condevt->manual = manual;
}

#else

#include <kos/genwait.h>

bool condevt_init(condevt_t* condevt, bool manual, bool signal) {
    assert(condevt);

    if (mutex_init(&condevt->mutex, MUTEX_TYPE_DEFAULT) != 0) {
        mutex_destroy(&condevt->mutex);
        return false;
    }

    condevt->manual = manual;
    condevt->signal = signal;
    return true;
}

bool condevt_signal(condevt_t* condevt) {
    assert(condevt);

    int res = -1;

    mutex_lock(&condevt->mutex);
    {
        if (!condevt->manual) condevt->signal = true;
        res = genwait_wake_cnt(condevt, -1, 0);
    }
    mutex_unlock(&condevt->mutex);

    return res == 0;
}

bool condevt_wait(condevt_t* condevt) {
    return condevt_wait_timed(condevt, 0);
}

bool condevt_wait_timed(condevt_t* condevt, int timeout) {
    assert(condevt);

    if (condevt->signal)
        return true;
    else
        return genwait_wait(condevt, NULL, timeout, NULL) == 0;
}

bool condevt_signal_one(condevt_t* condevt) {
    assert(condevt);

    int res = -1;

    mutex_lock(&condevt->mutex);
    {
        res = genwait_wake_cnt(condevt, 1, 0);
    }
    mutex_unlock(&condevt->mutex);

    return res == 0;
}

void condevt_destroy(condevt_t* condevt) {
    if (!condevt) return;
    genwait_wake_cnt(condevt, -1, 0);
    mutex_destroy(&condevt->mutex);
}

void condevt_reset(condevt_t* condevt) {
    assert(condevt);

    condevt->signal = false;
}

void condevt_set_manual_reset(condevt_t* condevt, bool manual) {
    assert(condevt);

    condevt->manual = manual;
}
#endif
