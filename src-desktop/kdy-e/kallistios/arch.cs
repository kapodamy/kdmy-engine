using System;

#pragma warning disable CS8981

namespace KallistiOS {

    public static class arch {

        /**
         * Kernel "exit to menu" call.
         */
        public static void menu() {
            Console.Error.WriteLine("[LOG] arch_menu() called, closing the engine");
        }

    }

}
