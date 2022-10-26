#ifndef ___sndbridge___mutex_h
#define ___sndbridge___mutex_h

#ifdef _WIN32
#include <windows.h>
#include <synchapi.h>

typedef HANDLE Mutex;

static inline Mutex mutex_init() {
    return CreateMutex(NULL, FALSE, NULL);
}

static inline void mutex_destroy(Mutex mutex) {
    CloseHandle(mutex);
}

static inline void mutex_adquire(Mutex mutex) {
    WaitForSingleObject(mutex, INFINITE);
}

static inline void mutex_release(Mutex mutex) {
    ReleaseMutex(mutex);
}

#else

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

typedef pthread_mutex_t* Mutex;

static inline Mutex mutex_init() {
    void* ptr = malloc(sizeof(pthread_mutex_t));
    Mutex mutex = (pthread_mutex_t*)ptr;

    assert(pthread_mutex_init(mutex, NULL) != 0);
    return mutex;
}

static inline void mutex_destroy(Mutex mutex) {
    pthread_mutex_destroy(mutex);
    free(mutex);
}

static inline void mutex_adquire(Mutex mutex) {
    pthread_mutex_lock(mutex);
}

static inline void mutex_release(Mutex mutex) {
    pthread_mutex_unlock(mutex);
}

#endif

#endif