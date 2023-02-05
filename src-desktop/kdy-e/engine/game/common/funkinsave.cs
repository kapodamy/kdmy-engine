using System;
using System.Diagnostics;
using System.Text;
using Engine.Utils;
using KallistiOS.MAPLE;
using KallistiOS.VFS;
using KallistiOS.VMU;

namespace Engine.Game {

    public static class FunkinSave {

        private const int MAX_STRING_SIZE = 32;
        private const int SAVEDATA_VERSION = 0x01;
        private const int VMS_HEADER_OFFSET_CRC16 = 0x0046;
        private const int VMS_HEADER_OFFSET_LENGTH = 0x0048;
        private static readonly byte[] VMS_HEADER = new byte[] {
            // space-padded short description (FnF savedata    )
            0x46, 0x6E, 0x46, 0x20, 0x73, 0x61, 0x76, 0x65, 0x64, 0x61, 0x74, 0x61, 0x20, 0x20, 0x20, 0x20,
            // space-padded long description (Friday Night Funkin' kdmy-engine)
            0x46, 0x72, 0x69, 0x64, 0x61, 0x79, 0x20, 0x4E, 0x69, 0x67, 0x68, 0x74, 0x20, 0x46, 0x75, 0x6E,
            0x6B, 0x69, 0x6E, 0x27, 0x20, 0x6B, 0x64, 0x6D, 0x79, 0x2D, 0x65, 0x6E, 0x67, 0x69, 0x6E, 0x65,
            // null-padded application id (ninjamuffin99FNF)
            0x6E, 0x69, 0x6E, 0x6A, 0x61, 0x6D, 0x75, 0x66, 0x66, 0x69, 0x6E, 0x39, 0x39, 0x46, 0x4E, 0x46,
            // numberinicons + icon animation speed + eyecatch type + CRC16 + payload size
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // reserved (unused space, all zeros)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // color palette (argb4444)
            0x00, 0x00, 0x83, 0xF3, 0x6C, 0xF1, 0x11, 0xFA, 0xB2, 0xF3, 0x44, 0xFE, 0x9E, 0xF0, 0x25, 0xF6,
            0x18, 0xFB, 0x22, 0xF2, 0x00, 0xF0, 0x14, 0xFF, 0xC8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            // bitmap (32x32 4bit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x11, 0x41, 0x10, 0x00, 0x00, 0x35, 0x33, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x01, 0x14, 0x44, 0x11, 0x03, 0x33, 0x35, 0x53, 0x30,
            0x00, 0x00, 0x00, 0x22, 0x26, 0x22, 0x20, 0x01, 0x44, 0x44, 0x41, 0x03, 0x33, 0x55, 0x55, 0x33,
            0x00, 0x00, 0x00, 0x26, 0x66, 0x66, 0x20, 0x01, 0x11, 0x41, 0x11, 0x03, 0x33, 0x35, 0x53, 0x30,
            0x00, 0x00, 0x00, 0x22, 0x66, 0x62, 0x20, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x35, 0x33, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x26, 0x22, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x07, 0x77, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x77, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x07, 0x78, 0x87, 0x77, 0x70, 0x00, 0x09, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xA0,
            0x77, 0x88, 0x88, 0x77, 0x70, 0x00, 0x95, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xAA,
            0x07, 0x78, 0x87, 0x77, 0x70, 0x0A, 0xAB, 0xAB, 0xBA, 0x2B, 0xBB, 0xBB, 0x55, 0x55, 0x53, 0x3A,
            0x00, 0x77, 0x87, 0x00, 0x00, 0xA6, 0xA2, 0x6A, 0xB6, 0x2A, 0xBB, 0xBB, 0xB5, 0x55, 0x33, 0x3A,
            0x00, 0x07, 0x77, 0x00, 0x0A, 0x66, 0x26, 0x62, 0xA2, 0x62, 0xAB, 0xBB, 0xBB, 0x53, 0x33, 0x3A,
            0x00, 0x00, 0x00, 0x00, 0xA6, 0x66, 0x66, 0x66, 0x26, 0x66, 0x2A, 0xBB, 0xBB, 0x33, 0x33, 0x3A,
            0x00, 0x00, 0x00, 0x0A, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x22, 0x9B, 0xBB, 0x33, 0x33, 0x3A,
            0x00, 0x00, 0x00, 0xA6, 0x62, 0x22, 0xA6, 0x66, 0x66, 0x66, 0x62, 0xAB, 0xBB, 0x33, 0x33, 0x3A,
            0x00, 0x00, 0x0A, 0x22, 0x20, 0x0A, 0x66, 0x66, 0x66, 0x66, 0x66, 0x29, 0xBB, 0x33, 0x33, 0x3A,
            0x00, 0x00, 0x00, 0x00, 0x00, 0xA6, 0x66, 0x66, 0xA6, 0x66, 0x66, 0x2A, 0x9B, 0x33, 0x33, 0x3A,
            0x00, 0x00, 0x00, 0x00, 0x9A, 0x66, 0x66, 0x66, 0x6A, 0x66, 0x66, 0xA6, 0x9B, 0x33, 0x33, 0x3A,
            0x00, 0x00, 0x00, 0x0A, 0x66, 0x66, 0x66, 0x99, 0x99, 0xA9, 0x9A, 0x99, 0xC9, 0xBA, 0xAA, 0xAA,
            0x00, 0x00, 0x00, 0xA2, 0x22, 0x22, 0x69, 0xCC, 0xCA, 0xCA, 0xAC, 0xAC, 0xCC, 0x26, 0x66, 0x62,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0xCC, 0xCA, 0xCC, 0xCC, 0xAC, 0xCC, 0xC2, 0x66, 0x62,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xCC, 0xCA, 0xCC, 0xCC, 0xAC, 0xCC, 0xCC, 0x26, 0x2A,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCC, 0xCA, 0xCC, 0xCC, 0xAC, 0xCC, 0xCC, 0xC2, 0xA0,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCA, 0xCC, 0xC2, 0xA0,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xAC, 0xCC, 0xCA, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCC, 0xCC, 0xAA, 0xAA, 0xA9, 0xCC, 0xCC, 0xA0, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00
        };

        private const byte DIRECTIVE_TYPE_UNLOCK = 0x00;
        private const ushort MAX_INDEXED_NAME = 0xFFFF - 1;


        public static uint maple_port;
        public static uint maple_unit;
        public static LinkedList<Setting> settings;
        public static LinkedList<Directive> directives;
        public static LinkedList<string> weeks_names;
        public static LinkedList<string> difficulty_names;
        public static LinkedList<Progress> progress;
        public static LinkedList<Storage> storages;

        public static int last_played_week_index;
        public static int last_played_difficulty_index;


        /**
         * This hold all information loaded from the savefile stored in the Sega Dreamcast VMU
         */
        static FunkinSave() {
            FunkinSave.maple_port = 0;
            FunkinSave.maple_unit = 1;

            FunkinSave.settings = new LinkedList<Setting>();
            FunkinSave.directives = new LinkedList<Directive>();
            FunkinSave.weeks_names = new LinkedList<string>();
            FunkinSave.difficulty_names = new LinkedList<string>();
            FunkinSave.progress = new LinkedList<Progress>();
            FunkinSave.storages = new LinkedList<Storage>();

            FunkinSave.last_played_week_index = -1;
            FunkinSave.last_played_difficulty_index = -1;
        }


        public static int ReadFromVMU() {


            // step 1: check if the VMU is inserted at the given port+slot
            if (FunkinSave.InternalIsVMUMissing()) return 1;

            // step 2: open savedata file
            string vmu_path = FunkinSave.InternalGetVMUPath(FunkinSave.maple_port, FunkinSave.maple_unit);
            int vms_file = fs.open(vmu_path, VFSOpenMode.O_RDONLY);

            if (vms_file == -1) {
                Console.WriteLine("[INFO] funkinsave_read_from_vmu() no savedata present in " + vmu_path);
                //free(vmu_path);
                return 2;
            }

            // step 3:  read the whole VMS file
            int readed = 0;
            long total = fs.total(vms_file);
            byte[] vms_data = new byte[total];

            if (total < 0) {
                // something bad happen
                Console.Error.WriteLine("[ERROR] fs_total() on " + vmu_path + " failed");
                //free(vmu_path);
                //free(vmu_data);
                return 3;
            }

            while (readed < total) {
                int read = fs.read(vms_file, vms_data, readed, (int)total);
                if (read < 0) {
                    // something bad happen
                    //free(vmu_path);
                    //free(vmu_data);
                    fs.close(vms_file);
                    return 3;
                }
                if (read == 0) break;// EOF was reached
                readed += read;
            }

            fs.close(vms_file);

            if (readed != total) {
                //free(vmu_path);
                //free(vmu_data);
                Console.Error.WriteLine("[ERROR] failed to read the VMS file at " + vmu_path);
                return 3;
            }

            // step 4: parse VMS header using vmu_pkg_parse() public static  from KallistiOS (required for CRC16 check)
            vmu_pkg_t vmu_pkg = new vmu_pkg_t();

            if (vmu.pkg_parse(vms_data, ref vmu_pkg) != 0) {
                Console.Error.WriteLine("[WARN] vmu_pkg_parse() returned failed on: " + vmu_path);

                //free(vmu_path);
                //free(vmu_data);
                return 4;
            }

            // step 5: parse savedata
            int offset = 0;
            DataView savedata = new DataView(vmu_pkg.data, 0/*, vmu_pkg.data_len*/);
            int version = savedata.GetUint8(0); offset += 1;
            offset++;// reserved space

            // version check
            if (version != FunkinSave.SAVEDATA_VERSION) {
                //free(vmu_path);
                //free(vmu_data);
                return 5;
            }

            // clear in-memory savedata
            //free(vmu_path);
            FunkinSave.InternalClearSavedata();

            ushort settings_count = savedata.GetUint16(offset); offset += 2;
            ushort directives_count = savedata.GetUint16(offset); offset += 2;
            ushort week_names_table_size = savedata.GetUint16(offset); offset += 2;
            ushort difficulty_names_table_size = savedata.GetUint16(offset); offset += 2;
            ushort progress_count = savedata.GetUint16(offset); offset += 2;
            ushort storages_count = savedata.GetUint16(offset); offset += 2;
            ushort last_played_week_name_index_in_table = savedata.GetUint16(offset); offset += 2;
            ushort last_played_difficulty_name_index_in_table = savedata.GetUint16(offset); offset += 2;

            for (int i = 0 ; i < settings_count ; i++) {
                ushort id = savedata.GetUint16(offset); offset += 2;
                bool is_integer = FunkinSave.InternalIsSettingInteger(id);
                long value_long = 0; double value_double = 0;

                if (is_integer) value_long = savedata.GetInt64(offset);
                else value_double = savedata.GetFloat64(offset);
                offset += 8;

                FunkinSave.settings.AddItem(new Setting() { id = id, value_long = value_long, value_double = value_double });
            }

            for (int i = 0 ; i < directives_count ; i++) {
                byte type = savedata.GetUint8(offset); offset++;
                string name; offset += FunkinSave.InternalReadString(savedata, offset, out name);
                double value = savedata.GetFloat64(offset); offset += 8;

                Debug.Assert(name != null && name.Length > 0);
                FunkinSave.directives.AddItem(new Directive() { type = type, value = value, name = name });
            }

            for (int i = 0 ; i < week_names_table_size ; i++) {
                string name; offset += FunkinSave.InternalReadString(savedata, offset, out name);

                Debug.Assert(name != null && name.Length > 0);
                FunkinSave.weeks_names.AddItem(name);
            }

            for (int i = 0 ; i < difficulty_names_table_size ; i++) {
                string name; offset += FunkinSave.InternalReadString(savedata, offset, out name);

                Debug.Assert(name != null && name.Length > 0);
                FunkinSave.difficulty_names.AddItem(name);
            }

            for (int i = 0 ; i < progress_count ; i++) {
                ushort week_name_index_in_table = savedata.GetUint16(offset); offset += 2;
                ushort difficulty_name_index_in_table = savedata.GetUint16(offset); offset += 2;
                long score = savedata.GetInt64(offset); offset += 8;

                Debug.Assert(week_name_index_in_table < week_names_table_size);
                Debug.Assert(difficulty_name_index_in_table < difficulty_names_table_size);

                FunkinSave.progress.AddItem(new Progress() {
                    week_id = week_name_index_in_table,
                    difficulty_id = difficulty_name_index_in_table,
                    score = score
                });
            }

            for (int i = 0 ; i < storages_count ; i++) {
                ushort week_name_index_in_table = savedata.GetUint16(offset); offset += 2;
                string name; offset += FunkinSave.InternalReadString(savedata, offset, out name);
                uint data_size = savedata.GetUint32(offset); offset += 4;

                Debug.Assert(week_name_index_in_table < week_names_table_size);
                Debug.Assert(data_size > 0);
                Debug.Assert((offset + data_size) <= total);

                byte[] data = new byte[data_size];
                savedata.Read(data, offset, (int)data_size);
                offset += (int)data_size;

                FunkinSave.storages.AddItem(new Storage() {
                    week_id = week_name_index_in_table,
                    name = name,
                    data_size = data_size,
                    data = data
                });
            }

            if (last_played_week_name_index_in_table == 0xFFFF)
                FunkinSave.last_played_week_index = -1;
            else
                FunkinSave.last_played_week_index = last_played_week_name_index_in_table;

            if (last_played_difficulty_name_index_in_table == 0xFFFF)
                FunkinSave.last_played_difficulty_index = -1;
            else
                FunkinSave.last_played_difficulty_index = last_played_difficulty_name_index_in_table;

            // done
            //free(savedata);
            return 0;
        }

        public static int WriteToVMU() {


            /*
                savedata layout:
                        {
                            version,                        // 1 byte
                            reserved,                       // 1 byte
                            settings_count,                 // 2 bytes
                            directives_count,               // 2 bytes
                            progress_count,                 // 2 bytes
                            storages_count,                 // 2 bytes
                            week_names_table_size,          // 2 bytes
                            difficulty_names_table_size,    // 2 bytes
                            settings: [{id, value}, ...],                       // 10 bytes
                            directives: [{type, name, value}, ...],             // 1 + 8 + n bytes
                            week_names_table: [week_name, ...],                 // n bytes
                            difficulty_names_table: [difficulty_name, ...],     // n bytes
                            progress: [
                                {
                                    week_name_index_in_table,// 2 bytes (index to week_names_table array)
                                    difficulty_name_index_in_table,// 2 bytes (index to difficulty_names_table array)
                                    score,// 8 bytes
                                },
                                ...
                            ],
                            storage: [
                                {
                                    week_name_index_in_table,// 2 bytes
                                    data_size,// 4 bytes
                                    data,// n bytes
                                },
                                ...
                            ]
                        }
            */

            int offset = 0;
            long length = 0;
            int week_names_table_size = 0;
            int difficulty_names_table_size = 0;

            ushort last_played_week_index = 0xFFFF;
            ushort last_played_difficulty_index = 0xFFFF;

            if (FunkinSave.last_played_week_index > 0)
                last_played_week_index = (ushort)FunkinSave.last_played_week_index;
            if (FunkinSave.last_played_difficulty_index >= 0)
                last_played_difficulty_index = (ushort)FunkinSave.last_played_difficulty_index;


            if (FunkinSave.InternalIsVMUMissing()) return 1;

            // step 1: count amount bytes needed
            length += FunkinSave.settings.Count() * (sizeof(ushort) + sizeof(double));
            length += FunkinSave.progress.Count() * (sizeof(ushort) + sizeof(ushort) + sizeof(long));

            foreach (Directive directive in FunkinSave.directives) {
                int string_length = FunkinSave.InternalStringByteLength(directive.name);
                length += string_length + sizeof(long) + sizeof(byte);
            }

            foreach (string week_name in FunkinSave.weeks_names) {
                length += FunkinSave.InternalStringByteLength(week_name);
                week_names_table_size++;
            }

            foreach (string difficulty_name in FunkinSave.difficulty_names) {
                length += FunkinSave.InternalStringByteLength(difficulty_name);
                difficulty_names_table_size++;
            }

            foreach (Storage week_storage in FunkinSave.storages) {
                length += FunkinSave.InternalStringByteLength(week_storage.name);
                length += week_storage.data_size + sizeof(int) + sizeof(short);
            }

            length += 10;//version + reserved + settings_count + directives_count + progress_count + storages_count
            length += 4;// last_played_week_name_index_in_table + last_played_difficulty_name_index_in_table
            length += 4;// week_names_table_size + difficulty_names_table_size

            // step 2: prepare buffer
            Debug.Assert(length < Math2D.MAX_INT32);
            DataView savedata = new DataView(new byte[length]);

            // step 3: build funkinsave header
            savedata.SetUint8(offset, FunkinSave.SAVEDATA_VERSION); offset++;
            savedata.SetUint8(offset, 0x00); offset++;
            savedata.SetUint16(offset, (ushort)FunkinSave.settings.Count()); offset += sizeof(ushort);
            savedata.SetUint16(offset, (ushort)FunkinSave.directives.Count()); offset += sizeof(ushort);
            savedata.SetUint16(offset, (ushort)week_names_table_size); offset += sizeof(ushort);
            savedata.SetUint16(offset, (ushort)difficulty_names_table_size); offset += sizeof(ushort);
            savedata.SetUint16(offset, (ushort)FunkinSave.progress.Count()); offset += sizeof(ushort);
            savedata.SetUint16(offset, (ushort)FunkinSave.storages.Count()); offset += sizeof(ushort);
            savedata.SetUint16(offset, last_played_week_index); offset += sizeof(ushort);
            savedata.SetUint16(offset, last_played_difficulty_index); offset += sizeof(ushort);

            // dump settings
            foreach (Setting setting in FunkinSave.settings) {
                bool is_integer = FunkinSave.InternalIsSettingInteger(setting.id);
                savedata.SetUint16(offset, setting.id); offset += sizeof(ushort);
                if (is_integer) savedata.SetInt64(offset, setting.value_long);
                else savedata.SetFloat64(offset, setting.value_double);
                offset += sizeof(long);
            }

            // dump directives
            foreach (Directive directive in FunkinSave.directives) {
                savedata.SetUint8(offset, directive.type); offset++;
                offset += FunkinSave.InternalDumpString(directive.name, savedata, offset);
                savedata.SetFloat64(offset, directive.value); offset += sizeof(long);
            }

            // dump week names
            foreach (string week_name in FunkinSave.weeks_names) {
                offset += FunkinSave.InternalDumpString(week_name, savedata, offset);
            }

            // dump difficulty names
            foreach (string difficulty_name in FunkinSave.difficulty_names) {
                offset += FunkinSave.InternalDumpString(difficulty_name, savedata, offset);
            }

            // dump week progress
            foreach (Progress week_progress in FunkinSave.progress) {
                savedata.SetUint16(offset, week_progress.week_id); offset += sizeof(ushort);
                savedata.SetUint16(offset, week_progress.difficulty_id); offset += sizeof(ushort);
                savedata.SetInt64(offset, week_progress.score); offset += sizeof(long);
            }

            // dump week storages
            foreach (Storage week_storage in FunkinSave.storages) {
                savedata.SetUint16(offset, week_storage.week_id); offset += sizeof(ushort);
                offset += FunkinSave.InternalDumpString(week_storage.name, savedata, offset);
                savedata.SetUInt32(offset, week_storage.data_size); offset += sizeof(uint);
                savedata.Write(week_storage.data, offset, (int)week_storage.data_size); offset += (int)week_storage.data_size;
            }

            // step 4: check overflows
            Debug.Assert(offset == length, "offset != length");

            // step 5: check available space and delete previous savedata
            int vms_file;
            int available_space = Math2D.MAX_INT32;
            string vmu_path = FunkinSave.InternalGetVMUPath(FunkinSave.maple_port, FunkinSave.maple_unit);
            stat_t vmu_stat = new stat_t() { st_blksize = 0, st_size = 0 };

            if (fs.stat("/vmu", ref vmu_stat, 0x00) == 0) {
                // calculate in bytes the space available
                available_space = vmu_stat.st_blksize * vmu_stat.st_size;
            }

            // read the old savedata size (if exists)
            vms_file = fs.open(vmu_path, VFSOpenMode.O_RDONLY);
            if (vms_file != -1) {
                int existing_size = fs.total(vms_file);
                if (existing_size > 0) available_space += existing_size;
                fs.close(vms_file);
            }

            // the VMU stores data in blocks in 512 bytes, calculates how many blocks is required
            int required_size = (int)(Math.Ceiling((FunkinSave.VMS_HEADER.Length + length) / 512f) * 512f);
            if (required_size > available_space) return 2;// no space left

            // delete savedata
            if (vms_file != -1 && fs.unlink(vmu_path) != 0) {
                //free(vmu_path);
                //free(savedata);
                return 5;
            }

            // create a new savedata
            vms_file = fs.open(vmu_path, VFSOpenMode.O_WRONLY);

            if (vms_file == -1) {
                Console.Error.WriteLine("[ERROR] funkinsave_flush_to_vmu() failed to write to " + vmu_path + ", also the old savedata is lost :(");
                //free(vmu_path);
                //free(savedata);
                return 3;
            }

            // step 6: write funkinsave in the VMU
            DataView headerView = new DataView(FunkinSave.VMS_HEADER);
            headerView.SetInt32(FunkinSave.VMS_HEADER_OFFSET_LENGTH, (int)length);
            headerView.SetUint16(FunkinSave.VMS_HEADER_OFFSET_CRC16, 0x00);

            // step 7: calculate checksum
            uint crc16 = FunkinSave.InternalCalcCRC16(headerView.buffer, headerView.ByteLength, 0x0000);
            crc16 = FunkinSave.InternalCalcCRC16(savedata.buffer, savedata.ByteLength, crc16);
            headerView.SetUint16(FunkinSave.VMS_HEADER_OFFSET_CRC16, (ushort)(crc16 & 0xFFFF));

            // step 9: write in the VMU
            if (fs.write(vms_file, FunkinSave.VMS_HEADER, FunkinSave.VMS_HEADER.Length) == -1) return 4;
            if (fs.write(vms_file, savedata.buffer, (int)length) == -1) return 4;

            // done
            //free(savedata);
            return fs.close(vms_file) == 0 ? 0 : 4;
        }


        public static long GetWeekScore(string week_name, string week_difficult) {
            int week_id = FunkinSave.weeks_names.IndexOf(week_name);
            int difficulty_id = FunkinSave.difficulty_names.IndexOf(week_difficult);
            if (week_id < 0 || difficulty_id < 0) return 0;

            foreach (Progress progress in FunkinSave.progress) {
                if (progress.week_id == week_id && progress.difficulty_id == difficulty_id) {
                    return progress.score;
                }
            }

            return 0;
        }

        public static int GetCompletedCount() {
            return FunkinSave.weeks_names.Count();
        }

        public static string GetLastPlayedWeek() {
            if (FunkinSave.last_played_week_index < 0) return null;
            return FunkinSave.weeks_names.GetByIndex(FunkinSave.last_played_week_index);
        }

        public static string GetLastPlayedDifficult() {
            if (FunkinSave.last_played_difficulty_index < 0) return null;
            return FunkinSave.difficulty_names.GetByIndex(FunkinSave.last_played_difficulty_index);
        }


        public static void CreateUnlockDirective(string name, double value) {
            foreach (Directive directive in FunkinSave.directives) {
                if (directive.type == FunkinSave.DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
                    directive.value = value;
                    return;
                }
            }

            Directive new_directive = new Directive() { name = name, value = value, type = FunkinSave.DIRECTIVE_TYPE_UNLOCK };
            FunkinSave.directives.AddItem(new_directive);
            return;
        }

        public static void DeleteUnlockDirective(string name) {
            foreach (Directive directive in FunkinSave.directives) {
                if (directive.type == FunkinSave.DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
                    //free(directive.name);
                    FunkinSave.directives.RemoveItem(directive);
                    return;
                }
            }
        }

        public static bool ContainsUnlockDirective(string name) {
            if (String.IsNullOrEmpty(name)) return true;

            foreach (Directive directive in FunkinSave.directives) {
                if (directive.type == FunkinSave.DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
                    return true;
                }
            }
            return false;
        }

        public static bool ReadUnlockDirective(string name, out double value) {
            foreach (Directive directive in FunkinSave.directives) {
                if (directive.type == FunkinSave.DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
                    value = directive.value;
                    return true;
                }
            }
            value = Double.NaN;
            return false;
        }


        public static void SetLastPlayed(string week_name, string difficulty_name) {
            int week_id = FunkinSave.InternalNameIndex(FunkinSave.weeks_names, week_name);
            int difficulty_id = FunkinSave.InternalNameIndex(FunkinSave.difficulty_names, difficulty_name);
            if (week_id >= 0) FunkinSave.last_played_week_index = week_id;
            if (difficulty_id >= 0) FunkinSave.last_played_difficulty_index = difficulty_id;
        }

        public static void SetWeekScore(string week_name, string difficulty_name, long score) {
            int week_id = FunkinSave.InternalNameIndex(FunkinSave.weeks_names, week_name);
            int difficulty_id = FunkinSave.InternalNameIndex(FunkinSave.difficulty_names, difficulty_name);

            if (week_id < 0 || difficulty_id < 0) return;

            FunkinSave.progress.AddItem(new Progress() { week_id = (ushort)week_id, difficulty_id = (ushort)difficulty_id, score = score });
        }

        public static void SetSetting(ushort setting_id, long setting_value) {
            foreach (Setting setting in FunkinSave.settings) {
                if (setting.id == setting_id) {
                    setting.value_long = setting_value;
                    return;
                }
            }
            FunkinSave.settings.AddItem(new Setting() { id = setting_id, value_long = setting_value });
        }
        public static void SetSetting(ushort setting_id, double setting_value) {
            foreach (Setting setting in FunkinSave.settings) {
                if (setting.id == setting_id) {
                    setting.value_double = setting_value;
                    return;
                }
            }
            FunkinSave.settings.AddItem(new Setting() { id = setting_id, value_double = setting_value });
        }


        public static bool StorageSet(string week_name, string name, byte[] data, uint data_size) {
            int week_id;

            if (week_name == null) {
                week_id = FunkinSave.MAX_INDEXED_NAME;
            } else {
                week_id = FunkinSave.InternalNameIndex(FunkinSave.weeks_names, week_name);
                if (week_id < 0) return false;
            }

            Storage storage = null;

            foreach (Storage s in FunkinSave.storages) {
                if (s.week_id == week_id && s.name == name) {
                    storage = s;
                    break;
                }
            }

            if (data_size < 1) {
                if (storage != null) {
                    FunkinSave.storages.RemoveItem(storage);
                    //free(storage.data);
                    //free(storage);
                }
                return true;
            }

            if (storage == null) storage = new Storage();

            data = CloneUtils.CloneArray(data, (int)data_size);

            //free(storage.data);
            storage.data = data;
            storage.data_size = data_size;
            storage.week_id = (ushort)week_id;

            return true;
        }

        public static uint StorageGet(string week_name, string name, out byte[] data) {
            int week_id = week_name == null ? FunkinSave.MAX_INDEXED_NAME : FunkinSave.weeks_names.IndexOf(name);

            foreach (Storage storage in FunkinSave.storages) {
                if (storage.week_id == week_id && storage.name == name) {
                    data = storage.data;
                    return storage.data_size;
                }
            }

            data = null;
            return 0;
        }


        public static bool HasSavedataInVMU(uint port, uint unit) {
            maple_device_t dev = maple.enum_dev(port, unit);
            if (dev == null) return false;

            //
            // There no such thing like fs_file_exists, try open the file
            //
            string vmu_path = FunkinSave.InternalGetVMUPath(port, unit);
            int vms_file = fs.open(vmu_path, VFSOpenMode.O_RDONLY);

            bool found = vms_file != -1;
            fs.close(vms_file);

            //free(vmu_path);
            return found;
        }


        public static void SetVMU(uint port, uint unit) {
            FunkinSave.maple_port = port;
            FunkinSave.maple_unit = unit;
        }


        private static int InternalStringByteLength(string str) {
            int string_length = StringUtils.ByteLength(str);
            return Math.Min(string_length, FunkinSave.MAX_STRING_SIZE);
        }

        private static int InternalDumpString(string str, DataView buffer, int offset) {
            byte[] buf = Encoding.UTF8.GetBytes(str);
            int length = Math.Min(buf.Length, FunkinSave.MAX_STRING_SIZE);
            buffer.Write(buf, offset, length);
            if (buf.Length < FunkinSave.MAX_STRING_SIZE) buffer.SetUint8(offset + length, 0x00);
            return length + 1;
        }

        private static void InternalClearSavedata() {
            FunkinSave.settings.Clear(/*free*/);

            //foreach (Directive directive in FunkinSave.directives) free(directive.name);
            FunkinSave.directives.Clear(/*free*/);

            FunkinSave.difficulty_names.Clear(/*free*/);

            FunkinSave.weeks_names.Clear(/*free*/);

            FunkinSave.progress.Clear(/*free*/);

            //foreach (Storage week_storage in FunkinSave.storages) //free(week_storage.data);
            FunkinSave.storages.Clear(/*free*/);

            FunkinSave.last_played_week_index = -1;
            FunkinSave.last_played_difficulty_index = -1;
        }

        private static string InternalGetVMUPath(uint port, uint unit) {
            char port_name = (char)(0x61 + port);
            char slot_name = (char)(0x30 + unit);

            // Important: the vms filename can not be longer than 12 bytes
            return "/vmu/" + port_name + "" + slot_name + "/FNF_KDY_SAVE";
        }

        private static bool InternalIsVMUMissing() {
            maple_device_t device = maple.enum_dev(FunkinSave.maple_port, FunkinSave.maple_unit);
            return device == null || (device.info.functions & MAPLE_FUNC.MEMCARD) == 0x00;
        }

        private static int InternalReadString(DataView buffer, int offset, out string output_string) {
            int string_offset = buffer.ByteOffset + offset;
            byte[] buf = buffer.buffer;
            int length = 0;
            bool null_found = false;

            for (int i = 0 ; i < FunkinSave.MAX_STRING_SIZE ; i++) {
                if (buf[string_offset + i] == 0x00) {
                    null_found = true;
                    break;
                }
                length++;
            }

            Debug.Assert(length > 0);
            output_string = Encoding.UTF8.GetString(buf, string_offset, length);

            if (null_found) length++;
            return length;
        }

        private static bool InternalIsSettingInteger(ushort id) {
            if (id == 0xabcd) return false;// debug only
            return true;
        }

        private static uint InternalCalcCRC16(byte[] buffer, int size, uint crc) {
            //
            // taken from vmu_pkg.c (KallistiOS sourcecode)
            //

            for (int i = 0 ; i < size ; i++) {
                crc ^= (uint)buffer[i] << 8;

                for (int c = 0 ; c < 8 ; c++) {
                    if ((crc & 0x8000) != 0x0000)
                        crc = (crc << 1) ^ 4129;
                    else
                        crc = (crc << 1);
                }
            }

            return crc;
        }

        private static int InternalNameIndex(LinkedList<string> linkedlist, string name) {
            if (name == null) return -1;
            int index = linkedlist.IndexOf(name);
            if (index < 0) {
                index = linkedlist.Count();
                if (index >= FunkinSave.MAX_INDEXED_NAME) {
                    Console.Error.WriteLine("funkinsave_internal_name_index() failed, ran out of indices");
                    return -1;
                }
                linkedlist.AddItem(name);
            }
            return index;
        }


        public class Setting {
            public ushort id;
            public long value_long;
            public double value_double;
        }

        public class Directive {
            public byte type;
            public double value;
            public string name;
        }

        public class Progress {
            public ushort week_id;
            public ushort difficulty_id;
            public long score;
        }

        public class Storage {
            public ushort week_id;
            public string name;
            public uint data_size;
            public byte[] data;
        }

    }

}
