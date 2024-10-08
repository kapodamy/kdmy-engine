using System;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game;

public struct DistributionNote {
    public string name;
    public string sick_effect_state_name;
    public string model_src;
    public string custom_sick_effect_model_src;
    public float damage_ratio;
    public float heal_ratio;
    public bool ignore_hit;
    public bool ignore_miss;
    public bool can_kill_on_hit;
    internal bool is_special;
}

public struct DistributionStrum {
    public string name;
    public int[] notes_ids;
    public int notes_ids_size;
}

public struct DistributionStrumState {
    public string name;
    public string model_marker;
    public string model_sick_effect;
    public string model_background;
    public string model_notes;
}

public class Distribution {
    public int notes_size;
    public DistributionNote[] notes;
    public int strums_size;
    public DistributionStrum[] strums;
    public GamepadButtons[] strum_binds;
    public bool strum_binds_is_custom;
    public DistributionStrumState[] states;
    public int states_size;
}

public class Strums : IDraw, IAnimate {

    //
    // buttons binding
    //
    public static readonly GamepadButtons[] BUTTONS_BIND_4 = {
        GamepadButtons.DALL_LEFT, GamepadButtons.DALL_DOWN,
        GamepadButtons.DALL_UP, GamepadButtons.DALL_RIGHT
    };

    public static readonly GamepadButtons[] BUTTONS_BIND_5 = {
        GamepadButtons.DALL_LEFT, GamepadButtons.DALL_DOWN,
        GamepadButtons.T_LR | GamepadButtons.B_LR | GamepadButtons.SELECT,
        GamepadButtons.DALL_UP, GamepadButtons.DALL_RIGHT
    };

    public static readonly GamepadButtons[] BUTTONS_BIND_6 = {
        GamepadButtons.DPAD_LEFT | GamepadButtons.DPAD2_LEFT, GamepadButtons.DPAD_UP | GamepadButtons.DPAD2_UP, GamepadButtons.DPAD_RIGHT | GamepadButtons.DPAD2_RIGHT,
        GamepadButtons.DPAD3_LEFT | GamepadButtons.DPAD4_LEFT, GamepadButtons.DPAD3_UP | GamepadButtons.DPAD4_UP, GamepadButtons.DPAD3_RIGHT | GamepadButtons.DPAD4_RIGHT
    };

    public static readonly GamepadButtons[] BUTTONS_BIND_7 = {
        GamepadButtons.DPAD_LEFT | GamepadButtons.DPAD2_LEFT, GamepadButtons.DPAD_UP | GamepadButtons.DPAD2_UP, GamepadButtons.DPAD_RIGHT | GamepadButtons.DPAD2_RIGHT,
        GamepadButtons.T_LR | GamepadButtons.B_LR | GamepadButtons.SELECT,
        GamepadButtons.DPAD3_LEFT | GamepadButtons.DPAD4_LEFT, GamepadButtons.DPAD3_UP | GamepadButtons.DPAD4_UP, GamepadButtons.DPAD3_RIGHT | GamepadButtons.DPAD4_RIGHT
    };

    public static readonly GamepadButtons[] BUTTONS_BIND_8 = {
        GamepadButtons.DPAD_LEFT | GamepadButtons.DPAD2_LEFT, GamepadButtons.DPAD_DOWN | GamepadButtons.DPAD2_DOWN, GamepadButtons.DPAD_UP | GamepadButtons.DPAD2_UP, GamepadButtons.DPAD_RIGHT | GamepadButtons.DPAD2_RIGHT,
        GamepadButtons.DPAD3_LEFT | GamepadButtons.DPAD4_LEFT, GamepadButtons.DPAD3_DOWN | GamepadButtons.DPAD4_DOWN, GamepadButtons.DPAD3_UP | GamepadButtons.DPAD4_UP, GamepadButtons.DPAD3_RIGHT | GamepadButtons.DPAD4_RIGHT,
    };

    public static readonly GamepadButtons[] BUTTONS_BIND_9 = {
        GamepadButtons.DPAD_LEFT | GamepadButtons.DPAD2_LEFT, GamepadButtons.DPAD_DOWN | GamepadButtons.DPAD2_DOWN, GamepadButtons.DPAD_UP | GamepadButtons.DPAD2_UP, GamepadButtons.DPAD_RIGHT | GamepadButtons.DPAD2_RIGHT,
        GamepadButtons.T_LR | GamepadButtons.B_LR | GamepadButtons.SELECT,
        GamepadButtons.DPAD3_LEFT | GamepadButtons.DPAD4_LEFT, GamepadButtons.DPAD3_DOWN | GamepadButtons.DPAD4_DOWN, GamepadButtons.DPAD3_UP | GamepadButtons.DPAD4_UP, GamepadButtons.DPAD3_RIGHT | GamepadButtons.DPAD4_RIGHT,
    };


    public static readonly Distribution DEFAULT_DISTRIBUTION = new Distribution() {
        notes_size = 4,
        notes = new DistributionNote[] {
           new DistributionNote() { name= "left" }, new DistributionNote() { name= "down" }, new DistributionNote(){ name= "up" },new DistributionNote() { name= "right" }
        },

        strums_size = 4,
        strums = new DistributionStrum[] {
            new DistributionStrum() { name= "left", notes_ids= new int[]{ 0 }, notes_ids_size= 1 },
            new DistributionStrum() { name= "down", notes_ids= new int[]{ 1 }, notes_ids_size= 1 },
            new DistributionStrum() { name= "up", notes_ids= new int[]{ 2 }, notes_ids_size= 1 },
            new DistributionStrum() { name= "right", notes_ids= new int[]{ 3 }, notes_ids_size= 1 }
        },
        strum_binds = BUTTONS_BIND_4,
        states = new DistributionStrumState[] {
            new DistributionStrumState() {
                name= null,
                model_marker= Funkin.COMMON_NOTES,
                model_sick_effect= Funkin.COMMON_NOTES_SPLASH,
                model_background= Funkin.COMMON_STRUM_BACKGROUND,
                model_notes= null
            }
        },
        states_size = 1
    };

    static Strums() {
        for (int i = 0 ; i < Strums.DEFAULT_DISTRIBUTION.notes_size ; i++) {
            Strums.DEFAULT_DISTRIBUTION.notes[i].sick_effect_state_name = null;
            Strums.DEFAULT_DISTRIBUTION.notes[i].model_src = Funkin.COMMON_NOTES;
            Strums.DEFAULT_DISTRIBUTION.notes[i].custom_sick_effect_model_src = null;
            Strums.DEFAULT_DISTRIBUTION.notes[i].damage_ratio = 1.0f;
            Strums.DEFAULT_DISTRIBUTION.notes[i].heal_ratio = 1.0f;
            Strums.DEFAULT_DISTRIBUTION.notes[i].ignore_hit = false;
            Strums.DEFAULT_DISTRIBUTION.notes[i].ignore_miss = false;
            Strums.DEFAULT_DISTRIBUTION.notes[i].can_kill_on_hit = false;
            Strums.DEFAULT_DISTRIBUTION.notes[i].is_special = false;
        }
    }



    private int size;
    private Strum[] lines;
    private StateSprite[] sick_effects;
    private int player_id;
    private float x;
    private float y;
    private float gap;
    private float invdimmen;
    private bool is_vertical;
    private bool is_inverse;
    private DDRKeysFIFO ddrkeys_fifo;
    private Drawable drawable;
    private AnimSprite drawable_animation;
    private Modifier modifier;
    private PlayerStats playerstats;
    private WeekScript weekscript;
    private double decorators_scroll_velocity;
    private double decorators_last_song_timestamp;
    private ArrayList<Decorator> decorators;
    private float decorators_alpha;
    private double decorators_offset_milliseconds;



    public Strums(float x, float y, float z, float dimmen, float invdimmen, float length, float gap, int player_id, bool is_vertical, bool keep_marker_scale, DistributionStrum[] strumsdefs, int strumsdefs_size) {

        this.size = strumsdefs_size;
        this.lines = EngineUtils.CreateArray<Strum>(strumsdefs_size);
        this.sick_effects = EngineUtils.CreateArray<StateSprite>(strumsdefs_size);

        this.player_id = player_id; this.x = x; this.y = y; this.gap = gap; this.invdimmen = invdimmen; this.is_vertical = is_vertical;

        this.ddrkeys_fifo = null; this.playerstats = null; this.weekscript = null;

        this.drawable = null;
        this.drawable_animation = null;
        this.modifier = null;

        this.drawable = new Drawable(z, this, this);
        this.modifier = this.drawable.GetModifier();
        this.modifier.x = x;
        this.modifier.x = y;

        this.is_vertical = is_vertical;
        this.is_inverse = false;

        float space = gap + invdimmen;

        for (int i = 0 ; i < this.size ; i++) {
            string name = strumsdefs[i].name;
            this.sick_effects[i] = null;

            this.lines[i] = new Strum(i, name, x, y, dimmen, invdimmen, length, keep_marker_scale);
            this.lines[i].SetPlayerID(player_id);

            if (is_vertical) x += space;
            else y += space;
        }

        float invlength = gap * this.size;
        if (this.size > 0) invlength -= gap;
        invlength += invdimmen * this.size;

        if (is_vertical) {
            this.modifier.width = invlength;
            this.modifier.height = dimmen;
        } else {
            this.modifier.width = dimmen;
            this.modifier.height = invlength;
        }

        this.decorators_scroll_velocity = 1.0;
        this.decorators = new ArrayList<Decorator>(1);
        this.decorators_alpha = 1f;
        this.decorators_offset_milliseconds = 0.0;
    }

    public void Destroy() {
        Luascript.DropShared(this);

        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].Destroy();

        this.drawable.Destroy();
        if (this.drawable_animation != null) this.drawable_animation.Destroy();

        foreach (Decorator decorator in this.decorators)
            decorator.statesprite.Destroy();
        decorators.Destroy();

        //free(this.lines);
        //free(this.sick_effects);
        //free(this);
    }

    public void SetParams(DDRKeymon ddrkeymon, PlayerStats playerstats, WeekScript weekscript) {
        this.ddrkeys_fifo = ddrkeymon != null ? ddrkeymon.GetFifo() : null;
        this.playerstats = playerstats;
        this.weekscript = weekscript;
    }

    public Drawable GetDrawable() {
        return this.drawable;
    }


    public void SetNotes(Chart chart, DistributionStrum[] strumsdefs, int strumsdefs_size, NotePool notepool) {
        for (int i = 0 ; i < this.size ; i++) {
            int success = this.lines[i].SetNotes(
                chart, strumsdefs, strumsdefs_size, this.player_id, notepool
            );
            if (success < 0)
                throw new Exception("Error setting the notes for strum id=" + i + " player_id=" + this.player_id);
        }
    }

    public void SetScrollSpeed(double speed) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].SetScrollSpeed(speed);
    }

    public void SetScrollDirection(ScrollDirection direction) {
        bool is_vertical = direction == ScrollDirection.UPSCROLL || direction == ScrollDirection.DOWNSCROLL;

        this.is_inverse = direction == ScrollDirection.DOWNSCROLL || direction == ScrollDirection.RIGHTSCROLL;

        if (is_vertical != this.is_vertical) {
            float temp = this.modifier.width;
            this.modifier.width = this.modifier.height;
            this.modifier.height = temp;
            this.is_vertical = is_vertical;
            InternalUpdateDrawLocation();
        }

        for (int i = 0 ; i < this.size ; i++) this.lines[i].SetScrollDirection(direction);
    }

    public void SetMarkerDurationMultiplier(float multipler) {
        for (int i = 0 ; i < this.size ; i++) this.lines[i].SetMarkerDurationMultiplier(multipler);
    }

    public void SetBpm(float beats_per_minute) {
        for (int i = 0 ; i < this.size ; i++) this.lines[i].SetBpm(beats_per_minute);
    }

    public void DisableBeatSyncedIdleAndContinous(bool disabled) {
        for (int i = 0 ; i < this.size ; i++) this.lines[i].DisableBeatSyncedIdleAndContinous(disabled);
    }

    public void Reset(double scroll_speed, string state_name) {
        for (int i = 0 ; i < this.size ; i++) {
            this.lines[i].Reset(scroll_speed, state_name);
        }
        this.drawable.SetAntialiasing(PVRFlag.DEFAULT);
        this.DecoratorsSetScrollSpeed(scroll_speed);
        this.DecoratorsSetVisible(-1.0, true);
        this.decorators_last_song_timestamp = 0.0;
    }

    public int ScrollFull(double song_timestamp) {
        this.decorators_last_song_timestamp = song_timestamp;

        if (this.ddrkeys_fifo == null) {
            // this never should happen, use ScrollAuto() instead
            for (int i = 0 ; i < this.size ; i++)
                this.lines[i].ScrollAuto(song_timestamp, this.playerstats, this.weekscript);
            return 0;
        }

        int keys_processed = 0;

        // step 1: scroll all lines
        for (int i = 0 ; i < this.size ; i++) {
            keys_processed += this.lines[i].Scroll(
                 song_timestamp, this.ddrkeys_fifo, this.playerstats, this.weekscript
            );
        }

        // step 2: find penalties hits between lines
        for (int i = 0 ; i < this.size ; i++) {
            keys_processed += this.lines[i].FindPenaltiesNoteHit(
                song_timestamp, this.ddrkeys_fifo, this.playerstats, this.weekscript
            );
        }

        // step 3: find penalties hits on empty lines
        for (int i = 0 ; i < this.size ; i++) {
            keys_processed += this.lines[i].FindPenaltiesEmptyHit(
                song_timestamp, this.ddrkeys_fifo, this.playerstats
            );
        }

        if (keys_processed > 0) {
            this.ddrkeys_fifo.Purge();
        }

        return keys_processed;
    }

    public void ScrollAuto(double song_timestamp) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].ScrollAuto(song_timestamp, this.playerstats, this.weekscript);

        if (this.ddrkeys_fifo != null) {
            // drop all key events, they are useless
            this.ddrkeys_fifo.available = 0;
        }

        this.decorators_last_song_timestamp = song_timestamp;
    }

    public void ForceKeyRelease() {
        for (int i = 0 ; i < this.size ; i++) {
            this.lines[i].ForceKeyRelease();
        }
    }


    public int Animate(float elapsed) {
        int res = 0;
        if (this.drawable_animation != null) {
            res += this.drawable_animation.Animate(elapsed);
            this.drawable_animation.UpdateDrawable(this.drawable, true);
        }

        for (int i = 0 ; i < this.size ; i++)
            res += this.lines[i].Animate(elapsed);

        Decorator[] decorators_array = this.decorators.PeekArray();
        int decorators_size = this.decorators.Size();
        for (int i = 0 ; i < decorators_size ; i++)
            res += decorators_array[i].statesprite.Animate(elapsed);

        return res;
    }

    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();

        this.drawable.HelperApplyInContext(pvrctx);

        for (int i = 0 ; i < this.size ; i++) {
            if (!this.lines[i].GetDrawable().IsVisible()) continue;
            this.lines[i].Draw(pvrctx);
        }

        for (int i = 0 ; i < this.size ; i++) {
            if (!this.lines[i].GetDrawable().IsVisible()) continue;
            if (this.sick_effects[i] != null && this.sick_effects[i].IsVisible())
                this.sick_effects[i].Draw(pvrctx);
        }

        pvrctx.Save();
        pvrctx.SetGlobalAlpha(this.decorators_alpha);

        int decorators_size = this.decorators.Size();
        Decorator[] decorators_array = this.decorators.PeekArray();
        double song_timestamp = this.decorators_last_song_timestamp + this.decorators_offset_milliseconds;
        double draw_x = this.modifier.x;
        double draw_y = this.modifier.y;

        //
        // Draw all decorators and let the PVRContext decide what is visible, because
        // decorators can have different dimmensions. The marker duration and scroll window
        // does not apply here.
        //
        for (int i = 0 ; i < decorators_size ; i++) {
            double decorator_timestamp = decorators_array[i].timestamp;
            double scroll_offset;
            double x = draw_x;
            double y = draw_y;

            if (this.is_inverse) {
                scroll_offset = song_timestamp - decorator_timestamp;
            } else {
                scroll_offset = decorator_timestamp - song_timestamp;
            }

            scroll_offset *= this.decorators_scroll_velocity;

            if (this.is_vertical)
                y += scroll_offset;
            else
                x += scroll_offset;

            StateSprite statesprite = decorators_array[i].statesprite;
            statesprite.SetDrawLocation((float)x, (float)y);
            statesprite.Draw(pvrctx);
        }

        pvrctx.Restore();
        pvrctx.Restore();
    }


    public float SetAlpha(float alpha) {
        float old = this.drawable.GetAlpha();
        this.drawable.SetAlpha(alpha);
        return old;
    }

    public void EnableBackground(bool enable) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].EnableBackground(enable);
    }

    public void SetKeepAspectRatioBackground(bool enable) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].SetKeepAspectRatioBackground(enable);
    }

    public void SetAlphaBackground(float alpha) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].SetAlphaBackground(alpha);

    }

    public void SetAlphaSickEffect(float alpha) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].SetAlphaSickEffect(alpha);
    }

    public void SetDrawOffset(double offset_milliseconds) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].SetDrawOffset(offset_milliseconds);
        this.decorators_offset_milliseconds = offset_milliseconds;
    }


    public void StateAdd(ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, string state_name) {
        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].StateAdd(mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
    }

    public int StateToggle(string state_name) {
        int toggles = 0;
        for (int i = 0 ; i < this.size ; i++)
            toggles += this.lines[i].StateToggle(state_name);
        return toggles;
    }

    public int StateToggleNotes(string state_name) {
        int toggles = 0;
        for (int i = 0 ; i < this.size ; i++)
            toggles += this.lines[i].StateToggleNotes(state_name);
        return toggles;
    }

    public void StateToggleMarkerAndSickEffect(string state_name) {
        for (int i = 0 ; i < this.size ; i++) {
            this.lines[i].StateToggleMarker(state_name);
            this.lines[i].StateToggleSickEffect(state_name);
        }
    }

    public int GetLinesCount() {
        return this.size;
    }

    public Strum GetStrumLine(int index) {
        if (index < 0 || index >= this.size) return null;
        return this.lines[index];
    }

    public void EnablePostSickEffectDraw(bool enable) {
        for (int i = 0 ; i < this.size ; i++) {
            if (enable)
                this.sick_effects[i] = this.lines[i].DrawSickEffectApart(enable);
            else
                this.sick_effects[i] = null;
        }
    }

    public void UseFunkinMakerDuration(bool enable) {
        for (int i = 0 ; i < this.size ; i++) {
            this.lines[i].UseFunkinMakerDuration(enable);
        }
    }


    public void SetOffsetcolor(float r, float g, float b, float a) {
        this.drawable.SetOffsetColor(r, g, b, a);
    }

    public void SetVisible(bool visible) {
        this.drawable.SetVisible(visible);
    }

    public Modifier GetModifier() {
        return this.drawable.GetModifier();
    }


    public void AnimationSet(AnimSprite animsprite) {
        this.drawable_animation.Destroy();
        this.drawable_animation = animsprite != null ? animsprite.Clone() : null;
    }

    public void AnimationRestart() {
        if (this.drawable_animation != null) this.drawable_animation.Restart();

        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].AnimationRestart();

        Decorator[] decorators_array = this.decorators.PeekArray();
        int decorators_size = this.decorators.Size();
        for (int i = 0 ; i < decorators_size ; i++)
            decorators_array[i].statesprite.AnimationRestart();
    }

    public void AnimationEnd() {
        if (this.drawable_animation != null) {
            this.drawable_animation.ForceEnd();
            this.drawable_animation.UpdateDrawable(this.drawable, true);
        }

        for (int i = 0 ; i < this.size ; i++)
            this.lines[i].AnimationEnd();

        Decorator[] decorators_array = this.decorators.PeekArray();
        int decorators_size = this.decorators.Size();
        for (int i = 0 ; i < decorators_size ; i++)
            decorators_array[i].statesprite.AnimationEnd();
    }


    public int DecoratorsGetCount() {
        return this.decorators.Size();
    }

    public bool DecoratorsAdd(ModelHolder modelholder, string animation_name, double timestamp) {
        return DecoratorsAdd2(modelholder, animation_name, timestamp, 0, this.size - 1);
    }

    public bool DecoratorsAdd2(ModelHolder modelholder, string animation_name, double timestamp, int from_strum_index, int to_strum_index) {
        if (modelholder == null || timestamp < 0 || Double.IsNaN(timestamp)) return false;
        if (from_strum_index < 0 || to_strum_index >= this.size || to_strum_index < from_strum_index) return false;

        StateSprite statesprite = StateSprite.InitFromVertexColor(modelholder.GetVertexColor());
        StateSpriteState state = statesprite.StateAdd(modelholder, animation_name, null);

        if (state == null) {
            statesprite.Destroy();
            return false;
        }

        InternalCalcDecoratorBounds(state, from_strum_index, to_strum_index);
        statesprite.StateApply(state);

        Decorator decorator = new Decorator() {
            timestamp = timestamp,
            statesprite = statesprite,
            to_strum_index = to_strum_index,
            from_strum_index = from_strum_index,
            is_visible = true
        };

        this.decorators.Add(decorator);

        return true;
    }

    public void DecoratorsSetScrollSpeed(double speed) {
        this.decorators_scroll_velocity = Funkin.CHART_SCROLL_VELOCITY * speed;

        // Use half of the scolling speed if the screen aspect ratio is 4:3 (dreamcast)
        if (!PVRContext.global_context.IsWidescreen()) this.decorators_scroll_velocity *= 0.5;
    }

    public void DecoratorsSetAlpha(float alpha) {
        this.decorators_alpha = alpha;
    }

    public void DecoratorsSetVisible(double decorator_timestamp, bool visible) {
        foreach (Decorator decorator in this.decorators) {
            if (decorator_timestamp < 0.0 || Math2D.DoublesAreNearEqual(decorator.timestamp, decorator_timestamp)) {
                decorator.is_visible = visible;
            }
        }
    }


    public void ForceRebuild(float x, float y, float z, float dimmen, float invdimmen, float length_dimmen, float gap, bool is_vertical, bool keep_markers_scale) {
        this.drawable.SetZIndex(z);

        this.modifier.x = x;
        this.modifier.y = y;
        this.is_vertical = is_vertical;
        this.is_inverse = false;
        this.gap = gap;
        this.invdimmen = invdimmen;

        float invlength = gap * this.size;
        if (this.size > 0) invlength -= gap;
        invlength += invdimmen * this.size;

        if (is_vertical) {
            this.modifier.width = invlength;
            this.modifier.height = dimmen;
        } else {
            this.modifier.width = dimmen;
            this.modifier.height = invlength;
        }

        InternalUpdateDrawLocation();

        for (int i = 0 ; i < this.size ; i++) {
            this.lines[i].ForceRebuild(dimmen, invdimmen, length_dimmen, keep_markers_scale);
        }
    }

    private void InternalUpdateDrawLocation() {
        float space = this.gap + this.invdimmen;
        float x = this.x;
        float y = this.y;

        for (int i = 0 ; i < this.size ; i++) {
            this.lines[i].UpdateDrawLocation(x, y);

            if (this.is_vertical) x += space;
            else y += space;
        }

        foreach (Decorator decorator in this.decorators) {
            StateSpriteState state = decorator.statesprite.StateGet();
            InternalCalcDecoratorBounds(state, decorator.from_strum_index, decorator.to_strum_index);
            decorator.statesprite.StateApply(state);
        }
    }

    private void InternalCalcDecoratorBounds(StateSpriteState state, int from_strum_index, int to_strum_index) {
        float offset_start = (this.gap + this.invdimmen) * from_strum_index;
        float offset_end = (this.gap + this.invdimmen) * to_strum_index;
        float length = offset_end - offset_start;

        float x, y;
        float width, height;
        Align horizontal, vertical;

        if (this.is_vertical) {
            x = offset_start;
            y = 0f;
            width = length;
            height = -1f;
            horizontal = Align.CENTER;
            vertical = this.is_inverse ? Align.END : Align.START;
        } else {
            x = 0f;
            y = offset_start;
            width = -1;
            height = length;
            horizontal = this.is_inverse ? Align.END : Align.START;
            vertical = Align.CENTER;
        }

        if (state.texture != null) {
            ImgUtils.CalcRectangleInStateSpriteState(x, y, width, height, horizontal, vertical, state);
        } else {
            state.offset_x = x;
            state.offset_y = y;
            state.draw_width = this.is_vertical ? length : this.invdimmen;
            state.draw_height = this.is_vertical ? this.invdimmen : length;
        }
    }


    private class Decorator {
        public double timestamp;
        public StateSprite statesprite;
        public int from_strum_index;
        public int to_strum_index;
        public bool is_visible;
    }
}
