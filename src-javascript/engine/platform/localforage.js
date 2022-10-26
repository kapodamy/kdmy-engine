
class LocalForaje {
    #instance;

    constructor(db_name) {
        // @ts-ignore
        this.#instance = new __localForaje(db_name);
    }

    /** @param {string} key_name */
    /** @returns {Promise<any>} */
    async get(key_name) {
        let localForaje = this.#instance;
        return await new Promise(function (resolve, reject) {
            return localForaje.get(key_name, resolve, reject);
        });
    }

    /** @param {string} key_name */
    /** @param {any} value */
    /** @returns {Promise<any>} */
    async set(key_name, value) {
        let localForaje = this.#instance;
        return await new Promise(function (resolve, reject) {
            return localForaje.set(key_name, value, resolve, reject);
        });
    }

    /** @param {string} key_name */
    /** @returns {Promise<void>} */
    async delete(key_name) {
        let localForaje = this.#instance;
        return await new Promise(function (resolve, reject) {
            return localForaje.del(key_name, resolve, reject);
        });
    }

    drop() {
        this.#instance.drop();
    }
}

var __localForaje = function (e) {
    function n(o, u, i, c) {
        var s = r.open(e, "number" == typeof i ? i : void 0);
        s.onupgradeneeded = function (e) {
            s.result.objectStoreNames.contains(t) || c || s.result.createObjectStore(t)
        },
            s.onsuccess = function (r) {
                if (t && !s.result.objectStoreNames.contains(t) && !c)// @ts-ignore
                    return s.result.close(), void n(e, t, c, o, u, s.result.version + 1);
                o(s.result)
            },
            s.onerror = function (e) {
                s.result && s.result.close(),
                    u(s.error)
            }
    }
    function o(e, o, r, u, i) {
        n(function (n) {
            function c(e) {
                n.close(),
                    u instanceof Function && u(e)
            }
            function s(e) {
                n && n.close(),
                    i instanceof Function && i(e)
            }
            var a = n.transaction(t, "readwrite").objectStore(t).openCursor();
            a.onerror = function (e) {
                s(a.error)
            },
                a.onsuccess = function (u) {
                    try {
                        if (a.result) {
                            if (a.result.key != e)
                                return void a.result.continue();
                            if ("r" == r)
                                return void c(a.result.value);
                            var i = "w" == r ? a.result.update(o) : a.result.delete();
                            return i.onsuccess = function (e) {
                                c(i.result)
                            },
                                void (i.onerror = function (e) {
                                    s(i.error)
                                })
                        }
                        if ("w" == r) {
                            var d = n.transaction(t, "readwrite").objectStore(t).put(o, e);
                            return d.onsuccess = function () {
                                c(d.result)
                            },
                                void (d.onerror = function () {
                                    s(d.error)
                                })
                        }
                        c(void 0)
                    } catch (e) {
                        s(e.message)
                    }
                }
        }, i)
    }
    var r = window.indexedDB;
    if (!r)
        throw "indexedDB is not available on this browser";
    var t = "__store__";
    return {
        get: function (e, n, r) {
            o(e, null, "r", n, r || function () { })
        },
        set: function (e, n, r, t) {
            o(e, n, "w", r, t || function () { })
        },
        del: function (e, n, r) {
            o(e, null, "d", n, r || function () { })
        },
        drop: function () {
            r.deleteDatabase(e)
        }
    }
};

