using System;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Engine.Utils;


[StructLayout(LayoutKind.Explicit, Pack = 1, Size = 4)]
public struct Packed4bytes {
    [FieldOffset(0)] public int value_int;
    [FieldOffset(0)] public uint value_uint;

    [FieldOffset(0)] public sbyte value_sbyte;
    [FieldOffset(0)] public byte value_byte;

    [FieldOffset(0)] public short value_short;
    [FieldOffset(0)] public ushort value_ushort;

    [FieldOffset(0)] public bool value_bool;

    [FieldOffset(0)] public float value_float;

    [FieldOffset(0)] private byte byte0;
    [FieldOffset(1)] private byte byte1;
    [FieldOffset(2)] private byte byte2;
    [FieldOffset(3)] private byte byte3;

    public void SelfReadFrom(BinaryReader reader) {
        this.byte0 = reader.ReadByte();
        this.byte1 = reader.ReadByte();
        this.byte2 = reader.ReadByte();
        this.byte3 = reader.ReadByte();
    }

    public void SelfWriteTo(BinaryWriter writer) {
        writer.Write(this.byte0);
        writer.Write(this.byte1);
        writer.Write(this.byte2);
        writer.Write(this.byte3);
    }

    public override string ToString() {
        return
            "0x" +
            this.byte0.ToString("X") +
            this.byte1.ToString("X") +
            this.byte2.ToString("X") +
            this.byte3.ToString("X")
            ;
    }
}


public class DataView {

    private readonly MemoryStream stream;
    private readonly BinaryReader reader;
    private readonly BinaryWriter writer;

    public DataView(byte[] buffer) : this(buffer, 0) { }

    public DataView(byte[] buffer, int offset) {
        this.stream = new MemoryStream(buffer, offset, buffer.Length - offset, true, true);
        this.reader = new BinaryReader(stream, Encoding.UTF8, true);
        this.writer = new BinaryWriter(stream, Encoding.UTF8, true);
    }

    ~DataView() {
        this.writer.Flush();
        this.reader.Dispose();
        this.writer.Dispose();
        this.stream.Dispose();
    }


    public byte[] Buffer => this.stream.GetBuffer();
    public int Offset => (int)this.stream.Position;


    public byte GetUint8() {
        return this.reader.ReadByte();
    }

    public ushort GetUint16() {
        return this.reader.ReadUInt16();
    }

    public int GetInt32() {
        return this.reader.ReadInt32();
    }

    public long GetInt64() {
        return this.reader.ReadInt64();
    }

    public float GetFloat32() {
        return this.reader.ReadSingle();
    }

    public double GetFloat64() {
        return this.reader.ReadDouble();
    }

    public uint GetUint32() {
        return this.reader.ReadUInt32();
    }

    public void Read(byte[] buffer, int bytesToRead) {
        this.reader.Read(buffer, 0, bytesToRead);
    }

    public Packed4bytes GetPack4() {
        Packed4bytes value = default;
        value.SelfReadFrom(this.reader);
        return value;
    }


    public void SetUint8(byte value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void SetUint16(ushort value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void SetInt64(long value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void SetFloat32(float value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void SetFloat64(double value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void SetUint32(uint value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void SetInt32(int value) {
        this.writer.Write(value);
        this.writer.Flush();
    }

    public void Write(byte[] buffer, int bytesToWrite) {
        this.writer.Write(buffer, 0, bytesToWrite);
        this.writer.Flush();
    }

    public void SetPack4(Packed4bytes value) {
        value.SelfWriteTo(this.writer);
        this.writer.Flush();
    }


    public void Assert() {
        if (this.stream.Position != this.stream.Length) {
            throw new Exception("Assertion failed: this.offset != this.buffer.Length");
        }
    }

    public void Skip(int count) {
        this.stream.Seek(count, SeekOrigin.Current);
    }


    public void SetInt32(int offset, int value) {
        int orig = (int)this.stream.Position;
        this.writer.Seek(offset, SeekOrigin.Begin);

        this.writer.Write(value);
        this.writer.Seek(orig, SeekOrigin.Begin);
    }

    public void SetUint16(int offset, ushort value) {
        int orig = (int)this.stream.Position;
        this.writer.Seek(offset, SeekOrigin.Begin);

        this.writer.Write(value);
        this.writer.Seek(orig, SeekOrigin.Begin);
    }


    public int GetInt32(int offset) {
        long orig = this.stream.Position;
        this.stream.Position = offset;

        int value = this.reader.ReadInt32();
        this.stream.Position = orig;

        return value;
    }

    public ushort GetUint16(int offset) {
        long orig = this.stream.Position;
        this.stream.Position = offset;

        ushort value = this.reader.ReadUInt16();
        this.stream.Position = orig;

        return value;
    }

}
