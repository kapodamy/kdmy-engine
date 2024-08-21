using System;
using System.Diagnostics;
using Engine.Platform;
using Engine.Utils;
using KallistiOS.VBLANK;
using KallistiOS.TIMER;

namespace Engine.Game;

public class DDRKey {
    public double in_song_timestamp;
    public int strum_id;
    public bool holding;
    public bool discard;
    public bool strum_invisible;
    public GamepadButtons button;
    //public Gamepad gamepad;
}

public class DDRKeysFIFO {
    public DDRKey[] queue;
    public int available;


    public void Purge() {
        Purge2(false);
    }

    public void Purge2(bool force_drop_first) {
        if (this.available < 1) return;

        int available = 0;

        for (int i = force_drop_first ? 1 : 0 ; i < this.available ; i++) {
            if (this.queue[i].discard) continue;

            if (i == available) {
                available++;
                continue;
            }

            this.queue[i] = this.queue[available];
            available++;

#if DEBUG
            // debugging only
            this.queue[i].in_song_timestamp = -2;
            this.queue[i].strum_id = -2;
            this.queue[i].button = 0;
#endif
        }

        this.available = available;
    }

}


public class DDRKeymon {

    public const byte FIFO_LENGTH = 64;


    private DDRKeysFIFO ddrkeys_fifo;
    private double start_timestamp;
    private Bind[] strum_binds;
    private int strum_binds_size;
    private int vbl_hnd;
    private Gamepad gamepad;


    public DDRKeymon(Gamepad gamepad, GamepadButtons[] strum_binds, int strum_binds_size) {

        this.ddrkeys_fifo = new DDRKeysFIFO() {
            queue = new DDRKey[DDRKeymon.FIFO_LENGTH],
            available = 0
        };

        this.start_timestamp = 0;

        this.strum_binds = EngineUtils.CreateArray<Bind>(strum_binds_size);
        this.strum_binds_size = strum_binds_size;

        this.vbl_hnd = -1;
        this.gamepad = gamepad;


        for (int i = 0 ; i < strum_binds_size ; i++) {
            uint count = Math2D.BitCount((uint)strum_binds[i]);
            this.strum_binds[i] = new Bind() {
                is_visible = true,
                button_flags = strum_binds[i],
                button_count = count,
                button_array = EngineUtils.CreateArray<GamepadButtons>(count)
            };

            for (int j = 0, k = 0 ; k < count ; j++) {
                uint flag = (uint)strum_binds[i] & (0x01U << j);
                if (flag == 0x00) continue;
                this.strum_binds[i].button_array[k++] = (GamepadButtons)flag;
            }
        }

        for (int i = 0 ; i < DDRKeymon.FIFO_LENGTH ; i++)
            this.ddrkeys_fifo.queue[i] = new DDRKey();

    }

    public void Destroy() {
        // note: "ddrkeys_fifo.queue" is a fixed array do not dispose

        // stop the monitor thread if running
        Stop();

        for (int i = 0 ; i < this.strum_binds_size ; i++) {
            //free(this.strum_binds[i].button_array);
        }

        //free(this.strum_binds);
        //free(this);
    }


    public DDRKeysFIFO GetFifo() {
        return this.ddrkeys_fifo;
    }

    public void Clear() {
        for (int i = 0 ; i < this.ddrkeys_fifo.available ; i++)
            this.ddrkeys_fifo.queue[i].discard = true;
        this.ddrkeys_fifo.available = 0;
    }

    public void Start(double offset_timestamp) {
        if (this.vbl_hnd >= 0) return;

        this.start_timestamp = timer.ms_gettime64() + offset_timestamp;
        this.vbl_hnd = vblank.handler_add(this.InternalVBL);

#if DEBUG
        Debug.Assert(this.vbl_hnd >= 0);
#endif
    }

    public void Stop() {
        if (this.vbl_hnd < 0) return;

        vblank.handler_remove(this.vbl_hnd);
        this.vbl_hnd = -1;

        Clear();
    }

    public double PeekTimestamp() {
        return timer.ms_gettime64() - this.start_timestamp;
    }

    public void Resync(double offset_timestamp) {
        this.start_timestamp = timer.ms_gettime64() + offset_timestamp;
    }


    private void InternalAppendKey(double timestamp, int strum_id, GamepadButtons button_id, bool holding) {
        DDRKey[] queue = this.ddrkeys_fifo.queue;
        int available = this.ddrkeys_fifo.available;

        // offset correction
        timestamp -= this.start_timestamp;

        for (int i = available - 1 ; i >= 0 ; i--) {
            if (queue[i].discard) continue;
            if (queue[i].strum_id != strum_id) continue;
            if (queue[i].button != button_id) continue;

            // can not exists repeated events
            if (queue[i].holding != holding) break;

            // non-monotomous key event, this happens if the queue overflows
            // which under normal conditions this never should happen
            queue[i].in_song_timestamp = timestamp;
            return;
        }

        if (available >= DDRKeymon.FIFO_LENGTH) {
            // imminent overflow, drop first key
            Logger.Warn("ddrkeymon_append_key() queue overflow, ¿are you checking the queue?");
            this.ddrkeys_fifo.Purge2(true);

            if (available >= DDRKeymon.FIFO_LENGTH) {
                throw new Exception("ddrkeymon_append_key() queue overflow");
            }
        }

        // the queue is healthy, append the key
        queue[available].in_song_timestamp = timestamp;
        queue[available].strum_id = strum_id;
        queue[available].holding = holding;
        queue[available].discard = false;
        queue[available].strum_invisible = !this.strum_binds[strum_id].is_visible;
        queue[available].button = button_id;

        this.ddrkeys_fifo.available = available + 1;
    }

    private void InternalProcessKey(double timestamp, GamepadButtons old_buttons, GamepadButtons new_buttons) {
        for (int i = 0 ; i < this.strum_binds_size ; i++) {
            GamepadButtons buttons_flags = this.strum_binds[i].button_flags;
            GamepadButtons old_state = old_buttons & buttons_flags;
            GamepadButtons new_state = new_buttons & buttons_flags;

            if (old_state == new_state) continue;

            for (int j = 0 ; j < this.strum_binds[i].button_count ; j++) {
                GamepadButtons button_flag = this.strum_binds[i].button_array[j];
                GamepadButtons button_state_old = old_state & button_flag;
                GamepadButtons button_state_new = new_state & button_flag;

                if (button_state_old == button_state_new) continue;

                InternalAppendKey(timestamp, i, button_flag, button_state_new.Bool());
            }
        }
    }

    private void InternalVBL(uint code) {
        double timestamp = timer.ms_gettime64();
        GamepadButtonsChanges ret = this.gamepad.DirectStateUpdate();

        if (ret.previous == GamepadButtons.INVALID || ret.current == GamepadButtons.INVALID) {
            // invalid state or dettached controller
            return;
        }

        InternalProcessKey(timestamp, ret.previous, ret.current);
    }


    private class Bind {
        public bool is_visible;
        public GamepadButtons button_flags;
        public uint button_count;
        public GamepadButtons[] button_array;
    }

}
