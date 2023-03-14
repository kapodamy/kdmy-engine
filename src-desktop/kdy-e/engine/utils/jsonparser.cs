using System;
using Engine.Platform;
using Newtonsoft.Json.Linq;

namespace Engine.Utils {

    public class JSONParserDestroyedException : Exception {
        public JSONParserDestroyedException() : base("JSONParser was disposed") { }
    }

    public class JSONToken {
        internal Guard guard;
        internal JObject obj;
        internal JArray arr;
        internal JSONToken() { }

        internal class Guard {
            public bool disposed = false;
        }
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

    public class JSONParser : JSONToken {

        public static JSONParser LoadFrom(string src) {
            string source = FS.ReadText(src);
            if (source == null || source.Length < 1) return null;

            try {
                JToken json = JToken.Parse(source);
                JSONParser parser = new JSONParser() { obj = json as JObject, arr = json as JArray };
                parser.guard = new Guard();
                return parser;
            } catch (Exception e) {
                Console.Error.WriteLine("json_load() can not parse " + src + "\n", e.Message);
                return null;
            }
        }

        public static JSONParser LoadDirectFrom(string abosolute_src) {
            string source = System.IO.File.ReadAllText(abosolute_src);
            if (source == null || source.Length < 1) return null;

            try {
                JToken json = JToken.Parse(source);
                JSONParser parser = new JSONParser() { obj = json as JObject, arr = json as JArray };
                parser.guard = new Guard();
                return parser;
            } catch (Exception e) {
                Console.Error.WriteLine("json_direct_load() can not parse " + abosolute_src + "\n", e.Message);
                return null;
            }
        }

        public static void Destroy(JSONParser json) {
            if (json == null) return;
            json.guard.disposed = true;
        }

        public static JSONToken ReadObject(JSONToken json, string name) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return null;

            switch (value.Type) {
                case JTokenType.Null:
                    return null;
                case JTokenType.Object:
                    return new JSONToken() { guard = json.guard, obj = (JObject)value };
            }

            Console.Error.WriteLine("json: expected object in " + name);
            return null;
        }

        public static JSONToken ReadArray(JSONToken json, string name) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return null;

            if (value.Type == JTokenType.Array) {
                return new JSONToken() { guard = json.guard, arr = (JArray)value };
            }

            Console.Error.WriteLine("json: expected array in " + name);
            return null;
        }

        public static int ReadArrayLength(JSONToken json_array) {
            if (json_array == null) return -1;
            if (json_array.guard.disposed) throw new JSONParserDestroyedException();
            if (json_array.arr == null) return -1;

            return json_array.arr.Count;
        }

        public static double ReadArrayItemNumberDouble(JSONToken json_array, int index, double default_value) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return default_value;

            if (value.Type == JTokenType.Float || value.Type == JTokenType.Integer) return (double)value;

            Console.Error.WriteLine("json: expected number in " + index + ": " + value);
            return default_value;
        }

        public static long ReadArrayItemNumberLong(JSONToken json_array, int index, long default_value) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return default_value;

            if (value.Type == JTokenType.Integer) return (long)value;

            Console.Error.WriteLine("json: expected number in " + index + ": " + value);
            return default_value;
        }

        public static bool ReadArrayItemBoolean(JSONToken json_array, int index, bool default_value) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return default_value;

            if (value.Type == JTokenType.Boolean) return (bool)value;

            Console.Error.WriteLine("json: expected boolean in " + index + ": " + value);
            return default_value;
        }

        public static string ReadArrayItemString(JSONToken json_array, int index, string default_value) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return default_value;

            if (value.Type == JTokenType.String) return (string)value;

            Console.Error.WriteLine("json: expected string in " + index + ": " + value);
            return default_value;
        }

        public static uint ReadArrayItemHex(JSONToken json_array, int index, uint default_value) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return default_value;

            switch (value.Type) {
                case JTokenType.Integer:
                    return (uint)((long)value);
                case JTokenType.String:
                    return VertexProps.ParseHex2((string)value, default_value, false);
            }

            Console.Error.WriteLine("json: expected number or hexadecimal number string in " + index + ": " + value);
            return default_value;
        }

        public static JSONToken ReadArrayItemObject(JSONToken json_array, int index) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return null;

            if (value.Type == JTokenType.Object) {
                return new JSONToken() { guard = json_array.guard, obj = (JObject)value };
            }

            Console.Error.WriteLine("json: expected object at index " + index);
            return null;
        }

        public static JSONToken ReadArrayItemArray(JSONToken json_array, int index) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return null;

            if (value.Type == JTokenType.Null) return null;

            if (value.Type == JTokenType.Array) {
                return new JSONToken() { guard = json_array.guard, arr = (JArray)value };
            }

            Console.Error.WriteLine("json: expected array at index " + index);
            return null;
        }


        public static double ReadNumberDouble(JSONToken json, string name, double default_value) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return default_value;
            if (value.Type == JTokenType.Float || value.Type == JTokenType.Integer) return (double)value;

            Console.Error.WriteLine("json: expected number in " + name + ": " + value);
            return default_value;
        }

        public static long ReadNumberLong(JSONToken json, string name, long default_value) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return default_value;
            if (value.Type == JTokenType.Integer) return (long)value;

            Console.Error.WriteLine("json: expected number in " + name + ": " + value);
            return default_value;
        }

        public static bool ReadBoolean(JSONToken json, string name, bool default_value) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return default_value;

            if (value.Type == JTokenType.Boolean) return (bool)value;

            Console.Error.WriteLine("json: expected boolean in " + name + ": " + value);
            return default_value;
        }

        public static string ReadString(JSONToken json, string name, string default_value) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return default_value;

            if (value.Type == JTokenType.Null) return null;
            if (value.Type == JTokenType.String) return (string)value;

            Console.Error.WriteLine("json: expected string in " + name + ": " + value);
            return default_value;
        }

        public static uint ReadHex(JSONToken json, string name, uint default_value) {
            JToken value = InternalGetPropertyValue(json, name);
            if (value == null) return default_value;

            if (value.Type == JTokenType.Integer) return (uint)value;
            if (value.Type == JTokenType.String) return VertexProps.ParseHex2((string)value, default_value, true);

            Console.Error.WriteLine("json: expected number or hexadecimal number string in " + name + ": " + value);
            return default_value;
        }


        public static bool HasProperty(JSONToken json, string name) {
            if (json == null) return false;
            if (json.guard.disposed) throw new JSONParserDestroyedException();
            if (json.obj == null) return false;

            return json.obj[name] != null;
        }


        public static bool HasPropertyBoolean(JSONToken json, string name) {
            return InternalHasItemType(json, name, JTokenType.Boolean);
        }

        public static bool HasPropertyString(JSONToken json, string name) {
            return InternalHasItemType(json, name, JTokenType.String);
        }

        public static bool HasPropertyNumber(JSONToken json, string name) {
            return
                InternalHasItemType(json, name, JTokenType.Integer) |
                InternalHasItemType(json, name, JTokenType.Float)
            ;
        }

        public static bool HasPropertyArray(JSONToken json, string name) {
            return InternalHasItemType(json, name, JTokenType.Array);
        }

        public static bool HasPropertyObject(JSONToken json, string name) {
            return InternalHasItemType(json, name, JTokenType.Object);
        }

        public static bool HasPropertyHex(JSONToken json, string name) {
            return
                InternalHasItemType(json, name, JTokenType.Integer) ||
                InternalHasItemType(json, name, JTokenType.String)
             ;
        }


        public static bool IsPropertyNull(JSONToken json, string name) {
            return InternalHasItemType(json, name, JTokenType.Null);
        }

        public static bool IsPropertyArray(JSONToken json, string name) {
            return InternalHasItemType(json, name, JTokenType.Array);
        }

        public static bool IsArrayItemNull(JSONToken json_array, int index) {
            JToken item = InternalGetArrayItem(json_array, index);
            return item == null || item.Type == JTokenType.Null;
        }

        public static JSONValueType GetArrayItemType(JSONToken json_array, int index) {
            JToken value = InternalGetArrayItem(json_array, index);
            if (value == null) return JSONValueType.Unknown;

            switch (value.Type) {
                case JTokenType.Array:
                    return JSONValueType.Array;
                case JTokenType.Boolean:
                    return JSONValueType.Boolean;
                case JTokenType.Float:
                    return JSONValueType.NumberDouble;
                case JTokenType.Integer:
                    return JSONValueType.NumberLong;
                case JTokenType.String:
                    return JSONValueType.String;
                case JTokenType.Null:
                    return JSONValueType.Null;
                case JTokenType.Object:
                    return JSONValueType.Object;
                default:
                    return JSONValueType.Unknown;
            }
        }

        public static bool IsArray(JSONToken json) {
            if (json == null) return false;
            if (json.guard.disposed) throw new JSONParserDestroyedException();
            if (json.arr == null) return false;

            return true;
        }

        public static JSONProperty[] GetObjectProperties(JSONToken json) {
            if (json == null) return null;
            if (json.guard.disposed) throw new JSONParserDestroyedException();
            if (json.obj == null) return null;

            if (json.obj.Type != JTokenType.Object) {
                // not a object
                return null;
            }

            int index = 0;
            JSONProperty[] properties = new JSONProperty[json.obj.Count];

            foreach (JProperty property in json.obj.Properties()) {
                properties[index].name = property.Name;

                switch (property.Value.Type) {
                    case JTokenType.Array:
                        properties[index].type = JSONValueType.Array;
                        break;
                    case JTokenType.Boolean:
                        properties[index].type = JSONValueType.Boolean;
                        break;
                    case JTokenType.Float:
                        properties[index].type = JSONValueType.NumberDouble;
                        break;
                    case JTokenType.Integer:
                        properties[index].type = JSONValueType.NumberLong;
                        break;
                    case JTokenType.String:
                        properties[index].type = JSONValueType.String;
                        break;
                    case JTokenType.Null:
                        properties[index].type = JSONValueType.Null;
                        break;
                    case JTokenType.Object:
                        properties[index].type = JSONValueType.Object;
                        break;
                    default:
                        properties[index].type = JSONValueType.Unknown;
                        break;
                }
                index++;
            }

            return properties;
        }


        private static bool InternalHasItemType(JSONToken json, string name, JTokenType type) {
            if (json == null) return false;
            if (json.guard.disposed) throw new JSONParserDestroyedException();
            if (json.obj == null) return false;

            JToken value = json.obj[name];
            if (value == null) return false;

            return value.Type == type;
        }

        private static JToken InternalGetArrayItem(JSONToken json_array, int index) {
            if (json_array == null) return null;
            if (json_array.guard.disposed) throw new JSONParserDestroyedException();
            if (json_array.arr == null) return null;

            if (index < 0 || index >= json_array.arr.Count) return null;// throw new ArgumentOutOfRangeException("index");

            return json_array.arr[index];
        }

        private static JToken InternalGetPropertyValue(JSONToken json, string name) {
            if (json == null) return null;
            if (json.guard.disposed) throw new JSONParserDestroyedException();
            if (json.obj == null) return null;

            return json.obj[name];
        }


    }
}
