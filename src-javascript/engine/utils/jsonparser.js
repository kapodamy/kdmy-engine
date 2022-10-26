"use string";

async function json_load_from(src) {
    return await fs_readjson__(src);
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
    if (typeof (value) == "number") return value == 1;
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
    return json && (json[name] === null || json[name] instanceof Object);
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

