using Engine.Utils;
using KallistiOS;

namespace Engine; 

public class BeatWatcher {

    public static double beatwatcher_global_timestamp = 0.0;

    public static void GlobalSetTimestamp(double timestamp) {
        beatwatcher_global_timestamp = timestamp;
    }

    public static void GlobalSetTimestampFromKosTimer() {
        beatwatcher_global_timestamp = timer.ms_gettime64();
    }


    public BeatWatcher() { }


    public bool count_beats_or_quarters;
    public float tick;
    public int count;
    public int drift_count;
    public double drift_timestamp;
    public float since;
    public int resyncs;
    public double last_global_timestamp;


    public void Reset(bool count_beats_or_quarters, float beats_per_minute) {
        this.count_beats_or_quarters = count_beats_or_quarters;
        this.tick = Math2D.BeatsPerMinuteToBeatPerMilliseconds(beats_per_minute);
        if (!count_beats_or_quarters) this.tick /= 8.0f;
        this.count = 0;
        this.drift_count = 0;
        this.drift_timestamp = 0.0;
        this.since = 0.0f;
        this.resyncs = 0;
        this.last_global_timestamp = beatwatcher_global_timestamp;
    }

    public void ChangeBpm(float beats_per_minute) {
        this.tick = Math2D.BeatsPerMinuteToBeatPerMilliseconds(beats_per_minute);
        this.drift_timestamp = beatwatcher_global_timestamp;
        this.drift_count += this.count;

        if (!this.count_beats_or_quarters) this.tick /= 8.0f;
    }

    public float RemainingUntilNext() {
        return (float)(((this.count + 1) * this.tick) - beatwatcher_global_timestamp);
    }

    public bool Poll() {
        // all beatwatcher instances are in sync with the global beatwatcher
        if (this.last_global_timestamp > beatwatcher_global_timestamp) {
            this.resyncs++;
            this.drift_timestamp -= this.last_global_timestamp - beatwatcher_global_timestamp;
            this.last_global_timestamp = beatwatcher_global_timestamp;
            this.since = 0f;
            return false;
        }

        int old_count = this.count;
        double timestamp = beatwatcher_global_timestamp - this.drift_timestamp;

        this.since = 0f;
        this.count = (int)(timestamp / this.tick);
        this.count += this.drift_count;
        this.last_global_timestamp = beatwatcher_global_timestamp;

        if (this.count > old_count) {
            this.since = (float)(timestamp - (this.tick * old_count) - this.tick);
            if (this.since < 0f) this.since = 0f;
            return true;
        }

        return false;
    }

}
