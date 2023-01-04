using System;
using System.Diagnostics;
using System.Text;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game.Gameplay {

    public class Dialogue : IAnimate, IDraw {
        private const string IDLE = "idle";
        private const string SPEAK = "speak";
        private const string ICON = "icon";
        private const string OPEN = "open";
        //private const string LAYOUT = "/assets/common/image/dialogue/layout.xml";
        //private const string LAYOUT_DREAMCAST = "/assets/common/image/dialogue/layout~dreamcast.xml";
        private const string DEFAULTS_DEFINITIONS = "/assets/common/data/dialogue_defaults.xml";

        private Audio[] audios;
        private int audios_size;
        private Font[] fonts;
        private int fonts_size;
        private Background[] backgrounds;
        private int backgrounds_size;
        private Portrait[] portraits;
        private int portraits_size;
        private Dialog[] dialogs;
        private int dialogs_size;
        private State[] states;
        private int states_size;
        private MultipleChoice[] multiplechoices;
        private int multiplechoices_size;
        private SpeechImage[] speechimages;
        private int speechimages_size;
        private AnimsUI anims_ui;

        private LinkedList<Portrait> visible_portraits;
        private bool is_completed;
        private Dialog dialog_external;

        private int current_background;
        private int change_background_from;
        private Luascript script;
        private bool do_exit;
        private SoundPlayer click_text;
        private SoundPlayer click_char;
        private int char_delay;
        private int chars_per_second;
        private bool do_skip;
        private bool do_instant_print;
        private bool do_no_wait;
        private MultipleChoice do_multiplechoice;
        private TextSprite texsprite_speech;
        private TextSprite texsprite_title;
        private SpeechImage current_speechimage;
        private bool current_speechimage_is_opening;
        private RepeatAnim current_speechimage_repeat;
        private bool is_speaking;
        private int current_dialog_codepoint_index;
        private int current_dialog_codepoint_length;
        private double current_dialog_duration;
        private double current_dialog_elapsed;
        private string current_dialog_mask;
        private Dialog current_dialog;
        private int current_dialog_line;
        private StringBuilder current_dialog_buffer;
        private bool draw_portraits_on_top;
        private Gamepad gamepad;
        private Drawable self_drawable;
        private bool self_hidden;


        private Dialogue() { }



        public static Dialogue Init(string src) {
            float viewport_width = Funkin.SCREEN_RESOLUTION_WIDTH;
            float viewport_height = Funkin.SCREEN_RESOLUTION_HEIGHT;

            src = FS.GetFullPath(src);
            XmlParser xml = XmlParser.Init(src);
            if (xml == null || xml.GetRoot() == null) {
                if (xml != null) xml.Destroy();
                Console.Error.WriteLine("[ERROR] dialogue_init() can not load dialogue xml file: " + src);
                //free(src);
                return null;
            }

            XmlParserNode xml_root = xml.GetRoot();
            AnimList animlist = AnimList.Init("/assets/common/anims/dialogue-ui.xml");
            ArrayList<Audio> audios = new ArrayList<Audio>();
            ArrayList<Font> fonts = new ArrayList<Font>();
            ArrayList<Background> backgrounds = new ArrayList<Background>();
            ArrayList<Portrait> portraits = new ArrayList<Portrait>();
            ArrayList<Dialog> dialogs = new ArrayList<Dialog>();
            ArrayList<State> states = new ArrayList<State>();
            ArrayList<MultipleChoice> multiplechoices = new ArrayList<MultipleChoice>();
            ArrayList<SpeechImage> speechimages = new ArrayList<SpeechImage>();

            AnimsUI anims_ui = new AnimsUI() {
                portrait_left_in = null,
                portrait_center_in = null,
                portrait_right_in = null,
                portrait_left_out = null,
                portrait_center_out = null,
                portrait_right_out = null,
                background_in = null,
                background_out = null
            };

            if (animlist != null) {
                anims_ui.portrait_left_in = AnimSprite.InitFromAnimlist(animlist, "portrait-add-left");
                anims_ui.portrait_center_in = AnimSprite.InitFromAnimlist(animlist, "portrait-add-center");
                anims_ui.portrait_right_in = AnimSprite.InitFromAnimlist(animlist, "portrait-add-right");
                anims_ui.portrait_left_out = AnimSprite.InitFromAnimlist(animlist, "portrait-remove-left");
                anims_ui.portrait_center_out = AnimSprite.InitFromAnimlist(animlist, "portrait-remove-center");
                anims_ui.portrait_right_out = AnimSprite.InitFromAnimlist(animlist, "portrait-remove-right");
                anims_ui.background_in = AnimSprite.InitFromAnimlist(animlist, "background-in");
                anims_ui.background_out = AnimSprite.InitFromAnimlist(animlist, "background-out");
                anims_ui.open = AnimSprite.InitFromAnimlist(animlist, "open");
                anims_ui.close = AnimSprite.InitFromAnimlist(animlist, "close");
                animlist.Destroy();
            }

            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);
            //free(src);

            bool import_defaults = VertexProps.ParseBoolean(xml_root, "importDefaultDefinition", true);
            XmlParser xml_defaults = null;
            XmlParserNodeList childs_defaults = null;

            // import default character portraits and speech images
            if (import_defaults) {
                xml_defaults = XmlParser.Init(Dialogue.DEFAULTS_DEFINITIONS);
                if (xml_defaults != null && xml_defaults.GetRoot() != null) {
                    childs_defaults = xml_defaults.GetRoot().Children;
                }
            }

            int index = 0;
            bool self_parse = true;
            XmlParserNodeList childs = xml_root.Children;

            while (true) {
                XmlParserNode node;

                // parse first the desired xml, and later the default xml
                if (self_parse) {
                    if (index >= childs.Length) {
                        if (childs_defaults == null) break;
                        index = 0;
                        self_parse = false;
                        continue;
                    }
                    node = childs[index++];
                } else {
                    if (index >= childs_defaults.Length) break;
                    node = childs_defaults[index++];
                }

                switch (node.TagName) {
                    case "Definition":
                        foreach (XmlParserNode node2 in node.Children) {
                            switch (node2.TagName) {
                                case "AudioList":
                                    Dialogue.InternalParseAudioList(node2, audios);
                                    break;
                                case "BackgroundList":
                                    Dialogue.InternalParseBackgroundList(node2, viewport_width, viewport_height, backgrounds);
                                    break;
                                case "PortraitList":
                                    Dialogue.InternalParsePortraitList(node2, portraits);
                                    break;
                                case "AnimationsUI":
                                    Dialogue.InternalParseAnimationUI(node2, anims_ui);
                                    break;
                                case "Font":
                                    Dialogue.InternalParseFont(node2, fonts);
                                    break;
                                case "SpeechImageList":
                                    Dialogue.InternalParseSpeechImageList(node2, speechimages);
                                    break;
                                case "ImportPortraitList":
                                    Dialogue.InternalParseImportPortraitList(node2, portraits);
                                    break;
                                default:
                                    Console.Error.WriteLine("[ERROR] dialogue_init() unknown definition: " + node2.TagName);
                                    break;
                            }
                        }
                        break;
                    case "State":
                        Dialogue.InternalParseState(node, states);
                        break;
                    case "MultipleChoice":
                        Dialogue.InternalParseMultipleChoice(node, animlist, dialogs, multiplechoices);
                        break;
                }
            }

            if (xml_defaults != null) xml_defaults.Destroy();

            FS.FolderStackPop();
            xml.Destroy();

            Dialogue dialogue = new Dialogue() {
                audios = null,
                audios_size = 0,
                fonts = null,
                fonts_size = 0,
                backgrounds = null,
                backgrounds_size = 0,
                portraits = null,
                portraits_size = 0,
                dialogs = null,
                dialogs_size = 0,
                states = null,
                states_size = 0,
                multiplechoices = null,
                multiplechoices_size = 0,
                speechimages = null,
                speechimages_size = 0,

                anims_ui = anims_ui,
                visible_portraits = new LinkedList<Portrait>(),
                texsprite_speech = null,
                texsprite_title = null,
                draw_portraits_on_top = false,

                current_background = -1,
                change_background_from = -1,
                script = null,
                do_exit = false,
                click_text = null,
                click_char = null,
                char_delay = 0,
                do_skip = false,
                do_instant_print = false,
                do_no_wait = false,
                do_multiplechoice = null,
                current_speechimage = null,
                current_speechimage_is_opening = false,
                current_speechimage_repeat = RepeatAnim.ALWAYS,
                is_speaking = false,
                current_dialog_codepoint_index = 0,
                current_dialog_codepoint_length = 0,
                current_dialog_duration = 0f,
                current_dialog_elapsed = 0f,
                current_dialog_mask = null,
                current_dialog_buffer = new StringBuilder(64),
                current_dialog = null,
                current_dialog_line = -1,
                gamepad = new Gamepad(-1),
                dialog_external = null,
                is_completed = true,
                chars_per_second = 0,
                self_drawable = null,
                self_hidden = false
            };

            dialogue.self_drawable = new Drawable(300, dialogue, dialogue);

            dialogue.gamepad.SetButtonsDelay(200);

            audios.Destroy2(out dialogue.audios_size, ref dialogue.audios);
            fonts.Destroy2(out dialogue.fonts_size, ref dialogue.fonts);
            backgrounds.Destroy2(out dialogue.backgrounds_size, ref dialogue.backgrounds);
            portraits.Destroy2(out dialogue.portraits_size, ref dialogue.portraits);
            dialogs.Destroy2(out dialogue.dialogs_size, ref dialogue.dialogs);
            states.Destroy2(out dialogue.states_size, ref dialogue.states);
            multiplechoices.Destroy2(out dialogue.multiplechoices_size, ref dialogue.multiplechoices);
            speechimages.Destroy2(out dialogue.speechimages_size, ref dialogue.speechimages);

            // set defaults
            Audio tmp_audio;

            tmp_audio = dialogue.InternalGetAudio("click_text");
            if (tmp_audio != null) dialogue.click_text = tmp_audio.soundplayer;

            tmp_audio = dialogue.InternalGetAudio("click_char");
            if (tmp_audio != null) dialogue.click_char = tmp_audio.soundplayer;

            dialogue.current_background = dialogue.InternalGetBackgroundIndex("default_background");
            dialogue.chars_per_second = 0;
            dialogue.char_delay = 40;

            for (int i = 0 ; i < dialogue.speechimages_size ; i++) {
                SpeechImage speechimage = dialogue.speechimages[i];
                if (speechimage.name != "normal-left") continue;

                dialogue.current_speechimage = speechimage;
                bool toggle_default = dialogue.current_speechimage.statesprite.StateToggle(null);
                bool toggle_open = dialogue.current_speechimage.statesprite.StateToggle(Dialogue.OPEN);
                dialogue.current_speechimage_is_opening = toggle_default || toggle_open;

                if (!dialogue.current_speechimage_is_opening) {
                    // switch to idle animation
                    dialogue.current_speechimage.statesprite.StateToggle(Dialogue.IDLE);
                }

                break;
            }

            // create textsprite speech if not customized
            dialogue.texsprite_speech = TextSprite.Init(null, false, 34f, 0x00000);
            dialogue.texsprite_speech.SetParagraphSpace(8f);
            dialogue.texsprite_speech.SetWordbreak(VertexProps.FONT_WORDBREAK_LOOSE);

            FontHolder fontholder = dialogue.InternalGetFont("font");
            if (fontholder != null) dialogue.texsprite_speech.ChangeFont(fontholder);


            // create textsprite title
            dialogue.texsprite_title = TextSprite.Init(null, false, 24f, 0x00000);
            dialogue.texsprite_title.SetParagraphSpace(8f);
            dialogue.texsprite_title.SetWordbreak(VertexProps.FONT_WORDBREAK_LOOSE);

            fontholder = dialogue.InternalGetFont("font");
            if (fontholder != null) dialogue.texsprite_speech.ChangeFont(fontholder);

            return dialogue;
        }

        public void Destroy() {
            InternalDestroyExternalDialog();

            for (int i = 0 ; i < this.audios_size ; i++) {
                //free(this.audios[i].name);
                this.audios[i].soundplayer.Destroy();
            }
            for (int i = 0 ; i < this.fonts_size ; i++) {
                //free(this.fonts[i].name);
                this.fonts[i].fontholder.Destroy();
            }
            for (int i = 0 ; i < this.backgrounds_size ; i++) {
                //free(this.backgrounds[i].name);
                this.backgrounds[i].sprite.DestroyFull();
            }
            for (int i = 0 ; i < this.portraits_size ; i++) {
                //free(this.portraits[i].name);
                //free(this.portraits[i].random_from_prefix);
                this.portraits[i].statesprite.DestroyTextureIfStateless();
                this.portraits[i].statesprite.Destroy();
            }
            for (int i = 0 ; i < this.dialogs_size ; i++) {
                for (int j = 0 ; j < this.dialogs[i].lines_size ; j++) {
                    //free(this.dialogs[i].lines[j].target_state_name);
                    //free(this.dialogs[i].lines[j].text);
                }
                //free(this.dialogs[i].lines);
                //free(this.dialogs[i].full_path);
            }
            for (int i = 0 ; i < this.states_size ; i++) {
                for (int j = 0 ; j < this.states[i].actions_size ; j++) {
                    //free(this.states[i].actions[j].name);
                    //free(this.states[i].actions[j].click_text);
                    //free(this.states[i].actions[j].click_char);
                    //free(this.states[i].actions[j].lua_eval);
                    //free(this.states[i].actions[j].lua_function);
                    //free(this.states[i].actions[j].random_from_prefix);
                    //free(this.states[i].actions[j].speech_name);
                    //free(this.states[i].actions[j].title);
                }
                //free(this.states[i].name);
                //free(this.states[i].if_line);
                //free(this.states[i].actions);
            }
            for (int i = 0 ; i < this.multiplechoices_size ; i++) {
                for (int j = 0 ; j < this.multiplechoices[i].choices_size ; j++) {
                    //free(this.multiplechoices[i].choices[j].lua_eval);
                    //free(this.multiplechoices[i].choices[j].lua_function);
                    //free(this.multiplechoices[i].choices[j].run_multiple_choice);
                    //free(this.multiplechoices[i].choices[j].text);
                }
                this.multiplechoices[i].hint.DestroyFull();
                this.multiplechoices[i].icon.DestroyFull();
                //free(this.multiplechoices[i].state_on_leave);
                //free(this.multiplechoices[i].title);
                //free(this.multiplechoices[i].name);
                //free(this.multiplechoices[i].choices);
            }
            for (int i = 0 ; i < this.speechimages_size ; i++) {
                //free(this.speechimages[i].name);
                this.speechimages[i].statesprite.DestroyTextureIfStateless();
                this.speechimages[i].statesprite.Destroy();
            }

            //free(this.audios);
            //free(this.fonts);
            //free(this.backgrounds);
            //free(this.portraits);
            //free(this.dialogs);
            //free(this.states);
            //free(this.multiplechoices);
            //free(this.speechimages);

            if (this.anims_ui.portrait_left_in != null) this.anims_ui.portrait_left_in.Destroy();
            if (this.anims_ui.portrait_center_in != null) this.anims_ui.portrait_center_in.Destroy();
            if (this.anims_ui.portrait_right_in != null) this.anims_ui.portrait_right_in.Destroy();
            if (this.anims_ui.portrait_left_out != null) this.anims_ui.portrait_left_out.Destroy();
            if (this.anims_ui.portrait_center_out != null) this.anims_ui.portrait_center_out.Destroy();
            if (this.anims_ui.portrait_right_out != null) this.anims_ui.portrait_right_out.Destroy();
            if (this.anims_ui.background_in != null) this.anims_ui.background_in.Destroy();
            if (this.anims_ui.background_out != null) this.anims_ui.background_out.Destroy();
            if (this.anims_ui.open != null) this.anims_ui.open.Destroy();
            if (this.anims_ui.close != null) this.anims_ui.close.Destroy();

            this.texsprite_speech.Destroy();
            this.texsprite_title.Destroy();

            this.visible_portraits.Destroy();
            this.gamepad.Destroy();
            this.self_drawable.Destroy();
            //this.current_dialog_buffer.Destroy();
            //free(this.current_dialog_mask);

            //free(this);
        }

        public bool ApplyState(string state_name) {
            return ApplyState2(state_name, null);
        }

        public bool ApplyState2(string state_name, string if_line_label) {
            if (this.do_exit) return false;

            State state = null;
            for (int i = 0 ; i < this.states_size ; i++) {
                if (this.states[i].name == state_name && this.states[i].if_line == if_line_label) {
                    state = this.states[i];
                    break;
                }
            }
            if (state == null) return false;

            return InternalApplyState(state);
        }

        public int Animate(float elapsed) {
            if (this.self_hidden || this.is_completed) return 1;

            if (this.anims_ui.open != null && this.anims_ui.open.Animate(elapsed) < 1) {
                this.anims_ui.open.UpdateDrawable(this.self_drawable, true);
            }

            bool anim_portrait_left_in = this.anims_ui.portrait_left_in != null;
            bool anim_portrait_center_in = this.anims_ui.portrait_center_in != null;
            bool anim_portrait_right_in = this.anims_ui.portrait_right_out != null;
            bool anim_portrait_left_out = this.anims_ui.portrait_left_out != null;
            bool anim_portrait_center_out = this.anims_ui.portrait_center_out != null;
            bool anim_portrait_right_out = this.anims_ui.portrait_right_out != null;

            for (int i = 0 ; i < this.backgrounds_size ; i++) {
                this.backgrounds[i].sprite.Animate(elapsed);
            }

            if (anim_portrait_left_in) {
                if (this.anims_ui.portrait_left_in.Animate(elapsed) > 0) anim_portrait_left_in = false;
            }
            if (anim_portrait_center_in) {
                if (this.anims_ui.portrait_center_in.Animate(elapsed) > 0) anim_portrait_center_in = false;
            }
            if (anim_portrait_right_in) {
                if (this.anims_ui.portrait_right_in.Animate(elapsed) > 0) anim_portrait_right_in = false;
            }
            if (anim_portrait_left_out) {
                if (this.anims_ui.portrait_left_out.Animate(elapsed) > 0) anim_portrait_left_out = false;
            }
            if (anim_portrait_center_out) {
                if (this.anims_ui.portrait_center_out.Animate(elapsed) > 0) anim_portrait_center_out = false;
            }
            if (anim_portrait_right_out) {
                if (this.anims_ui.portrait_right_out.Animate(elapsed) > 0) anim_portrait_right_out = false;
            }

            foreach (Portrait portrait in this.visible_portraits) {
                bool exists;
                AnimSprite anim;

                int completed = portrait.statesprite.Animate(elapsed);

                if (portrait.is_added) {
                    // guess the correct animation direction
                    if (portrait.position < 0.5f) {
                        exists = anim_portrait_left_in;
                        anim = this.anims_ui.portrait_left_in;
                    } else if (portrait.position == 0.5f) {
                        exists = anim_portrait_center_in;
                        anim = this.anims_ui.portrait_center_in;
                    } else if (portrait.position > 0.5f) {
                        exists = anim_portrait_right_in;
                        anim = this.anims_ui.portrait_right_in;
                    } else {
                        // this never should happen
                        continue;
                    }

                    if (exists) {
                        anim.UpdateStatesprite(portrait.statesprite, false);
                    } else {
                        portrait.is_added = false;
                        bool toggled_default = portrait.statesprite.StateToggle(null);
                        bool toggled_speak = portrait.statesprite.StateToggle(Dialogue.SPEAK);

                        if (this.is_speaking && (toggled_default || toggled_speak)) {
                            portrait.is_speaking = true;
                            portrait.statesprite.AnimationRestart();
                        } else {
                            // no speak animation, fallback to idle
                            portrait.is_speaking = false;
                            portrait.statesprite.StateToggle(Dialogue.IDLE);
                        }
                    }
                } else if (portrait.is_removed) {
                    // guess the correct animation direction
                    if (portrait.position < 0.5f) {
                        exists = anim_portrait_left_out;
                        anim = this.anims_ui.portrait_left_out;
                    } else if (portrait.position == 0.5f) {
                        exists = anim_portrait_center_out;
                        anim = this.anims_ui.portrait_center_out;
                    } else if (portrait.position > 0.5f) {
                        exists = anim_portrait_right_out;
                        anim = this.anims_ui.portrait_right_out;
                    } else {
                        // this never should happen
                        continue;
                    }

                    if (exists) {
                        anim.UpdateStatesprite(portrait.statesprite, false);
                    } else {
                        portrait.is_removed = false;
                        this.visible_portraits.RemoveItem(portrait);
                    }
                } else if (completed < 1) {
                    // the animation is not completed, nothing to do
                    continue;
                }

                // if the speak animation is completed and there not longer speech switch to idle
                if (!this.is_speaking && portrait.is_speaking) {
                    portrait.is_speaking = false;
                    Dialogue.InternalStopPortraitAnimation(portrait);
                    continue;
                }

                // check if the animation should be looped again
                if (!(portrait.is_speaking ? portrait.speak_anim_looped : portrait.idle_anim_looped)) {
                    continue;
                }

                // only loop if the desired state is applied and exists
                bool can_loop = portrait.is_speaking ? portrait.has_speak : portrait.has_idle;

                // restart the animation if necessary
                if (can_loop || (!portrait.has_speak && !portrait.has_idle)) {
                    portrait.statesprite.AnimationRestart();
                }
            }

            if (this.current_background >= 0 && this.anims_ui.background_in != null) {
                Sprite sprite = this.backgrounds[this.current_background].sprite;
                if (this.anims_ui.background_in.Animate(elapsed) < 1) {
                    this.anims_ui.background_in.UpdateSprite(sprite, true);
                }
            }

            if (this.change_background_from >= 0 && this.anims_ui.background_out != null) {
                Sprite sprite = this.backgrounds[this.change_background_from].sprite;
                if (this.anims_ui.background_out.Animate(elapsed) < 1) {
                    this.anims_ui.background_out.UpdateSprite(sprite, true);
                } else {
                    this.change_background_from = -1;
                }
            }

            if (this.current_speechimage != null) {
                bool is_opening = this.current_speechimage_is_opening;
                StateSprite sprite = this.current_speechimage.statesprite;

                bool completed = sprite.Animate(elapsed) > 0;

                // once opening animation is done, switch to idle animation
                if (completed && is_opening && sprite.StateToggle(Dialogue.IDLE)) {
                    sprite.AnimationRestart();
                    this.current_speechimage_is_opening = false;
                } else if (completed && !is_opening) {
                    switch (this.current_speechimage_repeat) {
                        case RepeatAnim.ALWAYS:
                            sprite.AnimationRestart();
                            break;
                        case RepeatAnim.WHILESPEAKS:
                            if (this.is_speaking) sprite.AnimationRestart();
                            break;
                    }
                }
            }

            if (this.do_exit) {
                if (this.anims_ui.close != null && this.anims_ui.close.Animate(elapsed) < 1) {
                    this.anims_ui.close.UpdateDrawable(this.self_drawable, true);
                } else {
                    if (this.script != null) this.script.CallFunction("f_dialogue_exit");
                    this.is_completed = true;
                    for (int i = 0 ; i < this.audios_size ; i++) this.audios[i].soundplayer.Stop();
                    return 0;
                }
            }

            if (this.current_dialog != null) {
                this.current_dialog_elapsed += elapsed;
                bool preapare_next_line = false;

                GamepadButtons buttons = this.gamepad.HasPressedDelayed(
                    GamepadButtons.A | GamepadButtons.X | GamepadButtons.START | GamepadButtons.BACK
                );
                if ((buttons & GamepadButtons.A) != GamepadButtons.NOTHING) {
                    this.current_dialog_elapsed = this.current_dialog_duration;
                    preapare_next_line = true;
                } else if ((buttons & GamepadButtons.X) != GamepadButtons.NOTHING) {
                    if (this.is_speaking)
                        this.do_no_wait = true;
                    else
                        preapare_next_line = true;
                } else if ((buttons & (GamepadButtons.START | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    this.do_exit = true;
                    Close();
                }

                if (this.is_speaking)
                    InternalPrintText();
                else if (!this.do_exit && preapare_next_line)
                    InternalPreparePrintText();
            }

            return 0;
        }

        public void Draw(PVRContext pvrctx) {
            if (this.self_hidden || this.is_completed) return;

            pvrctx.Save();
            this.self_drawable.HelperApplyInContext(pvrctx);
            pvrctx.Save();

            InternalDrawBackground(pvrctx);
            if (!this.draw_portraits_on_top) InternalDrawPortraits(pvrctx);
            InternalDrawSpeech(pvrctx);
            if (this.draw_portraits_on_top) InternalDrawPortraits(pvrctx);

            pvrctx.Restore();
            pvrctx.Restore();
        }

        public bool IsCompleted() {
            return this.is_completed;
        }

        public bool IsHidden() {
            return this.self_hidden;
        }

        public bool ShowDialog(string src_dialog) {
            if (src_dialog == null) return false;

            ArrayList<Dialog> dialogs = new ArrayList<Dialog>(this.dialogs_size);
            for (int i = 0 ; i < this.dialogs_size ; i++) dialogs.Add(this.dialogs[i]);
            if (this.dialog_external != null) dialogs.Add(this.dialog_external);

            int total = dialogs.Size();
            int id = Dialogue.InternalParseDialog(src_dialog, dialogs);
            if (id < 0) {
                dialogs.Destroy(false);
                return false;
            } else if (id >= total) {
                InternalDestroyExternalDialog();
                this.dialog_external = dialogs.Get(id);
                dialogs.Destroy(false);
            }

            this.current_dialog = this.dialog_external;
            this.do_skip = false;
            this.do_instant_print = false;
            this.do_multiplechoice = null;
            this.do_exit = false;
            this.current_dialog_line = 0;
            this.self_hidden = false;
            this.is_completed = false;
            this.self_drawable.SetAntialiasing(PVRContextFlag.DEFAULT);
            this.self_drawable.SetAlpha(1f);
            this.self_drawable.SetOffsetColorToDefault();
            this.self_drawable.GetModifier().Clear();
            this.visible_portraits.Clear();

            // apply any initial state
            this.ApplyState2(null, null);
            for (int i = 0 ; i < this.states_size ; i++) {
                if (this.states[i].initial) this.InternalApplyState(this.states[i]);
            }

            if (!this.texsprite_speech.HasFont()) {
                Console.Error.WriteLine("[ERROR] dialogue_show_dialog() speech textsprite does not have font");
                this.do_exit = true;
                return false;
            }

            InternalPreparePrintText();

            if (this.anims_ui.open != null) this.anims_ui.open.Restart();
            if (this.anims_ui.close != null) this.anims_ui.close.Restart();

            if (this.current_speechimage == null && this.speechimages_size > 0) {
                Console.Error.WriteLine("[WARN] dialogue_show_dialog() no speech background choosen, auto-choosing the first one declared");
                this.current_speechimage = this.speechimages[0];
            }

            return true;
        }

        public void Close() {
            if (this.script != null) this.script.CallFunction("f_dialogue_closing");
            this.do_exit = true;
            this.current_dialog = null;

            for (int i = 0 ; i < this.audios_size ; i++) {
                if (this.audios[i].soundplayer.IsPlaying()) {
                    if (this.anims_ui.close != null)
                        this.audios[i].soundplayer.Fade(false, 500f);
                    else
                        this.audios[i].soundplayer.Stop();
                }
            }
        }

        public void Hide(bool hidden) {
            this.self_hidden = hidden;
        }

        public void Suspend() {
            if (this.is_completed) return;
            for (int i = 0 ; i < this.audios_size ; i++) {
                this.audios[i].was_playing = this.audios[i].soundplayer.IsPlaying();
                if (this.audios[i].was_playing) {
                    this.audios[i].soundplayer.Pause();
                    if (this.audios[i].soundplayer.HasFadding() == Fading.OUT) this.audios[i].soundplayer.SetVolume(0f);
                }
            }
        }

        public void Resume() {
            if (this.is_completed) return;
            for (int i = 0 ; i < this.audios_size ; i++) {
                if (this.audios[i].was_playing) this.audios[i].soundplayer.Play();
            }
            this.gamepad.ClearButtons();
        }

        public Modifier GetModifier() {
            return this.self_drawable.GetModifier();
        }

        public Drawable GetDrawable() {
            return this.self_drawable;
        }

        public void SetOffsetColor(float r, float g, float b, float a) {
            this.self_drawable.SetOffsetColor(r, g, b, a);
        }

        public void SetAntialiasing(PVRContextFlag pvrflag) {
            this.self_drawable.SetAntialiasing(pvrflag);
        }

        public void SetAlpha(float alpha) {
            this.self_drawable.SetAlpha(alpha);
        }

        public void SetScript(WeekScript weekscript) {
            if (weekscript != null)
                this.script = weekscript.GetLuaScript();
            else
                this.script = null;
        }


        private bool InternalApplyState(State state) {
            if (this.do_exit) return false;

            Audio audio;
            int background;
            bool background_changed = false;
            bool speechimage_changed = false;

            for (int action_index = 0 ; action_index < state.actions_size ; action_index++) {
                Action action = state.actions[action_index];

                switch (action.type) {
                    case Type.AUDIO_PLAY:
                        audio = InternalGetAudio(action.name);
                        if (audio != null) audio.soundplayer.Play();
                        break;
                    case Type.AUDIO_PAUSE:
                        audio = InternalGetAudio(action.name);
                        if (audio != null) {
                            audio.soundplayer.Pause();
                        } else if (action.name == null) {
                            for (int i = 0 ; i < this.audios_size ; i++) {
                                this.audios[i].soundplayer.Pause();
                            }
                        }
                        break;
                    case Type.AUDIO_FADEIN:
                        audio = InternalGetAudio(action.name);
                        if (audio != null) audio.soundplayer.Fade(true, action.duration);
                        break;
                    case Type.AUDIO_FADEOUT:
                        audio = InternalGetAudio(action.name);
                        if (audio != null) audio.soundplayer.Fade(false, action.duration);
                        break;
                    case Type.AUDIO_STOP:
                        audio = InternalGetAudio(action.name);
                        if (audio != null) {
                            audio.soundplayer.Stop();
                        } else if (action.name == null) {
                            for (int i = 0 ; i < this.audios_size ; i++) {
                                this.audios[i].soundplayer.Stop();
                            }
                        }
                        break;
                    case Type.AUDIO_VOLUME:
                        audio = InternalGetAudio(action.name);
                        if (audio != null) audio.soundplayer.SetVolume(action.volume);
                        break;
                    case Type.BACKGROUND_SET:
                        background = InternalGetBackgroundIndex(action.name);
                        if (background >= 0) {
                            this.current_background = background;
                            this.change_background_from = -1;
                            background_changed = true;
                        }
                        break;
                    case Type.BACKGROUND_CHANGE:
                        this.change_background_from = this.current_background;
                        this.current_background = InternalGetBackgroundIndex(action.name);
                        background_changed = true;
                        break;
                    case Type.BACKGROUND_REMOVE:
                        this.current_background = -1;
                        this.change_background_from = -1;
                        background_changed = true;
                        break;
                    case Type.LUA:
                        this.script.Eval(action.lua_eval);
                        break;
                    case Type.EXIT:
                        Close();
                        return true;
                    case Type.PORTRAIT_ADD:
                        int portrait_index = -1;
                        if (action.random_from_prefix != null && this.portraits_size > 0) {
                            int index;
                            if (action.random_from_prefix.Length == 0) {
                                // random choose
                                index = Math2D.RandomInt(0, this.portraits_size - 1);
                            } else {
                                index = -1;
                                int count = 0;
                                int j = 0;

                                for (int i = 0 ; i < this.portraits_size ; i++) {
                                    if (this.portraits[i].name.StartsWithKDY(action.name, 0)) count++;
                                }

                                // random choose
                                int choosen = Math2D.RandomInt(0, count - 1);

                                for (int i = 0 ; i < this.portraits_size ; i++) {
                                    if (this.portraits[i].name.StartsWithKDY(action.name, 0)) {
                                        if (j == choosen) {
                                            index = i;
                                            break;
                                        }
                                        j++;
                                    }
                                }
                            }
                        }
                        for (int i = 0 ; i < this.portraits_size ; i++) {
                            if (this.portraits[i].name == action.name) {
                                portrait_index = i;
                                break;
                            }
                        }
                        if (portrait_index < 0 || portrait_index >= this.portraits_size) {
                            break;
                        }

                        if (this.anims_ui.portrait_left_in != null) this.anims_ui.portrait_left_in.Restart();
                        if (this.anims_ui.portrait_center_in != null) this.anims_ui.portrait_center_in.Restart();
                        if (this.anims_ui.portrait_right_in != null) this.anims_ui.portrait_right_in.Restart();

                        this.visible_portraits.RemoveItem(this.portraits[portrait_index]);
                        this.visible_portraits.AddItem(this.portraits[portrait_index]);
                        this.portraits[portrait_index].is_added = true;
                        this.portraits[portrait_index].is_removed = false;
                        this.portraits[portrait_index].is_speaking = false;
                        this.portraits[portrait_index].statesprite.StateToggle(null);
                        this.portraits[portrait_index].statesprite.StateToggle(
                            action.no_speak ? Dialogue.IDLE : Dialogue.SPEAK
                        );
                        break;
                    case Type.PORTRAIT_REMOVE:
                        for (int i = 0 ; i < this.portraits_size ; i++) {
                            if (this.portraits[i].name == action.name) {
                                this.portraits[i].is_added = false;
                                this.portraits[i].is_removed = true;

                                if (this.anims_ui.portrait_left_out != null)
                                    this.anims_ui.portrait_left_out.Restart();
                                if (this.anims_ui.portrait_center_out != null)
                                    this.anims_ui.portrait_center_out.Restart();
                                if (this.anims_ui.portrait_right_out != null)
                                    this.anims_ui.portrait_right_out.Restart();

                                break;
                            }
                        }
                        break;
                    case Type.PORTRAIT_REMOVEALL:
                        if (action.animate_remove) {
                            foreach (Portrait portrait in this.visible_portraits) {
                                portrait.is_added = false;
                                portrait.is_removed = true;
                            }

                            if (this.anims_ui.portrait_left_out != null)
                                this.anims_ui.portrait_left_out.Restart();
                            if (this.anims_ui.portrait_center_out != null)
                                this.anims_ui.portrait_center_out.Restart();
                            if (this.anims_ui.portrait_right_out != null)
                                this.anims_ui.portrait_right_out.Restart();
                        } else {
                            this.visible_portraits.Clear();
                        }
                        break;
                    case Type.AUDIO_UI:
                        if (action.click_char != null) {
                            audio = InternalGetAudio(action.click_char);
                            if (this.click_char != null) this.click_char.Stop();

                            if (audio != null)
                                this.click_char = audio.soundplayer;
                            else
                                this.click_char = null;
                        }
                        if (action.click_text != null) {
                            audio = InternalGetAudio(action.click_text);
                            if (this.click_text != null) this.click_text.Stop();

                            if (audio != null)
                                this.click_text = audio.soundplayer;
                            else
                                this.click_text = null;
                        }
                        break;
                    case Type.TEXT_SPEED:
                        if (action.chars_per_second >= 0) this.chars_per_second = action.chars_per_second;
                        if (action.char_delay >= 0) this.char_delay = action.char_delay;
                        break;
                    case Type.TEXT_SKIP:
                        this.do_skip = true;
                        break;
                    case Type.TEXT_INSTANTPRINT:
                        this.do_instant_print = true;
                        break;
                    case Type.SPEECH_BACKGROUND:
                        if (action.name == null) {
                            //random choose
                            int index = Math2D.RandomInt(0, this.backgrounds_size - 1);
                            this.current_speechimage = this.speechimages[index];
                            speechimage_changed = true;
                        } else if (action.name == "none") {
                            speechimage_changed = false;
                            this.current_speechimage = null;
                            this.current_speechimage_is_opening = false;
                        } else {
                            for (int i = 0 ; i < this.speechimages_size ; i++) {
                                if (this.speechimages[i].name == action.name) {
                                    this.current_speechimage = this.speechimages[i];
                                    speechimage_changed = true;
                                    break;
                                }
                            }
                        }
                        if (speechimage_changed && action.repeat_anim != RepeatAnim.NONE)
                            this.current_speechimage_repeat = action.repeat_anim;
                        break;
                    case Type.TEXT_FONT:
                        if (this.texsprite_speech == null) break;
                        FontHolder fontholder = InternalGetFont(action.name);
                        if (fontholder != null) this.texsprite_speech.ChangeFont(fontholder);
                        break;
                    case Type.TEXT_COLOR:
                        if (this.texsprite_speech == null) break;
                        this.texsprite_speech.SetColor(action.rgba[0], action.rgba[1], action.rgba[2]);
                        if (!Single.IsNaN(action.rgba[3])) this.texsprite_speech.SetAlpha(action.rgba[3]);
                        break;
                    case Type.TEXT_BORDERCOLOR:
                        if (this.texsprite_speech == null) break;
                        this.texsprite_speech.BorderSetColor(action.rgba[0], action.rgba[1], action.rgba[2], action.rgba[3]);
                        break;
                    case Type.TEXT_BORDEROFFSET:
                        if (this.texsprite_speech == null) break;
                        this.texsprite_speech.BorderSetOffset(action.offset_x, action.offset_y);
                        break;
                    case Type.TEXT_SIZE:
                        if (this.texsprite_speech == null) break;
                        if (!Single.IsNaN(action.size)) this.texsprite_speech.SetFontSize(action.size);
                        break;
                    case Type.TEXT_BORDERSIZE:
                        if (this.texsprite_speech == null) break;
                        if (!Single.IsNaN(action.size)) this.texsprite_speech.BorderSetSize(action.size);
                        break;
                    case Type.TEXT_BORDERENABLE:
                        if (this.texsprite_speech == null) break;
                        this.texsprite_speech.BorderEnable(action.enabled);
                        break;
                    case Type.TEXT_PARAGRAPHSPACE:
                        if (this.texsprite_speech == null) break;
                        if (!Single.IsNaN(action.size)) this.texsprite_speech.SetParagraphSpace(action.size);
                        break;
                    case Type.TEXT_ALIGN:
                        if (this.texsprite_speech == null) break;
                        this.texsprite_speech.SetAlign(action.align_vertical, action.align_horizontal);
                        if (action.align_paragraph != Align.NONE) this.texsprite_speech.SetParagraphAlign(action.align_paragraph);
                        break;
                    case Type.RUNMULTIPLECHOICE:
                        this.do_multiplechoice = InternalGetMultiplechoice(action.name);
                        break;
                    case Type.TITLE:
                        if (this.texsprite_title == null) break;
                        this.texsprite_title.SetTextIntern(true, action.title);
                        break;
                    case Type.NOWAIT:
                        this.do_no_wait = true;
                        break;
                }

            }

            if (background_changed) {
                if (this.anims_ui.background_in != null) this.anims_ui.background_in.Restart();
                if (this.anims_ui.background_out != null) this.anims_ui.background_out.Restart();
            }

            if (speechimage_changed && this.current_speechimage != null) {
                bool toggle_default = this.current_speechimage.statesprite.StateToggle(null);
                bool toggle_open = this.current_speechimage.statesprite.StateToggle(Dialogue.OPEN);

                this.current_speechimage_is_opening = toggle_default || toggle_open;

                if (!this.current_speechimage_is_opening) {
                    // switch to idle animation
                    this.current_speechimage.statesprite.StateToggle(Dialogue.IDLE);
                }

                this.current_speechimage.statesprite.AnimationRestart();

                // set speech background location
                this.current_speechimage.statesprite.SetDrawLocation(
                    this.current_speechimage.offset_x,
                    this.current_speechimage.offset_y
                );

                // set speech text bounds
                float text_x = this.current_speechimage.text_x;
                float text_y = this.current_speechimage.text_y;
                if (this.current_speechimage.text_is_relative) {
                    text_x += this.current_speechimage.offset_x;
                    text_y += this.current_speechimage.offset_y;
                }
                this.texsprite_speech.SetDrawLocation(text_x, text_y);
                this.texsprite_speech.SetMaxDrawSize(
                    this.current_speechimage.text_width,
                    this.current_speechimage.text_height
                );

                // set title location
                float title_x = this.current_speechimage.title_x;
                float title_y = this.current_speechimage.title_y;
                if (this.current_speechimage.title_is_relative) {
                    title_x += this.current_speechimage.offset_x;
                    title_y += this.current_speechimage.offset_y;
                }
                this.texsprite_title.SetDrawLocation(title_x, title_y);
            }

            return true;
        }

        private void InternalDrawBackground(PVRContext pvrctx) {
            if (this.change_background_from >= 0)
                this.backgrounds[this.change_background_from].sprite.Draw(pvrctx);

            if (this.current_background >= 0)
                this.backgrounds[this.current_background].sprite.Draw(pvrctx);
        }

        private void InternalDrawPortraits(PVRContext pvrctx) {
            float draw_width, draw_height;
            float portrait_line_x, portrait_line_y, portrait_line_width;

            if (this.current_speechimage != null) {
                portrait_line_x = this.current_speechimage.portrait_line_x;
                portrait_line_y = this.current_speechimage.portrait_line_y;
                portrait_line_width = this.current_speechimage.portrait_line_width;
                if (this.current_speechimage.portrait_line_is_relative) {
                    portrait_line_x += this.current_speechimage.offset_x;
                    portrait_line_y += this.current_speechimage.offset_y;
                }
            } else {
                portrait_line_x = 0;
                portrait_line_y = Funkin.SCREEN_RESOLUTION_HEIGHT / 2.0f;
                portrait_line_width = 0.9f * Funkin.SCREEN_RESOLUTION_WIDTH;
            }

            foreach (Portrait portrait in this.visible_portraits) {
                portrait.statesprite.GetDrawSize(out draw_width, out draw_height);

                float draw_x = portrait.position * portrait_line_width;
                float draw_y = 0;

                switch (this.current_speechimage.align_horizontal) {
                    case Align.NONE:
                        if (portrait.position == 0.5f) {
                            draw_x -= draw_width / 2f;
                        } else if (portrait.position > 0.5f) {
                            draw_x -= draw_width;
                        }
                        break;
                    case Align.END:
                        draw_x -= draw_width;
                        break;
                    case Align.CENTER:
                        draw_x -= draw_width / 2f;
                        break;
                }

                switch (this.current_speechimage.align_vertical) {
                    case Align.CENTER:
                        draw_y = draw_height / -2f;
                        break;
                    case Align.NONE:
                    case Align.END:
                        draw_y -= draw_height;
                        break;
                }

                if (portrait.is_speaking) {
                    draw_x += portrait.offset_speak_x;
                    draw_y += portrait.offset_speak_y;
                } else {
                    draw_x += portrait.offset_idle_x;
                    draw_y += portrait.offset_idle_y;
                }

                pvrctx.Save();
                pvrctx.CurrentMatrix.Translate(draw_x + portrait_line_x, draw_y + portrait_line_y);
                portrait.statesprite.Draw(pvrctx);
                pvrctx.Restore();
            }

        }

        private void InternalDrawSpeech(PVRContext pvrctx) {
            if (this.current_speechimage == null) return;

            this.current_speechimage.statesprite.Draw(pvrctx);
            this.texsprite_title.Draw(pvrctx);
            this.texsprite_speech.Draw(pvrctx);
        }

        private void InternalPreparePrintText() {
            if (this.current_dialog == null) return;

            if (this.current_dialog_line < this.current_dialog.lines_size) {
                DialogLine dialog_line = this.current_dialog.lines[this.current_dialog_line];
                ApplyState2(dialog_line.target_state_name, null);
                ApplyState2(dialog_line.target_state_name, dialog_line.text);
                if (this.do_exit) return;
            }

            if (this.do_skip) {
                this.current_dialog_line++;
                this.do_skip = false;
                InternalPreparePrintText();
                return;
            }

            if (this.current_dialog_line >= this.current_dialog.lines_size) {
                Close();
                return;
            }

            DialogLine line = this.current_dialog.lines[this.current_dialog_line];

            if (this.do_instant_print) {
                InternalNotifyScript(true);
                this.texsprite_speech.SetText(line.text);
                InternalNotifyScript(false);
                InternalToggleIdle();
                this.do_instant_print = false;
                this.is_speaking = false;
                return;
            }

            StringBuilder buffer = this.current_dialog_buffer;
            Grapheme grapheme = new Grapheme() { code = 0x00, size = 0 };
            int index = 0;

            buffer.Clear();
            //free(this.current_dialog_mask);
            this.current_dialog_mask = null;

            while (StringUtils.GetCharacterCodepoint(line.text, index, grapheme)) {
                switch (grapheme.code) {
                    case 0x20:// space
                    case 0x09:// tab
                    case 0x0A:// new line
                              //case 0x2E:// dot
                              //case 0x3A:// double-dot
                              //case 0x2C:// comma
                              //case 0x3B:// dot-comma
                        buffer.AddCharCodepointKDY(grapheme.code);
                        break;
                    default:
                        buffer.AddCharCodepointKDY(0xA0);// hard-space
                        break;
                }
                index += grapheme.size;
            }
            this.current_dialog_codepoint_index = 0;
            this.current_dialog_codepoint_length = index;
            this.current_dialog_mask = buffer.GetCopyKDY();
            this.is_speaking = true;

            if (this.chars_per_second > 0)
                this.current_dialog_duration = ((double)index / this.chars_per_second) * 1000.0;
            else
                this.current_dialog_duration = 0;
            this.current_dialog_duration += index * this.char_delay;
            this.current_dialog_elapsed = 0.0;

            if (this.current_dialog_line > 0 && this.click_text != null) this.click_text.Replay();

            InternalNotifyScript(true);
            this.texsprite_speech.SetText(this.current_dialog_mask);
        }

        private void InternalPrintText() {
            if (this.current_dialog == null) return;

            DialogLine line = this.current_dialog.lines[this.current_dialog_line];
            StringBuilder buffer = this.current_dialog_buffer;
            int length = this.current_dialog_codepoint_length;

            int next_index = (int)(length * (this.current_dialog_elapsed / this.current_dialog_duration));
            if (next_index == this.current_dialog_codepoint_index && !this.do_no_wait) return;

            if (this.click_char != null) this.click_char.Replay();
            this.current_dialog_codepoint_index = next_index;

            if (next_index < length) {
                buffer.Clear();
                buffer.AddSubstringKDY(line.text, 0, next_index);
                buffer.AddSubstringKDY(this.current_dialog_mask, next_index, length);

                this.texsprite_speech.SetText(buffer.InternKDY());
                return;
            }

            InternalNotifyScript(false);
            this.texsprite_speech.SetText(line.text);

            this.is_speaking = false;
            this.current_dialog_line++;

            if (this.do_no_wait) {
                this.do_no_wait = false;
                InternalPreparePrintText();
                return;
            }

            InternalToggleIdle();
        }

        private void InternalToggleIdle() {
            foreach (Portrait portrait in this.visible_portraits) {
                if (!portrait.is_speaking) continue;
                portrait.is_speaking = false;
                if (portrait.statesprite.StateToggle(Dialogue.IDLE) || portrait.statesprite.StateToggle(null)) {
                    AnimSprite anim = portrait.statesprite.StateGet().animation;
                    if (anim != null) anim.Restart();
                } else {
                    Dialogue.InternalStopPortraitAnimation(portrait);
                }
            }
        }

        private void InternalDestroyExternalDialog() {
            if (this.dialog_external == null) return;

            for (int i = 0 ; i < this.dialogs_size ; i++) {
                if (this.dialogs[i] == this.dialog_external) return;
            }

            for (int i = 0 ; i < this.dialog_external.lines_size ; i++) {
                //free(this.dialog_external.lines[i].target_state_name);
                //free(this.dialog_external.lines[i].text);
            }
            //free(this.dialog_external.lines);
            //free(this.dialog_external.full_path);
            //free(this.dialog_external);
            this.dialog_external = null;
        }

        private void InternalNotifyScript(bool is_line_start) {
            if (this.script == null) return;

            int current_dialog_line = this.current_dialog_line;
            string state_name = this.current_dialog.lines[this.current_dialog_line].target_state_name;
            string text = this.current_dialog.lines[this.current_dialog_line].text;

            if (is_line_start)
                this.script.notify_dialogue_line_starts(current_dialog_line, state_name, text);
            else
                this.script.notify_dialogue_line_ends(current_dialog_line, state_name, text);
        }



        private static void InternalParseAudioList(XmlParserNode root_node, ArrayList<Audio> audios) {
            foreach (XmlParserNode node in root_node.Children) {
                switch (node.TagName) {
                    case "Audio":
                        Dialogue.InternalParseAudio(node, audios);
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_audiolist() unknown node: " + node.TagName);
                        break;
                }
            }
        }

        private static void InternalParseBackgroundList(XmlParserNode root_node, float max_width, float max_height, ArrayList<Background> backgrounds) {
            string base_src = root_node.GetAttribute("baseSrc");

            foreach (XmlParserNode node in root_node.Children) {
                switch (node.TagName) {
                    case "Image":
                        Dialogue.InternalParseImage(node, max_width, max_height, base_src, backgrounds);
                        break;
                    case "Color":
                        Dialogue.InternalParseColor(node, max_width, max_height, backgrounds);
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_backgroundlist() unknown node: " + node.TagName);
                        break;
                }
            }
        }

        private static void InternalParsePortraitList(XmlParserNode root_node, ArrayList<Portrait> portraits) {
            string base_model = root_node.GetAttribute("baseModel");

            foreach (XmlParserNode node in root_node.Children) {
                switch (node.TagName) {
                    case "Portrait":
                        Dialogue.InternalParsePortrait(node, base_model, portraits);
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_portraitlist() unknown node: " + node.TagName);
                        break;
                }
            }
        }

        private static void InternalParseAnimationUI(XmlParserNode root_node, AnimsUI anims_ui) {
            string animation_list = root_node.GetAttribute("animationList");
            AnimList animlist = null;

            if (!String.IsNullOrEmpty(animation_list)) {
                animlist = AnimList.Init(animation_list);
                if (String.IsNullOrEmpty(animation_list)) {
                    Console.Error.WriteLine("[ERROR] dialogue_internal_parse_animationui() can not initialize: " + root_node.OuterHTML);
                    return;
                }
            }

            foreach (XmlParserNode node in root_node.Children) {
                switch (node.TagName) {
                    case "Set":
                    case "UnSet":
                        Dialogue.InternalParseAnimationUISet(node, animlist, anims_ui);
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_animationui() unknown node: " + node.TagName);
                        break;
                }
            }

            if (animlist != null) animlist.Destroy();
        }

        private static void InternalParseState(XmlParserNode root_node, ArrayList<State> states) {
            bool initial = VertexProps.ParseBoolean(root_node, "initial", false);
            string name = root_node.GetAttribute("name");
            string if_line = root_node.GetAttribute("ifLine");
            ArrayList<Action> actions = new ArrayList<Action>();

            foreach (XmlParserNode node in root_node.Children) {
                Action action = new Action() {
                    name = node.GetAttribute("name"),
                    rgba = new float[] { Single.NaN, Single.NaN, Single.NaN, Single.NaN }
                };

                switch (node.TagName) {
                    case "AudioPlay":
                        action.type = Type.AUDIO_PLAY;
                        break;
                    case "AudioPause":
                        action.type = Type.AUDIO_PAUSE;
                        break;
                    case "AudioFadeIn":
                        action.type = Type.AUDIO_FADEIN;
                        action.duration = VertexProps.ParseFloat(node, "duration", 1000f);
                        break;
                    case "AudioFadeOut":
                        action.type = Type.AUDIO_FADEOUT;
                        action.duration = VertexProps.ParseFloat(node, "duration", 1000f);
                        break;
                    case "AudioStop":
                        action.type = Type.AUDIO_STOP;
                        break;
                    case "AudioVolume":
                        action.type = Type.AUDIO_VOLUME;
                        action.volume = VertexProps.ParseFloat(node, "volume", 1f);
                        break;
                    case "BackgroundSet":
                        action.type = Type.BACKGROUND_SET;
                        break;
                    case "BackgroundChange":
                        action.type = Type.BACKGROUND_CHANGE;
                        break;
                    case "BackgroundRemove":
                        action.type = Type.BACKGROUND_REMOVE;
                        break;
                    case "Lua":
                        action.type = Type.LUA;
                        action.lua_eval = node.TextContent;
                        action.lua_function = node.GetAttribute("function");
                        break;
                    case "Exit":
                        action.type = Type.EXIT;
                        break;
                    case "PortraitAdd":
                        action.type = Type.PORTRAIT_ADD;
                        action.random_from_prefix = node.GetAttribute("randomFromPrefix");
                        action.no_speak = VertexProps.ParseBoolean(node, "noSpeak", false);
                        break;
                    case "PortraitRemove":
                        action.type = Type.PORTRAIT_REMOVE;
                        break;
                    case "PortraitRemoveAll":
                        action.type = Type.PORTRAIT_REMOVEALL;
                        action.animate_remove = VertexProps.ParseBoolean(node, "animateRemove", false);
                        break;
                    case "AudioUI":
                        action.type = Type.AUDIO_UI;
                        action.click_text = node.GetAttribute("clickText");
                        action.click_char = node.GetAttribute("clickChar");
                        break;
                    case "TextSpeed":
                        action.type = Type.TEXT_SPEED;
                        action.chars_per_second = VertexProps.ParseInteger(node, "charsPerSecond", 0);
                        action.char_delay = VertexProps.ParseInteger(node, "charDelay", 75);
                        break;
                    case "TextSkip":
                        action.type = Type.TEXT_SKIP;
                        break;
                    case "TextInstantPrint":
                        action.type = Type.TEXT_INSTANTPRINT;
                        break;
                    case "SpeechBackground":
                        action.type = Type.SPEECH_BACKGROUND;
                        if (action.name == null) {
                            string tmp = node.GetAttribute("fromCommon");
                            if (tmp != "none") action.name = tmp;
                        }
                        switch (node.GetAttribute("repeatAnim")) {
                            case "":
                            case null:
                                action.repeat_anim = RepeatAnim.NONE;
                                break;
                            case "once":
                                action.repeat_anim = RepeatAnim.ONCE;
                                break;
                            case "whileSpeaks":
                            case "whilespeaks":
                                action.repeat_anim = RepeatAnim.WHILESPEAKS;
                                break;
                            case "always":
                                action.repeat_anim = RepeatAnim.ALWAYS;
                                break;
                            default:
                                action.repeat_anim = RepeatAnim.NONE;
                                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_state() unknown repeatAnim value: " + node.OuterHTML);
                                break;
                        }
                        break;
                    case "TextFont":
                        action.type = Type.TEXT_FONT;
                        break;
                    case "TextColor":
                        action.type = Type.TEXT_COLOR;
                        Dialogue.InternalReadColor(node, action.rgba);
                        break;
                    case "TextBorderColor":
                        action.type = Type.TEXT_BORDERCOLOR;
                        Dialogue.InternalReadColor(node, action.rgba);
                        break;
                    case "TextBorderOffset":
                        action.type = Type.TEXT_BORDEROFFSET;
                        action.offset_x = VertexProps.ParseFloat(node, "x", Single.NaN);
                        action.offset_y = VertexProps.ParseFloat(node, "y", Single.NaN);
                        break;
                    case "TextSize":
                        action.type = Type.TEXT_SIZE;
                        action.size = VertexProps.ParseFloat(node, "size", 18f);
                        break;
                    case "TextBorderSize":
                        action.type = Type.TEXT_BORDERSIZE;
                        action.size = VertexProps.ParseFloat(node, "size", 2f);
                        break;
                    case "TextBorderEnable":
                        action.type = Type.TEXT_BORDERENABLE;
                        action.enabled = VertexProps.ParseBoolean(node, "enabled", false);
                        break;
                    case "TextParagraphSpace":
                        action.type = Type.TEXT_PARAGRAPHSPACE;
                        action.size = VertexProps.ParseFloat(node, "size", 0f);
                        break;
                    case "TextAlign":
                        action.type = Type.TEXT_ALIGN;
                        action.align_vertical = Dialogue.InternalReadAlign(node, "vertical");
                        action.align_horizontal = Dialogue.InternalReadAlign(node, "horizontal");
                        action.align_paragraph = Dialogue.InternalReadAlign(node, "paragraph");
                        break;
                    case "RunMultipleChoice":
                        action.type = Type.RUNMULTIPLECHOICE;
                        break;
                    case "Title":
                        action.type = Type.TITLE;
                        action.title = node.TextContent;
                        break;
                    case "NoWait":
                        action.type = Type.NOWAIT;
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_state() unknown state action: " + node.OuterHTML);
                        continue;
                }

                actions.Add(action);
            }

            State state = new State() {
                name = name,
                if_line = if_line,
                initial = initial,
                actions = null,
                actions_size = 0
            };

            actions.Destroy2(out state.actions_size, ref state.actions);
            states.Add(state);
        }

        private static void InternalParseMultipleChoice(XmlParserNode root_node, AnimList animlist, ArrayList<Dialog> dialogs, ArrayList<MultipleChoice> multiplechoices) {
            string title = root_node.GetAttribute("title");
            string name = root_node.GetAttribute("name");
            string orientation = root_node.GetAttribute("orientation");
            bool can_leave = VertexProps.ParseBoolean(root_node, "canLeave", false);
            string state_on_leave = root_node.GetAttribute("stateOnLeave");
            uint icon_color = VertexProps.ParseHex2(root_node.GetAttribute("selectorIconColor"), 0x00FFFF, false);
            string icon_model = root_node.GetAttribute("selectorIconColor");
            string icon_model_name = root_node.GetAttribute("selectorIconModelName");
            int default_index = VertexProps.ParseInteger(root_node, "defaultIndex", 0);
            float font_size = VertexProps.ParseFloat(root_node, "fontSize", -1);

            bool is_vertical;

            switch (orientation) {
                case null:
                case "":
                case "vertical":
                    is_vertical = true;
                    break;
                case "horizontal":
                    is_vertical = false;
                    break;
                default:
                    is_vertical = true;
                    Console.Error.WriteLine("[ERROR] dialogue_internal_parse_multiple_choice() unknown orientation value:" + orientation);
                    break;
            }

            Texture texture = null;
            AnimSprite anim = null;

            if (!String.IsNullOrEmpty(icon_model)) {
                if (ModelHolder.UtilsIsKnownExtension(icon_model)) {
                    ModelHolder modeholder = ModelHolder.Init(icon_model);
                    if (modeholder == null) {
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_multiple_choice() can not initialize: " + icon_model);
                    } else {
                        texture = modeholder.GetTexture(true);
                        anim = modeholder.CreateAnimsprite(icon_model_name ?? Dialogue.ICON, true, false);
                        icon_color = modeholder.GetVertexColor();
                        modeholder.Destroy();
                    }
                } else {
                    texture = Texture.Init(icon_model);
                }
            }

            if (anim == null) anim = AnimSprite.InitFromAnimlist(animlist, Dialogue.ICON);

            Sprite icon = Sprite.Init(texture);
            icon.SetVertexColorRGB8(icon_color);
            icon.ExternalAnimationSet(anim);

            Sprite hint = Sprite.InitFromRGB8(0xF9CF51);
            hint.SetAlpha(0f);

            MultipleChoice multiplechoice = new MultipleChoice() {
                icon = icon,
                hint = hint,
                name = name,
                title = title,
                can_leave = can_leave,
                state_on_leave = state_on_leave,
                default_index = default_index,
                font_size = font_size,
                choices = null,
                choices_size = 0,
                is_vertical = is_vertical
            };

            ArrayList<Choice> choices = new ArrayList<Choice>();

            foreach (XmlParserNode node in root_node.Children) {
                switch (node.TagName) {
                    case "Choice":
                        Dialogue.InternalParseChoice(node, dialogs, choices);
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_multiplechoice() unknown: " + node.OuterHTML);
                        break;
                }
            }

            choices.Destroy2(out multiplechoice.choices_size, ref multiplechoice.choices);
            multiplechoices.Add(multiplechoice);
        }

        private static void InternalParseSpeechImageList(XmlParserNode root_node, ArrayList<SpeechImage> speechimages) {
            string base_src = root_node.GetAttribute("baseSrc");
            float text_x = 0f;
            float text_y = 0f;
            float text_width = 0f;
            float text_height = 0f;
            float portrait_line_x = 0f;
            float portrait_line_y = 0f;
            float portrait_line_width = 0f;
            float title_x = 0f;
            float title_y = 0f;
            float offset_x = 0f;
            float offset_y = 0f;
            float offset_idle_x = 0f;
            float offset_idle_y = 0f;
            float offset_open_x = 0f;
            float offset_open_y = 0f;
            Align align_vertical = Align.NONE;
            Align align_horizontal = Align.NONE;
            bool portrait_line_is_relative = false;
            bool title_is_relative = false;
            bool text_is_relative = false;


            foreach (XmlParserNode node in root_node.Children) {
                switch (node.TagName) {
                    case "SpeechImage":
                        SpeechImage speechimage = Dialogue.InternalParseSpeechImage(node, base_src, speechimages);
                        if (speechimage == null) continue;

                        speechimage.text_x = text_x;
                        speechimage.text_y = text_y;
                        speechimage.text_width = text_width;
                        speechimage.text_height = text_height;
                        speechimage.portrait_line_x = portrait_line_x;
                        speechimage.portrait_line_y = portrait_line_y;
                        speechimage.portrait_line_width = portrait_line_width;
                        speechimage.title_x = title_x;
                        speechimage.title_y = title_y;
                        speechimage.offset_x = offset_x;
                        speechimage.offset_y = offset_y;
                        speechimage.portrait_line_is_relative = portrait_line_is_relative;
                        speechimage.title_is_relative = title_is_relative;
                        speechimage.text_is_relative = text_is_relative;
                        speechimage.align_vertical = align_vertical;
                        speechimage.align_horizontal = align_horizontal;

                        foreach (StateSpriteState state in speechimage.statesprite.StateList()) {
                            switch (state.state_name) {
                                case Dialogue.OPEN:
                                    state.offset_x = offset_open_x;
                                    state.offset_y = offset_open_y;
                                    break;
                                case Dialogue.IDLE:
                                    state.offset_x = offset_idle_x;
                                    state.offset_y = offset_idle_y;
                                    break;
                            }
                        }
                        break;
                    case "TextBox":
                        text_x = VertexProps.ParseFloat(node, "x", text_x);
                        text_y = VertexProps.ParseFloat(node, "y", text_y);
                        text_width = VertexProps.ParseFloat(node, "width", text_width);
                        text_height = VertexProps.ParseFloat(node, "height", text_height);
                        text_is_relative = VertexProps.ParseBoolean(node, "isRelative", text_is_relative);
                        break;
                    case "PortraitLine":
                        portrait_line_x = VertexProps.ParseFloat(node, "x", portrait_line_x);
                        portrait_line_y = VertexProps.ParseFloat(node, "y", portrait_line_y);
                        portrait_line_width = VertexProps.ParseFloat(node, "width", portrait_line_width);
                        portrait_line_is_relative = VertexProps.ParseBoolean(node, "isRelative", portrait_line_is_relative);
                        break;
                    case "TitleLocation":
                        title_x = VertexProps.ParseFloat(node, "titleLeft", title_x);
                        title_y = VertexProps.ParseFloat(node, "titleBottom", title_y);
                        title_is_relative = VertexProps.ParseBoolean(node, "isRelative", title_is_relative);
                        break;
                    case "Location":
                        offset_x = VertexProps.ParseFloat(node, "x", offset_x);
                        offset_y = VertexProps.ParseFloat(node, "y", offset_y);
                        break;
                    case "OffsetIdle":
                        offset_idle_x = VertexProps.ParseFloat(node, "x", offset_idle_x);
                        offset_idle_y = VertexProps.ParseFloat(node, "y", offset_idle_y);
                        break;
                    case "OffsetOpen":
                        offset_open_x = VertexProps.ParseFloat(node, "x", offset_open_x);
                        offset_open_y = VertexProps.ParseFloat(node, "y", offset_open_y);
                        break;
                    case "PortraitAlign":
                        align_vertical = Dialogue.InternalReadAlign(node, "vertical");
                        align_horizontal = Dialogue.InternalReadAlign(node, "horizontal");
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] dialogue_internal_parse_speechimagelist() unknown node: " + node.TagName);
                        break;
                }
            }
        }

        private static void InternalParseImportPortraitList(XmlParserNode root_node, ArrayList<Portrait> portraits) {
            string dialogue_src = root_node.GetAttribute("dialogueSrc");
            if (String.IsNullOrEmpty(dialogue_src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_importportraitlist() missing dialogueSrc: " + root_node.OuterHTML);
                return;
            }

            if (dialogue_src.LowercaseEndsWithKDY(".json")) {
                Dialogue.InternalLoadPsychCharacterJSON(dialogue_src, portraits);
                return;
            }

            XmlParser xmlparser = XmlParser.Init(dialogue_src);
            if (xmlparser == null || xmlparser.GetRoot() == null) {
                if (xmlparser != null) xmlparser.Destroy();
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_importportraitlist() can not load: " + dialogue_src);
                return;
            }

            FS.FolderStackPush();
            FS.SetWorkingFolder(dialogue_src, true);

            foreach (XmlParserNode node in root_node.Children) {
                if (node.TagName != "Definition") continue;
                foreach (XmlParserNode node2 in root_node.Children) {
                    if (node2.TagName != "PortraitList") continue;
                    Dialogue.InternalParseImportPortraitList(node2, portraits);
                }
            }

            FS.FolderStackPop();
            xmlparser.Destroy();
        }


        private static void InternalParseAudio(XmlParserNode node, ArrayList<Audio> audios) {
            //<Audio name="bg_music2" src="/assets/music/weeb.ogg" volume="1.0" looped="true" defaultAs="clickText|clickChar" />
            string name = node.GetAttribute("name");
            string src = node.GetAttribute("src");
            float volume = VertexProps.ParseFloat(node, "volume", 1.0f);
            bool looped = VertexProps.ParseBoolean(node, "looped", false);

            // avoid duplicates
            for (int i = 0, size = audios.Size() ; i < size ; i++) {
                if (audios.Get(i).name == name) return;
            }

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_audio() missing name: " + node.OuterHTML);
                return;
            }
            if (String.IsNullOrEmpty(src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_audio() missing src: " + node.OuterHTML);
                return;
            }

            SoundPlayer soundplayer = SoundPlayer.Init(src);
            if (soundplayer == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_audio() can not initialize: " + node.OuterHTML);
                return;
            }

            soundplayer.SetVolume(volume);
            soundplayer.LoopEnable(looped);

            Audio audio = new Audio() {
                name = name,
                was_playing = false,
                soundplayer = soundplayer
            };

            audios.Add(audio);
        }

        private static void InternalParseImage(XmlParserNode node, float max_width, float max_height, string base_src, ArrayList<Background> backgrounds) {
            //<Image
            //          name="scene0"
            //          src="/assets/image/theater.xml"
            //          entryName="floor"
            //          center="true"
            //          cover="true"
            //          looped="true"
            //          alpha="0.7"
            //          />

            string name = node.GetAttribute("name");
            string src = node.GetAttribute("src");
            string entry_name = node.GetAttribute("entryName");
            bool center = VertexProps.ParseBoolean(node, "center", false);
            bool cover = VertexProps.ParseBoolean(node, "cover", false);
            bool looped = VertexProps.ParseBoolean(node, "looped", false);
            bool has_looped = node.HasAttribute("looped");
            float alpha = VertexProps.ParseFloat(node, "alpha", 1f);

            // avoid duplicates
            for (int i = 0, size = backgrounds.Size() ; i < size ; i++) {
                if (backgrounds.Get(i).name == name) return;
            }

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_image() missing name: " + node.OuterHTML);
                return;
            }

            if (src == null) src = base_src;
            if (String.IsNullOrEmpty(src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_image() missing src: " + node.OuterHTML);
                return;
            }

            Texture texture = null;
            AnimSprite animsprite = null;
            uint vertex_color_rgb8 = 0xFFFFFF;
            bool init_failed;

            if (ModelHolder.UtilsIsKnownExtension(src)) {
                ModelHolder modelholder = ModelHolder.Init(src);
                init_failed = modelholder == null;
                if (modelholder != null) {
                    texture = modelholder.GetTexture(true);
                    animsprite = modelholder.CreateAnimsprite(entry_name, true, false);
                    vertex_color_rgb8 = modelholder.GetVertexColor();
                    if (has_looped && animsprite != null) animsprite.SetLoop(looped ? -1 : 1);
                    modelholder.Destroy();
                }
            } else {
                texture = Texture.Init(src);
                init_failed = texture == null;
            }

            if (init_failed) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_image() can not initialize: " + node.OuterHTML);
                return;
            }

            Sprite sprite = Sprite.Init(texture);
            sprite.SetVertexColorRGB8(vertex_color_rgb8);
            sprite.ExternalAnimationSet(animsprite);
            sprite.SetDrawLocation(0f, 0f);
            sprite.SetAlpha(alpha);
            ImgUtils.CalcResizeSprite(sprite, max_width, max_height, cover, center);

            Background background = new Background() {
                name = name,
                sprite = sprite
            };

            backgrounds.Add(background);
        }

        private static void InternalParseColor(XmlParserNode node, float max_width, float max_height, ArrayList<Background> backgrounds) {
            //<Color name="faded_white" rgba="0xFFFFFF7F" />
            //<Color name="faded_red" r="1.0" g="0.0" g="0.0" a="0.7" />

            string name = node.GetAttribute("name");
            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_image() missing name: " + node.OuterHTML);
                return;
            }

            // avoid duplicates
            for (int i = 0, size = backgrounds.Size() ; i < size ; i++) {
                if (backgrounds.Get(i).name == name) return;
            }

            float[] rgba = { 0f, 0f, 0f, 1f };
            Dialogue.InternalReadColor(node, rgba);

            Sprite sprite = Sprite.Init(null);
            sprite.SetVertexColor(rgba[0], rgba[1], rgba[2]);
            sprite.SetAlpha(rgba[3]);
            sprite.SetDrawSize(max_width, max_height);

            Background background = new Background() {
                name = name,
                sprite = sprite
            };

            backgrounds.Add(background);
        }

        private static void InternalParseFont(XmlParserNode node, ArrayList<Font> fonts) {
            //<Font name="vcr" src="vcr.ttf" glyphSuffix="bold" glyphAnimated="false"  colorByDifference="true" />

            string name = node.GetAttribute("name");
            string src = node.GetAttribute("src");
            string glyph_suffix = node.GetAttribute("glyphSuffix");
            bool glyph_animated = VertexProps.ParseBoolean(node, "glyphAnimated", false);
            bool color_by_difference = VertexProps.ParseBoolean(node, "colorByDifference", false);

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_font() missing name: " + node.OuterHTML);
                return;
            }
            if (String.IsNullOrEmpty(src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_font() missing src: " + node.OuterHTML);
                return;
            }
            if (!FS.FileExists(src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_font() font file not found: " + node.OuterHTML);
                return;
            }

            // avoid duplicates
            for (int i = 0, size = fonts.Size() ; i < size ; i++) {
                if (fonts.Get(i).name == name) return;
            }

            IFontRender instance;
            bool is_atlas;

            if (Atlas.UtilsIsKnownExtension(src)) {
                instance = FontGlyph.Init(src, glyph_suffix, glyph_animated);
                instance.EnableColorByDifference(color_by_difference);
                is_atlas = true;
            } else {
                instance = FontType.Init(src);
                is_atlas = false;
            }

            if (instance == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_font() can not initialize: " + node.OuterHTML);
                return;
            }

            Font font = new Font() {
                name = name,
                fontholder = new FontHolder(instance, is_atlas, 18f)
            };

            fonts.Add(font);
        }

        private static void InternalParsePortrait(XmlParserNode node, string base_model, ArrayList<Portrait> portraits) {
            // <Portrait
            //          name="senpai" src="/assets/image/senpai.png" scale="1.0"
            //          speakAnim="talking" idleAnim="idle_anim" anim="talking_and_idle_anim"
            //          speakAnimLooped="true|false" idleAnimLooped="true|false" animLooped="true|false"
            //          mirror="true|false"
            //          positionPercent="0.0" position="left|center|right"
            //          x="0" y="0"
            //          offestSpeakX="0" offestSpeakY="0"
            //          offestIdleX="0" offestIdleY="0" 
            // />

            string name = node.GetAttribute("name");
            string src = node.GetAttribute("src");
            float scale = VertexProps.ParseFloat(node, "scale", 1f);
            string speak_anim = node.GetAttribute("speakAnim");
            string idle_anim = node.GetAttribute("idleAnim");
            string simple_anim = node.GetAttribute("anim");
            bool speak_anim_looped = VertexProps.ParseBoolean(node, "speakAnimLooped", true);
            bool idle_anim_looped = VertexProps.ParseBoolean(node, "idleAnimLooped", true);
            bool simple_anim_looped = VertexProps.ParseBoolean(node, "animLooped", true);
            bool mirror = VertexProps.ParseBoolean(node, "mirror", false);
            float position = VertexProps.ParseFloat(node, "positionPercent", 0f);
            string position_align = node.GetAttribute("position");
            float x = VertexProps.ParseFloat(node, "x", 0f);
            float y = VertexProps.ParseFloat(node, "y", 0f);
            float offset_speak_x = VertexProps.ParseFloat(node, "offestSpeakX", 0f);
            float offset_speak_y = VertexProps.ParseFloat(node, "offestSpeakY", 0f);
            float offset_idle_x = VertexProps.ParseFloat(node, "offestIdleX", 0f);
            float offset_idle_y = VertexProps.ParseFloat(node, "offestIdleY", 0f);

            // avoid duplicates
            for (int i = 0, size = portraits.Size() ; i < size ; i++) {
                if (portraits.Get(i).name == name) return;
            }

            switch (position_align) {
                case "left":
                    position = 0f;
                    break;
                case "center":
                    position = 0.5f;
                    break;
                case "right":
                    position = 1f;
                    break;
                case "":
                case null:
                    break;
                default:
                    Console.Error.WriteLine("[WARN] dialogue_internal_parse_portrait() unknown position: " + node.OuterHTML);
                    break;
            }

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_portrait() missing name: " + node.OuterHTML);
                return;
            }

            if (src == null) src = base_model;
            if (String.IsNullOrEmpty(src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_portrait() missing src: " + node.OuterHTML);
                return;
            }

            StateSprite statesprite = null;

            if (ModelHolder.UtilsIsKnownExtension(src)) {
                ModelHolder modelholder = ModelHolder.Init(src);
                if (modelholder == null) goto L_check_failed;

                statesprite = StateSprite.InitFromTexture(null);
                statesprite.ChangeDrawSizeInAtlasApply(true, scale);
                statesprite.SetDrawLocation(0f, 0f);

                Dialogue.InternalAddState(statesprite, modelholder, simple_anim, null, scale, simple_anim_looped);
                Dialogue.InternalAddState(statesprite, modelholder, idle_anim, Dialogue.IDLE, scale, false);
                Dialogue.InternalAddState(statesprite, modelholder, speak_anim, Dialogue.SPEAK, scale, false);

                modelholder.Destroy();
            } else {
                Texture texture = Texture.Init(src);
                if (texture == null) goto L_check_failed;

                statesprite = StateSprite.InitFromTexture(texture);
                statesprite.SetDrawLocation(0f, 0f);

                float width, height;
                texture.GetOriginalDimmensions(out width, out height);
                statesprite.SetDrawSize(width * scale, height * scale);
            }

L_check_failed:
            if (statesprite == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_portrait() can not initialize: " + node.OuterHTML);
                return;
            }

            foreach (StateSpriteState state in statesprite.StateList()) {
                state.offset_x += x;
                state.offset_y += y;
            }
            statesprite.FlipTexture(mirror, null);
            //statesprite.FlipRenderedTextureEnableCorrection(false);

            Portrait portrait = new Portrait() {
                name = name,
                statesprite = statesprite,
                position = position,
                is_removed = false,
                is_added = false,
                is_speaking = false,
                offset_speak_x = offset_speak_x,
                offset_speak_y = offset_speak_y,
                offset_idle_x = offset_idle_x,
                offset_idle_y = offset_idle_y,
                has_speak = statesprite.StateHas(Dialogue.SPEAK),
                has_idle = statesprite.StateHas(Dialogue.IDLE),
                speak_anim_looped = speak_anim_looped,
                idle_anim_looped = idle_anim_looped
            };

            portraits.Add(portrait);
        }

        private static void InternalParseAnimationUISet(XmlParserNode node, AnimList animlist, AnimsUI anims_ui) {
            //<Set name="backgroundIn|backgroundOut" anim="anim123" />
            //<Set name="portraitLeftIn|portraitCenterIn|portraitRightIn" anim="anim123" />
            //<Set name="portraitIn|portraitOut" anim="anim123" />
            //<Set name="portraitLeftOut|portraitCenterOut|portraitRightOut" anim="anim123" />

            string name = node.GetAttribute("name");
            string anim = node.GetAttribute("anim");

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_animation_ui_set() missing name: " + node.OuterHTML);
                return;
            }

            AnimSprite animsprite = null;
            if (!String.IsNullOrEmpty(anim)) {
                if (animlist == null) {
                    Console.Error.WriteLine("[ERROR] dialogue_internal_parse_animation_ui_set() can not initialize without animlist: " + node.OuterHTML);
                    return;
                }
                animsprite = AnimSprite.InitFromAnimlist(animlist, anim);
                if (animsprite == null) {
                    Console.Error.WriteLine("[ERROR] dialogue_internal_parse_animation_ui_set() can not initialize: " + node.OuterHTML);
                    return;
                }
            }

            AnimSprite old_anim = null;

            switch (name.ToLower()) {
                case "portraitin":
                    if (anims_ui.portrait_left_in != null) anims_ui.portrait_left_in.Destroy();
                    if (anims_ui.portrait_center_in != null) anims_ui.portrait_center_in.Destroy();
                    if (anims_ui.portrait_right_in != null) anims_ui.portrait_right_in.Destroy();

                    anims_ui.portrait_left_in = animsprite != null ? animsprite.Clone() : null;
                    anims_ui.portrait_center_in = animsprite != null ? animsprite.Clone() : null;
                    anims_ui.portrait_right_in = animsprite != null ? animsprite.Clone() : null;
                    break;
                case "portraitout":
                    if (anims_ui.portrait_center_out != null) anims_ui.portrait_center_out.Destroy();
                    if (anims_ui.portrait_right_out != null) anims_ui.portrait_right_out.Destroy();
                    if (anims_ui.portrait_right_out != null) anims_ui.portrait_right_out.Destroy();

                    anims_ui.portrait_left_out = animsprite != null ? animsprite.Clone() : null;
                    anims_ui.portrait_center_out = animsprite != null ? animsprite.Clone() : null;
                    anims_ui.portrait_right_out = animsprite != null ? animsprite.Clone() : null;
                    break;
                case "portraitleftin":
                    old_anim = anims_ui.portrait_left_in;
                    anims_ui.portrait_left_in = animsprite;
                    break;
                case "portraitcenterin":
                    old_anim = anims_ui.portrait_center_in;
                    anims_ui.portrait_center_in = animsprite;
                    break;
                case "portrairighttin":
                    old_anim = anims_ui.portrait_right_in;
                    anims_ui.portrait_right_in = animsprite;
                    break;
                case "portraitleftout":
                    old_anim = anims_ui.portrait_left_out;
                    anims_ui.portrait_left_out = animsprite;
                    break;
                case "portraitcenterout":
                    old_anim = anims_ui.portrait_center_out;
                    anims_ui.portrait_center_out = animsprite;
                    break;
                case "portraitrightout":
                    old_anim = anims_ui.portrait_right_out;
                    anims_ui.portrait_right_out = animsprite;
                    break;
                case "backgroundin":
                    old_anim = anims_ui.background_in;
                    anims_ui.background_in = animsprite;
                    break;
                case "backgroundout":
                    old_anim = anims_ui.background_out;
                    anims_ui.background_out = animsprite;
                    break;
                case "open":
                    old_anim = anims_ui.open;
                    anims_ui.open = animsprite;
                    break;
                case "close":
                    old_anim = anims_ui.close;
                    anims_ui.close = animsprite;
                    break;
                default:
                    Console.Error.WriteLine("[ERROR] dialogue_internal_parse_animation_ui_set() unknown name: " + node.OuterHTML);
                    if (animsprite != null) animsprite.Destroy();
                    break;
            }

            if (old_anim != null) old_anim.Destroy();
        }

        private static void InternalParseChoice(XmlParserNode node, ArrayList<Dialog> dialogs, ArrayList<Choice> choices) {
            // <Choice
            //          text="Exit"
            //          dialogFile="dialogs.txt"
            //          exit="true|false"
            //          runMultipleChoice="questions"
            //          luaFunction="somefunc"
            //          luaEval="doexit('from leave', true)"
            // >

            string text = node.GetAttribute("text");
            string dialogs_file = node.GetAttribute("dialogFile");
            bool exit = VertexProps.ParseBoolean(node, "exit", false);
            string run_multiple_choice = node.GetAttribute("runMultipleChoice");
            string lua_function = node.GetAttribute("luaFunction");
            string lua_eval = node.GetAttribute("luaEval");

            if (text == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_choice() missing text in:" + node.OuterHTML);
                return;
            }

            int dialog_id = -1;

            if (!String.IsNullOrEmpty(dialogs_file))
                dialog_id = Dialogue.InternalParseDialog(dialogs_file, dialogs);

            Choice choice = new Choice() {
                text = text,
                dialog_id = dialog_id,
                exit = exit,
                lua_eval = lua_eval,
                lua_function = lua_function,
                run_multiple_choice = run_multiple_choice
            };

            choices.Add(choice);
        }

        private static SpeechImage InternalParseSpeechImage(XmlParserNode node, string base_src, ArrayList<SpeechImage> speechimages) {
            // <Image
            //          name="normal"
            //          mirror="true|false"
            //          openAnim="Speech Bubble Normal Open"
            //          idleAnim="speech bubble normal"
            //          idleLooped="true|false"
            //          src="/assets/image/model.xml"
            //          scale="1.0"
            //  >

            string name = node.GetAttribute("name");
            float scale = VertexProps.ParseFloat(node, "scale", 1f);
            string open_anim = node.GetAttribute("openAnim");
            string idle_anim = node.GetAttribute("idleAnim");
            bool idle_looped = VertexProps.ParseBoolean(node, "idleLooped", true);
            bool mirror = VertexProps.ParseBoolean(node, "mirror", false);
            bool has_mirror = node.HasAttribute("mirror");
            string src = node.GetAttribute("src");

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_speechimage() missing name: " + node.OuterHTML);
                return null;
            }

            // avoid duplicates
            for (int i = 0, size = speechimages.Size() ; i < size ; i++) {
                if (speechimages.Get(i).name == name) return null;
            }

            if (src == null) src = base_src;
            if (String.IsNullOrEmpty(src)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_speechimage() missing src: " + node.OuterHTML);
                return null;
            }

            StateSprite statesprite = null;

            if (ModelHolder.UtilsIsKnownExtension(src)) {
                ModelHolder modelholder = ModelHolder.Init(src);
                if (modelholder == null) goto L_check_failed;

                statesprite = StateSprite.InitFromTexture(null);
                statesprite.SetDrawLocation(0f, 0f);

                Dialogue.InternalAddState(statesprite, modelholder, open_anim, Dialogue.OPEN, scale, false);
                Dialogue.InternalAddState(statesprite, modelholder, idle_anim, Dialogue.IDLE, scale, idle_looped);

                if (statesprite.StateList().Count() < 1) {
                    Texture texture = modelholder.GetTexture(true);
                    if (texture != null) {
                        float orig_width, orig_height;
                        texture.GetOriginalDimmensions(out orig_width, out orig_height);
                        statesprite.SetTexture(texture, true);
                        statesprite.SetDrawSize(orig_width * scale, orig_height * scale);
                    } else {
                        statesprite.Destroy();
                        statesprite = null;
                    }
                }

                modelholder.Destroy();
            } else {
                Texture texture = Texture.Init(src);
                if (texture == null) goto L_check_failed;

                statesprite = StateSprite.InitFromTexture(texture);
                statesprite.SetDrawLocation(0f, 0f);
            }

L_check_failed:
            if (statesprite == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_speechimage() can not initialize: " + node.OuterHTML);
                return null;
            }

            if (has_mirror) statesprite.FlipTexture(mirror, null);
            if (scale > 0f) statesprite.ChangeDrawSizeInAtlasApply(true, scale);

            SpeechImage speechimage = new SpeechImage() {
                name = name,
                statesprite = statesprite
            };

            speechimages.Add(speechimage);
            return speechimage;
        }

        private static void InternalLoadPsychCharacterJSON(string src, ArrayList<Portrait> portraits) {
            JSONParser json = JSONParser.LoadFrom(src);
            if (json == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_load_psych_character_json() can not load: " + src);
                return;
            }

            JSONToken animations = JSONParser.ReadArray(json, "animations");
            int animations_length = JSONParser.ReadArrayLength(animations);

            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);

            float position_x, position_y;
            Dialogue.InternalReadOffset(json, "position", out position_x, out position_y);
            float scale = (float)JSONParser.ReadNumberDouble(json, "scale", 1.0);
            float dialogue_pos;

            string unparsed_dialogue_pos = JSONParser.ReadString(json, "dialogue_pos", null);
            switch (unparsed_dialogue_pos) {
                case null:
                case "left":
                    dialogue_pos = 0f;
                    break;
                case "center":
                    dialogue_pos = 0.5f;
                    break;
                case "right":
                    dialogue_pos = 1f;
                    break;
                default:
                    dialogue_pos = 0f;
                    Console.Error.WriteLine("[ERROR] dialogue_internal_load_psych_character_json() unrecognized dialogue_pos: " + src);
                    break;
            }

            string image = JSONParser.ReadString(json, "image", null);
            if (String.IsNullOrEmpty(image)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_load_psych_character_json() missing 'image' in json: " + src);
                goto L_return;
            } else if (image.IndexOf('.', 0) >= 0) {
                // append atlas extension
                string tmp = StringUtils.Concat(image, ".xml");
                //free(image);
                image = tmp;
            }

            //
            // Note:
            //      There no such path like "/assets/shared/images/dialogue/required_atlas_file.xml"
            //      Attempt load the atlas from the current folder "./required_atlas_file.xml"
            //      or fallback to "/assets/common/image/dialogue/required_atlas_file.xml"
            //
            ModelHolder modelholder = null;

            if (FS.FileExists(image)) {
                modelholder = ModelHolder.Init(src);
            } else {
                // try load from common folder
                string tmp = StringUtils.Concat("/assets/common/image/dialogue/", image);
                if (FS.FileExists(tmp)) {
                    modelholder = ModelHolder.Init(tmp);
                }
                //free(tmp);
            }

            if (modelholder == null) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_load_psych_character_json() unreconized image path: " + image);
                goto L_return;
            }

            // parse animations
            for (int i = 0 ; i < animations_length ; i++) {
                JSONToken obj = JSONParser.ReadArrayItemObject(animations, i);

                float idle_offset_x, idle_offset_y;
                float loop_offset_x, loop_offset_y;
                string anim, idle_name, loop_name;

                Dialogue.InternalReadOffset(obj, "idle_offsets", out idle_offset_x, out idle_offset_y);
                Dialogue.InternalReadOffset(obj, "loop_offsets", out loop_offset_x, out loop_offset_y);
                anim = JSONParser.ReadString(obj, "anim", null);
                idle_name = JSONParser.ReadString(obj, "idle_name", null);
                loop_name = JSONParser.ReadString(obj, "loop_name", null);

                // prepare portrait
                AnimSprite anim_idle = modelholder.CreateAnimsprite(idle_name, true, false);
                AnimSprite anim_speak = modelholder.CreateAnimsprite(loop_name, true, false);
                StateSprite statesprite = StateSprite.InitFromTexture(null);
                statesprite.SetDrawLocation(0f, 0f);

                // accumulate offsets, assume 'position' as offset in portrait line
                idle_offset_x += position_x;
                idle_offset_y += position_y;
                loop_offset_x += position_x;
                loop_offset_y += position_y;

                if (scale >= 0f) statesprite.ChangeDrawSizeInAtlasApply(true, scale);
                Dialogue.InternalComputeState(modelholder, statesprite, anim_idle, idle_offset_x, idle_offset_y, false);
                Dialogue.InternalComputeState(modelholder, statesprite, anim_speak, loop_offset_x, loop_offset_y, true);

                Portrait portrait = new Portrait() {
                    name = anim,
                    is_added = false,
                    is_removed = false,
                    is_speaking = false,
                    position = dialogue_pos,
                    statesprite = statesprite
                };
                portraits.Add(portrait);
            }

            modelholder.Destroy();

L_return:
            FS.FolderStackPop();
            JSONParser.Destroy(json);
        }

        private static int InternalParseDialog(string src, ArrayList<Dialog> dialogs) {
            string full_path = FS.GetFullPathAndOverride(src);

            // check if is already loaded
            int id = 0;
            foreach (Dialog existing_dialog in dialogs) {
                if (existing_dialog.full_path == full_path) return id;
                id++;
            }

            // load and parse txt file
            string source = FS.ReadText(full_path);

            if (String.IsNullOrEmpty(source)) {
                Console.Error.WriteLine("[ERROR] dialogue_internal_parse_dialog() can not read: " + src);
                //free(full_path);
                return -1;
            }

            Tokenizer tokenizer = Tokenizer.Init("\n", false, false, source);
            Debug.Assert(tokenizer != null);

            ArrayList<DialogLine> lines = new ArrayList<DialogLine>(tokenizer.CountOccurrences());

            string line;
            string buffered_line = null;
            while ((line = tokenizer.ReadNext()) != null) {
                int end_index = -1;

                if (line.Length > 0 && line[0] == ':') {
                    end_index = line.IndexOf(':', 1);
                }

                if (end_index < 0) {
                    // buffer the current line
                    string tmp = StringUtils.Concat(buffered_line, "\n", line);
                    //free(buffered_line);
                    buffered_line = tmp;
                } else {
                    string state = line.SubstringKDY(1, end_index);
                    string tmp_line = line.SubstringKDY(end_index + 1, line.Length);
                    string final_line = StringUtils.Concat(buffered_line, tmp_line);

                    //free(tmp_line);
                    //free(buffered_line);
                    buffered_line = null;

                    DialogLine dialog_line = new DialogLine() {
                        target_state_name = state,
                        text = final_line
                    };

                    lines.Add(dialog_line);
                }

                //free(line);
            }

            if (buffered_line != null) {
                DialogLine dialog_line;
                if (lines.Size() < 1) {
                    dialog_line = new DialogLine() {
                        target_state_name = null,
                        text = buffered_line
                    };
                    lines.Add(dialog_line);
                } else {
                    dialog_line = lines.Get(lines.Size() - 1);
                    string tmp = StringUtils.Concat(dialog_line.text, buffered_line);
                    //free(dialog_line.text);
                    //free(buffered_line);
                    dialog_line.text = tmp;
                }
            }

            tokenizer.Destroy();
            //free(source);

            Dialog dialog = new Dialog() {
                full_path = full_path,
                lines = null,
                lines_size = 0
            };

            lines.Destroy2(out dialog.lines_size, ref dialog.lines);
            id = dialogs.Size();
            dialogs.Add(dialog);

            return id;
        }

        private static void InternalReadColor(XmlParserNode node, float[] rgba) {
            uint rgb;
            if (VertexProps.ParseHex(node.GetAttribute("rgb"), out rgb, false)) {
                Math2D.ColorBytesToFloats(rgb, false, rgba);
                rgba[3] = VertexProps.ParseFloat(node, "alpha", 1f);
            } else {
                rgba[0] = VertexProps.ParseFloat(node, "r", rgba[0]);
                rgba[1] = VertexProps.ParseFloat(node, "g", rgba[1]);
                rgba[2] = VertexProps.ParseFloat(node, "b", rgba[2]);
                rgba[3] = VertexProps.ParseFloat(node, "a", rgba[3]);
            }
        }

        private static Align InternalReadAlign(XmlParserNode node, string attribute) {
            string unparsed_align = node.GetAttribute(attribute);
            if (String.IsNullOrEmpty(unparsed_align) || unparsed_align == "none") {
                return Align.NONE;
            }

            Align align = VertexProps.ParseAlign2(unparsed_align);
            switch (align) {
                case Align.START:
                case Align.CENTER:
                case Align.END:
                    break;
                default:
                    Console.Error.WriteLine("[ERROR] dialogue_internal_read_align() invalid align value: " + unparsed_align);
                    align = Align.NONE;
                    break;
            }

            return align;
        }

        private static void InternalReadOffset(JSONToken json_obj, string property, out float x, out float y) {
            JSONToken json_array = JSONParser.ReadArray(json_obj, property);
            x = (float)JSONParser.ReadArrayItemNumberDouble(json_array, 0, 0.0);
            y = (float)JSONParser.ReadArrayItemNumberDouble(json_array, 1, 0.0);
        }

        private static void InternalComputeState(ModelHolder mdlhldr, StateSprite sttsprt, AnimSprite anim, float offst_x, float offst_y, bool is_speak) {
            if (anim == null) return;

            string name = is_speak ? Dialogue.SPEAK : Dialogue.IDLE;
            StateSpriteState state = sttsprt.StateAdd2(mdlhldr.GetTexture(true), anim, null, 0x00, name);

            Debug.Assert(state != null);

            state.offset_x = offst_x;
            state.offset_y = offst_y;
        }

        private static void InternalAddState(StateSprite statesprite, ModelHolder modelholder, string anim_name, string state_name, float scale, bool looped) {
            if (String.IsNullOrEmpty(anim_name)) return;

            StateSpriteState state = statesprite.StateAdd(modelholder, anim_name, state_name);
            if (state == null) return;

            if (state.animation != null && !modelholder.HasAnimlist()) {
                AnimList animlist = modelholder.GetAnimlist();
                if (animlist.GetAnimation(anim_name) == null) {
                    // the animation was builded from an atlas, explicit set the loop count
                    state.animation.SetLoop(looped ? -1 : 1);
                }
            }

            float orig_width = -1f, orig_height = -1f;

            ImgUtils.GetStateSpriteOriginalSize(state, ref orig_width, ref orig_height);
            state.draw_width = orig_width * scale;
            state.draw_height = orig_height * scale;
        }

        private static void InternalStopPortraitAnimation(Portrait portrait) {
            StateSpriteState state = portrait.statesprite.StateGet();
            if (state != null && state.animation != null) {
                state.animation.ForceEnd3(portrait.statesprite);
                state.animation.Stop();
            }
        }

        private Audio InternalGetAudio(string name) {
            if (String.IsNullOrEmpty(name) && this.audios_size > 0) {
                // random choose
                int index = Math2D.RandomInt(0, this.audios_size - 1);
                return this.audios[index];
            }

            for (int i = 0 ; i < this.audios_size ; i++) {
                if (this.audios[i].name == name) {
                    return this.audios[i];
                }
            }
            return null;
        }

        private FontHolder InternalGetFont(string name) {
            if (String.IsNullOrEmpty(name) && this.fonts_size > 0) {
                // random choose
                int index = Math2D.RandomInt(0, this.fonts_size - 1);
                return this.fonts[index].fontholder;
            }

            for (int i = 0 ; i < this.fonts_size ; i++) {
                if (this.fonts[i].name == name) {
                    return this.fonts[i].fontholder;
                }
            }
            return null;
        }

        private int InternalGetBackgroundIndex(string name) {
            if (String.IsNullOrEmpty(name) && this.backgrounds_size > 0) {
                // random choose
                return Math2D.RandomInt(0, this.backgrounds_size - 1);
            }

            for (int i = 0 ; i < this.backgrounds_size ; i++) {
                if (this.backgrounds[i].name == name) {
                    return i;
                }
            }
            return -1;
        }

        private MultipleChoice InternalGetMultiplechoice(string name) {
            if (String.IsNullOrEmpty(name) && this.multiplechoices_size > 0) {
                // random choose
                int index = Math2D.RandomInt(0, this.multiplechoices_size - 1);
                return this.multiplechoices[index];
            }

            for (int i = 0 ; i < this.multiplechoices_size ; i++) {
                if (this.multiplechoices[i].name == name) {
                    return this.multiplechoices[i];
                }
            }
            return null;
        }


        private class Audio {
            public string name;
            public bool was_playing;
            public SoundPlayer soundplayer;
        }

        private class Portrait {
            public string name;
            public StateSprite statesprite;
            public float position;
            public bool is_added;
            public bool is_removed;
            public bool is_speaking;
            public float offset_speak_x;
            public float offset_speak_y;
            public float offset_idle_x;
            public float offset_idle_y;
            public bool has_speak;
            public bool has_idle;
            public bool speak_anim_looped;
            public bool idle_anim_looped;
        }

        private class AnimsUI {
            public AnimSprite portrait_left_in;
            public AnimSprite portrait_center_in;
            public AnimSprite portrait_right_in;
            public AnimSprite portrait_left_out;
            public AnimSprite portrait_center_out;
            public AnimSprite portrait_right_out;
            public AnimSprite background_in;
            public AnimSprite background_out;
            public AnimSprite open;
            public AnimSprite close;
        }

        private class State {
            public string name;
            public string if_line;
            public bool initial;
            public Action[] actions;
            public int actions_size;

        }

        private class Action {
            public Type type;
            public RepeatAnim repeat_anim;
            public string name;
            public string random_from_prefix;
            public string click_text;
            public string click_char;
            public string lua_eval;
            public string lua_function;
            public string title;
            public float duration;
            public float volume;
            public int chars_per_second;
            public int char_delay;
            public float[] rgba;
            public float size;
            public Align align_vertical;
            public Align align_horizontal;
            public Align align_paragraph;
            public bool no_speak;
            public bool animate_remove;
            public bool enabled;
            public float offset_x;
            public float offset_y;
        }

        private class MultipleChoice {
            public Sprite icon;
            public Sprite hint;
            public string state_on_leave;
            public string title;
            public string name;
            public bool can_leave;
            public bool is_vertical;
            public int default_index;
            public float font_size;
            public Choice[] choices;
            public int choices_size;
        }

        private class Choice {
            public int dialog_id;
            public bool exit;
            public string lua_eval;
            public string lua_function;
            public string run_multiple_choice;
            public string text;
        }

        private class Dialog {
            public string full_path;
            public DialogLine[] lines;
            public int lines_size;
        }

        private class DialogLine {
            public string target_state_name;
            public string text;
        }

        private class Font {
            public string name;
            public FontHolder fontholder;
        }

        private class Background {
            public string name;
            public Sprite sprite;
        }

        private class SpeechImage {
            public string name;
            public StateSprite statesprite;
            public float offset_x;
            public float offset_y;
            public float text_x;
            public float text_y;
            public float text_width;
            public float text_height;
            public float title_x;
            public float title_y;
            public float portrait_line_x;
            public float portrait_line_y;
            public float portrait_line_width;
            public Align align_vertical;
            public Align align_horizontal;
            public bool portrait_line_is_relative;
            public bool title_is_relative;
            public bool text_is_relative;
        }


        private enum Type {
            AUDIO_PLAY,
            AUDIO_PAUSE,
            AUDIO_FADEIN,
            AUDIO_FADEOUT,
            AUDIO_STOP,
            AUDIO_VOLUME,
            BACKGROUND_SET,
            BACKGROUND_CHANGE,
            BACKGROUND_REMOVE,
            LUA,
            EXIT,
            PORTRAIT_ADD,
            PORTRAIT_REMOVE,
            PORTRAIT_REMOVEALL,
            AUDIO_UI,
            TEXT_SPEED,
            TEXT_SKIP,
            TEXT_INSTANTPRINT,
            SPEECH_BACKGROUND,
            TEXT_FONT,
            TEXT_COLOR,
            TEXT_BORDERCOLOR,
            TEXT_BORDEROFFSET,
            TEXT_PARAGRAPHSPACE,
            TEXT_SIZE,
            TEXT_ALIGN,
            RUNMULTIPLECHOICE,
            TITLE,
            NOWAIT,
            TEXT_BORDERSIZE,
            TEXT_BORDERENABLE
        }
        private enum RepeatAnim {
            ONCE, WHILESPEAKS, ALWAYS, NONE
        }

    }
}
