using System;
using System.Threading;

#pragma warning disable CS8981

namespace KallistiOS.THD;

public class kthread_t {
    public STATE state;
    public object rv;
    internal bool ___detached;
    internal Thread ___thread;
}

public enum STATE {
    ZOMBIE = 0x0000,
    RUNNING = 0x0001,
    READY = 0x0002,
    WAIT = 0x0003,
    FINISHED = 0x0004
}

public static class thd {

    /**
     * This function manually yields the current thread's timeslice to the system,
     * forcing a reschedule to occur.
     * @summary Throw away the current thread's timeslice.
     */
    public static void pass() {
        Thread.Sleep(0);
    }

    /** 
     * @summary  Create a new thread.
        This function creates a new kernel thread with default parameters to run the
        given routine. The thread will terminate and clean up resources when the
        routine completes if the thread is created detached, otherwise you must
        join the thread with thd_join() to clean up after it.
        @param {bool} detach    Set to 1 to create a detached thread. Set to 0 to create a joinable thread.
        @param {void* (*)(void*)} routine     The function to call in the new thread.
        @param {object} param      A parameter to pass to the function called.
        @returns {kthread_t}       The new thread on success, NULL on failure.
    */
    public static kthread_t create(bool detach, Func<object, object> routine, object param) {
        kthread_t thd = new kthread_t() {
            ___detached = detach,
            ___thread = new Thread(ThreadMain),
            rv = null,
            state = STATE.READY
        };
        ThreadWrapper wrapper = new ThreadWrapper() { kthread = thd, routine = routine, param = param };
        thd.___thread.Start(wrapper);
        return thd;
    }

    /** 
     * @summary  Brutally kill the given thread.
        This function kills the given thread, removing it from the execution chain,
        cleaning up thread-local data and other internal structures. You should
        never call this function on the current thread. In general, you shouldn't
        call this function at all.
        @param {kthread_t} thd             The thread to destroy.
        @returns {int} 0               On success.
    */
    public static int destroy(kthread_t thread) {
        return 0;
    }

    /** 
     * @summary brief  Wait for a thread to exit.
        This function "joins" a joinable thread. This means effectively that the
        calling thread blocks until the speified thread completes execution. It is
        invalid to join a detached thread, only joinable threads may be joined.
        @param {kthread_t} thd             The joinable thread to join.
        @param {object} value_ptr       A pointer to storage for the thread's return value,
                                or NULL if you don't care about it.
        @returns  {int}               0 on success, or less than 0 if the thread is
                                non-existant or not joinable.
    */
    public static int join(kthread_t thd, ref object value_ptr) {
        if (thd.___detached) return -2;
        if (thd.___thread.IsAlive) {
            thd.___thread.Join();
        }
        value_ptr = thd.rv;
        return 0;
    }



    private static void ThreadMain(object arg) {
        ThreadWrapper wrapper = (ThreadWrapper)arg;

        Thread.Sleep(1);

        try {
            wrapper.kthread.state = STATE.RUNNING;
            wrapper.kthread.rv = wrapper.routine(wrapper.param);
        } catch (Exception e) {
            Console.Error.WriteLine(e);
        }

        wrapper.kthread.state = STATE.FINISHED;
    }

    private struct ThreadWrapper {
        public kthread_t kthread;
        public object param;
        public Func<object, object> routine;
    }
}
