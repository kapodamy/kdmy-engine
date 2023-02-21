"use strict";

//
// Rudimentary layout visor
//

const layoutvisor_colors = [
    "tomato", "orange", "dodgerblue", "mediumseagreen",
    "gray", "slateblue", "violet", "crimson",
    "darkkhaki", "darkviolet", "limegreen", "teal"
];

var layoutvisor_layout = null;
var layoutvisor_layoutcamera = null;
var layoutvisor_animate = false;
var layoutvisor_offsetx = 0;
var layoutvisor_offsety = 0;
var layoutvisor_startx = 0;
var layoutvisor_starty = 0;
var layoutvisor_drag = false;
var layoutvisor_viewmode_viewonly = true;
var layoutvisor_viewmode_bycamera = false;
var layoutvisor_origoffsetx = 0;
var layoutvisor_origoffsety = 0;
var layoutvisor_origoffsetz = 0;
var layoutvisor_hookedfndraw = null;
var layoutvisor_hookedfndrawname = null;
var layoutvisor_hookedvertex = null;
var layoutvisor_hookedvertexindex = -1;
var layoutvisor_itemmousemove = false;
var layoutvisor_symbol = Symbol("layoutvisor");
var layoutvisor_characters = new Map();
var layoutvisor_streakcounter = null;
var layoutvisor_rankingcounter = null;
var layoutvisor_rankingaccuracy = null;
var layoutvisor_combobreak = false;
var layoutvisor_input_itemwidth = null;
var layoutvisor_input_itemheight = null;
var layoutvisor_input_itemx = null;
var layoutvisor_input_itemy = null;
var layoutvisor_bindbutton = null;
var layoutvisor_bindto = null;
var layoutvisor_bindtype = null;
var layoutvisor_uianimlist = null;
var layoutvisor_rankingmodelholder = null;
var layoutvisor_streakmodelholder = null;
var layoutvisor_uifont = null;
var layoutvisor_nosing = true;
var layoutvisor_drawabledrawfn = null;
var layoutvisor_placeholderalign = null;
var layoutvisor_placeholderalingvertical = null;
var layoutvisor_placeholderalinghorizontal = null;
var layoutvisor_itemparallaxx = null;
var layoutvisor_itemparallaxy = null;
var layoutvisor_itemparallaxz = null;


async function main_layout_visor() {
    fs_init();
    console.log("running layoutvisor");
    main_layout_add_listeners();

    document.getElementById("pvr-onoff").remove();
    document.getElementById("pvr-status").remove();

    // @ts-ignore
    layoutvisor_drawabledrawfn = drawable_draw;
    // @ts-ignore
    drawable_draw = layoutvisor_drawable_draw_hook;

    //cursor: pointer|move

    let camera_offset = null;
    let camera_offset_s = document.querySelector(".camera-offset");
    let camera_position = null;
    let camera_position_s = document.querySelector(".camera-position");
    let next_hit = 1000;
    let time = 0;
    let playerstats = playerstats_init();
    let directions = ['left', 'down', 'up', 'right'];
    let last_nosing = null;

    layoutvisor_uianimlist = await animlist_init("/assets/common/anims/week-round.xml");
    layoutvisor_rankingmodelholder = await modelholder_init("/assets/common/image/week-round/ranking");
    layoutvisor_streakmodelholder = await modelholder_init("/assets/common/font/numbers");
    layoutvisor_uifont = await fontholder_init("/assets/common/font/vcr.ttf", -1);

    while (1) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        beatwatcher_global_set_timestamp_from_kos_timer();

        if (!layoutvisor_layout) continue;

        if (layoutvisor_animate) layout_animate(layoutvisor_layout, elapsed);
        layout_draw(layoutvisor_layout, pvr_context);

        let new_camera_offset = `x="${layoutvisor_layoutcamera.offset_x}" y="${layoutvisor_layoutcamera.offset_y}" z="${layoutvisor_layoutcamera.offset_z}"`;
        let new_camera_position = `x="${layoutvisor_layoutcamera.modifier.translate_x}" y="${layoutvisor_layoutcamera.modifier.translate_y}" z="${layoutvisor_layoutcamera.modifier.scale_x}"`;
        if (camera_offset != new_camera_offset) {
            camera_offset_s.textContent = camera_offset = new_camera_offset;
        }
        if (camera_position != new_camera_position) {
            camera_position_s.textContent = camera_position = new_camera_position;
        }


        time += elapsed;
        if (layoutvisor_nosing) {
            if (last_nosing != layoutvisor_nosing) {
                last_nosing = layoutvisor_nosing;
                for (let character of layoutvisor_characters.keys()) character_play_idle(character);
            }
        } else if (time > next_hit) {
            next_hit = time + math2d_random(400, 800);
            let diff = math2d_random(0, 110);
            let idle = false;
            let miss = false;

            if (diff >= 108) {
                next_hit += 2000;
                playerstats_add_penality(playerstats, (math2d_random(0, 100) % 2) == 0);
            } else if (diff >= 104) {
                miss = true;
                playerstats_add_miss(playerstats, 1.0);
            } else if (diff > 100) {
                idle = true;
                next_hit += 5000;
            } else {
                playerstats_add_hit(playerstats, 1.0, 100, diff);
            }

            if (idle) {
                for (let character of layoutvisor_characters.keys()) character_play_idle(character);
            } else if (layoutvisor_combobreak) {
                layoutvisor_combobreak = false;
                for (let character of layoutvisor_characters.keys()) {
                    // @ts-ignore
                    let result = character_play_extra(character, "sad", false) ||
                        character_play_extra(character, "shaking", false) ||
                        character_play_extra(character, "hey", false)
                        ;
                }
            } else {
                for (let character of layoutvisor_characters.keys()) {
                    let direction = directions[math2d_random_int(0, directions.length - 1)];
                    if (miss) {
                        // @ts-ignore
                        let result = character_play_miss(character, direction, false) ||
                            character_play_extra(character, "sad", false) ||
                            character_play_extra(character, "shaking", false)
                            ;
                    } else {
                        // @ts-ignore
                        let result = character_play_sing(character, direction, false) ||
                            character_play_extra(character, "shaking", false) ||
                            character_play_idle(character)
                            ;
                    }
                }
            }
        }

        if (layoutvisor_rankingcounter)
            rankingcounter_peek_ranking(layoutvisor_rankingcounter, playerstats);
        if (layoutvisor_rankingaccuracy)
            rankingcounter_peek_ranking(layoutvisor_rankingaccuracy, playerstats);

        if (layoutvisor_streakcounter) {
            if (streakcounter_peek_streak(layoutvisor_streakcounter, playerstats)) {
                layoutvisor_streakcounter.last_streak = -8;
                layoutvisor_combobreak = true;
            }
        }

    }
}

function main_layout_add_listeners() {
    window.addEventListener("beforeunload", function (e) { e.returnValue = "¿leave?" }, false);

    document.getElementById("load-layout-file").addEventListener("change", layoutvisor_load, false);
    let input_basefolder = document.querySelector("input[name=base-folder]");

    // @ts-ignore
    // @ts-ignore
    input_basefolder.addEventListener("change", function (e) {
        layoutvisor_localstorage_save("baseLayoutFolder", this.value);
    }, false);
    // @ts-ignore
    input_basefolder.value = layoutvisor_localstorage_load("baseLayoutFolder");

    document.getElementById("widescreen").addEventListener("change", function (e) {
        // @ts-ignore
        let checked = this.checked;

        pvr_context._html5canvas.width = PVR_WIDTH = checked ? 864 : 640;
        pvr_context._html5canvas.height = PVR_HEIGHT = checked ? 486 : 480;
        pvr_context.resolution_changes++;
        webopengl_resize_projection(pvr_context.webopengl, 0, 0, PVR_WIDTH, PVR_HEIGHT);

        let classlist = document.querySelector(".layoutvisor").classList;

        if (checked)
            classlist.add("widescreen");
        else
            classlist.remove("widescreen");

        if (layoutvisor_layout) layout_update_render_size(layoutvisor_layout, PVR_WIDTH, PVR_HEIGHT);

    }, false);

    layoutvisor_loadoffsets();
    window.onscroll = layoutvisor_loadoffsets;
    window.onresize = layoutvisor_loadoffsets;
    pvr_context._html5canvas.onresize = layoutvisor_loadoffsets;

    pvr_context._html5canvas.onmousedown = layoutvisor_mousedown;
    window.onmousemove = layoutvisor_mousemove;
    window.onmouseup = layoutvisor_mouseup;
    pvr_context._html5canvas.onwheel = layoutvisor_mousewheel;

    pvr_context._html5canvas.onkeydown = function (e) {
        if (layoutvisor_layoutcamera) {
            switch (e.code) {
                case "ArrowLeft":
                    layoutvisor_layoutcamera.offset_x--;
                    break;
                case "ArrowRight":
                    layoutvisor_layoutcamera.offset_x++;
                    break;
                case "ArrowUp":
                    layoutvisor_layoutcamera.offset_y--;
                    break;
                case "ArrowDown":
                    layoutvisor_layoutcamera.offset_y++;
                    break;
                case "Numpad+":
                    layoutvisor_layoutcamera.offset_z += 0.001;
                    break;
                case "Numpad-":
                    layoutvisor_layoutcamera.offset_z += 0.001;
                    break;
            }
        }
    }

    let last_click = document.getElementById("viewmode-offset");

    document.getElementById("viewmode-offset").addEventListener("click", function () {
        if (last_click == this) return;
        last_click = this;
        layoutvisor_viewmode_viewonly = true;
        layoutvisor_viewmode_bycamera = false;
    });
    document.getElementById("viewmode-camera").addEventListener("click", function () {
        if (last_click == this) return;
        last_click = this;
        layoutvisor_viewmode_viewonly = false;
        layoutvisor_viewmode_bycamera = true;
    });

    document.getElementById("camera-offset-reset").addEventListener("click", function () {
        if (layoutvisor_layoutcamera) {
            camera_set_offset(
                layoutvisor_layoutcamera,
                layoutvisor_origoffsetx,
                layoutvisor_origoffsety,
                layoutvisor_origoffsetz
            );
        }
    });
    document.getElementById("camera-position-reset").addEventListener("click", function () {
        if (layoutvisor_layoutcamera) {
            camera_move(layoutvisor_layoutcamera, 0, 0, 1);
            camera_apply(layoutvisor_layoutcamera, null);
        }
    });

    document.getElementById("camera-offset-set").addEventListener("click", function () {
        if (!layoutvisor_layoutcamera) return;

        let result = layoutvisor_camera_prompt(
            layoutvisor_layoutcamera.offset_x,
            layoutvisor_layoutcamera.offset_y,
            layoutvisor_layoutcamera.offset_z,
            1
        );

        if (result) {
            camera_set_offset(layoutvisor_layoutcamera, result[0], result[1], result[2]);
        }
    });
    document.getElementById("camera-position-set").addEventListener("click", function () {
        if (!layoutvisor_layoutcamera) return;

        let result = layoutvisor_camera_prompt(
            layoutvisor_layoutcamera.modifier.translate_x,
            layoutvisor_layoutcamera.modifier.translate_y,
            layoutvisor_layoutcamera.modifier.scale_x,
        );

        if (result) {
            camera_move(layoutvisor_layoutcamera, result[0], result[1], result[2]);
            camera_apply(layoutvisor_layoutcamera, null);
        }
    });

    let layout_cameras = document.getElementById("layout-cameras");

    function apply_camera() {
        // @ts-ignore
        let index = layout_cameras.selectedIndex;
        // @ts-ignore
        let length = layout_cameras.options.length;

        if (index < 0 || index >= length) return false;
        if (!layoutvisor_layout) return false;

        let orig = layout_get_camera_placeholder;
        window["layout_get_camera_placeholder"] = layout => layout.camera_list[index];
        camera_from_layout(layoutvisor_layoutcamera, layoutvisor_layout, null);
        window["layout_get_camera_placeholder"] = orig;
        return true;
    };

    function apply_animate() {
        // @ts-ignore
        layoutvisor_animate = this.id == "displaymode-animate";
    }

    function show_bind_config(e) {
        // @ts-ignore
        let value = e.target.value;

        // @ts-ignore
        document.getElementById("item-bind-type-ranking").style.display = value == "ranking" ? "block" : "none";
        // @ts-ignore
        document.getElementById("item-bind-type-streak").style.display = value == "streak" ? "block" : "none";
        // @ts-ignore
        document.getElementById("item-bind-type-accuracy").style.display = value == "accuracy" ? "block" : "none";
        // @ts-ignore
        document.getElementById("item-bind-type-character").style.display = value == "character" ? "block" : "none";
    }

    document.getElementById("layout-cameras-start").addEventListener("click", function () {
        if (apply_camera()) {
            if (!layoutvisor_animate) {
                camera_animate(layoutvisor_layoutcamera, 0);
                camera_apply(layoutvisor_layoutcamera, null);
            }
        }
    });
    document.getElementById("layout-cameras-end").addEventListener("click", function () {
        if (apply_camera()) {
            camera_end(layoutvisor_layoutcamera);
            camera_apply(layoutvisor_layoutcamera, null);
        }
    });

    document.getElementById("layout-items").addEventListener("change", function () {
        // @ts-ignore
        let index = this.selectedIndex;
        layoutvisor_bindto.style.display = "none";
        if (!layoutvisor_layout || index < 0 && index >= layoutvisor_layout.vertex_list_size) return;
        layoutvisor_draw_hook_for(index);
        layoutvisor_pick_items_values(index);
    });
    document.getElementById("item-mousemove").addEventListener("click", function () {
        // @ts-ignore
        layoutvisor_itemmousemove = this.checked;
        // @ts-ignore
        this.parentNode.style.fontWeight = this.checked ? "bold" : "";
    });
    document.getElementById("displaymode-render").addEventListener("click", apply_animate);
    document.getElementById("displaymode-animate").addEventListener("click", apply_animate);
    document.getElementById("anim-speed").addEventListener("change", function () {
        // @ts-ignore
        layoutvisor_layout.animation_speed = this.valueAsNumber;
    });

    document.getElementById("layout-triggers-execute").addEventListener("click", function () {
        if (!layoutvisor_layout) return;
        let triggers = document.getElementById("layout-triggers");
        // @ts-ignore
        let index = triggers.selectedIndex;
        if (index < 0 || index >= layoutvisor_layout.trigger_list_size) return;

        layout_trigger_trigger(layoutvisor_layout, layoutvisor_layout.trigger_list[index].name);
    });
    document.getElementById("layout-actions-execute").addEventListener("click", function () {
        if (!layoutvisor_layout) return;
        /** @type {HTMLSelectElement} */
        // @ts-ignore
        let actions = document.getElementById("layout-actions");
        let index = actions.selectedIndex;
        if (index < 0 || index >= actions.options.length) return;

        let option = actions.options[index];
        let name = option.getAttribute("isnull") == (true).toString() ? null : option.textContent;
        layout_trigger_action(layoutvisor_layout, null, name);
    });
    document.getElementById("layout-triggers-stop").addEventListener("click", function () {
        if (!layoutvisor_layout) return;
        layout_stop_all_triggers(layoutvisor_layout);
    });


    layoutvisor_input_itemwidth = document.getElementById("item-width");
    layoutvisor_input_itemheight = document.getElementById("item-height");
    layoutvisor_input_itemx = document.getElementById("item-x");
    layoutvisor_input_itemy = document.getElementById("item-y");
    layoutvisor_placeholderalingvertical = document.getElementById("item-placeholder-align-vertical");
    layoutvisor_placeholderalinghorizontal = document.getElementById("item-placeholder-align-horizontal");
    layoutvisor_itemparallaxx = document.getElementById("item-parallax-x");
    layoutvisor_itemparallaxy = document.getElementById("item-parallax-y");
    layoutvisor_itemparallaxz = document.getElementById("item-parallax-z");

    layoutvisor_input_itemwidth.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_input_itemheight.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_input_itemx.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_input_itemy.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_itemparallaxx.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_itemparallaxy.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_itemparallaxz.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_placeholderalingvertical.addEventListener("change", layoutvisor_handle_item_event);
    layoutvisor_placeholderalinghorizontal.addEventListener("change", layoutvisor_handle_item_event);

    layoutvisor_placeholderalign = document.getElementById("item-placeholder-align");
    layoutvisor_bindbutton = document.getElementById("item-bind");
    layoutvisor_bindto = document.getElementById("item-bind-to");
    layoutvisor_bindtype = document.getElementById("item-bind-type");

    layoutvisor_bindbutton.addEventListener("click", function () {
        if (layoutvisor_bindto.style.display == "block") {
            layoutvisor_bindto.style.display = "none";
            return;
        }

        layoutvisor_bindtype.style.display = "block";
        layoutvisor_bindto.style.display = "block";

        // @ts-ignore
        let index = document.getElementById("layout-items").selectedIndex;
        let layout_placeholder = layoutvisor_layout.vertex_list[index].placeholder;
        let target_input = "input[name=item-bind-ranking]";

        if (layout_placeholder.name != null) {
            if (layout_placeholder.name.startsWith("character_")) {
                target_input = "[name=item-bind-type][value=character]";

                /**@type {HTMLInputElement} */// @ts-ignore
                let input_scale = document.getElementById("bind-character_scale");

                // pick attached value "character_girlfriend_scale" or "character_scale_###"
                let name = layout_placeholder.name;
                if (name == "character_girlfriend")
                    name = "character_girlfriend_scale";
                else
                    name = name.replace(/^(character)(_.+)/, "$1_scale$2")

                let scale = layout_get_attached_value_as_float(layoutvisor_layout, name, NaN);

                if (Number.isFinite(scale)) input_scale.valueAsNumber = scale;

            } else if (layout_placeholder.name == "ui_rankingcounter_rank")
                target_input = "[name=item-bind-type][value=ranking]";
            else if (layout_placeholder.name == "ui_rankingcounter_accuracy")
                target_input = "[name=item-bind-type][value=accuracy]";
            else if (layout_placeholder.name == "ui_streakcounter")
                target_input = "[name=item-bind-type][value=streak]";
        }

        let target = document.querySelector(target_input);
        // @ts-ignore
        target.checked = true;
        show_bind_config({ target });
    });

    for (let radio of layoutvisor_bindtype.querySelectorAll("input[type=radio]")) {
        radio.addEventListener("change", show_bind_config, true);
    }

    document.getElementById("item-bind-confirm").addEventListener("click", async function () {
        // @ts-ignore
        let index = document.getElementById("layout-items").selectedIndex;
        let layout_placeholder = layoutvisor_layout.vertex_list[index].placeholder;

        // @ts-ignore
        if (document.querySelector("[name=item-bind-type][value=character]").checked)
            await layoutvisor_update_character(layout_placeholder);
        // @ts-ignore
        else if (document.querySelector("[name=item-bind-type][value=ranking]").checked)
            layoutvisor_update_rankingcounter(layout_placeholder);
        // @ts-ignore
        else if (document.querySelector("[name=item-bind-type][value=accuracy]").checked)
            layoutvisor_update_rankingaccuracy(layout_placeholder);
        // @ts-ignore
        else if (document.querySelector("[name=item-bind-type][value=streak]").checked)
            layoutvisor_update_streakcounter(layout_placeholder);
        // @ts-ignore
        else if (document.querySelector("[name=item-bind-type][value=none]").checked)
            layoutvisor_update_none(layout_placeholder);

        layoutvisor_draw_hook_for(index);
    });

    let character_manifest = document.getElementById("bind-character_manifest");
    let value = layoutvisor_localstorage_load("characterManifest");

    // @ts-ignore
    if (value) character_manifest.value = value;

    character_manifest.addEventListener("change", function () {
        // @ts-ignore
        layoutvisor_localstorage_save("characterManifest", this.value);
    });

    document.getElementById("sing").addEventListener("change", function () {
        //@ts-ignore
        layoutvisor_nosing = !this.checked;
    });
}

async function layoutvisor_load(e) {
    if (layoutvisor_layout) layout_destroy(layoutvisor_layout);
    layoutvisor_layout = null;
    layoutvisor_layoutcamera = null;
    layoutvisor_origoffsetx = 0;
    layoutvisor_origoffsety = 0;
    layoutvisor_origoffsetz = 1;
    layoutvisor_draw_hook_for(-1);

    for (let character of layoutvisor_characters.keys()) character_destroy(character);
    layoutvisor_characters.clear();

    let cameras = document.getElementById("layout-cameras");
    cameras.textContent = "";
    let items = document.getElementById("layout-items");
    items.textContent = "";
    let triggers = document.getElementById("layout-triggers");
    triggers.textContent = "";
    let actions = document.getElementById("layout-actions");
    actions.textContent = "";

    layoutvisor_bindto.style.display = "none";

    /**@type {HTMLInputElement} */
    let input = e.target;
    let file = input.files[0];
    /**@type {HTMLInputElement} */
    let input2 = document.querySelector("input[name=base-folder]");
    let base_folder = input2.value.replace(/\\/g, '/');

    input.value = null;

    let orig_readxml = fs_readxml;
    window["fs_readxml"] = async function () {
        window["fs_readxml"] = orig_readxml;

        let text = await file.text();
        let xml = new DOMParser().parseFromString(text, "text/xml");

        if (xml.querySelector("parsererror")) {
            let message = "";
            for (let child of xml.querySelector("parsererror").children) {
                let msg = child.textContent.trim();
                if (msg.length > 0) message += child.textContent + "\n";
            }

            alert("can not read " + file.name);
            throw new SyntaxError("File: " + file.name + "\n" + message.trim());
        }
        return xml;
    }

    let fake_path = fs_build_path(base_folder, file.name);
    console.log(`#########################`);
    console.info(`Loading layout ${file.name} (fake path is "${fake_path}")`);

    layoutvisor_layout = await layout_init(fake_path);
    if (!layoutvisor_layout) return;

    layoutvisor_origoffsetx = layoutvisor_layout.camera_helper.offset_x;
    layoutvisor_origoffsety = layoutvisor_layout.camera_helper.offset_y;
    layoutvisor_origoffsetz = layoutvisor_layout.camera_helper.offset_z;
    layoutvisor_layoutcamera = layoutvisor_layout.camera_helper;
    //camera_apply(layoutvisor_layout.camera_helper, null);

    // @ts-ignore
    cameras.selectedIndex = -1;
    for (let i = 0; i < layoutvisor_layout.camera_list_size; i++) {
        let name = layoutvisor_layout.camera_list[i].name;
        if (!name) name = "<null>";
        cameras.appendChild(document.createElement("option")).textContent = name;
    }

    // @ts-ignore
    items.selectedIndex = -1;
    for (let i = 0; i < layoutvisor_layout.vertex_list_size; i++) {
        let name = layoutvisor_layout.vertex_list[i].name;
        if (!name && layoutvisor_layout.vertex_list[i].placeholder) {
            name = layoutvisor_layout.vertex_list[i].placeholder.name;
        } else if (!name) {
            let vertex = layoutvisor_layout.vertex_list[i].vertex;
            switch (layoutvisor_layout.vertex_list[i].type) {
                case VERTEX_SPRITE:
                    if (vertex.texture && vertex.texture.src_filename) {
                        let j = Math.max(vertex.texture.src_filename.lastIndexOf('/'), vertex.texture.src_filename.lastIndexOf('\\'));
                        if (j < 0)
                            name = vertex.texture.src_filename;
                        else
                            name = vertex.texture.src_filename.substring(j + 1);

                        name = "<texture=" + name + ">";
                    } else {
                        name = "<rgb=" + math2d_color_float_to_css_color(vertex.vertex_color, 0).substring(1) + ">";
                    }
                    break;
                case VERTEX_TEXTSPRITE:
                    if (vertex.text) {
                        name = vertex.text.substring(0, 16);
                        if (vertex.text.length >= 16) name += "...";
                    } else {
                        name = "<textsprite @ " + i + ">";
                    }
                    name = "<text=" + name + ">";
                    break;
            }

            if (!name) name = "<null>";
        }
        items.appendChild(document.createElement("option")).textContent = name;

        if (layoutvisor_layout.vertex_list[i].type == VERTEX_DRAWABLE) {
            layoutvisor_set_placeholder(layoutvisor_layout.vertex_list[i].placeholder, i);
        }
    }

    // @ts-ignore
    triggers.selectedIndex = -1;
    for (let i = 0; i < layoutvisor_layout.trigger_list_size; i++) {
        let name = layoutvisor_layout.trigger_list[i].name;
        if (!name) name = "<null>";
        triggers.appendChild(document.createElement("option")).textContent = name;
    }

    // @ts-ignore
    actions.selectedIndex = -1;
    let added = new Array();
    for (let i = 0; i < layoutvisor_layout.vertex_list_size; i++) {
        let item = layoutvisor_layout.vertex_list[i];
        for (let j = 0; j < item.actions_size; j++) {
            let name = item.actions[j].name;

            if (added.includes(name)) continue;
            added.push(name);

            let option = document.createElement("option");
            option.setAttribute("isnull", (name == null).toString());

            if (!name) name = "<null>";
            actions.appendChild(option).textContent = name;
        }
    }
    for (let i = 0; i < layoutvisor_layout.group_list_size; i++) {
        let group = layoutvisor_layout.group_list[i];
        for (let j = 0; j < group.actions_size; j++) {
            let name = group.actions[j].name;

            if (added.includes(name)) continue;
            added.push(name);

            let option = document.createElement("option");
            option.setAttribute("isnull", (name == null).toString());

            if (!name) name = "<null>";
            actions.appendChild(option).textContent = name;
        }
    }
    for (let i = 0; i < layoutvisor_layout.sound_list_size; i++) {
        let sound = layoutvisor_layout.sound_list[i];
        for (let j = 0; j < sound.actions_size; j++) {
            let name = sound.actions[j].name;

            if (added.includes(name)) continue;
            added.push(name);

            let option = document.createElement("option");
            option.setAttribute("isnull", (name == null).toString());

            if (!name) name = "<null>";
            actions.appendChild(option).textContent = name;
        }
    }
    for (let i = 0; i < layoutvisor_layout.video_list_size; i++) {
        let video = layoutvisor_layout.video_list[i];
        for (let j = 0; j < video.actions_size; j++) {
            let name = video.actions[j].name;

            if (added.includes(name)) continue;
            added.push(name);

            let option = document.createElement("option");
            option.setAttribute("isnull", (name == null).toString());

            if (!name) name = "<null>";
            actions.appendChild(option).textContent = name;
        }
    }

    // @ts-ignore
    if (items.selectedIndex >= 0) layoutvisor_pick_items_values(items.selectedIndex);

    // @ts-ignore
    document.getElementById("anim-speed").value = layoutvisor_layout.animation_speed;
    layoutvisor_draw_hook_for(0);

    // @ts-ignore
    document.getElementById("bind-ui_rankingcounter_accuracy_percent").checked = layout_get_attached_value(
        layoutvisor_layout, "ui_rankingcounter_accuracy_percent", LAYOUT_TYPE_BOOLEAN, false
    );
    // @ts-ignore
    document.getElementById("bind-ui_streakcounter_comboHeight").value = layout_get_attached_value(
        layoutvisor_layout, "ui_streakcounter_comboHeight", LAYOUT_TYPE_FLOAT, 0
    );
    // @ts-ignore
    document.getElementById("bind-ui_streakcounter_numberGap").value = layout_get_attached_value(
        layoutvisor_layout, "ui_streakcounter_numberGap", LAYOUT_TYPE_FLOAT, 0
    );
    // @ts-ignore
    document.getElementById("bind-ui_streakcounter_delay").value = layout_get_attached_value(
        layoutvisor_layout, "ui_streakcounter_delay", LAYOUT_TYPE_FLOAT, 0
    );
}

function layoutvisor_loadoffsets() {
    let rect = pvr_context._html5canvas.getBoundingClientRect();
    layoutvisor_offsetx = rect.left;
    layoutvisor_offsety = rect.top;
}


function layoutvisor_isitemcliked(mx, my, shape) {
    // is this shape an image?
    if (shape.image) {
        // this is a rectangle
        var rLeft = shape.x;
        var rRight = shape.x + shape.width;
        var rTop = shape.y;
        var rBott = shape.y + shape.height;
        // math test to see if mouse is inside image
        if (mx > rLeft && mx < rRight && my > rTop && my < rBott) {
            return (true);
        }
    }
    // the mouse isn't in any of this shapes
    return (false);
}

function layoutvisor_mousedown(evt) {
    if (evt.button != 0) return;

    if (!layoutvisor_viewmode_viewonly && !layoutvisor_viewmode_bycamera) return;
    pvr_context._html5canvas.focus();

    layoutvisor_startx = Math.trunc(evt.clientX - layoutvisor_offsetx);
    layoutvisor_starty = Math.trunc(evt.clientY - layoutvisor_offsety);
    layoutvisor_drag = true;
}

// @ts-ignore
function layoutvisor_mouseup(evt) {
    if (layoutvisor_drag) layoutvisor_drag = false;
}

function layoutvisor_mousemove(evt) {
    if (!layoutvisor_drag) return;
    evt.preventDefault();
    evt.stopPropagation();

    let x = Math.trunc(evt.clientX - layoutvisor_offsetx);
    let y = Math.trunc(evt.clientY - layoutvisor_offsety);
    let dx = x - layoutvisor_startx;
    let dy = y - layoutvisor_starty;
    layoutvisor_startx = x;
    layoutvisor_starty = y;

    if (layoutvisor_itemmousemove && layoutvisor_hookedvertex) {
        x = layoutvisor_input_itemx.valueAsNumber += dx * 2;
        y = layoutvisor_input_itemy.valueAsNumber += dy * 2;

        layoutvisor_move_item(layoutvisor_hookedvertexindex, x, y);
        return;
    }

    dx *= 3;
    dy *= 3;

    if (layoutvisor_viewmode_viewonly && layoutvisor_layoutcamera) {
        let x = layoutvisor_layoutcamera.offset_x += dx;
        let y = layoutvisor_layoutcamera.offset_y += dy;
        camera_move_offset(layoutvisor_layoutcamera, x, y, NaN);
        camera_apply(layoutvisor_layoutcamera, null);
    }
    if (layoutvisor_viewmode_bycamera && layoutvisor_layoutcamera) {
        dx += layoutvisor_layoutcamera.parallax_x;
        dy += layoutvisor_layoutcamera.parallax_y;
        camera_move(layoutvisor_layoutcamera, dx, dy, NaN);
        camera_apply(layoutvisor_layoutcamera, null);
    }
}

function layoutvisor_mousewheel(evt) {
    const SCALE = -0.0001;
    const SCALE_SIZE = 5;
    evt.preventDefault();


    if (layoutvisor_drag && layoutvisor_itemmousemove) {
        if (!layoutvisor_hookedvertex) return;
        let value = Math.sign(evt.deltaY) * SCALE_SIZE;
        let ratio = layoutvisor_input_itemwidth.valueAsNumber / layoutvisor_input_itemheight.valueAsNumber;
        let width = layoutvisor_input_itemwidth.valueAsNumber += value * ratio;
        let height = layoutvisor_input_itemheight.valueAsNumber += value;
        layoutvisor_resize_item(layoutvisor_hookedvertexindex, width, height);
        return;
    }

    let dz = (evt.deltaY * SCALE);

    if (layoutvisor_viewmode_viewonly && layoutvisor_layoutcamera) {
        let z = layoutvisor_layoutcamera.offset_z + dz;
        if (z > 0) layoutvisor_layoutcamera.offset_z = z;
    }

    if (layoutvisor_viewmode_bycamera && layoutvisor_layoutcamera) {
        let z = layoutvisor_layoutcamera.parallax_z + dz;
        if (z > 0) {
            camera_move(layoutvisor_layoutcamera, NaN, NaN, z);
            camera_apply(layoutvisor_layoutcamera, null);
        }
    }
}

function layoutvisor_drawable_draw_hook(drawable, pvrctx) {
    let x = drawable.modifier.x;
    let y = drawable.modifier.y;
    let selected = layoutvisor_hookedvertex == drawable;

    if (layoutvisor_hookedvertexindex >= 0) {
        let item = layoutvisor_layout.vertex_list[layoutvisor_hookedvertexindex];
        if (item.placeholder) item = item.placeholder;
        if (item.vertex == drawable) {
            x = item.x;
            y = item.y;
        }
    }

    let color = [selected ? 1 : 0, 0, 0];

    pvr_context_draw_solid_color(
        pvr_context, color,
        x - 100000, y,
        100000, 2
    );
    pvr_context_draw_solid_color(
        pvr_context, color,
        x, y - 100000,
        2, 100000
    );
    pvr_context_draw_solid_color(
        pvr_context, color,
        x, y,
        100000, 2
    );
    pvr_context_draw_solid_color(
        pvr_context, color,
        x, y,
        2, 100000
    );

    layoutvisor_drawabledrawfn(drawable, pvrctx);
}

function layoutvisor_draw_hook(vertex, /**@type {PVRContext}*/pvrctx) {
    if (!layoutvisor_hookedfndraw) return;

    if (vertex != null && layoutvisor_hookedvertex == vertex) {
        let internal_placeholder = vertex.layoutvisor == layoutvisor_symbol;

        webopengl_internal_enable_dotted(pvrctx.webopengl, true);
        pvr_context_save(pvrctx);

        if (internal_placeholder && (vertex.modifier.width < 1 || vertex.modifier.height < 1)) {
            let x = vertex.modifier.x;
            let y = vertex.modifier.y;
            let width = vertex.modifier.width < 0 ? 100 : vertex.modifier.width;
            let height = vertex.modifier.height < 0 ? 100 : vertex.modifier.height;

            if (vertex.modifier.width < 1) x -= 50;
            if (vertex.modifier.height < 1) y -= 50;

            pvr_context_set_vertex_alpha(pvr_context, 0.25);
            pvr_context_draw_solid_color(pvrctx, [0, 1, 1], x, y, width, height);
        }

        layoutvisor_hookedfndraw(vertex, pvrctx);
        pvr_context_restore(pvrctx);
        webopengl_internal_enable_dotted(pvrctx.webopengl, false);
    } else {
        layoutvisor_hookedfndraw(vertex, pvrctx);
    }
}

function layoutvisor_draw_hook_for(index) {
    if (layoutvisor_hookedfndraw) {
        window[layoutvisor_hookedfndrawname] = layoutvisor_hookedfndraw;
        layoutvisor_hookedvertex = layoutvisor_hookedfndraw = layoutvisor_hookedfndrawname = null;
    }

    layoutvisor_bindbutton.disabled = true;
    layoutvisor_placeholderalign.disabled = true;

    if (index < 0 || !layoutvisor_layout) return;

    layoutvisor_hookedvertexindex = index;
    let item = layoutvisor_layout.vertex_list[index];
    switch (item.type) {
        case VERTEX_DRAWABLE:
            item = item.placeholder;
            layoutvisor_bindbutton.disabled = false;
            layoutvisor_placeholderalign.disabled = false;
            layoutvisor_hookedfndrawname = "drawable_draw";
            break;
        case VERTEX_SPRITE:
            layoutvisor_hookedfndrawname = "sprite_draw";
            break;
        case VERTEX_TEXTSPRITE:
            layoutvisor_hookedfndrawname = "textsprite_draw";
            break;
        default:
            return;
    }

    layoutvisor_hookedvertex = item.vertex;
    layoutvisor_hookedfndraw = window[layoutvisor_hookedfndrawname];
    window[layoutvisor_hookedfndrawname] = layoutvisor_draw_hook;

}

function layoutvisor_set_placeholder(layout_placeholder, reference_index) {
    let css_color = layoutvisor_colors[reference_index % layoutvisor_colors.length];
    let color_pattern = layoutvisor_createpattern(css_color);

    layout_placeholder.vertex = {
        layoutvisor: layoutvisor_symbol,
        alpha: 1.0,
        offsetcolor: [],
        modifier: {},
        z_index: 0,
        z_offset: 0,
        callback_animate: null,
        private_data: color_pattern,
        visible: 1,
        // @ts-ignore
        callback_draw: function (private_data, pvrctx) {
            let matrix = pvrctx.current_matrix;

            let width = Math.max(this.modifier.width, 0);
            let height = Math.max(this.modifier.height, 0);
            if (this == layoutvisor_hookedvertex) {
                pvr_context_set_vertex_alpha(pvrctx, 0.5);
                pvr_context_draw_solid_color(
                    pvrctx, this.private_data.color,
                    this.modifier.x, this.modifier.y, width, height
                );
                return;
            }
            pvr_context_draw_texture(
                pvrctx, this.private_data.pattern,
                0, 0, 256, 256,
                this.modifier.x, this.modifier.y, width, height
            );
        }
    };

    pvrctx_helper_clear_offsetcolor(layout_placeholder.vertex.offsetcolor);
    drawable_helper_update_from_placeholder(layout_placeholder.vertex, layout_placeholder);
}

function layoutvisor_createpattern(color) {
    let canvas = document.createElement("canvas");
    canvas.width = 256;
    canvas.height = 256;
    let ctx = canvas.getContext("2d");

    let fillColor = ctx.fillStyle.toString().substring(1);
    let r = parseInt(fillColor.substring(0, 2), 16) / 0xFF;
    let g = parseInt(fillColor.substring(2, 4), 16) / 0xFF;
    let b = parseInt(fillColor.substring(4, 6), 16) / 0xFF;

    for (let i = 0; i < 200; i++) {
        ctx.beginPath();
        ctx.strokeStyle = i % 2 ? color : "transparent";
        ctx.lineWidth = 2;
        ctx.lineCap = "round";

        let space = 6;
        ctx.moveTo(i * space + space / 2 - 300, 0);
        ctx.lineTo(0 + i * space + space / 2, 300);
        ctx.stroke();
    }

    let ptr = webopengl_create_texture(pvr_context.webopengl, 256, 256, canvas);

    return {
        color: [r, g, b],
        pattern: texture_init_from_raw(ptr, 256 * 256 * 4, 1, 256, 256, 256, 256)
    }
}


function layoutvisor_pick_items_values(index) {
    let x, y, width, height;
    let item = layoutvisor_layout.vertex_list[index];

    switch (item.type) {
        case VERTEX_DRAWABLE:
            item = item.placeholder;
            x = item.x;
            y = item.y;
            width = item.width;
            height = item.height;
            layoutvisor_drawable_update(item);
            layoutvisor_placeholderalingvertical.value = item.align_vertical.toString();
            layoutvisor_placeholderalinghorizontal.value = item.align_horizontal.toString();
            break;
        case VERTEX_SPRITE:
            x = item.vertex.draw_x;
            y = item.vertex.draw_y;
            width = item.vertex.draw_width;
            height = item.vertex.draw_height;
            break;
        case VERTEX_TEXTSPRITE:
            x = item.vertex.x;
            y = item.vertex.y;
            width = item.vertex.max_width;
            height = item.vertex.max_height;
            break;
    }

    layoutvisor_itemparallaxx.value = item.parallax.x.toString();
    layoutvisor_itemparallaxy.value = item.parallax.y.toString();
    layoutvisor_itemparallaxz.value = item.parallax.z.toString();
    layoutvisor_input_itemwidth.value = width.toString();
    layoutvisor_input_itemheight.value = height.toString();
    layoutvisor_input_itemx.value = x.toString();
    layoutvisor_input_itemy.value = y.toString();
}

function layoutvisor_handle_item_event(evt) {
    if (!layoutvisor_hookedvertex) return;

    let width = layoutvisor_input_itemwidth.valueAsNumber;
    let height = layoutvisor_input_itemheight.valueAsNumber;
    let x = layoutvisor_input_itemx.valueAsNumber;
    let y = layoutvisor_input_itemy.valueAsNumber;
    let px = layoutvisor_itemparallaxx.valueAsNumber;
    let py = layoutvisor_itemparallaxy.valueAsNumber;
    let pz = layoutvisor_itemparallaxz.valueAsNumber;
    let align_vertical = parseInt(layoutvisor_placeholderalingvertical.value);
    let align_horizontal = parseInt(layoutvisor_placeholderalinghorizontal.value);
    let item = layoutvisor_layout.vertex_list[layoutvisor_hookedvertexindex];

    if (item.placeholder) item = item.placeholder;

    switch (evt.target.id) {
        case "item-width":
        case "item-height":
            layoutvisor_resize_item(layoutvisor_hookedvertexindex, width, height);
            break;
        case "item-x":
        case "item-y":
            layoutvisor_move_item(layoutvisor_hookedvertexindex, x, y);
            break;
        case "item-parallax-x":
        case "item-parallax-y":
        case "item-parallax-z":
            item.parallax.x = px;
            item.parallax.y = py;
            item.parallax.z = pz;
            break;
        case "item-placeholder-align-vertical":
        case "item-placeholder-align-horizontal":
            if (item.type == VERTEX_DRAWABLE) {
                item.align_horizontal = align_horizontal;
                item.align_vertical = align_vertical;
                layoutvisor_drawable_update(item);
            }
            break;
    }
}

function layoutvisor_move_item(index, x, y) {
    let item = layoutvisor_layout.vertex_list[index];

    switch (item.type) {
        case VERTEX_DRAWABLE:
            item = item.placeholder;
            item.x = x;
            item.y = y;
            layoutvisor_drawable_update(item);
            break;
        case VERTEX_SPRITE:
            sprite_set_draw_location(item.vertex, x, y);
            break;
        case VERTEX_TEXTSPRITE:
            textsprite_set_draw_location(item.vertex, x, y);
            break;
    }
}

function layoutvisor_resize_item(index, width, height) {
    let item = layoutvisor_layout.vertex_list[index];

    switch (item.type) {
        case VERTEX_DRAWABLE:
            item = item.placeholder;
            item.width = width;
            item.height = height;
            layoutvisor_drawable_update(item);
            break;
        case VERTEX_SPRITE:
            sprite_set_draw_size(item.vertex, width, height);
            break;
        case VERTEX_TEXTSPRITE:
            textsprite_set_max_draw_size(item.vertex, width, height);
            break;
    }
}

// @ts-ignore
function layoutvisor_drawable_update(layout_placeholder) {
    if (layout_placeholder.vertex && layout_placeholder.vertex.layoutvisor == layoutvisor_symbol) {
        drawable_helper_update_from_placeholder(layout_placeholder.vertex, layout_placeholder);
    } else if (!layout_placeholder.vertex) {
        return;
    }

    switch (layout_placeholder.vertex.layoutvisor_type) {
        case "rankingaccuracy":
            layoutvisor_update_rankingaccuracy(layout_placeholder);
            break;
        case "character":
            let character = layout_placeholder.vertex.private_data;
            character_set_draw_location(
                character, layout_placeholder.x, layout_placeholder.y
            );
            character_enable_reference_size(
                character, layout_placeholder.width >= 0 || layout_placeholder.height >= 0
            )
            character_update_reference_size(
                character, layout_placeholder.width, layout_placeholder.height
            );
            character_set_draw_align(
                character, layout_placeholder.align_vertical, layout_placeholder.align_horizontal
            );
            break;
        case "streakcounter":
            let last_streak = layoutvisor_streakcounter.last_streak - 1;
            layoutvisor_update_streakcounter(layout_placeholder);
            layoutvisor_streakcounter.last_streak = last_streak;
            break;
        case "rankingcounter":
            layoutvisor_update_rankingcounter(layout_placeholder);
            break;
    }
}


async function layoutvisor_update_character(layout_placeholder) {
    let old_character = layout_placeholder.layoutvisor_character;
    let charactermanifest = null;

    // @ts-ignore
    let character_facing = document.getElementById("bind-character_facing").checked;
    // @ts-ignore
    let manifest_path = document.getElementById("bind-character_manifest").value.replace(/\\/g, '/');
    // @ts-ignore
    let scale = document.getElementById("bind-character_scale").valueAsNumber;

    try {
        charactermanifest = await charactermanifest_init(manifest_path, 1);
    } catch (e) {
        console.error(e);
        alert(e.message);
        return;
    }

    let enable_reference_size = layout_placeholder.width >= 0 || layout_placeholder.height >= 0;
    let character = await character_init(charactermanifest);
    charactermanifest_destroy(charactermanifest);

    character_set_scale(character, scale)
    character_set_draw_location(character, layout_placeholder.x, layout_placeholder.y);
    character_set_z_index(character, layout_placeholder.z);
    character_update_reference_size(character, layout_placeholder.width, layout_placeholder.height);
    character_enable_reference_size(character, enable_reference_size);
    character_set_draw_align(character, layout_placeholder.align_vertical, layout_placeholder.align_horizontal);
    character_state_toggle(character, null);

    if (character_facing) character_flip_orientation(character, character_facing);

    // obligatory, otherwise, does not render nothing if the animation is disabled
    character_animate(character, 0);
    //character_play_idle(character);

    layoutvisor_characters.set(character, manifest_path);
    layout_placeholder.vertex = character_get_drawable(character);
    layout_placeholder.vertex.layoutvisor_type = "character";

    if (old_character) {
        layoutvisor_characters.delete(old_character);
        character_destroy(old_character);
    }
}

function layoutvisor_update_streakcounter(layout_placeholder) {
    layoutvisor_dispose_character(layout_placeholder);
    if (layoutvisor_streakcounter) {
        streakcounter_destroy(layoutvisor_streakcounter);
        layoutvisor_streakcounter.layoutvisor.vertex = null;
    }

    // @ts-ignore
    let streakcounter_comboheight = document.getElementById("bind-ui_streakcounter_comboHeight").valueAsNumber;
    // @ts-ignore
    let streakcounter_numbergap = document.getElementById("bind-ui_streakcounter_numberGap").valueAsNumber;
    // @ts-ignore
    let streakcounter_delay = document.getElementById("bind-ui_streakcounter_delay").valueAsNumber;

    layoutvisor_streakcounter = streakcounter_init(
        layout_placeholder,
        streakcounter_comboheight,
        streakcounter_numbergap,
        streakcounter_delay
    );
    streakcounter_state_add(
        layoutvisor_streakcounter, layoutvisor_rankingmodelholder, layoutvisor_streakmodelholder, null
    );
    streakcounter_state_toggle(layoutvisor_streakcounter, null);
    streakcounter_set_number_animation(layoutvisor_streakcounter, layoutvisor_uianimlist);
    streakcounter_set_combo_animation(layoutvisor_streakcounter, layoutvisor_uianimlist);
    layoutvisor_streakcounter.layoutvisor = layout_placeholder;
    layout_placeholder.vertex.layoutvisor_type = "streakcounter";
}

function layoutvisor_update_rankingcounter(layout_placeholder) {
    layoutvisor_dispose_character(layout_placeholder);
    if (layoutvisor_rankingcounter) {
        rankingcounter_destroy(layoutvisor_rankingcounter);
        layoutvisor_rankingcounter.layoutvisor.vertex = null;
    }

    layoutvisor_rankingcounter = rankingcounter_init(
        layout_placeholder, null, layoutvisor_uifont
    );
    rankingcounter_add_state(layoutvisor_rankingcounter, layoutvisor_rankingmodelholder, null);
    rankingcounter_toggle_state(layoutvisor_rankingcounter, null);
    rankingcounter_set_default_ranking_animation(layoutvisor_rankingcounter, layoutvisor_uianimlist);
    layoutvisor_rankingcounter.layoutvisor = layout_placeholder;
    layout_placeholder.vertex.layoutvisor_type = "rankingcounter";
}

function layoutvisor_update_rankingaccuracy(layout_placeholder) {
    layoutvisor_dispose_character(layout_placeholder);
    if (layoutvisor_rankingaccuracy) {
        rankingcounter_destroy(layoutvisor_rankingaccuracy);
        layoutvisor_rankingaccuracy.layoutvisor.vertex = null;
    }

    // @ts-ignore
    let accuracy_percent = document.getElementById("bind-ui_rankingcounter_accuracy_percent").checked;

    layoutvisor_rankingaccuracy = rankingcounter_init(
        null, layout_placeholder, layoutvisor_uifont
    );
    rankingcounter_add_state(layoutvisor_rankingaccuracy, layoutvisor_rankingmodelholder, null);
    rankingcounter_toggle_state(layoutvisor_rankingaccuracy, null);
    rankingcounter_set_default_ranking_text_animation(layoutvisor_rankingaccuracy, layoutvisor_uianimlist);
    rankingcounter_use_percent_instead(layoutvisor_rankingaccuracy, accuracy_percent);
    layoutvisor_rankingaccuracy.layoutvisor = layout_placeholder;
    layout_placeholder.vertex.layoutvisor_type = "rankingaccuracy";
}

function layoutvisor_update_none(layout_placeholder) {
    layoutvisor_dispose_character(layout_placeholder);
    layout_placeholder.vertex = null;

    let index = -1;
    for (let i = 0; i < layoutvisor_layout.vertex_list_size; i++) {
        if (layoutvisor_layout.vertex_list[i].placeholder == layout_placeholder) {
            index = i;
            break;
        }
    }

    if (index < 0) index = 0;
    layoutvisor_set_placeholder(layout_placeholder, index);
}


function layoutvisor_dispose_character(layout_placeholder) {
    if (!layout_placeholder.layoutvisor_character) return;
    layoutvisor_characters.delete(layout_placeholder.layoutvisor_character);
    character_destroy(layout_placeholder.layoutvisor_character);
}

function layoutvisor_camera_prompt(x, y, z, is_offset) {
    const rx = /(?:[xX][=:][\t\s]*"?(-?[\d\.\,]+)"?)?[\t\s]*(?:[yY][=:][\t\s]*"?(-?[\d\.\,]+)"?)?[\t\s]*(?:[zZ][=:][\t\s]*"?(-?[\d\.\,]+)"?)?/;

    let result = prompt(
        "Enter the camera " + (is_offset ? "offset" : "parallax") + ":",
        `x="${x}" y="${y}" z="${z}"`
    );

    if (!result || result.length < 1) return null;

    let matches = rx.exec(result);
    if (matches.length < 1) return null;

    return [
        Number.parseFloat(matches[1]),
        Number.parseFloat(matches[2]),
        Number.parseFloat(matches[3])
    ];
}

function layoutvisor_localstorage_save(key_name, value) {
    /**@type {string[]} */
    let array;
    let item = localStorage.getItem(key_name); 1

    if (item)
        array = JSON.parse(item);
    else
        array = new Array();

    value = value.replace(/\\/g, '/');

    array.unshift(value);

    for (let i = 1; i < array.length; i++) {
        if (array[i] != value) continue;
        array.splice(i, 1);
        i--;
    }

    localStorage.setItem(key_name, JSON.stringify(array));
    layoutvisor_localstorage_load(key_name);
}

function layoutvisor_localstorage_load(key_name) {
    /**@type {string[]} */
    let array;
    let item = localStorage.getItem(key_name);

    if (item) {
        array = JSON.parse(item);
    } else {
        array = new Array();

        /**@type {HTMLDataListElement} */// @ts-ignore
        let datalist = document.getElementById(`predefined-${key_name}-default`);

        for (let value of datalist.options) {
            array.push(value.value)
        }

        localStorage.setItem(key_name, JSON.stringify(array));
    }

    /**@type {HTMLDataListElement} */// @ts-ignore
    let loaded_datalist = document.getElementById(`predefined-${key_name}`);

    // clear datalist
    loaded_datalist.replaceChildren();

    for (let value of array) {
        let item = document.createElement("option");
        item.value = value;
        loaded_datalist.appendChild(item);
    }

    if (array.length < 1) return "";

    return array[0];
}
