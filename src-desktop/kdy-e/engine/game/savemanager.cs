using System;
using System.Diagnostics;
using Engine.Animation;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Game.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;
using KallistiOS.MAPLE;

namespace Engine.Game {

    public class SaveManager {

        private const string EMPTY = "empty";
        private const string FUNKINSAVE = "funkinsave";
        private const float SCAN_INTERVAL = 1000f;
        private const string LAYOUT = "/assets/common/image/save-manager/layout.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/save-manager/layout~dreamcast.xml";
        private const string MODDING_SCRIPT = "/assets/common/data/scripts/savemanager.lua";

        public static readonly MenuManifest MENU_MANIFEST = new MenuManifest() {
            parameters = new MenuManifest.Parameters() {
                suffix_selected = null,
                suffix_choosen = null,
                suffix_discarded = null,
                suffix_idle = null,// unused
                suffix_rollback = null,// unused
                suffix_in = null,// unused
                suffix_out = null,// unused

                atlas = "/assets/common/image/save-manager/vmu.xml",
                animlist = "/assets/common/image/save-manager/anims.xml",

                anim_selected = "selected",// unused
                anim_choosen = null,// unused
                anim_discarded = null,// unused
                anim_idle = "idle",// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = -50f,
                anim_transition_out_delay = -50f,// negative means start delays from the bottom

                font = null,// unused
                font_glyph_suffix = null,// unused
                font_color_by_difference = false,// unused
                font_size = 0f,// unused
                font_color = 0x00,// unused
                font_border_color = 0x00,// unused
                font_border_size = Single.NaN,// unused

                is_sparse = false,// unused
                is_vertical = false,
                is_per_page = true,

                items_align = Align.CENTER,// readed from the layout
                items_gap = 40,// readed from the layout
                items_dimmen = 80f,// readed from the layout
                static_index = 0,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = true// unused
            },
            items = null,
            items_size = 0
        };

        /**
         * This variable is set to true if the user does not want load and/or save
         * the progress. {@link this.ShouldShow} will always return -1 if true
         */
        public static bool game_withoutsavedata = false;


        private int error_code;
        private bool save_only;
        private Drawable drawable_wrapper;
        private VMUInfo[] vmu_array;
        private int vmu_size;
        private Menu menu;
        private MessageBox messagebox;
        private ModelHolder modelholder;
        private AnimList animlist;
        private Layout layout;
        private LayoutPlaceholder placeholder;
        private Sprite selected_background;
        private TextSprite selected_label;
        private Gamepad maple_pad;
        private float padding;
        private WeekSelectorHelpText help_cancel;
        private WeekSelectorHelpText help_ok;


        public SaveManager(bool save_only, int error_code) {
            Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
            if (layout == null) throw new Exception("missing savemanager layout");

            float label_height = (float)((double)layout.GetAttachedValue("label_height", AttachedValueType.FLOAT, 24.0));
            uint selected_bg_color = (uint)layout.GetAttachedValue("selected_background_color", AttachedValueType.HEX, 0x000000U);
            uint selected_lbl_color = (uint)layout.GetAttachedValue("selected_label_color", AttachedValueType.HEX, 0xFFFFFFU);

            FontHolder font = layout.GetAttachedFont("font");
            Sprite selected_background = Sprite.InitFromRGB8(selected_bg_color);
            TextSprite selected_label = TextSprite.Init2(font, label_height, selected_lbl_color);
            Gamepad maple_pad = new Gamepad(-1);

            float dimmen = (float)((double)layout.GetAttachedValue("menu_itemDimmen", AttachedValueType.FLOAT, 80.0));
            float gap = (float)((double)layout.GetAttachedValue("menu_itemGap", AttachedValueType.FLOAT, 40.0));
            float scale = (float)((double)layout.GetAttachedValue("menu_itemScale", AttachedValueType.FLOAT, 0.0));
            float padding = dimmen * 0.1f;

            maple_pad.SetButtonsDelay(200);
            selected_label.SetAlign(Align.NONE, Align.CENTER);
            selected_label.SetMaxDrawSize(dimmen * 1.2f, -1);
            selected_label.BorderEnable(true);
            selected_label.BorderSetColorRGBA8(0x000000FF);
            selected_label.BorderSetSize(1f);

            SaveManager.MENU_MANIFEST.parameters.items_dimmen = dimmen;
            SaveManager.MENU_MANIFEST.parameters.items_gap = gap;
            SaveManager.MENU_MANIFEST.parameters.texture_scale = scale;

            LayoutPlaceholder placeholder = layout.GetPlaceholder("menu");
            Debug.Assert(placeholder != null, "missing 'menu' placeholder");

            // for caching only
            ModelHolder modelholder = ModelHolder.Init(SaveManager.MENU_MANIFEST.parameters.atlas);
            AnimList animlist = AnimList.Init(SaveManager.MENU_MANIFEST.parameters.animlist);

            MessageBox messagebox = new MessageBox();
            messagebox.HideImage(true);
            messagebox.UseSmallSize(true);
            messagebox.ShowButtonsIcons(false);

            ModelHolder button_icons = ModelHolder.Init(WeekSelector.BUTTONS_MODEL);
            WeekSelectorHelpText help_cancel = new WeekSelectorHelpText(
                button_icons, layout, 2, false, "b", "Continue without save", null
            );
            WeekSelectorHelpText help_ok = new WeekSelectorHelpText(
                button_icons, layout, 4, false, "a", "Choose VMU", null
            );
            button_icons.Destroy();


            this.error_code = error_code < 1 ? 0 : error_code;
            this.save_only = save_only;
            this.drawable_wrapper = null;
            this.vmu_array = null;
            this.vmu_size = 0;
            this.menu = null;
            this.messagebox = messagebox;

            this.modelholder = modelholder;
            this.animlist = animlist;
            this.layout = layout;
            this.placeholder = placeholder;
            this.selected_background = selected_background;
            this.selected_label = selected_label;
            this.maple_pad = maple_pad;
            this.padding = padding;

            this.help_cancel = help_cancel;
            this.help_ok = help_ok;


            placeholder.vertex = this.drawable_wrapper = new Drawable(
                placeholder.z, this.InternalDraw, this.InternalAnimate
            );

            layout.TriggerAny(save_only ? "save-to" : "load-from");

            layout.ExternalVertexCreateEntries(2);
            layout.ExternalVertexSetEntry(
                0, PVRContextVertex.DRAWABLE, help_ok.GetDrawable(), 0
            );
            layout.ExternalVertexSetEntry(
                1, PVRContextVertex.DRAWABLE, help_cancel.GetDrawable(), 0
            );
        }

        public void Destroy() {
            this.menu.Destroy();
            this.drawable_wrapper.Destroy();
            //free(this.vmu_array);
            this.layout.Destroy();
            this.selected_background.Destroy();
            this.selected_label.Destroy();
            this.maple_pad.Destroy();
            this.messagebox.Destroy();
            this.modelholder.Destroy();
            this.animlist.Destroy();
            this.help_ok.Destroy();
            this.help_cancel.Destroy();

            //free(this);
        }

        public void Show() {
            float location_x, location_y;
            float size_width, size_height;

            int selected_index = -1;
            bool save_or_load_success = false;
            float next_scan = 0.0f;
            bool last_saved_selected = false;
            bool confirm_leave = false;

            Modding modding = new Modding(this.layout, SaveManager.MODDING_SCRIPT);
            modding.native_menu = modding.active_menu = this.menu;
            modding.callback_private_data = null;
            modding.callback_option = null;
            modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);
            modding.HelperNotifyEvent(this.save_only ? "do-save" : "do-load");

            while (!modding.has_exit) {
                int selection_offset_x = 0;
                int selection_offset_y = 0;
                float elapsed = PVRContext.global_context.WaitReady();
                GamepadButtons buttons = this.maple_pad.HasPressedDelayed(MainMenu.GAMEPAD_BUTTONS);

                // check for inserted VMUs
                next_scan -= elapsed;
                if (next_scan <= 0) {
                    int last_vmu_size = this.vmu_size;
                    next_scan = SaveManager.SCAN_INTERVAL;

                    if (InternalFindChanges()) {

                        InternalBuildList();
                        this.menu.SelectIndex(-1);
                        if (last_saved_selected) {
                            this.layout.TriggerAny("save-not-selected");
                            last_saved_selected = false;
                        }

                        if (modding.active_menu == modding.native_menu) modding.active_menu = this.menu;
                        modding.native_menu = this.menu;
                    }

                    if (last_vmu_size > 0 != this.vmu_size > 0) {
                        if (this.vmu_size > 0)
                            this.layout.TriggerAny("no-detected-hide");
                        else
                            this.layout.TriggerAny("no-detected-show");
                    }
                }

                if (this.menu.GetSelectedItemRect(out location_x, out location_y, out size_width, out size_height)) {
                    location_x -= this.padding;
                    location_y -= this.padding;
                    size_width += this.padding * 2.0f;
                    size_height += this.padding * 2.0f;

                    this.selected_background.SetDrawLocation(location_x, location_y);
                    this.selected_background.SetDrawSize(size_width, size_height);
                    this.selected_background.SetVisible(true);

                    this.selected_label.SetDrawLocation(location_x, location_y - 24.0f);
                    this.selected_label.SetVisible(true);
                } else {
                    this.selected_background.SetVisible(false);
                    this.selected_label.SetVisible(false);
                }

                ModdingHelperResult res = modding.HelperHandleCustomMenu(this.maple_pad, elapsed);
                if (res != ModdingHelperResult.CONTINUE) break;
                if (modding.has_halt || modding.active_menu != this.menu) buttons = GamepadButtons.NOTHING;

                PVRContext.global_context.Reset();
                this.layout.Animate(elapsed);
                this.layout.Draw(PVRContext.global_context);

                if (confirm_leave) {
                    this.messagebox.Animate(elapsed);
                    this.messagebox.Draw(PVRContext.global_context);

                    if ((buttons & GamepadButtons.A).Bool()) {
                        SaveManager.game_withoutsavedata = true;
                        break;
                    }
                    if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)).Bool()) confirm_leave = false;
                    continue;
                }

                if (this.error_code != 0) {
                    this.messagebox.HideButtons();
                    this.messagebox.Show(false);

                    InternalShowError(this.error_code);

                    this.error_code = 0;
                    continue;
                }

                if ((buttons & MainMenu.GAMEPAD_OK).Bool()) {
                    selected_index = this.menu.GetSelectedIndex();
                    if (selected_index >= 0 && selected_index < this.menu.GetItemsCount()) {

                        save_or_load_success = InternalCommit(selected_index);
                        SaveManager.game_withoutsavedata = save_or_load_success;
                        if (save_or_load_success && this.save_only) modding.has_funkinsave_changes = false;
                        if (save_or_load_success) break;
                    }
                } else if ((buttons & MainMenu.GAMEPAD_CANCEL).Bool() && !modding.HelperNotifyBack()) {
                    confirm_leave = true;
                    this.messagebox.SetButtonsIcons("a", "b");
                    this.messagebox.SetButtonsText("Yes", "No");
                    this.messagebox.SetTitle("Confirm");
                    this.messagebox.SetMessage(
                        this.save_only ? "¿Leave without saving?" : "¿Continue without load?"
                    );
                    this.messagebox.Show(true);
                    continue;
                } else if ((buttons & GamepadButtons.AD_DOWN).Bool())
                    selection_offset_y++;
                else if ((buttons & GamepadButtons.AD_UP).Bool())
                    selection_offset_y--;
                else if ((buttons & GamepadButtons.AD_LEFT).Bool())
                    selection_offset_x--;
                else if ((buttons & GamepadButtons.AD_RIGHT).Bool())
                    selection_offset_x++;

                if (selection_offset_x == 0 && selection_offset_y == 0) continue;

                bool success = false;

                if (selection_offset_x != 0 && this.menu.SelectHorizontal(selection_offset_x)) success = true;
                if (selection_offset_y != 0 && this.menu.SelectVertical(selection_offset_y)) success = true;

                selected_index = this.menu.GetSelectedIndex();
                if (!success || selected_index < 0 && selected_index >= this.vmu_size) continue;

                VMUInfo vmu = this.vmu_array[selected_index];

                char port_name = (char)(0x41 + vmu.port);
                char slot_name = (char)(0x30 + vmu.unit);
                string label_text = $"VMU {port_name}{slot_name}";

                this.selected_label.SetTextIntern(true, label_text);

                if (last_saved_selected != vmu.has_savedata) {
                    last_saved_selected = vmu.has_savedata;
                    this.layout.TriggerAny(vmu.has_savedata ? "save-selected" : "save-not-selected");
                }
            }

            this.layout.TriggerAny("outro");
            modding.HelperNotifyEvent("outro");

            if (save_or_load_success) {
                modding.has_exit = modding.has_halt = false;
                while (!modding.has_exit) {
                    float elapsed = PVRContext.global_context.WaitReady();
                    PVRContext.global_context.Reset();

                    ModdingHelperResult res = modding.HelperHandleCustomMenu(this.maple_pad, elapsed);
                    if (res != ModdingHelperResult.CONTINUE) break;

                    this.layout.Animate(elapsed);
                    this.layout.Draw(PVRContext.global_context);

                    if (modding.has_halt) continue;
                    if (this.layout.AnimationIsCompleted("transition_effect") > 0) {
                        // flush framebuffer again with last fade frame
                        PVRContext.global_context.WaitReady();
                        break;
                    }
                }
            }

            modding.HelperNotifyExit2();
            modding.Destroy();
        }

        public static int ShouldShow(bool attempt_to_save_or_load) {
            if (SaveManager.game_withoutsavedata) return -1;

            int index = 0;
            int count = 0;
            bool first_vmu = true;

            // count all attached VMUs
            while (true) {
                maple_device_t dev = maple.enum_type(index++, MAPLE_FUNC.MEMCARD);
                if (dev == null || !dev.valid) break;
                if (first_vmu) {
                    first_vmu = false;
                    FunkinSave.SetVMU(dev.port, dev.unit);
                }
                count++;
            }

            if (count != 1) return -1;

            // attempt to automatically load/save
            int result;
            if (attempt_to_save_or_load) {
                result = FunkinSave.WriteToVMU();
                SaveManager.game_withoutsavedata = result != 0;
            } else {
                result = FunkinSave.ReadFromVMU();
                if (result == 1 || result == 2) result = -1;
            }

            return result;
        }


        private void InternalBuildList() {
            if (this.menu != null) this.menu.Destroy();

            SaveManager.MENU_MANIFEST.items = new MenuManifest.Item[this.vmu_size];
            SaveManager.MENU_MANIFEST.items_size = this.vmu_size;

            for (int i = 0 ; i < this.vmu_size ; i++) {
                SaveManager.MENU_MANIFEST.items[i] = new MenuManifest.Item() {
                    name = null,
                    text = null,
                    placement = { x = 0, y = 0, dimmen = 0, gap = 0 },
                    anim_selected = null,
                    anim_choosen = null,
                    anim_discarded = null,
                    anim_idle = null,
                    anim_rollback = null,
                    anim_in = null,
                    anim_out = null,
                    hidden = false,
                    description = null,
                    texture_scale = 0f,
                    model = null
                };

                if (this.vmu_array[i].has_savedata)
                    SaveManager.MENU_MANIFEST.items[i].name = SaveManager.FUNKINSAVE;
                else
                    SaveManager.MENU_MANIFEST.items[i].name = SaveManager.EMPTY;
            }

            this.menu = new Menu(
                SaveManager.MENU_MANIFEST,
                this.placeholder.x,
                this.placeholder.y,
                this.placeholder.z,
                this.placeholder.width,
                this.placeholder.height
            );

            // note: nothing is allocated inside of items[]
            //free(SaveManager.MENU_MANIFEST.items);
            SaveManager.MENU_MANIFEST.items_size = 0;
        }

        private bool InternalFindChanges() {
            int index = 0;
            int count = 0;

            // count all attached VMUs
            while (true) {
                maple_device_t dev = maple.enum_type(index++, MAPLE_FUNC.MEMCARD);
                if (dev == null || !dev.valid) break;
                count++;
            }

            // build new scan
            VMUInfo[] new_vmu_array = new VMUInfo[count];
            for (int i = 0 ; i < count ; i++) {
                maple_device_t dev = maple.enum_type(i, MAPLE_FUNC.MEMCARD);
                if (!dev.valid) continue;

                bool found = FunkinSave.HasSavedataInVMU(dev.port, dev.unit);
                new_vmu_array[i] = new VMUInfo() { has_savedata = found, port = dev.port, unit = dev.unit };
            }

            // check changes with the previous scan
            int changes = Math.Max(count, this.vmu_size);
            for (int i = 0 ; i < this.vmu_size ; i++) {
                VMUInfo old_vmu = this.vmu_array[i];
                for (int j = 0 ; j < count ; j++) {
                    if (
                        old_vmu.port == new_vmu_array[j].port &&
                        old_vmu.unit == new_vmu_array[j].unit &&
                        old_vmu.has_savedata == new_vmu_array[j].has_savedata
                    ) {
                        changes--;
                    }
                }
            }

            if (changes < 1) return false;

            // drop old VMU scan
            //free(this.vmu_array);
            this.vmu_array = new_vmu_array;
            this.vmu_size = count;

            return true;
        }

        private int InternalAnimate(float elapsed) {
            int completed = 0;
            completed += this.selected_background.Animate(elapsed);
            if (this.menu != null) completed += this.menu.Animate(elapsed);
            completed += this.selected_label.Animate(elapsed);

            return completed;
        }

        private void InternalDraw(PVRContext pvrctx) {
            if (this.menu == null) return;
            if (this.selected_background.IsVisible()) this.selected_background.Draw(pvrctx);
            this.menu.Draw(pvrctx);
            if (this.selected_label.IsVisible()) this.selected_label.Draw(pvrctx);
        }

        private bool InternalCommit(int selected_index) {
            VMUInfo vmu = this.vmu_array[selected_index];

            if (!this.save_only && !vmu.has_savedata) {
                this.messagebox.SetMessage("This vmu is empty ¿Create a new save?");
                this.messagebox.SetButtonsIcons("a", "b");
                this.messagebox.SetButtonsText("Yes", "Pick another");
                this.messagebox.Show(true);

                this.maple_pad.ClearButtons();

                while (true) {
                    float elapsed = PVRContext.global_context.WaitReady();
                    GamepadButtons buttons = this.maple_pad.HasPressedDelayed(GamepadButtons.A | GamepadButtons.B | GamepadButtons.BACK);

                    PVRContext.global_context.Reset();
                    this.layout.Animate(elapsed);
                    this.layout.Draw(PVRContext.global_context);
                    this.messagebox.Animate(elapsed);
                    this.messagebox.Draw(PVRContext.global_context);

                    if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)).Bool()) return false;
                    if ((buttons & GamepadButtons.A).Bool()) break;
                }
            }

            FunkinSave.SetVMU(vmu.port, vmu.unit);

            if (!this.save_only && !vmu.has_savedata) {
                this.messagebox.SetTitle("Creating new save...");
                this.save_only = true;
            } else {
                this.messagebox.SetTitle(this.save_only ? "Storing save..." : "Loading save...");
            }

            this.messagebox.HideButtons();
            this.messagebox.UseFullTitle(true);
            this.messagebox.SetMessage(null);
            this.messagebox.Show(false);

            PVRContext.global_context.Reset();
            this.layout.Draw(PVRContext.global_context);
            this.messagebox.Draw(PVRContext.global_context);

            this.messagebox.UseFullTitle(false);

            int result;
            if (this.save_only)
                result = FunkinSave.WriteToVMU();
            else
                result = FunkinSave.ReadFromVMU();

            return InternalShowError(result);
        }

        private bool InternalShowError(int error_code) {
            if (this.save_only) {
                switch (error_code) {
                    case 0:
                        return true;
                    case 1:
                        this.messagebox.SetMessage(
                            "The VMU was removed"
                        );
                        break;
                    case 2:
                        this.messagebox.SetMessage(
                            "No enough free space"
                        );
                        break;
                    case 3:
                        this.messagebox.SetMessage(
                            "Error while writing the save,\nthe old save could be lost."
                        );
                        break;
                    case 4:
                        this.messagebox.SetMessage(
                            "The write could not be completed successfully"
                        );
                        break;
                    case 5:
                        this.messagebox.SetMessage(
                            "Can not delete the previous save"
                        );
                        break;
                }
            } else {
                switch (error_code) {
                    case 0:
                        // success
                        return true;
                    case 1:
                        this.messagebox.SetMessage(
                            "The VMU was removed"
                        );
                        break;
                    case 2:
                        this.messagebox.SetMessage(
                            "The VMU has changed and the save is missing"
                        );
                        break;
                    case 3:
                        this.messagebox.SetMessage(
                            "Error while reading the VMU"
                        );
                        break;
                    case 4:
                        this.messagebox.SetMessage(
                            "This save is damaged and can not be loaded"
                        );
                        break;
                    case 5:
                        this.messagebox.SetMessage(
                            "The version on the save is different,\nCan not be loaded."
                        );
                        break;
                }
            }

            this.messagebox.SetTitle("Error");
            this.messagebox.SetButtonSingle("OK");

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();
                GamepadButtons buttons = this.maple_pad.HasPressedDelayed(MainMenu.GAMEPAD_BUTTONS);

                PVRContext.global_context.Reset();
                this.layout.Animate(elapsed);
                this.layout.Draw(PVRContext.global_context);
                this.messagebox.Animate(elapsed);
                this.messagebox.Draw(PVRContext.global_context);

                if (buttons.Bool()) break;
            }

            return false;
        }


        private class VMUInfo {
            public bool has_savedata;
            public uint port;
            public uint unit;
        };

    }

}
