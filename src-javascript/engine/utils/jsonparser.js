"use string";


const JSON_VALUE_TYPE_UNKNOWN = 0;
const JSON_VALUE_TYPE_NULL = 1;
const JSON_VALUE_TYPE_NUMBER_DOUBLE = 2;
const JSON_VALUE_TYPE_NUMBER_LONG = 3;
const JSON_VALUE_TYPE_STRING = 4;
const JSON_VALUE_TYPE_BOOLEAN = 5;
const JSON_VALUE_TYPE_ARRAY = 6;
const JSON_VALUE_TYPE_OBJECT = 7;


async function json_load_from(src) {
    try {
        src = await fs_get_full_path_and_override(src);
        let ret = await io_request_file(src, IO_REQUEST_JSON);
        src = undefined;
        return ret;
    } catch (e) {
        console.error("json_load_from() failed", e);
        return null;
    }
}

function json_load_from_string(json_sourcecode) {
    if (!json_sourcecode) return null;

    try {
        return JSON.parse(json_sourcecode)
    } catch (e) {
        console.error("json_load_from_string() failed ", e);
        return null;
    }
}
async function json_load_direct(src) {
    try {
        return await io_native_foreground_request(src, IO_REQUEST_JSON);
    } catch (e) {
        console.error("json_load_direct() failed", e);
        return null;
    }
}

function json_destroy(json) {
    // STUB
}

function json_read_object(json, name) {
    if (!json) return null;

    let value = json[name];
    if (value === undefined || value == null) return null;
    L_object_check: {
        if (value instanceof Array) break L_object_check;
        switch (typeof (value)) {
            case "bigint":
            case "number":
            case "string":
            case "boolean":
                break L_object_check;
        }
        return value;
    }
    console.warn(`json:expected object in ${name}`);
    return null;
}

function json_read_array(json, name) {
    if (!json) return null;

    let value = json[name];
    if (value === undefined || value === null) return null;
    if (value instanceof Array) return value;
    console.warn(`json:expected array in ${name}`);
    return null;
}

function json_read_array_length(json_array) {
    return json_array ? json_array.length : -1;
}

function json_read_array_item_number(json_array, index, default_value) {
    if (!json_array) return default_value;

    let value = json_array[index];
    if (value === undefined) return default_value;
    if (typeof (value) == "number") return value;
    console.warn(`json:expected number in ${index}: ${value}`);
    return default_value;
}

function json_read_array_item_boolean(json_array, index, default_value) {
    if (!json_array) return default_value;

    let value = json_array[index];
    if (value === undefined) return default_value;
    if (typeof (value) == "boolean") return value;
    console.warn(`json:expected boolean in ${index}: ${value}`);
    return default_value;
}

function json_read_array_item_string(json_array, index, default_value) {
    if (!json_array) return default_value;

    let value = json_array[index];
    if (value === undefined) return default_value;
    if (typeof (value) == "string") return strdup(value);
    console.warn(`json:expected string in ${index}: ${value}`);
    return default_value;
}

function json_read_array_item_hex(json_array, index, default_value) {
    if (!json_array) return default_value;

    let value = json_array[index];
    if (value === undefined) return default_value;
    if (typeof (value) == "number") return Math.trunc(value);
    if (typeof (value) == "string") return vertexprops_parse_hex2(value, default_value);
    console.warn(`json:expected number or hexadecimal number string in ${index}: ${value}`);
    return default_value;
}

function json_read_array_item_object(json_array, index) {
    if (!json_array) return null;

    let value = json_array[index];
    if (value === undefined || value == null) return null;
    L_object_check: {
        if (value instanceof Array) break L_object_check;
        switch (typeof (value)) {
            case "bigint":
            case "number":
            case "string":
            case "boolean":
                break L_object_check;
        }
        return value;
    }
    console.warn(`json:expected object at index ${index}`);
    return null;
}

function json_read_array_item_array(json_array, index) {
    if (!json_array) return null;

    let value = json_array[index];
    if (value === undefined || value == null) return null;
    if (value instanceof Array) return value;
    console.warn(`json:expected array at index ${index}`);
    return null;
}


function json_read_number(json, name, default_value) {
    if (!json) return default_value;

    let value = json[name];
    if (value === undefined) return default_value;
    if (typeof (value) == "number") return value;
    console.warn(`json:expected number in ${name}: ${value}`);
    return default_value;
}

function json_read_boolean(json, name, default_value) {
    if (!json) return default_value;

    let value = json[name];
    if (value === undefined) return default_value;
    if (typeof (value) == "boolean") return value;
    console.warn(`json:expected boolean in ${name}: ${value}`);
    if (typeof (value) == "number") return value == 1;
    return default_value;
}

function json_read_string(json, name, default_value) {
    if (!json) return default_value;

    let value = json[name];
    if (value === undefined || value === null) return default_value;
    if (typeof (value) == "string") return strdup(value);
    console.warn(`json:expected string in ${name}: ${value}`);
    return default_value;
}

function json_read_hex(json, name, default_value) {
    if (!json) return default_value;

    let value = json[name];
    if (value === undefined) return default_value;
    if (typeof (value) == "number") return Math.trunc(value);
    if (typeof (value) == "string") return vertexprops_parse_hex2(value, default_value);
    console.warn(`json:expected number or hexadecimal number string in ${name}: ${value}`);
    return default_value;
}


function json_has_property(json, name) {
    return json && json[name] !== undefined;
}


function json_has_property_boolean(json, name) {
    return json && typeof (json[name]) == "boolean";
}

function json_has_property_string(json, name) {
    return json && (json[name] === null || typeof (json[name]) == "string");
}

function json_has_property_number(json, name) {
    return json && typeof (json[name]) == "number";
}

function json_has_property_array(json, name) {
    return json && json[name] instanceof Array;
}

function json_has_property_object(json, name) {
    return json && json[name] instanceof Object;
}

function json_has_property_hex(json, name) {
    return json && (typeof (json[name]) == "string" || typeof (json[name]) == "number");
}


function json_is_property_null(json, name) {
    return !json || json[name] === null;
}

function json_is_array_item_null(json_array, index) {
    return !json_array || json_array[index] === null;
}

function json_get_array_item_type(json_array, index) {
    if (!json_array) return JSON_VALUE_TYPE_UNKNOWN;

    let value = json_array[index];

    if (value !== undefined) {
        if (value === null) return JSON_VALUE_TYPE_NULL;
        switch (typeof value) {
            case "boolean":
                return JSON_VALUE_TYPE_BOOLEAN;
            case "number":
                return Number.isInteger(value) ? JSON_VALUE_TYPE_NUMBER_LONG: JSON_VALUE_TYPE_NUMBER_DOUBLE;
            case "object":
                if (value instanceof Array)
                    return JSON_VALUE_TYPE_ARRAY;
                else
                    return JSON_VALUE_TYPE_OBJECT;
            case "string":
                return JSON_VALUE_TYPE_STRING;
        }
    }

    return JSON_VALUE_TYPE_UNKNOWN;
}

