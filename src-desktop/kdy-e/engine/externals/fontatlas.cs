using System;
using System.Runtime.InteropServices;
using Engine.Utils;

namespace Engine.Externals {

    public class FontCharMap {
        public FontCharData[] char_array;
        public int char_array_size;
        public IntPtr texture;
        public ushort texture_width;
        public ushort texture_height;
        public uint texture_byte_size;
        public short ascender;
    }

    public struct FontCharDataKerning {
        public int codepoint;
        public int x;
    }

    public struct FontCharDataAtlasEntry {
        public ushort x;
        public ushort y;
    }

    public class FontCharData {
        public int codepoint;
        public short offset_x;
        public short offset_y;
        public short advancex;
        public int width;
        public int height;
        public FontCharDataKerning[] kernings;
        public int kernings_size;
        public FontCharDataAtlasEntry atlas_entry;
        public bool has_entry;
    }




    public static class FontAtlas {

        private const string DLL = "kdy_fontatlas";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void fontatlas_enable_sdf(bool enable);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr fontatlas_init(IntPtr font_data, int font_data_size);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]// FontAtlas** pointer_to_fontatlas_pointer
        private static extern void fontatlas_destroy(ref IntPtr pointer_to_fontatlas_pointer);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]// uint32_t characters_to_add
        private static extern IntPtr fontatlas_atlas_build(IntPtr fontatlas, byte font_height, sbyte gaps, IntPtr characters_to_add);

        /*[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        private static extern void fontatlas_atlas_destroy_texture_only(IntPtr fontcharmap);*/

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]//FontCharMap** pointer_to_fontcharmap_pointer
        private static extern void fontatlas_atlas_destroy(ref IntPtr pointer_to_fontcharmap_pointer);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, EntryPoint = "fontatlas_get_version")]
        private static extern IntPtr __fontatlas_get_version();


        public static IntPtr kdmyEngine_allocate(byte[] fontdata) {
            IntPtr ptr = Marshal.AllocHGlobal(fontdata.Length);
            Marshal.Copy(fontdata, 0, ptr, fontdata.Length);
            return ptr;
        }
        public static IntPtr kdmyEngine_allocate(int[] fontdata) {
            IntPtr ptr = Marshal.AllocHGlobal(fontdata.Length);
            Marshal.Copy(fontdata, 0, ptr, fontdata.Length);
            return ptr;
        }
        public static void kdmyEngine_deallocate(IntPtr ptr) {
            Marshal.FreeHGlobal(ptr);
        }

        public static FontCharMap kdmyEngine_parseFontCharMap(IntPtr fontcharmap_ptr) {
            if (fontcharmap_ptr == IntPtr.Zero) return null;

            FontCharMapUnparsed unparsed_fontcharmap = (FontCharMapUnparsed)Marshal.PtrToStructure(
                fontcharmap_ptr, typeof(FontCharMapUnparsed)
            );


            FontCharMap fontcharmap = new FontCharMap() {
                char_array = null,
                char_array_size = unparsed_fontcharmap.char_array_size,
                texture = unparsed_fontcharmap.texture,
                texture_width = unparsed_fontcharmap.texture_width,
                texture_height = unparsed_fontcharmap.texture_height,
                texture_byte_size = unparsed_fontcharmap.texture_byte_size,
                ascender = unparsed_fontcharmap.ascender
            };


            IntPtr char_array_ptr = unparsed_fontcharmap.char_array;
            fontcharmap.char_array = fontcharmap.char_array_size > 0 ? new FontCharData[fontcharmap.char_array_size] : null;
            for (int i = 0 ; i < fontcharmap.char_array_size ; i++) {
                FontCharDataUnparsed unparsed_fontchardata = (FontCharDataUnparsed)Marshal.PtrToStructure(
                    char_array_ptr, typeof(FontCharDataUnparsed)
                );

                fontcharmap.char_array[i] = new FontCharData() {
                    codepoint = unparsed_fontchardata.codepoint,
                    offset_x = unparsed_fontchardata.offset_x,
                    offset_y = unparsed_fontchardata.offset_y,
                    advancex = unparsed_fontchardata.advancex,
                    width = unparsed_fontchardata.width,
                    height = unparsed_fontchardata.height,
                    kernings = null,
                    kernings_size = unparsed_fontchardata.kernings_size,
                    atlas_entry = {
                        x= unparsed_fontchardata.atlas_entry.x,
                        y= unparsed_fontchardata.atlas_entry.y
                    },
                    has_entry = unparsed_fontchardata.has_atlas_entry != 0
                };

                IntPtr kernings_ptr = unparsed_fontchardata.kernings;
                char_array_ptr += Marshal.SizeOf(typeof(FontCharDataUnparsed));

                if (fontcharmap.char_array[i].kernings_size > 0) {
                    fontcharmap.char_array[i].kernings = new FontCharDataKerning[fontcharmap.char_array[i].kernings_size];
                    for (int j = 0 ; j < fontcharmap.char_array[i].kernings_size ; j++) {
                        FontCharDataKerningUnparsed unparsed_kerning = (FontCharDataKerningUnparsed)Marshal.PtrToStructure(
                            kernings_ptr, typeof(FontCharDataKerningUnparsed)
                        );
                        fontcharmap.char_array[i].kernings[j] = new FontCharDataKerning() {
                            codepoint = unparsed_kerning.codepoint,
                            x = unparsed_kerning.x
                        };
                        kernings_ptr += Marshal.SizeOf(typeof(FontCharDataKerningUnparsed));
                    }
                }
            }

            return fontcharmap;
        }


        public static IntPtr _fontatlas_init(IntPtr font_data, int font_data_size) {
            return fontatlas_init(font_data, font_data_size);
        }

        public static void _fontatlas_destroy(IntPtr fontatlas) {
            fontatlas_destroy(ref fontatlas);
        }

        public static IntPtr _fontatlas_atlas_build(IntPtr fontatlas, byte font_height, sbyte gaps, IntPtr characters_to_add) {
            return fontatlas_atlas_build(fontatlas, font_height, gaps, characters_to_add);
        }

        public static void _fontatlas_atlas_destroy(IntPtr fontcharmap) {
            fontatlas_atlas_destroy(ref fontcharmap);
        }

        public static IntPtr _fontatlas_atlas_build_complete(IntPtr fontatlas, byte font_height, sbyte gaps) {
            Grapheme grapheme = new Grapheme();

            int index;
            int count = 0;

            index = 0;
            while (StringUtils.GetCharacterCodepoint(FONTATLAS_BASE_LIST_COMMON, index, grapheme)) {
                count++;
                index += grapheme.size;
            }

            index = 0;
            while (StringUtils.GetCharacterCodepoint(FONTATLAS_BASE_LIST_EXTENDED, index, grapheme)) {
                count++;
                index += grapheme.size;
            }

            int[] codepoints = new int[count + 1/*null terminator*/];
            count = 0;

            index = 0;
            while (StringUtils.GetCharacterCodepoint(FONTATLAS_BASE_LIST_COMMON, index, grapheme)) {
                index += grapheme.size;
                codepoints[count++] = grapheme.code;
            }

            index = 0;
            while (StringUtils.GetCharacterCodepoint(FONTATLAS_BASE_LIST_EXTENDED, index, grapheme)) {
                index += grapheme.size;
                codepoints[count++] = grapheme.code;
            }

            GCHandle hnd = GCHandle.Alloc(codepoints, GCHandleType.Pinned);
            IntPtr ret = fontatlas_atlas_build(fontatlas, font_height, gaps, hnd.AddrOfPinnedObject());
            hnd.Free();

            return ret;
        }

        public static string fontatlas_get_version() {
            IntPtr ptr = __fontatlas_get_version();
            return Marshal.PtrToStringAnsi(ptr);
        }

        private const string FONTATLAS_BASE_LIST_COMMON = "  !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        private const string FONTATLAS_BASE_LIST_EXTENDED = "¿¡¢¥¦¤§¨©ª«»¬®¯°±´³²¹ºµ¶·ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßŸàáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþßÿ";


        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct FontCharDataKerningUnparsed {
            public int codepoint;
            public int x;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct FontCharDataAtlasEntryUnparsed {
            public ushort x;
            public ushort y;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct FontCharMapUnparsed {
            public IntPtr char_array;
            public int char_array_size;
            public IntPtr texture;
            public ushort texture_width;
            public ushort texture_height;
            public uint texture_byte_size;
            public short ascender;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct FontCharDataUnparsed {
            public int codepoint;
            public short offset_x;
            public short offset_y;
            public short advancex;
            public int width;
            public int height;
            public IntPtr kernings;
            public int kernings_size;
            public FontCharDataAtlasEntryUnparsed atlas_entry;
            public byte has_atlas_entry;
        }

    }

}
