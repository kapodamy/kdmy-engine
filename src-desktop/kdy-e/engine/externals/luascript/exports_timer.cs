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

        internal static void script_timer_register(ManagedLuaState lua) {
            lua.RegisterGlobalFunction("timer_ms_gettime", delgate_timer_ms_gettime);

            const string timer_sourcecode =
        "--[[Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]\n" +
        "______kdmyEngine_timer_ids = 0\n" +
        "______kdmyEngine_timer_table = { }\n" +
        "            ______kdmyEngine_timer_last_sngts = 0\n" +
        "______kdmyEngine_timer_removes = 0\n" +
        "function ______kdmyEngine_timer_add(intrv, sng, dly, cb, args)\n" +
        "    if type(cb) ~= \"function\" then\n" +
        "        error(\"The callback is not a function\")\n" +
        "    end\n" +
        "    if type(dly) ~= \"number\" then\n" +
        "        error(\"The delay is not a number\")\n" +
        "    end\n" +
        "    if type(dly) ~= \"number\" or dly ~= dly or dly == math.huge or dly == -math.huge then\n" +
        "        error(\"The delay is not a finite number\")\n" +
        "    end\n" +
        "    local timestamp\n" +
        "    if sng then\n" +
        "        if not week_get_songplayer then\n" +
        "            error(\"The script is not running in a week context, use timer_callback_* functions instead\")\n" +
        "        end\n" +
        "        timestamp = ______kdmyEngine_timer_last_sngts\n" +
        "    else\n" +
        "                timestamp = timer_ms_gettime()\n" +
        "    end\n" +
        "    timestamp = timestamp + dly\n" +
        "    local entry = {\n" +
        "        timestamp = timestamp,\n" +
        "        delay = dly,\n" +
        "        is_interval = intrv,\n" +
        "        is_song_driven = sng,\n" +
        "        callback = cb,\n" +
        "        args = args,\n" +
        "        id = ______kdmyEngine_timer_ids\n" +
        "    }\n" +
        "    table.insert(______kdmyEngine_timer_table, entry)\n" +
        "    ______kdmyEngine_timer_ids = ______kdmyEngine_timer_ids + 1\n" +
        "    return entry.id\n" +
        "end\n" +
        "function ______kdmyEngine_timer_run(timestamp, song_driven_only)\n" +
        "    if song_driven_only then\n" +
        "        ______kdmyEngine_timer_last_sngts = timestamp\n" +
        "    end\n" +
        "    local removes = ______kdmyEngine_timer_removes\n" +
        "    do\n" +
        "                local i = 1\n" +
        "        local length = #______kdmyEngine_timer_table\n" +
        "        while i <= length do\n" +
        "                    local entry = ______kdmyEngine_timer_table[i]\n" +
        "            if entry and entry.is_song_driven == song_driven_only and timestamp >= entry.timestamp then\n" +
        "                if entry.is_interval then\n" +
        "                    entry.timestamp = entry.timestamp + entry.delay\n" +
        "                else\n" +
        "                ______kdmyEngine_timer_table[i] = nil\n" +
        "                end\n" +
        "                local result, ____error = pcall(\n" +
        "                    entry.callback,\n" +
        "                    table.unpack(entry.args)\n" +
        "                )\n" +
        "                if not result then\n" +
        "                    print(____error)\n" +
        "                end\n" +
        "                if removes ~= ______kdmyEngine_timer_removes then\n" +
        "                    i = 0\n" +
        "                    length = #______kdmyEngine_timer_table\n" +
        "                    removes = ______kdmyEngine_timer_removes\n" +
        "                end\n" +
        "            end\n" +
        "            i = i + 1\n" +
        "        end\n" +
        "    end\n" +
        "end\n" +
        "function timer_callback_interval(delay, fn, ...)\n" +
        "    local args = { ...}\n" +
        "    return ______kdmyEngine_timer_add(\n" +
        "        true,\n" +
        "        false,\n" +
        "        delay,\n" +
        "        fn,\n" +
        "        args\n" +
        "    )\n" +
        "end\n" +
        "function timer_callback_timeout(delay, fn, ...)\n" +
        "    local args = { ...}\n" +
        "    return ______kdmyEngine_timer_add(\n" +
        "        false,\n" +
        "        false,\n" +
        "        delay,\n" +
        "        fn,\n" +
        "        args\n" +
        "    )\n" +
        "end\n" +
        "function timer_songcallback_interval(delay, fn, ...)\n" +
        "    local args = { ...}\n" +
        "    return ______kdmyEngine_timer_add(\n" +
        "        true,\n" +
        "        true,\n" +
        "        delay,\n" +
        "        fn,\n" +
        "        args\n" +
        "    )\n" +
        "end\n" +
        "function timer_songcallback_timeout(delay, fn, ...)\n" +
        "    local args = { ...}\n" +
        "    return ______kdmyEngine_timer_add(\n" +
        "        false,\n" +
        "        true,\n" +
        "        delay,\n" +
        "        fn,\n" +
        "        args\n" +
        "    )\n" +
        "end\n" +
        "function timer_callback_cancel(callback_id)\n" +
        "    if callback_id < 0 then\n" +
        "        return false\n" +
        "    end\n" +
        "    do\n" +
        "                local i = 1\n" +
        "        local length = #______kdmyEngine_timer_table\n" +
        "        while i <= length do\n" +
        "                    local entry = ______kdmyEngine_timer_table[i]\n" +
        "            if entry and entry.id == callback_id then\n" +
        "                ______kdmyEngine_timer_removes = ______kdmyEngine_timer_removes + 1\n" +
        "                ______kdmyEngine_timer_table[i] = nil\n" +
        "                return true\n" +
        "            end\n" +
        "            i = i + 1\n" +
        "        end\n" +
        "    end\n" +
        "    return false\n" +
        "end\n" +
        "function timer_callback_cancel_all()\n" +
        "    local length = #______kdmyEngine_timer_table\n" +
        "    ______kdmyEngine_timer_table = { }\n" +
        "            ______kdmyEngine_timer_removes = ______kdmyEngine_timer_removes + 1\n" +
        "    collectgarbage()\n" +
        "    return length\n" +
        "end\n";

            bool result = lua.EvaluateString(timer_sourcecode);
            Debug.Assert(result, "register_timer() failed");
        }

    }
}
