function clone_object(obj) {
    function common(o) {
        if (o === undefined || o === null) return true;
        switch(typeof(o)) {
            case "number":
            case "string":
            case "function":
            case "undefined":
                return true;
        }

        if (ArrayBuffer.isView(o)) return true;
        if (o instanceof ArrayBuffer) return true;
        if (o instanceof DataView) return true;
        if (o instanceof EventTarget) return true;
        return false;
    }

    function clone(i) {
        let o = {};
        let f = false;
        for (let p in i) {
            if (i[p] instanceof Array) {
                o[p] = clone_array(i[p]);
            } else {
                if (common(i[p]))
                    o[p] = i[p];
                else
                    o[p] = clone(i[p]);
            }
            f = true;
        }
        return f ? o : i;
    }

    function clone_array(i) {
        let c = 0;
        for (let x of i) c++;
        let o = new Array(c);
        c = -1;
        for (let x of i) {
            c++;
            if (x instanceof Array)
                o[c] = clone_array(x);
            else if (common(x))
                o[c] = x;
            else
                o[c] = clone(x);
        }
        return o;
    }

    if (common(obj)) return obj;

    return obj instanceof Array ? clone_array(obj) : clone(obj);
}
