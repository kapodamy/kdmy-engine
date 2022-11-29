using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace KallistiOS.VFS {

    public struct stat_t {
        public int st_blksize;
        public int st_size;
    }

    [Flags]
    public enum VFSOpenMode : int {
        O_NONE = 0, O_RDONLY = 1, O_WRONLY = 2
    }

    public enum VFSSeek : int {
        SEEK_SET = SeekOrigin.Begin,
        SEEK_CUR = SeekOrigin.Current,
        SEEK_END = SeekOrigin.End,
    }

    public static class fs {

        private class VSFHandle {
            public int hnd;
            public FileStream stream;
            public string FAT_filename;
        }

        private static int HANDLE_IDS = 0;
        private static readonly List<VSFHandle> HANDLES;

        static fs() {
            HANDLES = new List<VSFHandle>();
        }


        /** 
         * @summary  Delete the specified file.
         *   This function deletes the specified file from the filesystem. This should
         *   only be used for files, not for directories. For directories, use fs_rmdir()
         *   instead of this function.
         * @param {string}  fn              The path to remove.
         * @return {int} 0 on success, -1 on failure.
        */
        public static int unlink(string fn) {
            try {
                fn = GetVMSPath(fn);
                File.Delete(fn);
                return 0;
            } catch (Exception e) {
                Console.Error.WriteLine("[ERROR] fs_unlink() {0}\n{1}", fn, e.Message);
                return -1;
            }
        }

        /** 
         * @summary  Open a file on the VFS.
         *   This function opens the specified file, returning a new file descriptor to
         *   access the file.
         *   @param {string} fn              The path to open.
         *   @param {int}  mode            The mode to use with opening the file. This may
                                    include the standard open modes (O_RDONLY, O_WRONLY,
                                    etc), as well as values from the \ref open_modes
                                    list. Multiple values can be ORed together.
            @return {int}                 The new file descriptor on success, -1 on error.
        */
        public static int open(string fn, VFSOpenMode mode) {
            if (!fn.StartsWith("/vmu/")) {
                throw new NotImplementedException("expeted '/vmu' root folder, found " + fn);
            }

            string FAT_filename = fn.Split('/')[3];
            fn = GetVMSPath(fn);

            FileAccess _access = FileAccess.ReadWrite;
            FileMode _mode = FileMode.OpenOrCreate;

            if ((mode & VFSOpenMode.O_RDONLY) == VFSOpenMode.O_RDONLY) {
                _access = FileAccess.Read;
                _mode = FileMode.Open;
            } else if ((mode & VFSOpenMode.O_WRONLY) == VFSOpenMode.O_WRONLY) {
                _access = FileAccess.Write;
                _mode = FileMode.OpenOrCreate;
            }

            try {
                string saves_path = Path.GetDirectoryName(fn);
                if (!Directory.Exists(saves_path)) {
                    Directory.CreateDirectory(saves_path);
                    return -1;
                } else if (!File.Exists(fn) && _mode == FileMode.Open) {
                    return -1;
                }

                FileStream stream = new FileStream(fn, _mode, _access, FileShare.ReadWrite);
                int hnd = HANDLE_IDS++;
                VSFHandle obj = new VSFHandle { stream = stream, hnd = hnd, FAT_filename = FAT_filename };
                HANDLES.Add(obj);

                return hnd;
            } catch (Exception e) {
                Console.Error.WriteLine("[ERROR] fs_open() {0}\n{1}", fn, e);
                return -1;
            }
        }

        /** 
         * @summary  Close an opened file.
         *   This function closes the specified file descriptor, releasing all resources
         *   associated with the descriptor.
         *   @param {int} hnd             The file descriptor to close.
         *   @return {int}                0 for success, -1 for error
        */
        public static int close(int hnd) {
            foreach (VSFHandle obj in HANDLES) {
                if (obj.hnd == hnd) {
                    HANDLES.Remove(obj);
                    if (obj.stream.CanWrite) {
                        long padding = 512 - (obj.stream.Length % 512);
                        if (padding > 0) {
                            obj.stream.Write(new byte[padding], 0, (int)padding);
                            obj.stream.SetLength(obj.stream.Length);
                        }
                        UpdateVMI(obj.stream.Name, obj.FAT_filename, obj.stream.Length);
                    }
                    obj.stream.Close();
                    return 0;
                }
            }
            return -1;
        }

        /**
         * @summary  Write to an opened file.
         *   This function writes the specfied buffer into the file at the current file
         *   pointer.
         *   @param {int} hnd             The file descriptor to write into.
         *   @param {byte[]} buffer          The data to write into the file.
         *   @param {int} cnt             The size of the buffer, in bytes.
         *   @return {int}                 The int of bytes written, or -1 on failure. Note
         *                           that the int of bytes written may be less than
         *                           what was requested.
        */
        public static int write(int hnd, byte[] buffer, int cnt) {
            foreach (VSFHandle obj in HANDLES) {
                if (obj.hnd == hnd) {
                    obj.stream.Write(buffer, 0, cnt);
                    return cnt;
                }
            }
            return -1;
        }

        /** 
         * @summary  Retrieve the length of an opened file.
            This file retrieves the length of the file associated with the given file
            descriptor.
            @param  {int}  hnd             The file descriptor to retrieve the size from.
            @return  {int}                The length of the file on success, -1 on failure.
        */
        public static int total(int hnd) {
            foreach (VSFHandle obj in HANDLES) {
                if (obj.hnd == hnd) {
                    return (int)obj.stream.Length;
                }
            }
            return -1;
        }

        /** 
         * @summary  Seek to a new position within a file.
         *  This function moves the file pointer to the specified position within the
         *  file (the base of this position is determined by the whence parameter).
         *  @param {int} hnd             The file descriptor to move the pointer for.
         *  @param {int} offset          The offset in bytes from the specified base.
         *  @param {int} whence          The base of the pointer move. This should be one of
         *                          the \ref seek_modes values.
         *  @return {int}                The new position of the file pointer.
        */
        public static int seek(int hnd, int offset, VFSSeek whence) {
            foreach (VSFHandle obj in HANDLES) {
                if (obj.hnd == hnd) {
                    return (int)obj.stream.Seek(offset, (SeekOrigin)((int)whence));
                }
            }
            return -1;
        }

        /** 
         * @summary  Read from an opened file.
         *   This function reads into the specified buffer from the file at its current
         *   file pointer.
         *   @param {int} hnd             The file descriptor to read from.
         *   @param {byte[]} buffer          The buffer to read into.
         *   @param {int} cnt             The size of the buffer (or the int of bytes
         *                           requested).
         *   @return {int}                 The int of bytes read, or -1 on error. Note that
         *                           this may not be the full int of bytes requested.
        */
        public static int read(int hnd, byte[] buffer, int cnt) {
            return read(hnd, buffer, 0, cnt);
        }

        public static int read(int hnd, byte[] buffer, int buffer_offset, int cnt) {
            foreach (VSFHandle obj in HANDLES) {
                if (obj.hnd == hnd) {
                    return obj.stream.Read(buffer, buffer_offset, cnt);
                }
            }
            return -1;
        }

        /** 
         * @summary  Retrieve information about the specified path.
            This function retrieves status information on the given path. This function
            now returns the normal POSIX-style struct stat, rather than the old KOS
            stat_t structure. In addition, you can specify whether or not this function
            should resolve symbolic links on filesystems that support symlinks.
            @param {string} path            The path to retrieve information about.
            @param {stat_t} buf             The buffer to store stat information in.
            @param {int} flag            Specifies whether or not to resolve a symbolic link.
                                    If you don't want to resolve any symbolic links at
                                    the end of the path, pass AT_SYMLINK_NOFOLLOW,
                                    otherwise pass 0.
            @returns {int}                 0 on success, -1 on failure.
        */
        public static int stat(string path, ref stat_t buf, int flag) {
            if (path != "/vmu") return -1;

            // STUB
            buf.st_size = 800 * 16;
            buf.st_blksize = 512;
            return 0;
        }


        private static byte[] CreateVMI(string vms_filename, string FAT_filename, int filesize) {
            byte[] vmi = new byte[108];

            if (vms_filename.EndsWith(".vms")) vms_filename = vms_filename.Substring(0, vms_filename.Length - 4);

            if (FAT_filename == null) throw new ArgumentNullException("FAT_filename");
            if (FAT_filename.Length < 12) FAT_filename += new String('\x20', 12 - FAT_filename.Length);

            byte[] vms_filename_buffer = Encoding.UTF8.GetBytes(vms_filename);
            byte[] vmu_FAT_filename_buffer = Encoding.UTF8.GetBytes(FAT_filename);
            byte[] description = Encoding.UTF8.GetBytes("KDMY Engine STUB vmi file       ");
            DateTime now = DateTime.Now;
            byte[] year = BitConverter.GetBytes((ushort)now.Year);

            for (int i = 0 ; i < 4 ; i++) {
                if (i < vms_filename_buffer.Length)
                    vmi[i] = vms_filename_buffer[i];
                else
                    vmi[i] = 0x20;
            }

            // checksum
            vmi[0] &= 0x53;// S
            vmi[1] &= 0x45;// E
            vmi[2] &= 0x47;// G
            vmi[3] &= 0x41;// A

            // description
            Array.Copy(description, 0, vmi, 4, 32);

            // timestamp
            vmi[0x44] = year[0];
            vmi[0x45] = year[1];
            vmi[0x46] = (byte)now.Month;
            vmi[0x47] = (byte)now.Day;
            vmi[0x48] = (byte)now.Hour;
            vmi[0x49] = (byte)now.Minute;
            vmi[0x4A] = (byte)now.Second;
            vmi[0x4B] = (byte)now.DayOfWeek;

            // VMI version
            vmi[0x4C] = 0;
            vmi[0x4D] = 1;

            // File number
            vmi[0x4E] = 1;
            vmi[0x4F] = 0;

            // vms filename without extension
            Array.Copy(vms_filename_buffer, 0, vmi, 0x50, Math.Min(8, vms_filename_buffer.Length));

            // vmu FAT Filename
            Array.Copy(vmu_FAT_filename_buffer, 0, vmi, 0x58, Math.Min(12, vmu_FAT_filename_buffer.Length));

            // file flags
            vmi[0x64] = 0x00;
            vmi[0x65] = 0x00;

            // unknown
            vmi[0x66] = 0x00;
            vmi[0x67] = 0x00;

            // vms file size
            UpdateFilesizeInVMI(vmi, filesize);

            return vmi;
        }

        private static void UpdateFilesizeInVMI(byte[] vmi, int filesize) {
            byte[] filesize_buffer = BitConverter.GetBytes(filesize);
            Array.Copy(filesize_buffer, 0, vmi, 0x68, 4);

        }

        private static void UpdateVMI(string vms_path, string FAT_filename, long filesize) {
            string vmi_path;
            string vms_filename = Path.GetFileName(vms_path);

            int index = vms_path.LastIndexOf('.');
            if (index < 0) {
                vmi_path = vms_path + ".vmi";
            } else {
                vmi_path = vms_path.Substring(0, index) + ".vmi";
            }

            byte[] vmi_data;
            FileStream vmi_file = null;

            try {
                vmi_file = new FileStream(vmi_path, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.ReadWrite);
                if (vmi_file.Length < 1) {
                    try {
                        FileInfo fi = new FileInfo(vmi_path);
                        fi.Attributes |= FileAttributes.Hidden;
                    } catch { }

                    vmi_data = CreateVMI(vms_filename, FAT_filename, (int)filesize);
                } else {
                    vmi_data = new byte[vmi_file.Length];
                    vmi_file.Read(vmi_data, 0, vmi_data.Length);
                    UpdateFilesizeInVMI(vmi_data, (int)filesize);
                    vmi_file.Seek(0, SeekOrigin.Begin);
                }

                vmi_file.Write(vmi_data, 0, vmi_data.Length);
                vmi_file.Flush();
                vmi_file.Dispose();
            } catch (Exception e) {
                Console.Error.WriteLine("[WARN] Can not create/update " + vmi_path);
                Console.Error.WriteLine(e);
                Console.Error.WriteLine();
                if (vmi_file != null) vmi_file.Dispose();
            }
        }

        private static string GetVMSPath(string fn) {
            string name = Path.GetFileName(fn);
            if (name.Length > 7) fn = fn.Substring(0, fn.Length - (name.Length - 7));
            fn += ".vms";
            string saves_path = AppDomain.CurrentDomain.BaseDirectory + "saves";
            fn = saves_path + fn.Substring("/vmu".Length).Replace('/', Path.DirectorySeparatorChar);
            return fn;
        }

    }
}
