using System;

#pragma warning disable CS8981

namespace KallistiOS.UTSNAME;

internal ref struct utsname {
    public string sysname;
    public string nodename;
    public string release;
    public string version;
    public string machine;
}

internal static class UTSNAME {
    private const string UNAME_KERNEL = "KallistiOS";
    //private const string UNAME_MACHINE = "Dreamcast";

    /**
     * Retrieve version and other similar information about the kernel.
     * This function retrieves information about the current version of the kernel
     * that is running, storing it in the provided buffer.
     * @param {utsname} n   The buffer to store version information in.
     * @returns {number}    0 on success, -1 on error (setting errno appropriately).
*/
    public static int uname(out utsname n) {
        //if (!n) return -1;
        const string kern_version = "2.0.0-503-gc3e7be3";// STUB

        var os = Environment.OSVersion;

        n.nodename = null;
        n.sysname = UNAME_KERNEL;
        n.release = kern_version;
        n.version = os.VersionString;
        n.machine = os.Platform.ToString() + $" [has STUBs of {UNAME_KERNEL} {kern_version}]";
        return 0;
    }

}
