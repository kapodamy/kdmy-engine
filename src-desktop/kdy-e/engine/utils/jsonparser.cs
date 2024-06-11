using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.Json;
using Engine.Platform;

namespace Engine.Utils;

public class JSONParserDestroyedException : Exception {
    public JSONParserDestroyedException() : base("JSONParser was disposed") { }
}

public enum JSONValueType {
    String,
    Boolean,
    NumberDouble,
    NumberLong,
    Array,
    Null,
    Object,
    Unknown
}

public struct JSONProperty {
    public JSONValueType type;
    public string name;
}

public class JSONToken {
    internal bool disposed;// root only
    internal JSONToken root;// child fields only

    internal JSONValueType type;
    internal object value;
    internal string name;
}

public static class JSONParser {
    private static readonly byte[] UTF8_BOM = { 0xEF, 0xBB, 0xBF };

    public static JSONToken LoadFrom(string src) {
        byte[] source = FS.ReadArrayBuffer(src);
        if (source == null || source.Length < 1) return null;

        try {
            return JSONParser.Parse(source);
        } catch (Exception e) {
            Logger.Error($"json_load_from() can not parse {src}\n{e.Message}");
            return null;
        }
    }

    public static JSONToken LoadFromString(string json_sourcecode) {
        if (StringUtils.IsEmpty(json_sourcecode)) return null;

        try {
            byte[] buffer = Encoding.UTF8.GetBytes(json_sourcecode);
            return JSONParser.Parse(buffer);
        } catch (Exception e) {
            Logger.Error($"json_load_string() can not parse string '{json_sourcecode}':\n{e.Message}");
            return null;
        }
    }

    public static JSONToken LoadDirectFrom(string abosolute_src) {
        byte[] buffer = File.ReadAllBytes(abosolute_src);
        if (buffer == null || buffer.Length < 1) return null;

        try {
            return JSONParser.Parse(buffer);
        } catch (Exception e) {
            Logger.Error($"json_load_direct() can not parse {abosolute_src}\n{e.Message}");
            return null;
        }
    }

    public static void Destroy(JSONToken json) {
        if (json == null) return;
        if (json.root == null) throw new ArgumentException(null, "json");

        json.disposed = true;
    }

    public static JSONToken ReadObject(JSONToken json, string name) {
        JSONToken token = InternalGetObjectProperty(json, name);
        if (token == null) return null;

        switch (token.type) {
            case JSONValueType.Null:
                return null;
            case JSONValueType.Object:
                return token;
        }

        Logger.Warn($"json: expected object in {name}");
        return null;
    }

    public static JSONToken ReadArray(JSONToken json, string name) {
        JSONToken token = InternalGetObjectProperty(json, name);

        if (token == null) return null;

        if (token.type == JSONValueType.Array) return token;

        Logger.Warn($"json: expected array in {name}");
        return null;
    }

    public static int ReadArrayLength(JSONToken json_array) {
        if (json_array == null) return -1;
        if (json_array.root.disposed) throw new JSONParserDestroyedException();
        if (json_array.type != JSONValueType.Array) return -1;

        JSONToken[] fields = (JSONToken[])json_array.value;

        return fields.Length;
    }

    public static double ReadArrayItemNumberDouble(JSONToken json_array, int index, double default_value) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return default_value;

        if (token.type == JSONValueType.NumberLong) return (long)token.value;
        if (token.type == JSONValueType.NumberDouble) return (double)token.value;

        Logger.Warn($"json: expected number in {index}: {token}");
        return default_value;
    }

    public static long ReadArrayItemNumberLong(JSONToken json_array, int index, long default_value) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return default_value;

        if (token.type == JSONValueType.NumberLong) return (long)token.value;

        Logger.Warn($"json: expected number in {index}: {token}");
        return default_value;
    }

    public static bool ReadArrayItemBoolean(JSONToken json_array, int index, bool default_value) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return default_value;

        if (token.type == JSONValueType.Boolean) return (bool)token.value;

        Logger.Warn($"json: expected boolean in {index}: {token}");
        return default_value;
    }

    public static string ReadArrayItemString(JSONToken json_array, int index, string default_value) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return default_value;

        if (token.type == JSONValueType.String) return (string)token.value;

        Logger.Warn($"json: expected string in {index}: {token}");
        return default_value;
    }

    public static uint ReadArrayItemHex(JSONToken json_array, int index, uint default_value) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return default_value;

        switch (token.type) {
            case JSONValueType.NumberLong:
                return (uint)((long)token.value);
            case JSONValueType.String:
                return VertexProps.ParseHex2((string)token.value, default_value, false);
        }

        Logger.Warn($"json: expected number or hexadecimal number string in {index}: {token}");
        return default_value;
    }

    public static JSONToken ReadArrayItemObject(JSONToken json_array, int index) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return null;
        if (token.type == JSONValueType.Object) return token;

        Logger.Warn($"json: expected object at index {index}");
        return null;
    }

    public static JSONToken ReadArrayItemArray(JSONToken json_array, int index) {
        JSONToken token = InternalGetArrayItem(json_array, index);
        if (token == null) return null;
        if (token.type == JSONValueType.Null) return null;
        if (token.type == JSONValueType.Array) return token;

        Logger.Warn($"json: expected array at index {index}");
        return null;
    }


    public static double ReadNumberDouble(JSONToken json, string name, double default_value) {
        JSONToken token = InternalGetObjectProperty(json, name);
        if (token == null) return default_value;
        if (token.type == JSONValueType.NumberLong) return (long)token.value;
        if (token.type == JSONValueType.NumberDouble) return (double)token.value;

        Logger.Warn($"json: expected number in {name}: {token}");
        return default_value;
    }

    public static long ReadNumberLong(JSONToken json, string name, long default_value) {
        JSONToken token = InternalGetObjectProperty(json, name);
        if (token == null) return default_value;
        if (token.type == JSONValueType.NumberLong) return (long)token.value;

        Logger.Warn($"json: expected number in {name} : {token}");
        return default_value;
    }

    public static bool ReadBoolean(JSONToken json, string name, bool default_value) {
        JSONToken token = InternalGetObjectProperty(json, name);
        if (token == null) return default_value;

        if (token.type == JSONValueType.Boolean) return (bool)token.value;

        Logger.Warn($"json: expected boolean in {name}: {token}");
        return default_value;
    }

    public static string ReadString(JSONToken json, string name, string default_value) {
        JSONToken token = InternalGetObjectProperty(json, name);
        if (token == null) return default_value;

        if (token.type == JSONValueType.Null) return null;
        if (token.type == JSONValueType.String) return (string)token.value;

        Logger.Warn($"json: expected string in {name}: {token}");
        return default_value;
    }

    public static uint ReadHex(JSONToken json, string name, uint default_value) {
        JSONToken token = InternalGetObjectProperty(json, name);
        if (token == null) return default_value;

        if (token.type == JSONValueType.NumberLong) return (uint)((long)token.value);
        if (token.type == JSONValueType.String) return VertexProps.ParseHex2((string)token.value, default_value, true);

        Logger.Warn($"json: expected number or hexadecimal number string in {name}: {token}");
        return default_value;
    }


    public static bool HasProperty(JSONToken json, string name) {
        JSONToken token = InternalGetObjectProperty(json, name);
        return token != null;
    }


    public static bool HasPropertyBoolean(JSONToken json, string name) {
        return InternalObjectPropertyType(json, name, JSONValueType.Boolean);
    }

    public static bool HasPropertyString(JSONToken json, string name) {
        return InternalObjectPropertyType(json, name, JSONValueType.String);
    }

    public static bool HasPropertyNumber(JSONToken json, string name) {
        return
            InternalObjectPropertyType(json, name, JSONValueType.NumberLong) |
            InternalObjectPropertyType(json, name, JSONValueType.NumberDouble)
        ;
    }

    public static bool HasPropertyArray(JSONToken json, string name) {
        return InternalObjectPropertyType(json, name, JSONValueType.Array);
    }

    public static bool HasPropertyObject(JSONToken json, string name) {
        return InternalObjectPropertyType(json, name, JSONValueType.Object);
    }

    public static bool HasPropertyHex(JSONToken json, string name) {
        return
            InternalObjectPropertyType(json, name, JSONValueType.NumberLong) ||
            InternalObjectPropertyType(json, name, JSONValueType.String)
         ;
    }


    public static bool IsPropertyNull(JSONToken json, string name) {
        return InternalObjectPropertyType(json, name, JSONValueType.Null);
    }

    public static bool IsPropertyArray(JSONToken json, string name) {
        return InternalObjectPropertyType(json, name, JSONValueType.Array);
    }

    public static bool IsArrayItemNull(JSONToken json_array, int index) {
        JSONToken item = InternalGetArrayItem(json_array, index);
        return item == null || item.type == JSONValueType.Null;
    }

    public static JSONValueType GetArrayItemType(JSONToken json_array, int index) {
        JSONToken value = InternalGetArrayItem(json_array, index);
        if (value == null) return JSONValueType.Unknown;

        return value.type;
    }

    public static bool IsArray(JSONToken json) {
        if (json == null) return false;
        if (json.root.disposed) throw new JSONParserDestroyedException();

        return json.type == JSONValueType.Array;
    }

    public static JSONProperty[] GetObjectProperties(JSONToken json) {
        if (json == null) return null;
        if (json.root.disposed) throw new JSONParserDestroyedException();
        if (json.type != JSONValueType.Object) return null;// not an object

        JSONToken[] fields = (JSONToken[])json.value;
        JSONProperty[] properties = new JSONProperty[fields.Length];

        for (int i = 0 ; i < fields.Length ; i++) {
            properties[i].name = fields[i].name;
            properties[i].type = fields[i].type;
        }

        return properties;
    }

    public static object GetTokenValue(JSONToken json) {
        if (json == null) return JSONValueType.Unknown;
        if (json.root.disposed) throw new JSONParserDestroyedException();

        return json.value;
    }


    private static bool InternalObjectPropertyType(JSONToken json, string name, JSONValueType type) {
        JSONToken value = InternalGetObjectProperty(json, name);
        if (value == null) return false;

        return value.type == type;
    }

    private static JSONToken InternalGetArrayItem(JSONToken arr, int index) {
        if (arr == null) return null;
        if (arr.root.disposed) throw new JSONParserDestroyedException();
        if (arr.type != JSONValueType.Array) return null;

        JSONToken[] fields = (JSONToken[])arr.value;

        if (index < 0 || index >= fields.Length) return null;// throw new ArgumentOutOfRangeException("index");

        return fields[index];
    }

    private static JSONToken InternalGetObjectProperty(JSONToken obj, string name) {
        if (obj == null) return null;
        if (obj.root.disposed) throw new JSONParserDestroyedException();
        if (obj.type != JSONValueType.Object) return null;

        JSONToken[] fields = (JSONToken[])obj.value;

        foreach (JSONToken token in fields) {
            if (token.name == name) return token;
        }

        return null;
    }


    private static JSONToken Parse(ReadOnlySpan<byte> readonly_buffer) {
        if (readonly_buffer.StartsWith(UTF8_BOM)) {
            readonly_buffer = readonly_buffer.Slice(UTF8_BOM.Length);
        }

        JsonReaderOptions options = new JsonReaderOptions() {
            AllowTrailingCommas = false,
            CommentHandling = JsonCommentHandling.Disallow,
            MaxDepth = 0
        };
        Utf8JsonReader parser = new Utf8JsonReader(readonly_buffer, options);

        if (!parser.Read()) return null;

        JSONToken root = new JSONToken();
        JSONToken field;

        if (parser.TokenType == JsonTokenType.StartObject)
            field = JSONParser.ParseTree(ref parser, root, true);
        else if (parser.TokenType == JsonTokenType.StartArray)
            field = JSONParser.ParseTree(ref parser, root, false);
        else
            field = JSONParser.ParseField(ref parser);

        if (field == null) {
            throw new InvalidDataException($"unknown root token type: {parser.TokenType}");
        }

        root.type = field.type;
        root.value = field.value;
        root.disposed = false;
        root.root = root;

        return root;
    }

    private static JSONToken ParseTree(ref Utf8JsonReader parser, JSONToken root, bool is_object_or_array) {
        string property_name = null;
        List<JSONToken> fields = new List<JSONToken>(16);
        JSONToken token;

        while (parser.Read()) {
            switch (parser.TokenType) {
                case JsonTokenType.PropertyName:
                    if (property_name != null || !is_object_or_array) goto default;
                    property_name = parser.GetString();
                    continue;
                case JsonTokenType.StartObject:
                    if (is_object_or_array && property_name == null) goto default;
                    token = JSONParser.ParseTree(ref parser, root, true);
                    token.name = property_name;
                    fields.Add(token);
                    break;
                case JsonTokenType.StartArray:
                    if (is_object_or_array && property_name == null) goto default;
                    token = JSONParser.ParseTree(ref parser, root, false);
                    token.name = property_name;
                    fields.Add(token);
                    break;
                case JsonTokenType.EndObject:
                    if (property_name != null || !is_object_or_array) goto default;
                    goto L_return;
                case JsonTokenType.EndArray:
                    if (property_name != null || is_object_or_array) goto default;
                    goto L_return;
                case JsonTokenType.False:
                case JsonTokenType.True:
                case JsonTokenType.String:
                case JsonTokenType.Number:
                case JsonTokenType.Null:
                    if (is_object_or_array && property_name == null) goto default;

                    JSONToken field = JSONParser.ParseField(ref parser);
                    field.name = property_name;
                    field.root = root;
                    fields.Add(field);
                    break;
                default:
                    throw new InvalidDataException($"unexpected json token: {parser.TokenType} at {parser.TokenStartIndex}");
            }

            property_name = null;
        }

L_return:
        return new JSONToken() {
            type = is_object_or_array ? JSONValueType.Object : JSONValueType.Array,
            value = fields.ToArray(),
            root = root
        };
    }

    private static JSONToken ParseField(ref Utf8JsonReader parser) {
        switch (parser.TokenType) {
            case JsonTokenType.False:
            case JsonTokenType.True:
                return new JSONToken() {
                    value = parser.GetBoolean(),
                    type = JSONValueType.Boolean
                };
            case JsonTokenType.String:
                return new JSONToken() {
                    value = parser.GetString(),
                    type = JSONValueType.String
                };
            case JsonTokenType.Number:
                long value_int64;
                if (parser.TryGetInt64(out value_int64)) {
                    return new JSONToken() {
                        value = value_int64,
                        type = JSONValueType.NumberLong
                    };
                }
                return new JSONToken() {
                    value = parser.GetDouble(),
                    type = JSONValueType.NumberDouble
                };
            case JsonTokenType.Null:
                return new JSONToken() {
                    value = null,
                    type = JSONValueType.Null
                };
        }

        return null;
    }

}
