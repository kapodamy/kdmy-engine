using System;
using System.Diagnostics;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game;

public enum CharacterActionType {
    NONE,
    MISS,
    SING,
    EXTRA,
    IDLE
}

public class Character : IDraw, IAnimate {

    private const float ROLLBACK_SPEED = 8.0f;

    private enum CharacterAnimType {
        BASE,
        HOLD,
        ROLLBACK
    }


    private StateSprite statesprite;
    private int current_texture_id;
    private ArrayList<CharacterTextureInfo> textures;
    private int all_extras_names_size;
    private int all_directions_names_size;
    private string[] all_directions_names;
    private string[] all_extras_names;
    private CharacterState default_state;
    private ArrayList<CharacterState> states;
    private CharacterState current_state;
    private AnimSprite current_anim;
    private bool current_use_frame_rollback;
    private CharacterAnimType current_anim_type;
    private CharacterActionExtra current_action_extra;
    private CharacterActionMiss current_action_miss;
    private CharacterActionSing current_action_sing;
    private CharacterActionType current_action_type;
    private float current_expected_duration;
    private bool current_follow_hold;
    private bool current_idle_in_next_beat;
    private bool current_waiting_animation_end_and_idle;
    private bool current_waiting_animation_end;
    private bool alt_enabled;
    private bool continuous_idle;
    private BeatWatcher beatwatcher;
    private float idle_speed;
    private bool allow_speed_change;
    private Drawable drawable;
    private AnimSprite drawable_animation;
    private float draw_x;
    private float draw_y;
    private Align align_vertical;
    private Align align_horizontal;
    private Align manifest_align_vertical;
    private Align manifest_align_horizontal;
    private float reference_width;
    private float reference_height;
    private bool enable_reference_size;
    private float offset_x;
    private float offset_y;
    private bool is_left_facing;
    private bool inverted_enabled;
    private int inverted_size;
    private int[] inverted_from;
    private int[] inverted_to;
    private float character_scale;
    private int played_actions_count;
    private int commited_animations_count;
    private bool animation_freezed;
    private bool flip_x;


    public Character(CharacterManifest charactermanifest) {
        CharacterImportContext import_context = new CharacterImportContext {
            all_directions_names = new ArrayList<string>(),
            all_extras_names = new ArrayList<string>(),
            charactermanifest = charactermanifest,
            modelholder_arraylist = new ArrayList<CharacterModelInfo>(4),
            textures = new ArrayList<CharacterTextureInfo>(4),
            states = new ArrayList<CharacterState>(4)
        };

        Character.InternalGetModelholder(
            import_context.modelholder_arraylist, charactermanifest.model_character, false
        );

        this.statesprite = StateSprite.InitFromTexture(null);

        this.current_texture_id = -1;
        this.textures = import_context.textures;

        this.all_extras_names_size = -1; this.all_extras_names = null;
        this.all_directions_names_size = -1; this.all_directions_names = null;

        this.states = import_context.states;
        this.default_state = null;
        this.current_state = null;

        this.current_action_extra = null;
        this.current_action_miss = null;
        this.current_action_sing = null;

        this.current_anim_type = CharacterAnimType.BASE;
        this.current_action_type = CharacterActionType.NONE;
        this.current_anim = null;
        this.current_use_frame_rollback = false;
        this.current_follow_hold = false;
        this.current_expected_duration = 0f;
        this.current_idle_in_next_beat = false;
        this.current_waiting_animation_end_and_idle = false;
        this.current_waiting_animation_end = false;

        this.alt_enabled = false;
        this.continuous_idle = charactermanifest.continuous_idle;

        this.beatwatcher = new BeatWatcher();

        this.idle_speed = 1.0f;
        this.allow_speed_change = charactermanifest.actions_apply_chart_speed;

        this.drawable = null;
        this.drawable_animation = null;

        this.draw_x = 0f; this.draw_y = 1f;
        this.align_vertical = charactermanifest.align_vertical;
        this.align_horizontal = charactermanifest.align_horizontal;

        this.manifest_align_vertical = charactermanifest.align_vertical;
        this.manifest_align_horizontal = charactermanifest.align_horizontal;

        this.reference_width = 0f; this.reference_height = 0f;
        this.enable_reference_size = false;

        this.offset_x = charactermanifest.offset_x; this.offset_y = charactermanifest.offset_y;

        this.is_left_facing = charactermanifest.left_facing;

        this.inverted_enabled = false;

        this.inverted_size = -1;
        this.inverted_from = null;
        this.inverted_to = null;

        this.character_scale = 1.0f;
        this.played_actions_count = 0;
        this.commited_animations_count = 0;
        this.animation_freezed = false;
        this.flip_x = false;

        this.beatwatcher.Reset(true, 100f);

        if (charactermanifest.has_reference_size) {
            this.reference_width = charactermanifest.reference_width;
            this.reference_height = charactermanifest.reference_height;
        }

        this.statesprite.SetVisible(false);
        this.statesprite.FlipRenderedTextureEnableCorrection(false);

        // import default actions
        this.default_state = Character.InternalImportActions(import_context, charactermanifest.actions, null, null);
        this.current_state = this.default_state;

        // import additional actions as states
        for (int i = 0 ; i < charactermanifest.additional_states_size ; i++) {
            CharacterManifest.AdditionalState additionalstate = charactermanifest.additional_states[i];
            CharacterState state = Character.InternalImportActions(import_context, additionalstate.actions, additionalstate.model, additionalstate.name);
        }

        this.drawable = new Drawable(0, this, this);

        import_context.modelholder_arraylist.Destroy3(Character.InternalDestroyModelholder);
        import_context.all_directions_names.Destroy2(out this.all_directions_names_size, ref this.all_directions_names);
        import_context.all_extras_names.Destroy2(out this.all_extras_names_size, ref this.all_extras_names);

        this.inverted_size = charactermanifest.opposite_directions.sizes;
        if (this.inverted_size > 0 && charactermanifest.actions.sing_size > 0) {
            InternalImportOppositeDir(
                 ref this.inverted_from, charactermanifest.opposite_directions.from
             );
            InternalImportOppositeDir(
                ref this.inverted_to, charactermanifest.opposite_directions.to
            );
        }

        StateToggle(null);
        PlayIdle();
        //Character.face_as_opponent(this.is_left_facing);

    }

    public void Destroy() {
        Luascript.DropShared(this);

        this.statesprite.Destroy();

        int states_size = this.states.Size();

        for (int i = 0 ; i < states_size ; i++) {
            CharacterState state = this.states.Get(i);

            for (int j = 0 ; j < state.sing_size ; j++) {
                if (state.sing[j].@base != null) state.sing[j].@base.Destroy();
                if (state.sing[j].hold != null) state.sing[j].hold.Destroy();
                if (state.sing[j].rollback != null) state.sing[j].rollback.Destroy();

                if (state.sing_alt[j].@base != null) state.sing_alt[j].@base.Destroy();
                if (state.sing_alt[j].hold != null) state.sing_alt[j].hold.Destroy();
                if (state.sing_alt[j].rollback != null) state.sing_alt[j].rollback.Destroy();
            }

            for (int j = 0 ; j < state.miss_size ; j++) {
                if (state.miss[j].animation != null) state.miss[j].animation.Destroy();
            }

            for (int j = 0 ; j < state.extras_size ; j++) {
                if (state.extras[j].@base != null) state.extras[j].@base.Destroy();
                if (state.extras[j].hold != null) state.extras[j].hold.Destroy();
                if (state.extras[j].rollback != null) state.extras[j].rollback.Destroy();
            }

            if (state.hey.@base != null) state.hey.@base.Destroy();
            if (state.hey.hold != null) state.hey.hold.Destroy();
            if (state.hey.rollback != null) state.hey.rollback.Destroy();

            if (state.idle.@base != null) state.idle.@base.Destroy();
            if (state.idle.hold != null) state.idle.hold.Destroy();
            if (state.idle.rollback != null) state.idle.rollback.Destroy();

            if (state.idle_alt.@base != null) state.idle_alt.@base.Destroy();
            if (state.idle_alt.hold != null) state.idle_alt.hold.Destroy();
            if (state.idle_alt.rollback != null) state.idle_alt.rollback.Destroy();

            //free(state.name);
            //free(state.sing);
            //free(state.sing_alt);
            //free(state.miss);
            //free(state.extras);
            //free(state);
        }

        this.states.Destroy(false);

        //for (int i = 0 ; i < this.all_directions_names_size ; i++) {
        //    free(this.all_directions_names[i]);
        //}
        //for (int i = 0 ; i < this.all_extras_names_size ; i++) {
        //    free(this.all_extras_names[i]);
        //}

        this.drawable.Destroy();
        if (this.drawable_animation != null) this.drawable_animation.Destroy();

        //if (this.inverted_size > 0) {
        //    free(this.inverted_from);
        //    free(this.inverted_to);
        //}

        //free(this.all_directions_names);
        //free(this.all_extras_names);

        foreach (CharacterTextureInfo texture_info in this.textures)
            texture_info.texture.Destroy();
        this.textures.Destroy(false);

        //free(this);
    }


    public void UseAlternateSingAnimations(bool enable) {
        this.alt_enabled = enable;
    }

    public void SetDrawLocation(float x, float y) {
        Modifier modifier = this.drawable.GetModifier();
        if (!Single.IsNaN(x)) this.draw_x = modifier.x = x;
        if (!Single.IsNaN(y)) this.draw_y = modifier.y = y;
        InternalCalculateLocation();
    }

    public void SetDrawAlign(Align align_vertical, Align align_horizontal) {
        if (align_vertical == Align.BOTH || align_vertical == Align.NONE)
            this.align_vertical = this.manifest_align_vertical;
        else
            this.align_vertical = align_vertical;

        if (align_horizontal == Align.BOTH || align_horizontal == Align.NONE)
            this.align_horizontal = this.manifest_align_horizontal;
        else
            this.align_horizontal = align_horizontal;

        InternalCalculateLocation();
    }

    public void UpdateReferenceSize(float width, float height) {
        if (width >= 0f) this.reference_width = width;
        if (height >= 0f) this.reference_height = height;
        InternalCalculateLocation();
    }

    public void EnableReferenceSize(bool enable) {
        this.enable_reference_size = enable;
        InternalCalculateLocation();
    }

    public void SetOffset(float offset_x, float offset_y) {
        this.offset_x = offset_x;
        this.offset_y = offset_y;
    }


    public bool StateAdd(ModelHolder modelholder, string state_name) {
        //
        // expensive operation, call it before any gameplay
        //
        if (modelholder == null) return false;

        int states_size = this.states.Size();
        for (int i = 0 ; i < states_size ; i++) {
            CharacterState existing_state = this.states.Get(i);
            if (existing_state.name == state_name) return false;
        }

        int id_texture = InternalAddTexture(this.textures, modelholder);
        CharacterState default_state = this.default_state;

        CharacterState state = Character.InternalStateCreate(
            state_name, default_state.sing_size, default_state.miss_size, default_state.extras_size
        );

        for (int i = 0 ; i < default_state.sing_size ; i++) {
            state.sing[i].id_texture = id_texture;
            Character.InternalStateOfSing(state.sing[i], modelholder, state_name, default_state.sing[i]);

            state.sing_alt[i].id_texture = id_texture;
            Character.InternalStateOfSing(state.sing_alt[i], modelholder, state_name, default_state.sing_alt[i]);
        }

        for (int i = 0 ; i < default_state.miss_size ; i++) {
            AnimSprite animation = Character.InternalImportAnimation3(modelholder, state_name, default_state.miss[i].animation, false);

            state.miss[i].id_direction = default_state.miss[i].id_direction;
            state.miss[i].stop_after_beats = default_state.miss[i].stop_after_beats;
            state.miss[i].animation = animation;
            state.miss[i].id_texture = id_texture;
            state.miss[i].offset_x = default_state.miss[i].offset_x;
            state.miss[i].offset_y = default_state.miss[i].offset_y;
        }

        for (int i = 0 ; i < default_state.extras_size ; i++) {
            state.extras[i].id_texture = id_texture;
            Character.InternalStateOfExtra(state.extras[i], modelholder, state_name, default_state.extras[i]);
        }

        state.hey.id_texture = id_texture;
        Character.InternalStateOfExtra(state.hey, modelholder, state_name, default_state.hey);

        state.idle.id_texture = id_texture;
        Character.InternalStateOfExtra(state.idle, modelholder, state_name, default_state.idle);

        this.states.Add(state);
        return true;
    }

    public bool StateToggle(string state_name) {
        int states_size = this.states.Size();

        for (int i = 0 ; i < states_size ; i++) {
            CharacterState state = this.states.Get(i);
            if (state.name == state_name) {
                this.current_state = state;
                InternalUpdateTexture();
                return true;
            }
        }

        return false;
    }


    public bool PlayHey() {
        Debug.Assert(this.current_state != null, "this.current_state was NULL");

        CharacterActionExtra extra_info = this.current_state.hey;

        if (!extra_info.is_valid && this.current_state != this.default_state) {
            // attempt use default state
            extra_info = this.default_state.hey;
        }

        if (!extra_info.is_valid) {
            InternalFallbackIdle();
            return false;
        }

        // end current action
        InternalEndCurrentAction();

        if (extra_info.stop_after_beats <= 0f) {
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_waiting_animation_end = true;
            this.current_waiting_animation_end_and_idle = extra_info.stop_after_beats == 0f;
        } else {
            this.current_expected_duration = extra_info.stop_after_beats * this.beatwatcher.tick;
            this.current_waiting_animation_end = false;
            this.current_waiting_animation_end_and_idle = false;
        }

        if (extra_info.@base != null) {
            this.current_anim = extra_info.@base;
            this.current_anim_type = CharacterAnimType.BASE;
            this.current_follow_hold = extra_info.hold != null;
        } else {
            this.current_anim = extra_info.hold;
            this.current_anim_type = CharacterAnimType.HOLD;
            this.current_follow_hold = false;
        }

        this.current_anim.Restart();

        this.current_action_extra = extra_info;
        this.current_action_type = CharacterActionType.EXTRA;

        this.current_use_frame_rollback = false;
        this.current_idle_in_next_beat = false;

        InternalUpdateTexture();
        InternalCalculateLocation();

        this.played_actions_count++;
        this.commited_animations_count++;

        return true;
    }

    public int PlayIdle() {
        Debug.Assert(this.current_state != null, "this.current_state was NULL");

        this.played_actions_count++;

        CharacterActionType action_type = this.current_action_type;
        bool rollback_active = this.current_anim_type == CharacterAnimType.ROLLBACK;
        if (action_type != CharacterActionType.NONE && action_type != CharacterActionType.IDLE && !rollback_active) {

            // rollback the current action (if possible)
            int ret = 0;
            switch (this.current_action_type) {
                case CharacterActionType.SING:
                    ret = InternalAnimateSing(true, true);
                    break;
                case CharacterActionType.MISS:
                    ret = InternalAnimateMiss(true, true);
                    break;
                case CharacterActionType.EXTRA:
                    ret = InternalAnimateExtra(true, true);
                    break;
                default:
                    this.current_idle_in_next_beat = true;
                    this.current_action_type = CharacterActionType.NONE;
                    break;
            }

            // do nothing if rollback is active or idle was previously used
            if (!this.current_idle_in_next_beat || this.current_action_type == CharacterActionType.IDLE) {
                return ret;
            }
        }

        CharacterState state = this.current_state;
        CharacterActionExtra extra_info = null;

L_read_state:
        extra_info = state.idle;
        if (this.alt_enabled && state.idle_alt.is_valid) extra_info = state.idle_alt;

        if (!extra_info.is_valid) {
            if (state != this.default_state) {
                // attempt use default state
                state = this.default_state;
                goto L_read_state;
            }
            return 0;
        }

        // end current action
        InternalEndCurrentAction();

        if (extra_info.@base != null) {
            this.current_anim = extra_info.@base;
            this.current_anim_type = CharacterAnimType.BASE;
            this.current_follow_hold = extra_info.hold != null;
        } else {
            this.current_anim = extra_info.hold;
            this.current_anim_type = CharacterAnimType.HOLD;
            this.current_follow_hold = false;
        }

        this.current_anim.Restart();

        this.current_action_extra = extra_info;
        this.current_action_type = CharacterActionType.IDLE;

        this.current_expected_duration = 0f;
        this.current_use_frame_rollback = false;
        this.current_idle_in_next_beat = false;
        this.current_waiting_animation_end = false;

        InternalUpdateTexture();
        InternalCalculateLocation();

        this.commited_animations_count++;

        return 1;
    }

    public bool PlaySing(string direction, bool prefer_sustain) {
        Debug.Assert(this.current_state != null, "this.current_state was NULL");

        int id_direction = InternalGetDirectionId(direction);
        if (id_direction < 0) {
            // unknown direction
            InternalFallbackIdle();
            return false;
        };

        CharacterActionSing sing_info = null;
        CharacterState state = this.current_state;

L_read_state:
        CharacterActionSing[] array = this.alt_enabled ? state.sing_alt : state.sing;
        for (int i = 0 ; i < state.sing_size ; i++) {
            if (array[i].id_direction == id_direction) {
                sing_info = array[i];
                break;
            }
        }

        if (sing_info == null || (sing_info.@base == null && sing_info.hold == null)) {
            // attempt to use the non-alt sing direction
            if (this.alt_enabled) {
                for (int i = 0 ; i < state.sing_size ; i++) {
                    if (state.sing[i].id_direction == id_direction) {
                        sing_info = state.sing[i];
                        break;
                    }
                }
            }

            if (sing_info == null || (sing_info.@base == null && sing_info.hold == null)) {
                if (state != this.default_state) {
                    // attempt use default state
                    state = this.default_state;
                    goto L_read_state;
                }
                //throw new Exception("unknown sing direction: " + direction);
                InternalFallbackIdle();
                return false;
            }
        }

        // end current action
        InternalEndCurrentAction();

        if (prefer_sustain) {
            // ignore "stopAfterBeats"
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_waiting_animation_end = false;
            this.current_waiting_animation_end_and_idle = false;

            if (sing_info.full_sustain) {
                if (sing_info.@base != null) {
                    this.current_anim = sing_info.@base;
                    this.current_anim_type = CharacterAnimType.BASE;
                    this.current_follow_hold = sing_info.hold != null;
                } else {
                    this.current_anim = sing_info.hold;
                    this.current_anim_type = CharacterAnimType.HOLD;
                    this.current_follow_hold = false;
                }
            } else {
                this.current_anim = sing_info.hold ?? sing_info.@base;
                this.current_anim_type = sing_info.hold != null ? CharacterAnimType.HOLD : CharacterAnimType.BASE;
                this.current_follow_hold = false;
            }
        } else {
            this.current_anim = sing_info.@base ?? sing_info.hold;
            this.current_anim_type = sing_info.@base != null ? CharacterAnimType.BASE : CharacterAnimType.HOLD;
            this.current_follow_hold = sing_info.follow_hold && sing_info.@base != null && sing_info.hold != null;

            // ignore "stopAfterBeats" if negative (waits for animation completion)
            if (sing_info.stop_after_beats <= 0f) {
                this.current_waiting_animation_end = true;
                this.current_expected_duration = Single.PositiveInfinity;
                this.current_waiting_animation_end_and_idle = sing_info.stop_after_beats == 0f;
            } else {
                this.current_waiting_animation_end = false;
                this.current_expected_duration = sing_info.stop_after_beats * this.beatwatcher.tick;
                this.current_waiting_animation_end_and_idle = false;
            }
        }

        this.current_anim.Restart();

        this.current_action_sing = sing_info;
        this.current_action_type = CharacterActionType.SING;

        this.current_use_frame_rollback = false;
        this.current_idle_in_next_beat = false;

        InternalUpdateTexture();
        InternalCalculateLocation();

        this.played_actions_count++;
        this.commited_animations_count++;

        return true;
    }

    public bool PlayMiss(string direction, bool keep_in_hold) {
        Debug.Assert(this.current_state != null, "this.current_state was NULL");

        int id_direction = InternalGetDirectionId(direction);
        if (id_direction < 0) {
            // unknown direction
            InternalFallbackIdle();
            return false;
        };

        CharacterActionMiss miss_info = null;
        CharacterState state = this.current_state;

L_read_state:
        for (int i = 0 ; i < this.default_state.miss_size ; i++) {
            if (state.miss[i].id_direction == id_direction) {
                miss_info = state.miss[i];
                break;
            }
        }

        if (miss_info == null || miss_info.animation == null) {
            if (state != this.default_state) {
                // attempt use default state
                state = this.default_state;
                goto L_read_state;
            }
            InternalFallbackIdle();
            return false;
        }

        // end current action
        InternalEndCurrentAction();

        if (keep_in_hold) {
            // ignore "stopAfterBeats"
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_waiting_animation_end = false;
            this.current_waiting_animation_end_and_idle = false;
        } else if (miss_info.stop_after_beats <= 0f) {
            // wait for animation completion
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_waiting_animation_end = true;
            this.current_waiting_animation_end_and_idle = miss_info.stop_after_beats == 0f;
        } else {
            this.current_expected_duration = miss_info.stop_after_beats * this.beatwatcher.tick;
            this.current_waiting_animation_end = false;
            this.current_waiting_animation_end_and_idle = false;
        }

        miss_info.animation.Restart();

        this.current_anim = miss_info.animation;
        this.current_action_type = CharacterActionType.MISS;
        this.current_action_miss = miss_info;

        this.current_follow_hold = false;
        this.current_use_frame_rollback = false;
        this.current_idle_in_next_beat = false;

        InternalUpdateTexture();
        InternalCalculateLocation();

        this.played_actions_count++;
        this.commited_animations_count++;

        return true;
    }

    public bool PlayExtra(string extra_animation_name, bool prefer_sustain) {
        Debug.Assert(this.current_state != null, "this.current_state was NULL");

        int id_extra = InternalGetExtraId(extra_animation_name);
        if (id_extra < 0) {
            // unknown extra
            return false;
        }

        CharacterActionExtra extra_info = null;
        CharacterState state = this.current_state;

L_read_state:
        for (int i = 0 ; i < state.extras_size ; i++) {
            if (state.extras[i].id_extra == id_extra) {
                extra_info = state.extras[i];
                break;
            }
        }

        if (extra_info == null || !extra_info.is_valid) {
            if (state != this.default_state) {
                // attempt use default state
                state = this.default_state;
                goto L_read_state;
            }
            //InternalFallbackIdle();
            return false;
        }

        // end current action
        InternalEndCurrentAction();

        if (prefer_sustain) {
            // ignore "stopAfterBeats"
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_waiting_animation_end = false;
            this.current_waiting_animation_end_and_idle = false;
        } else if (extra_info.stop_after_beats <= 0f) {
            // wait for animation completion
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_waiting_animation_end = true;
            this.current_waiting_animation_end_and_idle = extra_info.stop_after_beats == 0f;
        } else {
            this.current_expected_duration = extra_info.stop_after_beats * this.beatwatcher.tick;
            this.current_waiting_animation_end = false;
            this.current_waiting_animation_end_and_idle = false;
        }

        if (prefer_sustain) {
            this.current_anim = extra_info.hold ?? extra_info.@base;
            this.current_anim_type = extra_info.hold != null ? CharacterAnimType.HOLD : CharacterAnimType.BASE;
            this.current_follow_hold = false;
        } else {
            this.current_anim = extra_info.@base ?? extra_info.hold;
            this.current_anim_type = extra_info.@base != null ? CharacterAnimType.BASE : CharacterAnimType.HOLD;
            this.current_follow_hold = extra_info.@base != null && extra_info.hold != null;
        }

        this.current_anim.Restart();

        this.current_action_extra = extra_info;
        this.current_action_type = CharacterActionType.EXTRA;

        this.current_use_frame_rollback = false;
        this.current_idle_in_next_beat = false;

        InternalUpdateTexture();
        InternalCalculateLocation();

        this.played_actions_count++;
        this.commited_animations_count++;

        return true;
    }

    public void ScheduleIdle() {
        this.played_actions_count++;
        this.commited_animations_count++;

        this.current_expected_duration = 0f;
        this.current_idle_in_next_beat = true;
    }


    public void SetBpm(float beats_per_minute) {
        this.beatwatcher.ChangeBpm(beats_per_minute);
    }

    public void SetIdleSpeed(float speed) {
        if (this.allow_speed_change) this.idle_speed = speed;
    }

    public void SetScale(float scale_factor) {
        this.character_scale = scale_factor;
        InternalUpdateTexture();
    }

    public void Reset() {
        this.beatwatcher.Reset(true, 100);

        this.idle_speed = 1.0f;
        this.alt_enabled = false;

        this.drawable.SetAntialiasing(PVRFlag.DEFAULT);

        Modifier modifier = this.drawable.GetModifier();
        modifier.Clear();
        modifier.x = this.draw_x;
        modifier.y = this.draw_y;

        // switch to the default state
        StateToggle(null);

        this.current_action_type = CharacterActionType.NONE;
        PlayIdle();
    }

    public void EnableContinuousIdle(bool enable) {
        this.continuous_idle = enable;
    }

    public bool IsIdleActive() {
        return this.current_action_type == CharacterActionType.IDLE;
    }

    public void EnableFlipCorrection(bool enable) {
        this.statesprite.FlipRenderedTextureEnableCorrection(enable);
    }

    public void FlipOrientation(bool enable) {
        this.flip_x = enable;
        this.inverted_enabled = this.inverted_size > 0 && enable;
        this.statesprite.FlipTexture(enable, null);
    }

    public void FaceAsOpponent(bool face_as_opponent) {
        if (face_as_opponent)
            this.flip_x = this.is_left_facing;
        else
            this.flip_x = !this.is_left_facing;

        this.inverted_enabled = this.inverted_size > 0 && flip_x;
        this.statesprite.FlipTexture(this.flip_x, null);
    }



    public int Animate(float elapsed) {
        bool has_beat = this.beatwatcher.Poll();

        if (this.drawable_animation != null) {
            this.drawable_animation.Animate(elapsed);
            this.drawable_animation.UpdateDrawable(this.drawable, true);
        }

        if (this.animation_freezed) {
            if (this.current_use_frame_rollback)
                this.current_anim.Rollback(0f);
            else
                this.current_anim.Animate(0f);

            this.current_anim.UpdateStatesprite(this.statesprite, true);
            InternalCalculateLocation();
            return 1;
        }

        this.statesprite.Animate(elapsed);

        if (this.current_action_type == CharacterActionType.NONE) return 1;

        bool completed;
        float orig_elapsed = elapsed;

        if (this.current_action_type == CharacterActionType.IDLE && this.idle_speed != 1.0) {
            elapsed *= this.idle_speed;
        }

        if (this.current_use_frame_rollback)
            completed = this.current_anim.Rollback(elapsed * Character.ROLLBACK_SPEED);
        else
            completed = this.current_anim.Animate(elapsed) > 0;

        this.current_anim.UpdateStatesprite(this.statesprite, true);
        InternalCalculateLocation();

        bool should_stop = this.current_expected_duration <= 0f;
        this.current_expected_duration -= orig_elapsed;

        if (this.current_idle_in_next_beat) {
            if (has_beat && should_stop) {
                this.current_action_type = CharacterActionType.NONE;
                return PlayIdle();
            }
            return 1;
        }

        if (this.current_anim_type == CharacterAnimType.ROLLBACK) {
            if (completed) {
                this.current_action_type = CharacterActionType.NONE;
                return PlayIdle();
            }
            return 0;
        }

        switch (this.current_action_type) {
            case CharacterActionType.SING:
                return InternalAnimateSing(completed, should_stop);
            case CharacterActionType.MISS:
                return InternalAnimateMiss(completed, should_stop);
            case CharacterActionType.IDLE:
                return InternalAnimateIdle(completed, has_beat);
            case CharacterActionType.EXTRA:
                return InternalAnimateExtra(completed, should_stop);
        }

        return 0;
    }

    public void Draw(PVRContext pvrctx) {
        // TODO: direction ghosting

        pvrctx.Save();
        this.drawable.HelperApplyInContext(pvrctx);
        this.statesprite.Draw(pvrctx);
        pvrctx.Restore();
    }


    public void SetZIndex(float z) {
        this.drawable.SetZIndex(z);
    }

    public void SetZOffset(float z) {
        this.drawable.SetZOffset(z);
    }

    public void AnimationSet(AnimSprite animsprite) {
        this.drawable_animation.Destroy();
        this.drawable_animation = animsprite != null ? animsprite.Clone() : null;
    }

    public void AnimationRestart() {
        if (this.drawable_animation != null) this.drawable_animation.Restart();
    }

    public void AnimationEnd() {
        if (this.drawable_animation != null) {
            this.drawable_animation.ForceEnd();
            this.drawable_animation.UpdateDrawable(this.drawable, true);
        }
    }

    public void SetColorOffset(float r, float g, float b, float a) {
        this.drawable.SetOffsetColor(r, g, b, a);
    }

    public void SetColorOffsetToDefault() {
        this.drawable.SetOffsetColorToDefault();
    }

    public void SetAlpha(float alpha) {
        this.drawable.SetAlpha(alpha);
    }

    public void SetVisible(bool visible) {
        this.drawable.SetVisible(visible);
    }

    public Modifier GetModifier() {
        return this.drawable.GetModifier();
    }

    public Drawable GetDrawable() {
        return this.drawable;
    }

    public int GetPlayCalls() {
        return this.played_actions_count;
    }

    public int GetCommitedAnimationsCount() {
        return this.commited_animations_count;
    }

    public CharacterActionType GetCurrentAction() {
        return this.current_action_type;
    }

    public bool HasDirection(string name, bool is_extra) {
        string[] array = is_extra ? this.all_extras_names : this.all_directions_names;
        int size = is_extra ? this.all_extras_names_size : this.all_directions_names_size;

        for (int i = 0 ; i < size ; i++) {
            if (array[i] == name) return true;
        }

        return false;
    }

    public void FreezeAnimation(bool enabled) {
        this.animation_freezed = enabled;
    }

    public void TrailingEnabled(bool enabled) {
        this.statesprite.TrailingEnabled(enabled);
    }

    public void TrailingSetParams(int length, float trail_delay, float trail_alpha, bool? darken_colors) {
        this.statesprite.TrailingSetParams(length, trail_delay, trail_alpha, darken_colors);
    }

    public void TrailingSetOffsetcolor(float r, float g, float b) {
        this.statesprite.TrailingSetOffsetcolor(r, g, b);
    }


    private static void InternalImportSing(CharacterActionSing sing_info, ModelHolder modelholder, CharacterManifest.Sing sing_entry, int id_direction, string prefix, string suffix) {

        sing_info.@base = Character.InternalImportAnimation(modelholder, sing_entry.anim, prefix, suffix, false);
        sing_info.hold = Character.InternalImportAnimation(modelholder, sing_entry.anim_hold, prefix, suffix, true);
        sing_info.rollback = Character.InternalImportAnimation(modelholder, sing_entry.anim_rollback, prefix, suffix, false);

        sing_info.id_direction = id_direction;
        sing_info.follow_hold = sing_entry.follow_hold;
        sing_info.full_sustain = sing_entry.full_sustain;
        sing_info.stop_after_beats = sing_entry.stop_after_beats;
        sing_info.offset_x = sing_entry.offset_x;
        sing_info.offset_y = sing_entry.offset_y;

        sing_info.base_can_rollback = false;
        sing_info.hold_can_rollback = false;

        if (sing_info.rollback == null && sing_entry.rollback) {
            if (sing_info.@base != null && sing_info.@base.IsFrameAnimation()) {
                sing_info.base_can_rollback = true;
            }
            if (sing_info.hold != null && sing_info.hold.IsFrameAnimation()) {
                sing_info.hold_can_rollback = true;
            }
        }

    }

    private static void InternalImportMiss(CharacterActionMiss miss_info, ModelHolder modelholder, CharacterManifest.Miss miss_entry, int id_direction) {
        miss_info.animation = Character.InternalImportAnimation(modelholder, miss_entry.anim, null, null, false);

        miss_info.id_direction = id_direction;
        miss_info.stop_after_beats = miss_entry.stop_after_beats;
        miss_info.offset_x = miss_entry.offset_x;
        miss_info.offset_y = miss_entry.offset_y;
    }

    private static void InternalImportExtra(CharacterActionExtra extra_info, ArrayList<CharacterModelInfo> mdlhldr_rrlst, ArrayList<CharacterTextureInfo> txtr_rrlst, CharacterManifest.Extra extra_entry, int id_extra, string prefix, string suffix, string model_src) {
        if (extra_entry == null) {
            extra_info.id_extra = -1;
            extra_info.is_valid = false;
            extra_info.@base = null;
            extra_info.hold = null;
            extra_info.rollback = null;
            extra_info.offset_x = 0;
            extra_info.offset_y = 0;
            return;
        }

        ModelHolder modelholder = Character.InternalGetModelholder(
            mdlhldr_rrlst, extra_entry.model_src ?? model_src, true
        );
        extra_info.id_texture = Character.InternalAddTexture(
            txtr_rrlst, modelholder
        );

        extra_info.stop_after_beats = extra_entry.stop_after_beats;

        // this originally was:
        //          extra_entry.ANIM_NAME != null && extra_entry.ANIM_NAME.Length < 1
        // instead of
        //          StringUtils.IsEmpty(extra_entry.ANIM_NAME)
        // revert if something breaks
        //

        if (StringUtils.IsEmpty(extra_entry.anim))
            extra_info.@base = null;
        else
            extra_info.@base = Character.InternalImportAnimation(modelholder, extra_entry.anim, prefix, suffix, false);

        if (StringUtils.IsEmpty(extra_entry.anim_hold)) {
            extra_info.hold = null;
        } else {
            extra_info.hold = Character.InternalImportAnimation(modelholder, extra_entry.anim_hold, prefix, suffix, true);
        }

        if (StringUtils.IsEmpty(extra_entry.anim_rollback)) {
            extra_info.rollback = null;
        } else {
            extra_info.rollback = Character.InternalImportAnimation(modelholder, extra_entry.anim_rollback, prefix, suffix, false);
        }

        extra_info.id_extra = id_extra;
        extra_info.is_valid = extra_info.@base != null || extra_info.hold != null;
        extra_info.offset_x = extra_entry.offset_x;
        extra_info.offset_y = extra_entry.offset_y;
    }



    private static AnimSprite InternalImportAnimation(ModelHolder mdlhldr, string anim_name, string prefix, string suffix, bool is_sustain) {
        if (StringUtils.IsEmpty(anim_name)) return null;

        anim_name = StringUtils.Concat(prefix, anim_name, suffix);
        AnimSprite animsprite = InternalImportAnimation2(mdlhldr, anim_name, is_sustain);

        //if (StringUtils.IsNotEmpty(anim_name)) free(anim_name);

        return animsprite;
    }

    private static AnimSprite InternalImportAnimation2(ModelHolder modelholder, string name, bool is_sustain) {
        AnimSprite animsprite;

        // read from the animation list
        animsprite = AnimSprite.InitFromAnimlist(modelholder.GetAnimlist(), name);
        if (animsprite != null) return animsprite;

        // animation not found, build from atlas
        Atlas atlas = modelholder.GetAtlas();
        float fps = atlas.GetGlyphFPS();
        if (fps <= 0) fps = Funkin.DEFAULT_ANIMATIONS_FRAMERATE;

        int loop_count = is_sustain ? 0 /*infinite*/ : 1/*once*/;
        animsprite = AnimSprite.InitFromAtlas(fps, loop_count, atlas, name, true);
        if (animsprite != null) return animsprite;

        // ¿static animation?
        AtlasEntry atlas_entry = atlas.GetEntry(name);
        if (atlas_entry == null) return null;

        return AnimSprite.InitFromAtlasEntry(atlas_entry, is_sustain, fps);
    }

    private static AnimSprite InternalImportAnimation3(ModelHolder modelholder, string state_name, AnimSprite animation, bool is_sustain) {
        if (animation == null) return null;

        string name = animation.GetName();
        string anim_name = StringUtils.ConcatForStateName(name, state_name);

        AnimSprite anim = InternalImportAnimation2(modelholder, anim_name, is_sustain);
        //free(anim_name);

        return anim;
    }




    private static CharacterState InternalStateCreate(string name, int sing_size, int miss_size, int extras_size) {
        CharacterState state = new CharacterState() {
            name = name,
            sing_size = sing_size,
            miss_size = miss_size,
            extras_size = extras_size,
            sing = new CharacterActionSing[sing_size],
            sing_alt = new CharacterActionSing[sing_size],
            miss = new CharacterActionMiss[miss_size],
            extras = new CharacterActionExtra[extras_size],
            hey = new CharacterActionExtra() {
                @base = null,
                hold = null,
                rollback = null,
                stop_after_beats = 0,
                id_extra = -1,
                id_texture = -1,
                is_valid = false,
                offset_x = 0f,
                offset_y = 0f
            },
            idle = new CharacterActionExtra() {
                @base = null,
                hold = null,
                rollback = null,
                stop_after_beats = 0,
                id_extra = -1,
                id_texture = -1,
                is_valid = false,
                offset_x = 0f,
                offset_y = 0f
            },
            idle_alt = new CharacterActionExtra() {
                @base = null,
                hold = null,
                rollback = null,
                stop_after_beats = 0,
                id_extra = -1,
                id_texture = -1,
                is_valid = false,
                offset_x = 0f,
                offset_y = 0f
            }
        };

        for (int i = 0 ; i < state.sing_size ; i++) state.sing[i] = new CharacterActionSing();
        for (int i = 0 ; i < state.sing_size ; i++) state.sing_alt[i] = new CharacterActionSing();
        for (int i = 0 ; i < state.miss_size ; i++) state.miss[i] = new CharacterActionMiss();
        for (int i = 0 ; i < state.extras_size ; i++) state.extras[i] = new CharacterActionExtra();

        return state;
    }

    private static void InternalStateOfSing(CharacterActionSing new_singinfo, ModelHolder modelholder, string state_name, CharacterActionSing sing_info) {
        new_singinfo.@base = InternalImportAnimation3(
            modelholder, state_name, sing_info.@base, false
        );
        new_singinfo.hold = InternalImportAnimation3(
            modelholder, state_name, sing_info.hold, true
        );
        new_singinfo.rollback = InternalImportAnimation3(
            modelholder, state_name, sing_info.rollback, false
        );

        new_singinfo.id_direction = sing_info.id_direction;
        new_singinfo.follow_hold = sing_info.follow_hold;
        new_singinfo.full_sustain = sing_info.full_sustain;
        new_singinfo.stop_after_beats = sing_info.stop_after_beats;
        new_singinfo.offset_x = sing_info.offset_x;
        new_singinfo.offset_y = sing_info.offset_y;
        new_singinfo.base_can_rollback = false;
        new_singinfo.hold_can_rollback = false;

        if (sing_info.rollback == null) {
            new_singinfo.base_can_rollback = new_singinfo.@base != null && new_singinfo.@base.IsFrameAnimation();
            new_singinfo.hold_can_rollback = new_singinfo.hold != null && new_singinfo.hold.IsFrameAnimation();
        }

    }

    private static void InternalStateOfExtra(CharacterActionExtra new_extrainfo, ModelHolder modelholder, string state_name, CharacterActionExtra extra_info) {
        new_extrainfo.@base = InternalImportAnimation3(
            modelholder, state_name, extra_info.@base, false
        );
        new_extrainfo.hold = InternalImportAnimation3(
            modelholder, state_name, extra_info.hold, true
        );
        new_extrainfo.rollback = InternalImportAnimation3(
            modelholder, state_name, extra_info.rollback, false
        );

        new_extrainfo.id_extra = extra_info.id_extra;
        new_extrainfo.stop_after_beats = extra_info.stop_after_beats;
        new_extrainfo.offset_x = extra_info.offset_x;
        new_extrainfo.offset_y = extra_info.offset_y;
        new_extrainfo.is_valid = new_extrainfo.@base != null && new_extrainfo.hold != null;
    }



    private int InternalGetDirectionId(string direction) {
        for (int i = 0 ; i < this.all_directions_names_size ; i++) {
            if (this.all_directions_names[i] != direction) continue;

            if (this.inverted_enabled) {
                for (int j = 0 ; j < this.inverted_size ; j++) {
                    if (this.inverted_from[j] == i) {
                        if (this.inverted_to[j] < 0)
                            break;// no inverse direction
                        else
                            return this.inverted_to[j];
                    }
                }
            }

            return i;
        }
        // unknown direction
        return -1;
    }

    private int InternalGetExtraId(string direction) {
        for (int i = 0 ; i < this.all_extras_names_size ; i++) {
            if (this.all_extras_names[i] == direction) return i;
        }
        return -1;
    }

    private static int InternalIndexName(ArrayList<string> arraylist, string name, bool add_if_not_found) {
        if (name == null) return -1;

        string[] array = arraylist.PeekArray();
        int size = arraylist.Size();

        for (int i = 0 ; i < size ; i++) {
            if (array[i] == name) return i;
        }

        if (!add_if_not_found) return -1;

        arraylist.Add(name);
        return size;
    }

    private void InternalUpdateTexture() {
        int id_texture = -1;
        switch (this.current_action_type) {
            case CharacterActionType.EXTRA:
            case CharacterActionType.IDLE:
                id_texture = this.current_action_extra.id_texture;
                break;
            case CharacterActionType.MISS:
                id_texture = this.current_action_miss.id_texture;
                break;
            case CharacterActionType.SING:
                id_texture = this.current_action_sing.id_texture;
                break;
        }

        float final_scale = this.character_scale;

        if (id_texture >= 0 && id_texture != this.current_texture_id) {

            CharacterTextureInfo texture_info = this.textures.Get(id_texture);
            this.current_texture_id = id_texture;

            //
            // (Does not work) 
            // Calculate the scale required to fit the sprite in the layout and/or screen
            // and later apply this scale to the character scale.
            //
            // As long as the viewport size of the layout is intended for a 1280x720 screen, there 
            // will be no problems.
            //
            //
            //let scale_factor = this.layout_height / texture_info.resolution_height;
            //final_scale *= scale_factor;

            this.statesprite.SetTexture(texture_info.texture, true);
        }

        this.statesprite.ChangeDrawSizeInAtlasApply(true, final_scale);
        if (this.current_anim != null) this.current_anim.UpdateStatesprite(this.statesprite, false);
    }

    private void InternalImportOppositeDir(ref int[] array, string[] ltr_array) {
        array = new int[this.inverted_size];

        for (int i = 0 ; i < this.inverted_size ; i++) {
            array[i] = -1;
            if (ltr_array[i] == null) continue;

            for (int j = 0 ; j < this.all_directions_names_size ; j++) {
                if (this.all_directions_names[j] == ltr_array[i]) {
                    array[i] = j;
                    break;
                }
            }

            if (array[i] < 0)
                Logger.Error($"character_init() unknown sing/miss direction: {ltr_array[i]}");
        }
    }

    private void InternalCalculateLocation() {
        float draw_width, draw_height;
        Modifier modifier = this.drawable.GetModifier();

        float draw_x = this.draw_x;
        float draw_y = this.draw_y;

        this.statesprite.GetDrawSize(out draw_width, out draw_height);
        if (this.enable_reference_size) {
            if (this.reference_width >= 0) draw_width = (this.reference_width - draw_width) / 2.0f;
            if (this.reference_height >= 0) draw_height = (this.reference_height - draw_height) / 2.0f;
        }

        float action_offset_x, action_offset_y;
        switch (this.current_action_type) {
            case CharacterActionType.SING:
                action_offset_x = this.current_action_sing.offset_x;
                action_offset_y = this.current_action_sing.offset_y;
                break;
            case CharacterActionType.MISS:
                action_offset_x = this.current_action_miss.offset_x;
                action_offset_y = this.current_action_miss.offset_y;
                break;
            case CharacterActionType.IDLE:
            case CharacterActionType.EXTRA:
                action_offset_x = this.current_action_extra.offset_x;
                action_offset_y = this.current_action_extra.offset_y;
                break;
            default:
            case CharacterActionType.NONE:
                action_offset_x = 0;
                action_offset_y = 0;
                break;
        }

        // step 1: apply align
        switch (this.align_vertical) {
            case Align.CENTER:
                draw_y -= draw_height / 2.0f;
                break;
            case Align.END:
                draw_y -= draw_height;
                break;
        }
        switch (this.align_horizontal) {
            case Align.CENTER:
                draw_x -= draw_width / 2.0f;
                break;
            case Align.END:
                draw_x -= draw_width;
                break;
        }

        // step 2: calc total offsets
        float offset_x = action_offset_x + this.offset_x;
        float offset_y = action_offset_y + this.offset_y;
        if (this.flip_x) offset_x = -offset_x;

        // step 3: apply offsets
        draw_x += offset_x * this.character_scale;
        draw_y += offset_y * this.character_scale;

        // step 4: change the sprite location
        modifier.x = draw_x;
        modifier.y = draw_y;
        this.statesprite.SetDrawLocation(draw_x, draw_y);
    }

    private void InternalEndCurrentAction() {
        if (this.current_anim == null || this.current_idle_in_next_beat) return;

        AnimSprite @base = null;
        AnimSprite hold = null;
        AnimSprite rollback = null;

        switch (this.current_action_type) {
            case CharacterActionType.SING:
                rollback = this.current_action_sing.rollback;
                switch (this.current_anim_type) {
                    case CharacterAnimType.BASE:
                        @base = this.current_action_sing.@base;
                        if (this.current_action_sing.follow_hold) hold = this.current_action_sing.hold;
                        break;
                    case CharacterAnimType.HOLD:
                        hold = this.current_action_sing.hold;
                        break;
                    case CharacterAnimType.ROLLBACK:
                        if (rollback != null) rollback.Restart();
                        break;
                }
                break;
            case CharacterActionType.MISS:
                @base = this.current_action_miss.animation;
                break;
            case CharacterActionType.IDLE:
            case CharacterActionType.EXTRA:
                rollback = this.current_action_extra.rollback;
                switch (this.current_anim_type) {
                    case CharacterAnimType.BASE:
                        @base = this.current_action_extra.@base;
                        hold = this.current_action_extra.hold;
                        break;
                    case CharacterAnimType.HOLD:
                        hold = this.current_action_extra.hold;
                        break;
                    case CharacterAnimType.ROLLBACK:
                        if (rollback != null) rollback.Restart();
                        break;
                }
                break;
        }

        if (@base != null) @base.ForceEnd3(this.statesprite);
        if (hold != null) hold.ForceEnd3(this.statesprite);
        if (rollback != null) rollback.ForceEnd3(this.statesprite);
    }

    private void InternalFallbackIdle() {
        if (this.current_action_type == CharacterActionType.IDLE) return;
        InternalEndCurrentAction();
        this.current_action_type = CharacterActionType.NONE;
        PlayIdle();
    }

    private static int InternalAddTexture(ArrayList<CharacterTextureInfo> texture_arraylist, ModelHolder modelholder) {
        CharacterTextureInfo[] array = texture_arraylist.PeekArray();
        int size = texture_arraylist.Size();

        Texture texture = modelholder.GetTexture(false);
        if (texture == null) return -1;

        for (int i = 0 ; i < size ; i++) {
            if (array[i].texture == texture) return i;
        }

        int width, height;
        modelholder.GetTextureResolution(out width, out height);

        texture_arraylist.Add(new CharacterTextureInfo() {
            texture = modelholder.GetTexture(true),
            resolution_width = width,
            resolution_height = height,
        });

        return size;
    }

    private static ModelHolder InternalGetModelholder(ArrayList<CharacterModelInfo> modelholder_arraylist, string model_src, bool is_optional) {
        if (StringUtils.IsEmpty(model_src)) {
            if (is_optional)
                return modelholder_arraylist.Get(0).modelholder;
            else
                throw new Exception("character model not specified in the manifest");
        }

        CharacterModelInfo[] array = modelholder_arraylist.PeekArray();
        int size = modelholder_arraylist.Size();

        for (int i = 0 ; i < size ; i++) {
            if (array[i].model_src == model_src) return array[i].modelholder;
        }

        ModelHolder modelholder = ModelHolder.Init(model_src);
        if (modelholder.IsInvalid()) throw new Exception("model not found: " + model_src);

        modelholder_arraylist.Add(new CharacterModelInfo() { model_src = model_src, modelholder = modelholder });
        return modelholder;
    }

    private static void InternalDestroyModelholder(CharacterModelInfo modelholder_arraylist_item) {
        //free(modelholder_arraylist_item.model_src);
        modelholder_arraylist_item.modelholder.Destroy();
    }

    private static CharacterState InternalImportActions(CharacterImportContext context, CharacterManifest.Actions actions, string model_src, string state_name) {
        CharacterState state = Character.InternalStateCreate(state_name, actions.sing_size, actions.miss_size, actions.extras_size);

        // import all sing actions
        for (int i = 0 ; i < actions.sing_size ; i++) {
            int index = Character.InternalIndexName(
                context.all_directions_names, actions.sing[i].direction, true
            );

            ModelHolder modelholder = Character.InternalGetModelholder(
                context.modelholder_arraylist, actions.sing[i].model_src ?? model_src, true
            );

            state.sing[i].id_texture = state.sing_alt[i].id_texture = Character.InternalAddTexture(
                context.textures, modelholder
            );

            Character.InternalImportSing(
                 state.sing[i],
                 modelholder, actions.sing[i],
                 index,
                 context.charactermanifest.sing_prefix,
                 context.charactermanifest.sing_suffix
             );
            Character.InternalImportSing(
                state.sing_alt[i],
                modelholder,
                actions.sing[i],
                index,
                context.charactermanifest.sing_alternate_prefix,
                context.charactermanifest.sing_alternate_suffix
            );
        }

        // import all miss actions
        for (int i = 0 ; i < actions.miss_size ; i++) {
            int index = Character.InternalIndexName(
                context.all_directions_names, actions.miss[i].direction, true
            );

            ModelHolder modelholder = Character.InternalGetModelholder(
                context.modelholder_arraylist, actions.miss[i].model_src ?? model_src, true
            );

            state.miss[i].id_texture = Character.InternalAddTexture(
                context.textures, modelholder
            );

            Character.InternalImportMiss(
                state.miss[i], modelholder, actions.miss[i], index
            );
        }

        // import all extras
        for (int i = 0 ; i < actions.extras_size ; i++) {
            int index = Character.InternalIndexName(
                context.all_extras_names, actions.extras[i].name, true
            );


            Character.InternalImportExtra(
                state.extras[i],
                context.modelholder_arraylist,
                context.textures,
                actions.extras[i],
                index,
                null,
                null,
                model_src
            );
        }


        Character.InternalImportExtra(
            state.hey,
            context.modelholder_arraylist,
            context.textures,
            actions.hey,
            -10,
            null,
            null,
            model_src
        );


        Character.InternalImportExtra(
            state.idle,
            context.modelholder_arraylist,
            context.textures,
            actions.idle,
            -11,
            context.charactermanifest.allow_alternate_idle ? context.charactermanifest.sing_prefix : null,
            context.charactermanifest.allow_alternate_idle ? context.charactermanifest.sing_suffix : null,
            model_src
        );

        if (context.charactermanifest.allow_alternate_idle) {
            Character.InternalImportExtra(
                state.idle_alt,
                context.modelholder_arraylist,
                context.textures,
                actions.idle,
                -11,
                context.charactermanifest.allow_alternate_idle ? context.charactermanifest.sing_alternate_prefix : null,
                context.charactermanifest.allow_alternate_idle ? context.charactermanifest.sing_alternate_suffix : null,
                model_src
            );
        }

        return context.states.Add(state);
    }

    private int InternalAnimateSing(bool completed, bool should_stop) {
        CharacterActionSing action = this.current_action_sing;

        if (!completed && !should_stop) {
            // keep waiting
            return 0;
        }

        // play "animHold" if necessary
        if (this.current_follow_hold && !should_stop) {
            this.commited_animations_count++;
            this.current_anim_type = CharacterAnimType.HOLD;
            this.current_follow_hold = false;
            this.current_anim = action.hold;
            this.current_anim.Restart();
            return 1;
        }

        if (this.current_waiting_animation_end) {
            if (!completed) return 0;
        } else if (!should_stop) {
            // edge case: sustain sing with "anim"+"animHold" ended. keep static
            // edge case: sustain sing with "anim" only ended. keep static
            return 1;
        }

        //
        // Note: only do rollback here for non-sustain sing actions
        //
        this.commited_animations_count++;

        // if exists a rollback animation use it
        if (action.rollback != null) {
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_anim = action.rollback;
            this.current_anim_type = CharacterAnimType.ROLLBACK;
            this.current_use_frame_rollback = false;
            this.current_anim.Restart();
            return 1;
        }

        // if not, try do self rollback
        bool is_base = this.current_anim_type == CharacterAnimType.BASE;
        bool can_frame_rollback = is_base ? action.base_can_rollback : action.hold_can_rollback;

        if (!can_frame_rollback) {
            if (this.current_waiting_animation_end_and_idle) {
                // do not wait for next beat, play idle now
                this.current_action_type = CharacterActionType.NONE;
                return PlayIdle();
            }
            // schedule idle
            this.current_expected_duration = 0f;
            this.current_idle_in_next_beat = true;
            return 1;
        }

        // do frame rollback
        this.current_use_frame_rollback = true;
        this.current_anim_type = CharacterAnimType.ROLLBACK;
        return 1;
    }

    private int InternalAnimateMiss(bool completed, bool should_stop) {
        if (!completed && !should_stop) {
            // nothing to do
            return 0;
        }

        if (this.current_waiting_animation_end) {
            if (!completed) return 0;
        } else if (!should_stop) {
            return 1;
        }

        this.commited_animations_count++;

        if (this.current_waiting_animation_end_and_idle) {
            // do not wait for next beat, play idle now
            this.current_action_type = CharacterActionType.NONE;
            return PlayIdle();
        }

        // schedule idle
        this.current_expected_duration = 0f;
        this.current_idle_in_next_beat = true;
        return 1;
    }

    private int InternalAnimateIdle(bool completed, bool has_beat) {
        if (!completed) {

            // interrupt "animHold" when a beat is detected
            if (has_beat && !this.continuous_idle && this.current_anim_type == CharacterAnimType.HOLD) {
                this.current_action_type = CharacterActionType.NONE;
                PlayIdle();
                return 1;
            }

            return 0;
        }

        this.commited_animations_count++;

        if (this.current_follow_hold) {
            this.current_follow_hold = false;
            this.current_anim = this.current_action_extra.hold;
            this.current_anim_type = CharacterAnimType.HOLD;
            this.current_anim.Restart();
            return 1;
        }

        if (this.continuous_idle) {
            this.current_action_type = CharacterActionType.NONE;
            PlayIdle();
            return 1;
        }

        this.current_expected_duration = 0f;
        this.current_idle_in_next_beat = true;
        return 1;
    }

    private int InternalAnimateExtra(bool completed, bool should_stop) {
        CharacterActionExtra action = this.current_action_extra;

        if (!completed) return 0;

        // play "animHold" if necessary
        if (this.current_follow_hold && !should_stop) {
            this.commited_animations_count++;
            this.current_anim_type = CharacterAnimType.HOLD;
            this.current_follow_hold = false;
            this.current_anim = action.hold;
            this.current_anim.Restart();
            return 1;
        }

        if (this.current_waiting_animation_end) {
            if (!completed) return 0;
        } else if (!should_stop) {
            return 1;
        }

        this.commited_animations_count++;

        // if exists a rollback animation use it
        if (action.rollback != null) {
            this.current_expected_duration = Single.PositiveInfinity;
            this.current_anim = action.rollback;
            this.current_anim_type = CharacterAnimType.ROLLBACK;
            this.current_use_frame_rollback = false;
            this.current_anim.Restart();
            return 1;
        }

        if (this.current_waiting_animation_end_and_idle) {
            // do not wait for next beat, play idle now
            this.current_action_type = CharacterActionType.NONE;
            return PlayIdle();
        }

        this.current_expected_duration = 0f;
        this.current_idle_in_next_beat = true;
        return 1;
    }


    private class CharacterModelInfo {
        public string model_src;
        public ModelHolder modelholder;
    }
    private class CharacterTextureInfo {
        public Texture texture;
        public int resolution_width;
        public int resolution_height;
    }
    private class CharacterActionExtra {
        public int id_extra;
        public float stop_after_beats;
        public bool is_valid;
        public AnimSprite @base;
        public AnimSprite hold;
        public AnimSprite rollback;
        public int id_texture;
        public float offset_x;
        public float offset_y;
    }
    private class CharacterActionSing {
        public int id_direction;
        public AnimSprite @base;
        public AnimSprite hold;
        public AnimSprite rollback;
        public bool base_can_rollback;
        public bool hold_can_rollback;
        public bool follow_hold;
        public bool full_sustain;
        public float stop_after_beats;
        public int id_texture;
        public float offset_x;
        public float offset_y;
    }
    private class CharacterActionMiss {
        public int id_direction;
        public AnimSprite animation;
        public float stop_after_beats;
        public int id_texture;
        public float offset_x;
        public float offset_y;
    }
    private class CharacterState {
        public string name;
        public int sing_size;
        public int miss_size;
        public int extras_size;
        public CharacterActionSing[] sing;
        public CharacterActionSing[] sing_alt;
        public CharacterActionMiss[] miss;
        public CharacterActionExtra[] extras;
        public CharacterActionExtra hey;
        public CharacterActionExtra idle;
        public CharacterActionExtra idle_alt;
    }
    private class CharacterImportContext {
        public ArrayList<string> all_directions_names;
        public ArrayList<string> all_extras_names;
        public ArrayList<CharacterTextureInfo> textures;
        public ArrayList<CharacterModelInfo> modelholder_arraylist;
        public ArrayList<CharacterState> states;
        public CharacterManifest charactermanifest;
    }

}
