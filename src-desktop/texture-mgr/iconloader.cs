using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;

namespace Engine.Platform {

    public class IconLoader : IDisposable {
        private const PixelFormat FORMAT = PixelFormat.Format32bppArgb;

        private IntPtr raw_data;
        public RawIcon[] icons;

        public IconLoader(byte[] icon_data) {
            if (icon_data == null) throw new ArgumentNullException("icon_data");
            unsafe {
                GCHandle hnd = GCHandle.Alloc(icon_data, GCHandleType.Pinned);
                MemoryStream stream = new MemoryStream(icon_data);
                try {
                    Load((byte*)hnd.AddrOfPinnedObject(), stream);
                } finally {
                    hnd.Free();
                    stream.Dispose();
                }
            }
        }

        public void Dispose() {
            Marshal.FreeHGlobal(raw_data);
            icons = null;
        }


        public unsafe void Load(byte* ptr, MemoryStream stream) {
            // check if the provided data is PNG, JPG, GIF, or TIFF
            if (LoadAsBitmap(stream)) return;

            int offset = sizeof(IconHeader);
            int size = 0;
            IconHeader* header = (IconHeader*)ptr;

            icons = new RawIcon[header->count];

            for (int i = 0, search_offset = offset ; i < header->count ; i++) {
                IconEntry* entry = (IconEntry*)(ptr + search_offset);
                search_offset += sizeof(IconEntry);

                size += entry->width * entry->height * sizeof(uint);
                icons[i].width = entry->width;
                icons[i].height = entry->height;
            }

            raw_data = Marshal.AllocHGlobal(size);

            for (int i = 0, icn_offset = 0 ; i < header->count ; i++) {
                IconEntry* entry = (IconEntry*)(ptr + offset);
                IntPtr icn = icons[i].pixels = new IntPtr((byte*)raw_data + icn_offset);
                int pixel_count = entry->width * entry->height;
                int stride = entry->width * sizeof(uint);

                offset += sizeof(IconEntry);
                icn_offset += stride * entry->height;
                stream.Seek(0, SeekOrigin.Begin);

                using (Bitmap dest_bitmap = new Bitmap(entry->width, entry->height, stride, FORMAT, icn)) {
                    using (Icon icon = new Icon(stream, entry->width, entry->height)) {
                        using (Graphics g = Graphics.FromImage(dest_bitmap)) {
                            g.Clear(Color.Transparent);
                            g.CompositingMode = CompositingMode.SourceCopy;
                            g.CompositingQuality = CompositingQuality.HighQuality;
                            g.DrawIcon(icon, 0, 0);
                        }
                    }
                }

                if (BitConverter.IsLittleEndian)
                    TextureLoader.ToRGBA_FromLittleEndian(pixel_count, (uint*)icn);
                else
                    TextureLoader.ToRGBA_FromBigEndian(pixel_count, (uint*)icn);
            }

        }

        private unsafe bool LoadAsBitmap(MemoryStream stream) {
            Bitmap bitmap;
            try {
                bitmap = new Bitmap(stream);
            } catch {
                stream.Seek(0, SeekOrigin.Begin);
                return false;
            }

            Console.Error.WriteLine(
                "[WARN] IconLoader::LoadAsBitmap() expected ICN file format not an image."
            );

            int pixel_count = bitmap.Width * bitmap.Height;
            int stride = bitmap.Width * sizeof(uint);
            icons = new RawIcon[1];
            raw_data = Marshal.AllocHGlobal(pixel_count * sizeof(uint));

            using (Bitmap dest_bitmap = new Bitmap(bitmap.Width, bitmap.Height, stride, FORMAT, raw_data)) {
                using (Graphics g = Graphics.FromImage(dest_bitmap)) {
                    g.Clear(Color.Transparent);
                    g.CompositingMode = CompositingMode.SourceCopy;
                    g.CompositingQuality = CompositingQuality.HighQuality;
                    g.DrawImage(bitmap, 0, 0, bitmap.Width, bitmap.Height);
                }
            }

            icons[0].width = bitmap.Width;
            icons[0].height = bitmap.Height;
            icons[0].pixels = raw_data;
            bitmap.Dispose();

            if (BitConverter.IsLittleEndian)
                TextureLoader.ToRGBA_FromLittleEndian(pixel_count, (uint*)raw_data);
            else
                TextureLoader.ToRGBA_FromBigEndian(pixel_count, (uint*)raw_data);

            return true;
        }


        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct IconHeader {
            public ushort reserved;
            public ushort type;
            public ushort count;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct IconEntry {
            public byte width;
            public byte height;
            public byte color_count;
            public byte reserved;
            public ushort planes;
            public ushort bit_count;
            public uint bytes_in_res;
            public uint image_offset;

        };

        [StructLayout(LayoutKind.Sequential)]
        public struct RawIcon {
            public int width;
            public int height;
            public IntPtr pixels;
        }

    }

}
