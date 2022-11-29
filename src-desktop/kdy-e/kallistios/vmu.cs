using System;
using Engine.Utils;

namespace KallistiOS.VMU {

    /**
     * @summary  VMU Package type.
     *   Anyone wanting to package a VMU file should create one of these somewhere;
     *   eventually it will be turned into a flat file that you can save using
     *   fs_vmu.
    */
    public struct vmu_pkg_t {
        /** @type {int} Number of data (payload) bytes */
        public int data_len;
        /** @type {byte[]} Payload data */
        public byte[] data;
    }

    public static class vmu {

        /**
         * @summary  Parse an array of uint8s into a vmu_pkg_t.
         *   This function does the opposite of vmu_pkg_build and is used to parse VMU
         *   files read in.
         *   @param {ArrayBuffer} data            The buffer to parse.
         *   @param {vmu_pkg_t} pkg             Where to store the vmu_pkg_t.
         *   @returns {number} -1 On invalid CRC in the data. 0 On success.
        */
        public static int pkg_parse(byte[] data, ref vmu_pkg_t pkg) {

            DataView dataView = new DataView(data);
            if (data.Length < 128) return -1;

            ushort icon_count = dataView.GetUint16(0x40);
            ushort eyecatch_type = dataView.GetUint16(0x44);
            ushort payload_size = dataView.GetUint16(0x48);
            int eyecatch_size = -1;

            switch (eyecatch_type) {
                case 0:
                    eyecatch_size = 0;
                    break;
                case 1:
                    eyecatch_size = 72 * 56 * 2;
                    break;
                case 2:
                    eyecatch_size = 512 + 72 * 56;
                    break;
                case 3:
                    eyecatch_size = 32 + 72 * 56 / 2;
                    break;
                default:
                    Console.Error.WriteLine("[ERROR] vmu_pkg_parse() unknown eyecatch type");
                    return -1;
            }

            int payload_offset = (icon_count * 512) + eyecatch_size + 0x80;

            if ((payload_offset + payload_size) > data.Length) return -1;

            pkg.data_len = payload_size;
            pkg.data = new byte[payload_size];

            Buffer.BlockCopy(data, payload_offset, pkg.data, 0, payload_size);
            return 0;
        }
    }
}
