using System;

namespace Engine.Utils;

public class DataView {

    public readonly byte[] buffer;
    private readonly int start_offset;

    public int ByteLength { get => this.buffer.Length; }
    public int ByteOffset { get => this.start_offset; }

    public DataView(byte[] buffer) : this(buffer, 0) { }

    public DataView(byte[] buffer, int offset) {
        this.buffer = buffer;
        this.start_offset = offset;
    }

    public byte GetUint8(int offset) {
        return this.buffer[offset + this.start_offset];
    }

    public ushort GetUint16(int offset) {
        return BitConverter.ToUInt16(this.buffer, offset + this.start_offset);
    }

    public int GetInt32(int offset) {
        return BitConverter.ToInt32(this.buffer, offset + this.start_offset);
    }

    public long GetInt64(int offset) {
        return BitConverter.ToInt64(this.buffer, offset + this.start_offset);
    }

    public float GetFloat32(int offset) {
        return BitConverter.ToSingle(this.buffer, offset + this.start_offset);
    }

    public double GetFloat64(int offset) {
        return BitConverter.ToDouble(this.buffer, offset + this.start_offset);
    }

    public uint GetUint32(int offset) {
        return BitConverter.ToUInt32(this.buffer, offset + this.start_offset);
    }

    public void Read(byte[] buffer, int offset, int bytesToRead) {
        Buffer.BlockCopy(this.buffer, offset + this.start_offset, buffer, 0, bytesToRead);
    }

    public void SetUint8(int offset, byte value) {
        this.buffer[this.start_offset + offset] = value;
    }

    public void SetUint16(int offset, ushort value) {
        offset += this.start_offset;
        this.buffer[offset + 0] = (byte)((value >> 0) & 0xFF);
        this.buffer[offset + 1] = (byte)((value >> 8) & 0xFF);
    }

    public void SetInt64(int offset, long value) {
        offset += this.start_offset;
        this.buffer[offset + 0] = (byte)((value >> 0) & 0xFF);
        this.buffer[offset + 1] = (byte)((value >> 8) & 0xFF);
        this.buffer[offset + 2] = (byte)((value >> 16) & 0xFF);
        this.buffer[offset + 3] = (byte)((value >> 24) & 0xFF);
        this.buffer[offset + 4] = (byte)((value >> 32) & 0xFF);
        this.buffer[offset + 5] = (byte)((value >> 40) & 0xFF);
        this.buffer[offset + 6] = (byte)((value >> 48) & 0xFF);
        this.buffer[offset + 7] = (byte)((value >> 56) & 0xFF);

    }

    public void SetFloat32(int offset, float value) {
        offset += this.start_offset;
        byte[] buff = BitConverter.GetBytes(value);
        this.buffer[offset + 0] = buff[0];
        this.buffer[offset + 1] = buff[1];
        this.buffer[offset + 2] = buff[2];
        this.buffer[offset + 3] = buff[3];

    }

    public void SetFloat64(int offset, double value) {
        offset += this.start_offset;
        byte[] buff = BitConverter.GetBytes(value);
        this.buffer[offset + 0] = buff[0];
        this.buffer[offset + 1] = buff[1];
        this.buffer[offset + 2] = buff[2];
        this.buffer[offset + 3] = buff[3];
        this.buffer[offset + 4] = buff[4];
        this.buffer[offset + 5] = buff[5];
        this.buffer[offset + 6] = buff[6];
        this.buffer[offset + 7] = buff[7];

    }

    public void SetUint32(int offset, uint value) {
        offset += this.start_offset;
        this.buffer[offset + 0] = (byte)((value >> 0) & 0xFF);
        this.buffer[offset + 1] = (byte)((value >> 8) & 0xFF);
        this.buffer[offset + 2] = (byte)((value >> 16) & 0xFF);
        this.buffer[offset + 3] = (byte)((value >> 24) & 0xFF);

    }

    public void SetInt32(int offset, int value) {
        offset += this.start_offset;
        this.buffer[offset + 0] = (byte)((value >> 0) & 0xFF);
        this.buffer[offset + 1] = (byte)((value >> 8) & 0xFF);
        this.buffer[offset + 2] = (byte)((value >> 16) & 0xFF);
        this.buffer[offset + 3] = (byte)((value >> 24) & 0xFF);

    }

    public void Write(byte[] buffer, int offset, int bytesToWrite) {
        Buffer.BlockCopy(buffer, 0, this.buffer, offset + this.start_offset, bytesToWrite);
    }

}
