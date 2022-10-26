using System.Diagnostics;
using Engine.Externals.LuaInterop;
using KallistiOS;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsTimer {

        static int script_timer_ms_gettime(LuaState L) {

            double result = timer.ms_gettime64();
            L.lua_pushnumber(result);

            return 1;
        }

        private static readonly LuaCallback delgate_timer_ms_gettime = script_timer_ms_gettime;

        internal static void register_timer(ManagedLuaState lua) {
            lua.RegisterGlobalFunction("timer_ms_gettime", delgate_timer_ms_gettime);

            int result = lua.EvaluateString(
                //--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
                "______kdmyEngine_timer_ids = 0\n" +
                "______kdmyEngine_timer_table = {}\n" +
                "function ______kdmyEngine_timer_add(is_interval, delay, callback, args)\n" +
                "    if type(callback) ~= \"function\" then\n" +
                "        error(\"The callback is not a function\")\n" +
                "    end\n" +
                "    if type(delay) ~= \"number\" or delay ~= delay or delay == math.huge or delay == -math.huge then\n" +
                "        error(\"The delay is not a valid number\")\n" +
                "    end\n" +
                "    local id = ______kdmyEngine_timer_ids\n" +
                "    local timestamp = timer_ms_gettime() + delay\n" +
                "    ______kdmyEngine_timer_table[id] = {\n" +
                "        timestamp = timestamp,\n" +
                "        delay = delay,\n" +
                "        is_interval = is_interval,\n" +
                "        callback = callback,\n" +
                "        args = args\n" +
                "    }\n" +
                "    ______kdmyEngine_timer_ids = ______kdmyEngine_timer_ids + 1\n" +
                "    return id\n" +
                "end\n" +
                "function ______kdmyEngine_timer_run(timestamp)\n" +
                "   local length\n" +
                "   ::L_check::" +
                "   length = #______kdmyEngine_timer_table\n" +
                "    for id, entry in pairs(______kdmyEngine_timer_table) do\n" +
                "        if timestamp >= entry.timestamp then\n" +
                "            if entry.is_interval then\n" +
                "                entry.timestamp = entry.timestamp + entry.delay\n" +
                "            else\n" +
                "                ______kdmyEngine_timer_table[id] = nil\n" +
                "            end\n" +
                "            local result, ____error = pcall(entry.callback, table.unpack(entry.args))\n" +
                "            if not result then\n" +
                "                print(____error)\n" +
                "            end\n" +
                "            if length ~= #______kdmyEngine_timer_table then\n" +
                "                goto L_check\n" +
                "            end\n" +
                "        end\n" +
                "    end\n" +
                "end\n" +
                "function timer_callback_interval(delay, fn, ...)\n" +
                "    return ______kdmyEngine_timer_add(true, delay, fn, {...})\n" +
                "end\n" +
                "function timer_callback_timeout(delay, fn, ...)\n" +
                "    return ______kdmyEngine_timer_add(false, delay, fn, {...})\n" +
                "end\n" +
                "function timer_callback_cancel(callback_id)\n" +
                "    ______kdmyEngine_timer_table[callback_id] = nil\n" +
                "    return true\n" +
                "end\n" +
                "function timer_callback_cancel_all()\n" +
                "    local length = #______kdmyEngine_timer_table\n" +
                "    ______kdmyEngine_timer_table = {}\n" +
                "    collectgarbage()\n" +
                "    return length\n" +
                "end\n",
                "timer.ts"
            );
            Debug.Assert(result == 0, "register_timer() failed");
        }

    }
}
