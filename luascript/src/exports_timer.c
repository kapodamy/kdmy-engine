#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(uint32_t, timer_ms_gettime32_JS, (), {
    return Math.trunc(performance.now());
});
uint64_t timer_ms_gettime64() {
    return (uint64_t)timer_ms_gettime32_JS();
}
#endif

static int script_timer_ms_gettime(lua_State* L) {

    uint64_t result = timer_ms_gettime64();
    lua_pushinteger(L, (lua_Integer)result);

    return 1;
}


void register_timer(lua_State* L) {

    lua_pushcfunction(L, script_timer_ms_gettime);
    lua_setglobal(L, "timer_ms_gettime");

    int result = luaL_dostring(L,
        //--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
        "______kdmyEngine_timer_ids = 0\n"
        "______kdmyEngine_timer_table = {}\n"
        "function ______kdmyEngine_timer_add(is_interval, delay, callback, args)\n"
        "    if type(callback) ~= \"function\" then\n"
        "        error(\"The callback is not a function\")\n"
        "    end\n"
        "    if type(delay) ~= \"number\" or delay ~= delay or delay == math.huge or delay == -math.huge then\n"
        "        error(\"The delay is not a valid number\")\n"
        "    end\n"
        "    local id = ______kdmyEngine_timer_ids\n"
        "    local timestamp = timer_ms_gettime() + delay\n"
        "    ______kdmyEngine_timer_table[id] = {\n"
        "        timestamp = timestamp,\n"
        "        delay = delay,\n"
        "        is_interval = is_interval,\n"
        "        callback = callback,\n"
        "        args = args\n"
        "    }\n"
        "    ______kdmyEngine_timer_ids = ______kdmyEngine_timer_ids + 1\n"
        "    return id\n"
        "end\n"
        "function ______kdmyEngine_timer_run(timestamp)\n"
        "   local length\n"
        "   ::L_check::"
        "   length = #______kdmyEngine_timer_table\n"
        "    for id, entry in pairs(______kdmyEngine_timer_table) do\n"
        "        if timestamp >= entry.timestamp then\n"
        "            if entry.is_interval then\n"
        "                entry.timestamp = entry.timestamp + entry.delay\n"
        "            else\n"
        "                ______kdmyEngine_timer_table[id] = nil\n"
        "            end\n"
        "            local result, ____error = pcall(entry.callback, table.unpack(entry.args))\n"
        "            if not result then\n"
        "                print(____error)\n"
        "            end\n"
        "            if length ~= #______kdmyEngine_timer_table then\n"
        "                goto L_check\n"
        "            end\n"
        "        end\n"
        "    end\n"
        "end\n"
        "function timer_callback_interval(delay, fn, ...)\n"
        "    return ______kdmyEngine_timer_add(true, delay, fn, {...})\n"
        "end\n"
        "function timer_callback_timeout(delay, fn, ...)\n"
        "    return ______kdmyEngine_timer_add(false, delay, fn, {...})\n"
        "end\n"
        "function timer_callback_cancel(callback_id)\n"
        "    ______kdmyEngine_timer_table[callback_id] = nil\n"
        "    return true\n"
        "end\n"
        "function timer_callback_cancel_all()\n"
        "    local length = #______kdmyEngine_timer_table\n"
        "    ______kdmyEngine_timer_table = {}\n"
        "    collectgarbage()\n"
        "    return length\n"
        "end\n"
    );
    assert(result == 0/*register_timer() failed*/);
}

