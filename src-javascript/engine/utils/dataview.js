"use strict";

/**
 * @typedef {object} Packed4bytes
 * @property {number} [value_byte]
 * @property {number} [value_int]
 * @property {number} [value_float]
 * @property {boolean} [value_boolean]
*/

class DataViewEx {

    /**@type {DataView}*/ #dataview;
    /**@type {boolean}*/ #little_endian;
    /**@type {number}*/ #offset;
    /**@type {Uint8Array}*/ #buffer_u8;

    constructor(/**@type {ArrayBuffer}*/buffer, /**@type {number}*/offset, /**@type {number}*/length) {
        this.#buffer_u8 = new Uint8Array(buffer, offset, length)
        this.#dataview = new DataView(buffer, offset, length);
        this.#little_endian = new Uint16Array(new Uint8Array([255, 0]).buffer)[0] == 255;
        this.#offset = 0;
    }

    get Buffer() { return this.#buffer_u8; };
    get Offset() { return this.#offset; };
    get Length() { return this.#buffer_u8.length; };

    getUint32() {
        let value = this.#dataview.getUint32(this.#offset, this.#little_endian);
        this.#offset += Uint32Array.BYTES_PER_ELEMENT;
        return value;
    }
    getInt32() {
        let value = this.#dataview.getInt32(this.#offset, this.#little_endian);
        this.#offset += Int32Array.BYTES_PER_ELEMENT;
        return value;
    }
    getUint16() {
        let value = this.#dataview.getUint16(this.#offset, this.#little_endian);
        this.#offset += Uint16Array.BYTES_PER_ELEMENT;
        return value;
    }
    getInt16() {
        let value = this.#dataview.getInt16(this.#offset, this.#little_endian);
        this.#offset += Int16Array.BYTES_PER_ELEMENT;
        return value;
    }
    getUint8() {
        let value = this.#dataview.getUint8(this.#offset);
        this.#offset += Uint8Array.BYTES_PER_ELEMENT;
        return value;
    }
    getInt8() {
        let value = this.#dataview.getInt8(this.#offset);
        this.#offset += Int8Array.BYTES_PER_ELEMENT;
        return value;
    }
    getFloat32() {
        let value = this.#dataview.getFloat32(this.#offset, this.#little_endian);
        this.#offset += Float32Array.BYTES_PER_ELEMENT;
        return value;
    }
    getFloat64() {
        let value = this.#dataview.getFloat64(this.#offset, this.#little_endian);
        this.#offset += Float64Array.BYTES_PER_ELEMENT;
        return value;
    }
    getArrayBuffer(/**@type {number}*/bytesToRead) {
        let readed = new ArrayBuffer(bytesToRead);
        let dest = new Uint8Array(readed);
        let src = this.#buffer_u8.slice(this.#offset, this.#offset + bytesToRead);

        src.set(dest, 0);
        this.#offset += bytesToRead;

        return readed;
    }
    getPack4() {
        /**@type {Packed4bytes}*/
        let pack4 = {
            value_byte: this.#dataview.getUint32(this.#offset) & 0xFF,
            value_int: this.#dataview.getInt32(this.#offset, this.#little_endian),
            value_float: this.#dataview.getFloat32(this.#offset, this.#little_endian),
            value_boolean: this.#dataview.getUint32(this.#offset, this.#little_endian) != 0x00,
        };

        this.#offset += 4;

        return pack4;
    }
    getInt64() {
        let value = this.#dataview.getBigInt64(this.#offset, this.#little_endian);
        this.#offset += BigInt64Array.BYTES_PER_ELEMENT;
        return Number(value);
    }

    setUint32(/**@type {number}*/value) {
        this.#dataview.setUint32(this.#offset, value, this.#little_endian);
        this.#offset += Uint32Array.BYTES_PER_ELEMENT;
    }
    setInt32(/**@type {number}*/value) {
        this.#dataview.setInt32(this.#offset, value, this.#little_endian);
        this.#offset += Int32Array.BYTES_PER_ELEMENT;
    }
    setUint16(/**@type {number}*/value) {
        this.#dataview.setUint16(this.#offset, value, this.#little_endian);
        this.#offset += Uint16Array.BYTES_PER_ELEMENT;
    }
    setInt16(/**@type {number}*/value) {
        this.#dataview.setInt16(this.#offset, value, this.#little_endian);
        this.#offset += Int16Array.BYTES_PER_ELEMENT;
    }
    setUint8(/**@type {number}*/value) {
        this.#dataview.setUint8(this.#offset, value);
        this.#offset += Uint8Array.BYTES_PER_ELEMENT;
    }
    setInt8(/**@type {number}*/value) {
        this.#dataview.setInt8(this.#offset, value);
        this.#offset += Int8Array.BYTES_PER_ELEMENT;
    }
    setFloat32(/**@type {number}*/value) {
        this.#dataview.setFloat32(this.#offset, value, this.#little_endian);
        this.#offset += Float32Array.BYTES_PER_ELEMENT;
    }
    setFloat64(/**@type {number}*/value) {
        this.#dataview.setFloat64(this.#offset, value, this.#little_endian);
        this.#offset += Float64Array.BYTES_PER_ELEMENT;
    }
    setArrayBuffer(/**@type {ArrayBuffer}*/buffer) {
        let src = new Uint8Array(buffer);
        let dest = this.#buffer_u8.slice(this.#offset, this.#offset + buffer.byteLength);

        src.set(dest, 0);
        this.#offset += buffer.byteLength;
    }
    setPack4(/**@type {Packed4bytes}*/pack4_with_a_single_property) {
        let props = Object.getOwnPropertyNames(pack4_with_a_single_property);

        if (props.length < 1) {
            this.setUint32(0x00);
            return;
        } else if (props.length != 1) {
            throw new Error("the 'Packed4bytes' type must have a single property");
        }

        switch (props[0]) {
            case "value_byte":
                this.setUint32(pack4_with_a_single_property.value_byte & 0xFF);
                return;
            case "value_int":
                this.setInt32(pack4_with_a_single_property.value_int);
                return;
            case "value_float":
                this.setFloat32(pack4_with_a_single_property.value_float);
                return;
            case "value_boolean":
                this.setUint32(pack4_with_a_single_property.value_boolean ? 0x01 : 0x00);
                return;
            default:
                throw new Error("unknown 'Packed4bytes' property: " + props[0]);
        }
    }
    setInt64(/**@type {number}*/value) {
        this.#dataview.setBigInt64(this.#offset, BigInt(value), this.#little_endian);
        this.#offset += BigInt64Array.BYTES_PER_ELEMENT;
    }

    setInt32At(/**@type {number}*/offset, /**@type {number}*/value) {
        this.#dataview.setInt32(offset, value, this.#little_endian);
    }
    setUint16At(/**@type {number}*/offset, /**@type {number}*/value) {
        this.#dataview.setUint16(offset, value, this.#little_endian);
    }

    getInt32At(/**@type {number}*/offset) {
        return this.#dataview.getInt32(offset, this.#little_endian);
    }
    getUint16At(/**@type {number}*/offset) {
        return this.#dataview.getUint16(offset, this.#little_endian);
    }

    Assert() {
        if (this.#offset != this.#buffer_u8.byteLength) {
            throw new Error("Assertion failed: this.#offset != this.#buffer_u8.byteLength");
        }
    }
    Skip(/**@type {number}*/ count) {
        this.#offset += count;
    }

}

