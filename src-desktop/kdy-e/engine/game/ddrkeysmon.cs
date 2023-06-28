using System;
using Engine.Platform;
using Engine.Utils;
using KallistiOS;
using KallistiOS.THD;

namespace Engine.Game;

public class DDRKey {
    public double in_song_timestamp;
    public int strum_id;
    public bool holding;
    public bool discard;
    public bool strum_invisible;
    public GamepadButtons button;
    public Gamepad gamepad;
}

public class DDRKeysFIFO {
    public DDRKey[] queue;
    public int available;


    public void Purge() {
        Purge2(false);
    }

    public void Purge2(bool force_drop_first) {
        if (this.available < 1) return;

        //
        // important: disable SH4 interrupts first, this is a critical part
        //

        // int old_irq = irqdisable();

        int available = 0;

        for (int i = force_drop_first ? 1 : 0 ; i < this.available ; i++) {
            if (this.queue[i].discard) continue;

            if (i == available) {
                available++;
                continue;
            }

            this.queue[i] = this.queue[available];
            available++;

            // debugging only
            this.queue[i].in_song_timestamp = -2;
            this.queue[i].strum_id = -2;
            this.queue[i].button = 0;
        }

        this.available = available;

        // now restore the CPU interrupts
        // irqrestore(old_irq);
    }

}


public class DDRKeymon {

    public const byte FIFO_LENGTH = 64;


    private DDRKeysFIFO ddrkeys_fifo;
    private double start_timestamp;
    private Bind[] strum_binds;
    private int strum_binds_size;
    private kthread_t thd_monitor;
    private volatile int thd_monitor_active;
    private Gamepad gamepad;


    public DDRKeymon(Gamepad gamepad, GamepadButtons[] strum_binds, int strum_binds_size) {

        this.ddrkeys_fifo = new DDRKeysFIFO() {
            queue = new DDRKey[DDRKeymon.FIFO_LENGTH],
            available = 0
        };

        this.start_timestamp = 0;

        this.strum_binds = new Bind[strum_binds_size];
        this.strum_binds_size = strum_binds_size;

        this.thd_monitor = null;
        this.thd_monitor_active = 0;
        this.gamepad = gamepad;


        for (int i = 0 ; i < strum_binds_size ; i++) {
            uint count = Math2D.BitCount((uint)strum_binds[i]);
            this.strum_binds[i] = new Bind() {
                is_visible = true,
                button_flags = strum_binds[i],
                button_count = count,
                button_array = new GamepadButtons[count]
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
        if (this.thd_monitor != null) return;

        this.start_timestamp = timer.ms_gettime64() + offset_timestamp;

        // JS & C# only
        this.thd_monitor = new kthread_t();
    }

    public void Stop() {
        if (this.thd_monitor == null) return;

        Clear();

        // JS & C# only
        this.thd_monitor = null;
        this.thd_monitor_active++;
    }

    public double PeekTimestamp() {
        return timer.ms_gettime64() - this.start_timestamp;
    }

    public void Resync(double offset_timestamp) {
        this.start_timestamp = timer.ms_gettime64() + offset_timestamp;
    }

    public void PollCSJS() {
        //
        // This function only needs to be called in C# and JS version of the engine
        //
        InternalReadGamepad();
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

                InternalAppendKey(timestamp, i, button_flag, button_state_new != GamepadButtons.NOTHING);
            }
        }
    }

    private void InternalReadGamepad() {
        GamepadButtons old_buttons = this.gamepad.GetLastPressed();
        foreach (GamepadButtons new_buttons in this.gamepad.InternalUpdateState_JSCSHARP()) {
            if (old_buttons == new_buttons) continue;
            double timestamp = this.gamepad.InternalGetUpdateTimestamp_JSCSHARP();
            InternalProcessKey(timestamp, old_buttons, new_buttons);
            old_buttons = new_buttons;
        }
    }

    private class Bind {
        public bool is_visible;
        public GamepadButtons button_flags;
        public uint button_count;
        public GamepadButtons[] button_array;
    }

}
