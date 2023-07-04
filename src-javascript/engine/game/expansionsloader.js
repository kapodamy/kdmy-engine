"use strict";

/**
 * @typedef {object} Expansion
 * @property {string} name
 * @property {string} directory
 * @property {string} icon
 * @property {string} version
 * @property {string} submiter
 * @property {string} description
 * @property {string} screenshoot
 * @property {string} window_icon
 * @property {string} window_title
 * @property {HTMLLIElement} li_element
 */


var /**@type {Expansion[]} */ expansionsloader_expansions = null;
var /**@type {function(string):void} */expansionsloader_resolve = null;

/* __attribute__((constructor)) void expansionsloader_init_defaults() */ {
    document.getElementById("exl-launch").addEventListener("click", expansionsloader_internal_launch, false);
    document.getElementById("exl-refresh").addEventListener("click", expansionsloader_internal_refresh, false);
    let exl_list = document.getElementById("exl-list");
    exl_list.addEventListener("click", expansionsloader_internal_click, false);
    exl_list.addEventListener("click", expansionsloader_internal_click, false);
    exl_list.addEventListener("dblclick", expansionsloader_internal_doubleClick, false);
    exl_list.addEventListener("keydown", expansionsloader_internal_keyDown, false);
}

function expansionsloader_main() {
    return new Promise(function (resolve, reject) {
        console.assert(!expansionsloader_expansions);
        expansionsloader_resolve = resolve;
        expansionsloader_internal_toggle_visibility(true);
        expansionsloader_internal_refresh(null);
    });
}


async function expansionsloader_internal_load_expansions() {
    let expansions_path = await io_native_get_absolute_path(EXPANSIONS_PATH, false, true, false);
    let expansions = new Array();

    if (!await io_native_resource_exists(expansions_path, false, true)) return expansions;
    let entries = await io_native_enumerate_folder(expansions_path);

    if (!entries) return expansions;

    for (let dir of entries) {
        let dir_path = `${expansions_path}${dir.name}${FS_CHAR_SEPARATOR}`
        let dir_relative_path = `${EXPANSIONS_PATH}${dir.name}${FS_CHAR_SEPARATOR}`;

        if (dir.name.toLowerCase() == "funkin") continue;

        let /**@type {Expansion}*/ expansion = {
            name: null,
            directory: dir.name,
            icon: null,
            version: null,
            submiter: null,
            description: null,
            screenshoot: null,
            window_icon: null,
            window_title: null,
            li_element: null
        };

        let about_src = `${dir_path}${EXPANSIONS_ABOUT_FILENAME}`;
        if (!await io_native_resource_exists(about_src, true, false)) {
            console.error(`expansionsloader_load_expansions() missing file ${about_src}`);
            expansions.push(expansion);
            continue;
        }

        let json = await json_load_direct(about_src);
        if (json == null) {
            console.warn(`expansionsloader_load_expansions() can not open: ${about_src}`);
            continue;
        }

        expansion.name = json_read_string(json, "name", dir.Name);
        expansion.version = json_read_string(json, "version", null);
        expansion.submiter = json_read_string(json, "submiter", null);
        expansion.description = json_read_string(json, "description", null);
        expansion.screenshoot = json_read_string(json, "screenshoot", null);
        expansion.icon = json_read_string(json, "icon", null);
        expansion.window_icon = json_read_string(json, "windowIcon", null);
        expansion.window_title = json_read_string(json, "windowTitle", null);
        json_destroy(json);

        if (expansion.screenshoot) {
            expansion.screenshoot = await io_native_get_absolute_path(
                fs_resolve_path(`${dir_relative_path}${expansion.screenshoot}`), true, false, false
            );
        }
        if (expansion.icon) {
            expansion.icon = await io_native_get_absolute_path(
                fs_resolve_path(`${dir_relative_path}${expansion.icon}`), true, false, false
            );
        }
        if (expansion.window_icon) {
            expansion.window_icon = await io_native_get_absolute_path(
                fs_resolve_path(`${dir_relative_path}${expansion.window_icon}`), true, false, false
            );
        }

        // TODO: markdown to RTF (https://es.wikipedia.org/wiki/Rich_Text_Format)
        expansions.push(expansion);
    }

    return expansions;
}

function expansionsloader_internal_doubleClick(/**@type {MouseEvent}*/e) {
    expansionsloader_internal_click(e);
    expansionsloader_internal_launch(e);
}

function expansionsloader_internal_click(/**@type {MouseEvent}*/e) {
    let list = document.getElementById("exl-list");

    // @ts-ignore
    let /**@type {HTMLElement}*/li_element = e.target;

    if (li_element == list) return;

    while (li_element && li_element.parentElement != list) li_element = li_element.parentElement;

    let selected = list.querySelector(".exl-item-selected");
    if (selected == li_element) return;

    let index = expansionsloader_internal_index_of(li_element);

    if (selected) selected.classList.remove("exl-item-selected");
    expansionsloader_expansions[index].li_element.classList.add("exl-item-selected");

    expansionsloader_internal_show_info(expansionsloader_expansions[index]);
}

function expansionsloader_internal_keyDown(/**@type {KeyboardEvent}*/e) {
    if (e.isComposing) return;

    let list = document.getElementById("exl-list");
    let selected = list.querySelector(".exl-item-selected");

    let index = expansionsloader_internal_index_of(selected);

    if (e.code == "Enter" && !e.repeat) {
        expansionsloader_internal_launch(e);
    } else if (e.code == "ArrowUp") {
        index--;
        if (index < 0) return;
    } else if (e.code == "ArrowDown") {
        index++;
        if (index >= expansionsloader_expansions.length) return;
    } else {
        return;
    }

    selected.classList.remove("exl-item-selected");
    expansionsloader_expansions[index].li_element.classList.add("exl-item-selected");

    if (e.type != "dblclick")
        expansionsloader_internal_show_info(expansionsloader_expansions[index]);
}

function expansionsloader_internal_launch(e) {
    let list = document.getElementById("exl-list");
    let selected = list.querySelector(".exl-item-selected");

    if (!selected) {
        alert("No expasion selected");
        return;
    }

    let index = expansionsloader_internal_index_of(selected);

    // resolve Promise returned of expansionsloader_internal_main() later
    setTimeout(expansionsloader_resolve, 0, expansionsloader_expansions[index].directory);

    // dispose resources
    for (let expansion of expansionsloader_expansions) expansion.li_element.remove();
    expansionsloader_expansions = null;
    expansionsloader_resolve = null;
    expansionsloader_internal_show_info(null);
    expansionsloader_internal_toggle_visibility(false);
}

async function expansionsloader_internal_refresh(e) {
    if (expansionsloader_expansions) {
        for (let expansion of expansionsloader_expansions) expansion.li_element.remove();
        expansionsloader_internal_show_info(null);
    }
    expansionsloader_expansions = await expansionsloader_internal_load_expansions();

    /**@type {HTMLUListElement}*///@ts-ignore
    let exl_list = document.getElementById("exl-list");

    /**@type {HTMLLIElement}*///@ts-ignore
    let exl_item_template = document.getElementById("exl-item").content.firstElementChild;

    for (let expansion of expansionsloader_expansions) {
        // @ts-ignore
        expansion.li_element = exl_item_template.cloneNode(true);

        // @ts-ignore
        expansion.li_element.querySelector(".exl-thumb").src = expansion.icon;

        expansion.li_element.querySelector(".exl-title").appendChild(
            expansionsloader_internal_build_anchors(expansion.name)
        );
        expansion.li_element.querySelector(".exl-info-version").appendChild(
            expansionsloader_internal_build_anchors(expansion.version)
        );
        expansion.li_element.querySelector(".exl-info-submiter").appendChild(
            expansionsloader_internal_build_anchors(expansion.name)
        );

        exl_list.appendChild(expansion.li_element);
    }
}

function expansionsloader_internal_index_of(node) {
    let nodes = node.parentNode.children;
    for (let i = 0; i < nodes.length; i++) {
        if (nodes[i] == node) return i;
    }

    console.error("expansionsloader_internal_index_of() unknown node", node);
    throw new Error("unknown node");
}

function expansionsloader_internal_show_info(/**@type {Expansion}*/expansion) {
    /**@type {HTMLImageElement}*/
    let preview = document.querySelector(".exl-preview");
    let summary = document.querySelector(".exl-summary");

    preview.style.visibility = expansion ? "" : "hidden";

    if (!expansion) {
        //@ts-ignore
        preview.src = "";
        summary.textContent = "";
        return;
    }

    //@ts-ignore
    preview.src = expansion.screenshoot;
    summary.replaceChildren();
    summary.appendChild(expansionsloader_internal_build_anchors(expansion.description));
}

function expansionsloader_internal_build_anchors(/**@type {string}*/ text) {
    let span = document.createElement("span");
    let index = 0;

    if (!text) return span;

    text = text.replaceAll("\r\n", "\n").replaceAll("\r", "\n");

    while (index < text.length) {
        let found = text.indexOf("http://", index);
        if (found < 0) found = text.indexOf("https://", index);
        if (found < 0) break;

        let end = text.indexOf("\n", found);
        if (end < 0) end = text.indexOf(" ", found);
        if (end < 0) end = text.indexOf(" ", found);
        if (end < 0) end = text.indexOf("\t", found);
        if (end < 0) end = text.length;

        span.appendChild(document.createTextNode(text.substring(index, found)));

        let link = text.substring(found, end);
        let anchor = document.createElement("a");
        anchor.href = link;
        anchor.textContent = link;
        span.appendChild(anchor);

        index = end;
    }

    if (index < text.length)
        span.appendChild(document.createTextNode(text.substring(index)));

    return span;
}

function expansionsloader_internal_toggle_visibility(is_visible) {
    document.getElementById("expansions-dialog").style.display = is_visible ? "" : "none";
    document.getElementById("window").style.display = is_visible ? "none" : "";
}

