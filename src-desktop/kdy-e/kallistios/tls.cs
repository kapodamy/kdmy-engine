using System.Threading;

namespace KallistiOS.TLS {

    public delegate void kthread_destructor<T>(T obj);

    public class kthread_key_t<T> {
        internal ThreadLocal<T> data;
    }


    internal static class kthread {
        public static bool key_create<T>(kthread_key_t<T> key, kthread_destructor<T> destructor) {
            if (key.data != null) return false;
            key.data = new ThreadLocal<T>();
            return true;
        }

        public static T getspecific<T>(kthread_key_t<T> key) {
            if (key.data != null) return key.data.Value;
            return default(T);
        }

        public static void setspecific<T>(kthread_key_t<T> key, T data) {
            if (key.data != null) key.data.Value = data;
        }
    }
}
