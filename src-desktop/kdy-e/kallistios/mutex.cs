using System.Threading;

#pragma warning disable CS8981

namespace KallistiOS.MUTEX;

public class mutex_t {
    internal Mutex _mutex;
}

class mutex {

    public const int TYPE_NORMAL = 1;

    public static int Init(out mutex_t m, int mtype) {
        if (mtype != TYPE_NORMAL) {
            m = null;
            return -1;
        }

        m = new mutex_t() { _mutex = new Mutex() };
        return 0;
    }

    public static int Destroy(mutex_t m) {
        if (m == null) return -1;
        m._mutex.Dispose();
        return 0;
    }

    public static int Lock(mutex_t m) {
        try {
            m._mutex.WaitOne();
            return 0;
        } catch {
            return -1;
        }
    }

    public static int Unlock(mutex_t m) {
        try {
            m._mutex.ReleaseMutex();
            return 0;
        } catch {
            return -1;
        }
    }


}
