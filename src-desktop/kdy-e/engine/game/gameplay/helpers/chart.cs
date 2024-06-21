using System;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Gameplay.Helpers;

public enum ChartEvent : int {
    NONE = 1,
    CAMERA_OPPONENT = 2,
    CAMERA_PLAYER = 3,
    CHANGE_BPM = 4,
    ALT_ANIM_OPPONENT = 5,
    ALT_ANIM_PLAYER = 6,
    UNKNOWN_NOTE = 7
}

public struct ChartNote {
    public double timestamp;
    public double direction;
    public double duration;
    public bool alt_anim;
    public double data;
}

public class ChartEntry {
    public ChartNote[] notes;
    public int notes_size;
}

public class ChartEventEntry : ICloneable {
    public double timestamp;
    public ChartEvent command;
    public double parameter;
    public double parameter2;
    public double parameter3;
    public bool is_player_or_opponent;

    public object Clone() => this.MemberwiseClone();
}


public class Chart {

    public readonly ChartEntry[] entries;
    public readonly int entries_size;

    public readonly ChartEventEntry[] events;
    public readonly int events_size;

    public readonly double speed;
    public readonly float bpm;


    public Chart(string src, string difficult) {
        bool is_normal = difficult == Funkin.DIFFICULT_NORMAL;
        int index = src.LastIndexOf(".json");
        if (index < 0) index = src.Length;

        string temp = StringUtils.Concat("-", difficult);
        difficult = temp.ToLowerInvariant();// funkin uses lowercase in chart filenames
        //free(temp);
        string chart_filename = StringUtils.CopyAndInsert(src, index, difficult);
        //free(difficult);

        JSONToken json;
        if (is_normal && FS.FileExists(src)) {
            json = JSONParser.LoadFrom(src);
        } else {
            json = JSONParser.LoadFrom(chart_filename);
        }

        if (json == null) throw new Exception("Missing chart file: " + chart_filename);
        //free(chart_filename);

        if (!JSONParser.HasPropertyObject(json, "song")) {
            throw new Exception("Unknown chart file, expecting 'funkin' format");
        }
        JSONToken song = JSONParser.ReadObject(json, "song");

        this.bpm = (float)JSONParser.ReadNumberDouble(song, "bpm", 100.0);
        this.speed = JSONParser.ReadNumberDouble(song, "speed", 1.0);

        JSONToken json_notes = JSONParser.ReadArray(song, "notes");
        int json_notes_length = JSONParser.ReadArrayLength(json_notes);

        // Important: charts with more than 4 directions (arrows) must include "directionCount"
        int direction_count = (int)JSONParser.ReadNumberLong(song, "directionCount", 4L);
        int unknown_notes = direction_count * 2;

        ArrayList<ChartNote> player_notes = new ArrayList<ChartNote>();
        ArrayList<ChartNote> opponent_notes = new ArrayList<ChartNote>();
        ArrayList<ChartEventEntry> chart_events = new ArrayList<ChartEventEntry>();

        bool? last_must_hit_section = null;
        bool last_alt_anim_player = false;
        bool last_alt_anim_opponent = false;
        float beat_duration = Math2D.BeatsPerMinuteToBeatPerMilliseconds(this.bpm);
        float quarter_duration = beat_duration / 4f;
        double timestamp_event_accumulated = 0;

        for (int i = 0 ; i < json_notes_length ; i++) {
            JSONToken json_section = JSONParser.ReadArrayItemObject(json_notes, i);

            // typeOfSection appears to be unused
            /*if (JSONParser.HasProperty(json, "typeOfSection")) {
                if (JSONParser.HasPropertyNumberLong(json, "typeOfSection")) {
                    long type_of_section = JSONParser.ReadArrayItemNumberLong(json_section, "typeOfSection", 0L);
                    if (type_of_section != 0L) {
                        Logger.Warn($"Unknown typeOfSection={type_of_section}");
                        continue;
                    }
                } else {
                    continue;
                }
            }*/

            long length_in_steps = JSONParser.ReadNumberLong(json_section, "lengthInSteps", 16L);
            bool must_hit_section = JSONParser.ReadBoolean(json_section, "mustHitSection", false);
            bool alt_anim = JSONParser.ReadBoolean(json_section, "altAnim", false);
            bool change_bpm = JSONParser.ReadBoolean(json_section, "changeBPM", false);
            double bpm = JSONParser.ReadNumberDouble(json_section, "bpm", 100.0);

            if (change_bpm) {
                beat_duration = Math2D.BeatsPerMinuteToBeatPerMilliseconds((float)bpm);
                quarter_duration = beat_duration / 4f;
            }

            double timestamp_event = timestamp_event_accumulated;
            timestamp_event_accumulated += quarter_duration * length_in_steps;

            JSONToken json_section_notes = JSONParser.ReadArray(json_section, "sectionNotes");
            int json_section_notes_length = JSONParser.ReadArrayLength(json_section_notes);

            for (int j = 0 ; j < json_section_notes_length ; j++) {
                bool set_camera = last_must_hit_section != must_hit_section;

                // this should have a size of 3 or 4
                JSONToken self_notes = JSONParser.ReadArrayItemArray(json_section_notes, j);
                int self_notes_length = JSONParser.ReadArrayLength(self_notes);

                if (self_notes_length < 3) {
                    Logger.Warn($"chart_init() section={i} note={j} has less than 3 fields");
                }

                double timestamp = JSONParser.ReadArrayItemNumberDouble(self_notes, 0, 0.0);
                double direction = JSONParser.ReadArrayItemNumberDouble(self_notes, 1, 0.0);
                double duration = JSONParser.ReadArrayItemNumberDouble(self_notes, 2, 0.0);
                double data;
                bool alt_note;

                // In Funkin v0.2.8 the value at index 3 can denote an alt animation
                if (JSONParser.GetArrayItemType(self_notes, 3) == JSONValueType.Boolean) {
                    alt_note = JSONParser.ReadArrayItemBoolean(self_notes, 3, false);
                    data = alt_note ? Double.PositiveInfinity : Double.NegativeInfinity;
                } else {
                    alt_note = false;
                    data = JSONParser.ReadArrayItemNumberDouble(self_notes, 3, 0.0);
                }

                if (set_camera) {
                    ChartEvent target_camera;
                    if (must_hit_section)
                        target_camera = ChartEvent.CAMERA_PLAYER;
                    else
                        target_camera = ChartEvent.CAMERA_OPPONENT;

                    set_camera = false;
                    last_must_hit_section = must_hit_section;

                    chart_events.Add(new ChartEventEntry() {
                        is_player_or_opponent = must_hit_section,
                        timestamp = timestamp_event,
                        command = target_camera,
                        parameter = 0.0,
                        parameter2 = 0.0
                    });
                }

                if (alt_anim != (must_hit_section ? last_alt_anim_player : last_alt_anim_opponent)) {
                    if (must_hit_section)
                        last_alt_anim_player = alt_anim;
                    else
                        last_alt_anim_opponent = alt_anim;

                    ChartEvent cam = must_hit_section ? ChartEvent.ALT_ANIM_PLAYER : ChartEvent.ALT_ANIM_OPPONENT;
                    chart_events.Add(new ChartEventEntry() {
                        timestamp = timestamp_event,
                        command = cam,
                        parameter = alt_anim ? 1.0 : 0.0,
                        parameter2 = 0.0
                    });
                }

                if (JSONParser.HasPropertyBoolean(json_section, "changeBPM") && change_bpm) {
                    chart_events.Add(new ChartEventEntry() {
                        timestamp = timestamp_event,
                        command = ChartEvent.CHANGE_BPM,
                        parameter = bpm,
                        parameter2 = 0.0
                    });
                }

                if (direction >= unknown_notes) {
                    // custom event data
                    Logger.Info(
                        "chart_init() custom direction detected will be used as event. " +
                        $"timestamp={timestamp} direction={direction} duration={duration} data={data}"
                    );

                    chart_events.Add(new ChartEventEntry() {
                        timestamp = timestamp,
                        command = ChartEvent.UNKNOWN_NOTE,
                        parameter = direction,
                        parameter2 = duration,
                        parameter3 = data
                    });
                    continue;
                }

                ArrayList<ChartNote> target;
                if (direction >= direction_count) {
                    // chour notes
                    target = must_hit_section ? opponent_notes : player_notes;
                    direction -= direction_count;
                } else {
                    // self notes
                    target = must_hit_section ? player_notes : opponent_notes;
                }
                target.Add(new ChartNote() { timestamp = timestamp, direction = direction, duration = duration, alt_anim = alt_note, data = data });
            }
        }

        this.entries_size = 0;
        if (opponent_notes.Size() > 0) this.entries_size++;
        if (player_notes.Size() > 0) this.entries_size++;

        int entry_index = 0;
        this.entries = new ChartEntry[this.entries_size];
        if (opponent_notes.Size() > 0) {
            this.entries[entry_index] = new ChartEntry() { notes = null, notes_size = 0 };
            opponent_notes.Destroy2(out this.entries[entry_index].notes_size, out this.entries[entry_index].notes);
            entry_index++;
        }
        if (player_notes.Size() > 0) {
            this.entries[entry_index] = new ChartEntry() { notes = null, notes_size = 0 };
            player_notes.Destroy2(out this.entries[entry_index].notes_size, out this.entries[entry_index].notes);
        }

        chart_events.Destroy2(out this.events_size, out this.events);
        JSONParser.Destroy(json);
    }

    public void Destroy() {
        //for (int i = 0; i < this.entries_size; i++) {
        //    free(this.entries[i].notes);
        //}

        //free(this.entries);
        //free(this.events);
        //free(this);
    }

}
