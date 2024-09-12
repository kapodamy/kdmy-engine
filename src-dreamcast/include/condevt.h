#ifndef __condevt_h
#define __condevt_h

#include <stdbool.h>
#include <stdint.h>

#ifdef SNDBRIDGE_DREAMCAST_STUB
#include <pthread.h>

typedef struct {
    volatile bool signal;
    bool manual;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} condevt_t;

#define CONDENVT_INITIALIZER #error "CONDENVT_INITIALIZER not available \
in the current platform, use condevt_init() instead"
#define CONDENVT_INITIALIZER_SIGNALED #error "CONDENVT_INITIALIZER_SIGNALED not \
available in the current platform, use condevt_init() instead"
#define CONDENVT_INITIALIZER_MANUAL #error "CONDENVT_INITIALIZER_MANUAL \
not available in the current platform, use condevt_init() instead"
#define CONDENVT_INITIALIZER_MANUAL_SIGNALED #error "CONDENVT_INITIALIZER_MANUAL_SIGNALED \
not available in the current platform, use condevt_init() instead"

#else
#include <kos/mutex.h>

typedef struct {
    mutex_t mutex;
    uint8_t signal;
    uint8_t manual;
} condevt_t;

#define CONDENVT_INITIALIZER {MUTEX_INITIALIZER, 0, 0}
#define CONDENVT_INITIALIZER_SIGNALED {MUTEX_INITIALIZER, 1, 0}
#define CONDENVT_INITIALIZER_MANUAL {MUTEX_INITIALIZER, 0, 1}
#define CONDENVT_INITIALIZER_MANUAL_SIGNALED {MUTEX_INITIALIZER, 1, 1}

#endif

/**
 * \brief Initialize a new condevt
 * \param condevt pointer to the condevt structure to initialize
 * \param manual true to reset manually, otherwise, false to reset after every "condevt_wait*()" call
 * \param signal true to place the signaled state, otherwise, false to automatically reset
 * \note use automatic reset only when a single thread is waiting
 * \returns true if success, otherwise, false if an error occurs
 */
bool condevt_init(condevt_t* condevt, bool manual, bool signal);
/**
 * \brief Puts the condevt to signaled, allowing all waiting threads to proceed
 * \returns true if success, otherwise, false if an error occurs
 */
bool condevt_signal(condevt_t* condevt);
/**
 * \brief Wait for condevt to be signaled
 * \param condevt the condevt instance
 * \returns true if success, otherwise, false if an error/cancelation occurs
 */
bool condevt_wait(condevt_t* condevt);
/**
 * \brief Wait for condevt to be signaled or until the timeout occurs
 * \param condevt the condevt instance
 * \returns true if success, otherwise, false if an error/cancelation/timeout occurs
 */
bool condevt_wait_timed(condevt_t* condevt, int timeout);
/**
 * \brief Puts the condevt to signaled, allowing a single waiting thread to proceed
 * \param condevt the condevt instance
 * \returns true if success, otherwise, false if an error occurs
 */
bool condevt_signal_one(condevt_t* condevt);
/**
 * \brief free all resources used by the condevt instance
 * \param condevt the condevt instance
 */
void condevt_destroy(condevt_t* condevt);
/**
 * \brief removes the signaled state, this only has effect if manual reset is enabled
 * \param condevt the condevt instance
 */
void condevt_reset(condevt_t* condevt);
/**
 * \brief enable or disables the manually reseting the signaled state, to manual reset call "condevt_reset()"
 * \param condevt the condevt instance
 * \param manual true to reset manually, otherwise, false to reset after every "condevt_wait*()" call
 */
void condevt_set_manual_reset(condevt_t* condevt, bool manual);

#endif
