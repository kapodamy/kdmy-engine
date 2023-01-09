function importScript(lua_script_filename: string): boolean {
    let str = fs_readfile(lua_script_filename);
    if (str == null) return false;

    let [fn, err] = load(str, lua_script_filename, "t");

    if (fn) {
        fn();
        return true;
    }

    print(err);
    return false;
}

function evalScript(lua_script_filename: string): any {
    let str = fs_readfile(lua_script_filename);
    if (str == null) return false;

    let [fn, err] = load(str, lua_script_filename, "t");

    if (fn) {
        return fn();
    }

    print(err);
    return false;
}
