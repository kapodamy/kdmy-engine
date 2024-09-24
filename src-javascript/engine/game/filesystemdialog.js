"use strict";

/**
 * @typedef {object} FSEntryKDY
 * @property {string} name
 * @property {number} size
 * @property {HTMLLIElement} li_element
 */
var /**@type {string}*/ filesystemdialog_current_path = null;
var /**@type {FSEntryKDY[]}*/ filesystemdialog_current_entries = null;
var /**@type {boolean}*/ filesystemdialog_choosing_folder = false;
var /**@type {function(string):void}*/ filesystemdialog_resolve = null;

/* __attribute__((constructor)) void expansionsloader_init_defaults() */ {
    document.querySelector(".ase-header-button.home").addEventListener(
        "click", filesystemdialog_internal_enumerate_root, false
    );
    document.getElementById("ase-cancel").addEventListener(
        "click", filesystemdialog_internal_button_cancel, false
    );
    document.getElementById("ase-pick-file").addEventListener(
        "click", filesystemdialog_internal_button_pick, false
    );
    document.getElementById("ase-pick-folder").addEventListener(
        "click", filesystemdialog_internal_button_pick, false
    );
    document.getElementById("ase-path").addEventListener(
        "change", filesystemdialog_internal_path_change, false
    );
    /*document.getElementById("ase-path").addEventListener(
        "keypress", filesystemdialog_internal_path_change, false
    );*/


    let ase_list = document.getElementById("ase-list");

    ase_list.addEventListener("click", filesystemdialog_internal_click, false);
    ase_list.addEventListener("dblclick", filesystemdialog_internal_click, false);
    ase_list.addEventListener("keydown", filesystemdialog_internal_keyDown, false);
}


function filesystemdialog_main(/**@type {boolean}*/folder_or_file_chooser, /**@type {string}*/start_folder) {
    return new Promise(async function (resolve, reject) {
        if (!start_folder || start_folder == FS_CHAR_SEPARATOR) {
            filesystemdialog_resolve = resolve;
            filesystemdialog_choosing_folder = folder_or_file_chooser;
            filesystemdialog_internal_toggle_visibility(true);
            filesystemdialog_internal_enumerate_root();
            return;
        }

        let native_start_folder;
        try {
            native_start_folder = await io_native_get_path(start_folder, false, true, false);
            if (!await io_native_resource_exists(native_start_folder, false, true)) {
                reject(new KDMYEngineIOError(`The path "${start_folder}" is not a valid folder`, start_folder));
                return;
            }
        } catch (e) {
            reject(e);
        }
        filesystemdialog_resolve = resolve;
        filesystemdialog_choosing_folder = folder_or_file_chooser;
        filesystemdialog_internal_toggle_visibility(true);
        filesystemdialog_internal_enumerate(start_folder);
    });
}


function filesystemdialog_internal_click(/**@type {MouseEvent}*/e) {
    let list = document.getElementById("ase-list");

    // @ts-ignore
    let /**@type {HTMLElement}*/li_element = e.target;

    if (li_element == list) return;

    while (li_element && li_element.parentElement != list) li_element = li_element.parentElement;

    let selected = list.querySelector(".ase-item-selected");
    if (selected == li_element && e.type != "dblclick") return;

    let index = expansionsloader_internal_index_of(li_element) - 1;

    if (e.type == "dblclick") {
        filesystemdialog_internal_open(index);
        return;
    }

    if (index < 0)
        li_element.classList.add("ase-item-selected");
    else
        filesystemdialog_current_entries[index].li_element.classList.add("ase-item-selected");

    if (selected) selected.classList.remove("ase-item-selected");
}

function filesystemdialog_internal_keyDown(/**@type {KeyboardEvent}*/e) {
    if (e.isComposing) return;

    let list = document.getElementById("ase-list");
    let selected = list.querySelector(".ase-item-selected");
    let index = expansionsloader_internal_index_of(selected) - 1;

    if (e.code == "Enter" && !e.repeat) {
        filesystemdialog_internal_open(index);
    } else if (e.code == "ArrowUp") {
        index--;
        if (index < 0) return;
    } else if (e.code == "ArrowDown") {
        index++;
        if (index >= filesystemdialog_current_entries.length) return;
    } else {
        return;
    }

    selected.classList.remove("ase-item-selected");
    filesystemdialog_current_entries[index].li_element.classList.add("ase-item-selected");
}

async function filesystemdialog_internal_enumerate(path) {
    let absolute_path = await io_native_get_path(path, true, true, false);
    let entries = await io_native_enumerate_folder(absolute_path);

    if (!entries) {
        alert(`Failed to read:\n${path}`);
        return;
    }

    let /**@type {FSEntryKDY[]}*/ folder_entries = new Array();

    for (let entry of entries) {
        if (!entry.is_file && !entry.is_folder) continue;

        folder_entries.push({
            name: entry.name,
            size: entry.is_folder ? -1 : entry.length,
            li_element: null
        });
    }

    filesystemdialog_current_path = path;
    filesystemdialog_current_entries = folder_entries;

    /**@type {HTMLInputElement}*///@ts-ignore
    let ase_path = document.getElementById("ase-path");
    ase_path.value = path;

    filesystemdialog_internal_render(folder_entries, true);
}

function filesystemdialog_internal_render(/**@type {FSEntryKDY[]}*/ entries, add_to_parent) {
    let ase_list = document.getElementById("ase-list");
    ase_list.replaceChildren();

    ase_list.scrollTop = 0;
    let selection = window.getSelection();
    if (
        selection.focusNode == ase_list || ase_list.contains(selection.focusNode) ||
        selection.anchorNode == ase_list || ase_list.contains(selection.anchorNode)
    ) {
        selection.removeAllRanges();
    }

    if (!entries) return;

    /**@type {HTMLLIElement}*///@ts-ignore
    let ase_item_template = document.getElementById("ase-item").content.firstElementChild;

    /**@type {HTMLLIElement}*///@ts-ignore
    let upper = ase_item_template.cloneNode(true);
    upper.classList.add("bold");
    upper.querySelector(".ase-icon").textContent = '📂';
    upper.querySelector(".ase-info-name").textContent = "…"
    upper.querySelector(".ase-info-name").classList.add("bold");
    upper.querySelector(".ase-info-size").textContent = "";
    if (!add_to_parent) upper.style.display = "none";
    ase_list.appendChild(upper);

    for (let entry of entries) {
        // @ts-ignore
        entry.li_element = ase_item_template.cloneNode(true);

        let size;

        if (entry.size < 0)
            size = "folder";
        else
            size = filesystemdialog_internal_get_size(entry.size);

        entry.li_element.querySelector(".ase-icon").textContent = filesystemdialog_internal_get_icon(entry);
        entry.li_element.querySelector(".ase-info-name").textContent = entry.name;
        entry.li_element.querySelector(".ase-info-size").textContent = size;

        if (filesystemdialog_choosing_folder && entry.size >= 0) {
            entry.li_element.classList.add("disabled");
        }

        ase_list.appendChild(entry.li_element);
    }

}

function filesystemdialog_internal_get_icon(/**@type {FSEntryKDY}*/ entry) {
    if (!entry) return '';
    if (!Number.isFinite(entry.size)) return '?';
    if (entry.size < 0) return '📁';

    let idx = entry.name.lastIndexOf('.');
    let ext = idx < 0 ? "" : entry.name.substring(idx + 1).toLowerCase();


    switch (ext) {
        case "xml":
            return '📐';
        case "png":
        case "tiff":
        case "gif":
        case "dds":
        case "jpg":
        case "jepg":
        case "webp":
        case "bmp":
            return '🎴';
        case "ogg":
        case "logg":
        case "wav":
        case "mp3":
        case "m4a":
            return '🎵';
        case "webm":
        case "mkv":
        case "mp4":
        case "m4v":
            return '🎬';
        case "lua":
        case "json":
        case "ini":
            return '📜';
        case "txt":
            return '📃';
        case "ttf":
        case "woff":
        case "woff2":
        case "otf":
            return '🗚';
        default:
            return '📄';
    }

}

function filesystemdialog_internal_get_size(/**@type {number}*/ size) {
    const UNITS = ["bytes", "KiB", "MiB", "GiB", "TiB"];
    const DIVISOR = 1024;

    for (let i = 0; i < UNITS.length; i++) {
        if (size < DIVISOR) {
            let size_str;
            if (Number.isInteger(size))
                size_str = size.toString();
            else
                size_str = size.toFixed(2);

            return `${size_str} ${UNITS[i]}`;
        }
        size /= DIVISOR;
    }

    return `${size} ${UNITS[UNITS.length - 1]}`;
}

function filesystemdialog_internal_toggle_visibility(is_visible) {
    document.getElementById("fs-dialog").style.display = is_visible ? "" : "none";
    document.getElementById("window").style.display = is_visible ? "none" : "";

    if (is_visible)
        document.body.classList.add("fs-dialog-visible");
    else
        document.body.classList.remove("fs-dialog-visible");

    let file_btn = document.getElementById("ase-pick-file");
    let folder_btn = document.getElementById("ase-pick-folder");

    file_btn.style.display = filesystemdialog_choosing_folder ? "none" : "";
    folder_btn.style.display = filesystemdialog_choosing_folder ? "" : "none";
}

function filesystemdialog_internal_open(index) {
    if (index < 0) {
        if (!filesystemdialog_current_path.includes(FS_CHAR_SEPARATOR)) {
            console.warn("filesystemdialog_internal_open() root reached");
            return;
        }

        let parent_folder = fs_get_parent_folder(filesystemdialog_current_path);

        if (parent_folder == FS_CHAR_SEPARATOR || !parent_folder)
            filesystemdialog_internal_enumerate_root();
        else
            filesystemdialog_internal_enumerate(parent_folder);
        return;
    }

    let entry = filesystemdialog_current_entries[index];
    let path = filesystemdialog_current_path + FS_CHAR_SEPARATOR + entry.name;

    if (entry.size < 0) {
        filesystemdialog_internal_enumerate(path);
        return;
    }

    if (filesystemdialog_choosing_folder) return;

    filesystemdialog_internal_resolve(path);
}

function filesystemdialog_internal_resolve(selected_path) {
    let resolve = filesystemdialog_resolve;

    document.getElementById("ase-list").replaceChildren();
    filesystemdialog_current_path = null;
    filesystemdialog_choosing_folder = null;
    filesystemdialog_resolve = null;

    filesystemdialog_internal_toggle_visibility(false);
    resolve(selected_path);
}

function filesystemdialog_internal_button_pick(e) {
    for (let entry of filesystemdialog_current_entries) {
        if (!entry.li_element.classList.contains("ase-item-selected")) continue;

        let is_folder = entry.size < 0;
        if (is_folder == filesystemdialog_choosing_folder) {
            let path = filesystemdialog_current_path + FS_CHAR_SEPARATOR + entry.name;
            filesystemdialog_internal_resolve(path);
        } else if (filesystemdialog_choosing_folder) {
            filesystemdialog_internal_resolve(filesystemdialog_current_path);
        }

        return;
    }

    alert("¡Choose a " + (filesystemdialog_choosing_folder ? "folder" : "file") + "!");
}

function filesystemdialog_internal_button_cancel(e) {
    filesystemdialog_internal_resolve(null);
}

async function filesystemdialog_internal_path_change(e) {
    //if (e.type == "keypress" && e.code != "Enter") return;

    let /**@type {HTMLInputElement}*/ ase_path = e.target;
    let path = ase_path.value;

    try {
        let temp = path.replaceAll(FS_CHAR_SEPARATOR, "")
        if (fs_is_invalid_filename(temp)) {
            alert(`Invalid path "${path}"`);
            ase_path.value = filesystemdialog_current_path;
            return;
        }

        path = fs_resolve_path(path);
        let absolute_path = await io_native_get_path(path, false, true, false);

        if (!await io_native_resource_exists(absolute_path, false, true)) {
            alert(`Can not find "${path}"`);
            ase_path.value = filesystemdialog_current_path;
            return;
        }

        await filesystemdialog_internal_enumerate(path);
    } catch (e) {
        alert(`Failed to open "${path}"`);
        ase_path.value = filesystemdialog_current_path;
    }
}

async function filesystemdialog_internal_enumerate_root() {
    /**@type {HTMLInputElement}*///@ts-ignore
    let ase_path = document.getElementById("ase-path");
    ase_path.value = "/";

    filesystemdialog_current_path = "";
    filesystemdialog_current_entries = new Array();

    let root_paths = [FS_ASSETS_FOLDER, EXPANSIONS_PATH];

    for (let root_path of root_paths) {
        let path = await io_native_get_path(root_path, false, true, false);
        if (await io_native_resource_exists(path, false, true)) {
            filesystemdialog_current_entries.push({
                name: root_path.substring(1), size: -1, li_element: null
            });
        }
    }

    filesystemdialog_internal_render(filesystemdialog_current_entries, false);
}

