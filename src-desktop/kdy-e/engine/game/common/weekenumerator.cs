using System;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game.Common;

public class WeekInfo {

    public class UnlockableCharacter {
        public bool hide_if_locked;
        public string unlock_directive;
        public string manifest;
        public string name;
    }
    public class CustomDifficult {
        public string name;
        public string unlock_directive;
    }
    public class Song {
        public string name;
        public string freeplay_host_icon_model;
        public string freeplay_host_icon_name;
        public string freeplay_locked_host_icon_model;
        public string freeplay_locked_host_icon_name;
        public string freeplay_background;
        public bool freeplay_only;
        public string freeplay_unlock_directive;
        public bool freeplay_hide_if_week_locked;
        public bool freeplay_hide_if_locked;
        public string freeplay_gameplaymanifest;
        public int freeplay_song_index_in_gameplaymanifest;
        public string freeplay_song_filename;
        public string freeplay_description;
        public float freeplay_seek_time;
    }
    public class Unlockables {
        public UnlockableCharacter[] boyfriend_models;
        public int boyfriend_models_size;
        public UnlockableCharacter[] girlfriend_models;
        public int girlfriend_models_size;
    }


    public string name;
    public string display_name;
    public string description;
    public string week_title_model;
    public string week_title_model_animation_name;
    public bool host_flip_sprite;
    public bool host_enable_beat;
    public bool host_hide_if_week_locked;
    public string week_host_character_manifest;
    public string week_host_model;
    public string week_host_model_idle_animation_name;
    public string week_host_model_choosen_animation_name;
    public Song[] songs;
    public int songs_count;
    public uint selector_background_color;
    public bool selector_background_color_override;
    public bool has_difficulty_easy;
    public bool has_difficulty_normal;
    public bool has_difficulty_hard;
    public string custom_difficults_model;
    public CustomDifficult[] custom_difficults;
    public int custom_difficults_size;
    public string default_difficulty;
    public string unlock_directive;
    public string emit_directive;
    public string warning_message;
    public string sensible_content_message;
    public bool disallow_custom_boyfriend;
    public bool disallow_custom_girlfriend;
    public Unlockables unlockables;
    public string custom_selector_layout;
    public string custom_folder;
    public string custom_folder_gameplay;
    public bool has_greetings;
    public string songs_default_freeplay_host_icon_model;
}

public struct WeekArray {
    public int size;
    public WeekInfo[] array;
}

public static class WeekEnumerator {

    public static void Enumerate() {
        FS.FolderStackPush();
        FS.SetWorkingFolder(Funkin.WEEKS_FOLDER, false);

        FSFolderEnumerator folder_enumerator = new FSFolderEnumerator();
        string weeks_path = Expansions.overrided_weeks_folder ?? Funkin.WEEKS_FOLDER;

        if (!FS.FolderEnumerate(weeks_path, ref folder_enumerator)) {
            Logger.Error($"weekenumerator_enumerate() can not read {weeks_path}");
            FS.FolderStackPop();
            return;
        }

        LinkedList<WeekInfo> parsed_weeks = new LinkedList<WeekInfo>();

        while (FS.FolderEnumerateNext(ref folder_enumerator)) {
            if (folder_enumerator.is_file) continue;
            string folder_name = folder_enumerator.name;

            FS.SetWorkingSubfolder(folder_name);
            WeekInfo parsed_week = ParseWeek(folder_name);
            if (parsed_week != null) parsed_weeks.AddItem(parsed_week);
            FS.SetWorkingFolder(Funkin.WEEKS_FOLDER, false);
        }

        FS.FolderEnumerateClose(ref folder_enumerator);

        Funkin.weeks_array.size = parsed_weeks.Count();
        Funkin.weeks_array.array = parsed_weeks.ToArray();

        // In C all string on the list must be disposed
        parsed_weeks.Destroy2(/*free*/);
        FS.FolderStackPop();
    }

    public static string GetHostModel(WeekInfo weekinfo) {
        return StringUtils.Concat(
            Funkin.WEEKS_FOLDER, weekinfo.name, FS.CHAR_SEPARATOR.ToString(), Funkin.WEEK_HOST_FILENAME
        );
    }

    public static string GetTitleTexture(WeekInfo weekinfo) {
        return StringUtils.Concat(
            Funkin.WEEKS_FOLDER, weekinfo.name, FS.CHAR_SEPARATOR.ToString(), Funkin.WEEK_TITLE_FILENAME
        );
    }

    public static string GetWeekFolder(WeekInfo weekinfo) {
        return StringUtils.Concat(Funkin.WEEKS_FOLDER, weekinfo.name);
    }

    /*public static string GetCustomFolder(WeekInfo weekinfo) {
        return StringUtils.Concat(
            Funkin.WEEKS_FOLDER, weekinfo.name, FS.CHAR_SEPARATOR.ToString(), Funkin.WEEK_CUSTOM_FOLDER
        );
    }*/

    public static string GetGameplayManifest(WeekInfo weekinfo) {
        return StringUtils.Concat(
            Funkin.WEEKS_FOLDER, weekinfo.name, FS.CHAR_SEPARATOR.ToString(), Funkin.WEEK_GAMEPLAY_MANIFEST
        );
    }

    public static string GetGreetings(WeekInfo weekinfo) {
        if (weekinfo.has_greetings) {
            return StringUtils.Concat(
                Funkin.WEEKS_FOLDER, weekinfo.name, FS.CHAR_SEPARATOR.ToString(), Funkin.WEEK_GREETINGS_FILE
            );
        }
        return null;
    }

    public static string GetAsset(WeekInfo weekinfo, string relative_path) {
        return StringUtils.Concat(
            Funkin.WEEKS_FOLDER, weekinfo.name, FS.CHAR_SEPARATOR.ToString(), relative_path
        );
    }

    private static WeekInfo ParseWeek(string week_name) {
        WeekInfo week_parsed = null;

        JSONToken json = JSONParser.LoadFrom(Funkin.WEEK_ABOUT_FILE);

        if (!JSONParser.HasPropertyArray(json, "songs")) {
            Logger.Error("weekenumerator_parse_week() missing or invalid songs in week: " + week_name);
            goto L_failed;
        }

        bool has_difficulty_easy = JSONParser.ReadBoolean(json, "hasDifficultyEasy", true);
        bool has_difficulty_normal = JSONParser.ReadBoolean(json, "hasDifficultyNormal", true);
        bool has_difficulty_hard = JSONParser.ReadBoolean(json, "hasDifficultyHard", true);

        JSONToken customdifficulties_array = JSONParser.ReadArray(json, "customDifficulties");
        int customdifficulties_array_size = JSONParser.ReadArrayLength(customdifficulties_array);

        if (!has_difficulty_easy &&
            !has_difficulty_normal &&
            !has_difficulty_hard &&
            customdifficulties_array_size < 1) {
            Logger.Error("weekenumerator_parse_week() There no difficults in week: " + week_name);
            goto L_failed;
        }

        string customdifficults_model = ParsePath(json, "customDifficultsModel");
        if (StringUtils.IsEmpty(customdifficults_model) && customdifficulties_array_size > 0) {
            //free(customdifficults_model);
            Logger.Error("weekenumerator_parse_week() Missing or invalid custom difficult model");
            goto L_failed;
        }

        // parse all custom difficults
        WeekInfo.CustomDifficult[] customdifficults = null;
        if (customdifficulties_array_size > 0) {
            customdifficults = new WeekInfo.CustomDifficult[customdifficulties_array_size];
            for (int i = 0 ; i < customdifficulties_array_size ; i++) {
                JSONToken customdifficult_obj = JSONParser.ReadArrayItemObject(customdifficulties_array, i);

                customdifficults[i] = new WeekInfo.CustomDifficult() {
                    name = JSONParser.ReadString(customdifficult_obj, "name", null),
                    unlock_directive = JSONParser.ReadString(customdifficult_obj, "unlockDirective", null)
                };

                if (StringUtils.IsEmpty(customdifficults[i].name)) {
                    Logger.Error($"weekenumerator_parse_week() difficult name in week: {week_name}");
                } else if (FS.IsInvalidFilename(customdifficults[i].name)) {
                    Logger.Error($"weekenumerator_parse_week() forbidden difficult name: {customdifficults[i].name}");
                    //free(customdifficults[i].name);
                    customdifficults[i].name = null;
                }
            }
        } else {
            customdifficulties_array_size = 0;
        }

        JSONToken json_songs = JSONParser.ReadArray(json, "songs");
        int json_songs_size = JSONParser.ReadArrayLength(json_songs);
        WeekInfo.Song[] songs = new WeekInfo.Song[json_songs_size];
        for (int i = 0 ; i < json_songs_size ; i++) {
            JSONToken json_song = JSONParser.ReadArrayItemObject(json_songs, i);
            songs[i] = new WeekInfo.Song() {
                name = JSONParser.ReadString(json_song, "name", null),
                freeplay_host_icon_model = ParsePath(json_song, "freeplayHostIconModel"),
                freeplay_host_icon_name = JSONParser.ReadString(json_song, "freeplayHostIconName", null),
                freeplay_locked_host_icon_model = ParsePath(json_song, "freeplayLockedHostIconModel"),
                freeplay_locked_host_icon_name = JSONParser.ReadString(json_song, "freeplayLockedHostIconName", null),
                freeplay_background = JSONParser.ReadString(json_song, "freeplayBackground", null),
                freeplay_only = JSONParser.ReadBoolean(json_song, "freeplayOnly", false),
                freeplay_unlock_directive = JSONParser.ReadString(json_song, "freeplayUnlockDirective", null),
                freeplay_hide_if_week_locked = JSONParser.ReadBoolean(json_song, "freeplayHideIfWeekLocked", false),
                freeplay_hide_if_locked = JSONParser.ReadBoolean(json_song, "freeplayHideIfLocked", false),
                freeplay_gameplaymanifest = JSONParser.ReadString(json_song, "freeplayGameplayManifest", null),
                freeplay_song_index_in_gameplaymanifest = (int)JSONParser.ReadNumberLong(json_song, "freeplaySongIndexInGameplayManifest", -1),
                freeplay_song_filename = ParsePath(json_song, "freeplaySongFilename"),
                freeplay_description = JSONParser.ReadString(json_song, "freeplayDescription", null),
                freeplay_seek_time = (float)JSONParser.ReadNumberDouble(json_song, "freeplaySeekTime", Double.NaN)
            };
        }

        JSONToken unlockables_json = JSONParser.ReadObject(json, "unlockables");
        JSONToken boyfriend_array = JSONParser.ReadArray(unlockables_json, "boyfriend");
        JSONToken girlfriend_array = JSONParser.ReadArray(unlockables_json, "girlfriend");

        week_parsed = new WeekInfo() {
            name = week_name,
            display_name = JSONParser.ReadString(json, "displayName", null),
            description = JSONParser.ReadString(json, "description", null),
            week_title_model = ParsePath(json, "weekTitleModel"),
            week_title_model_animation_name = JSONParser.ReadString(json, "weekTitleModelAnimationName", null),
            host_flip_sprite = JSONParser.ReadBoolean(json, "hostFlipSprite", false),
            host_enable_beat = JSONParser.ReadBoolean(json, "hostEnableBeat", true),
            host_hide_if_week_locked = JSONParser.ReadBoolean(json, "hostHideIfWeekLocked", false),
            week_host_character_manifest = ParsePath(json, "hostCharacterManifest"),
            week_host_model = ParsePath(json, "hostModel"),
            week_host_model_idle_animation_name = JSONParser.ReadString(json, "hostModelIdleAnimationName", null),
            week_host_model_choosen_animation_name = JSONParser.ReadString(json, "hostModelChoosenAnimationName", null),
            songs = songs,
            songs_count = json_songs_size,
            selector_background_color = JSONParser.ReadHex(json, "selectorBackgroundColor", 0x000000),
            selector_background_color_override = JSONParser.HasPropertyHex(json, "selectorBackgroundColor"),
            has_difficulty_easy = has_difficulty_easy,
            has_difficulty_normal = has_difficulty_normal,
            has_difficulty_hard = has_difficulty_hard,
            custom_difficults_model = customdifficults_model,
            custom_difficults = customdifficults,
            custom_difficults_size = customdifficulties_array_size,
            default_difficulty = JSONParser.ReadString(json, "customDifficultyDefault", null),
            unlock_directive = JSONParser.ReadString(json, "unlockDirectiveName", null),
            emit_directive = JSONParser.ReadString(json, "emitUnlockDirectiveName", null),
            warning_message = JSONParser.ReadString(json, "warningMessage", null),
            sensible_content_message = JSONParser.ReadString(json, "sensibleContentMessage", null),
            disallow_custom_boyfriend = JSONParser.ReadBoolean(json, "disallowCustomBoyfriend", false),
            disallow_custom_girlfriend = JSONParser.ReadBoolean(json, "disallowCustomGirlfriend", false),
            unlockables = new WeekInfo.Unlockables() {
                boyfriend_models = ParseCharacters(boyfriend_array),
                boyfriend_models_size = Math.Max(JSONParser.ReadArrayLength(boyfriend_array), 0),
                girlfriend_models = ParseCharacters(girlfriend_array),
                girlfriend_models_size = Math.Max(JSONParser.ReadArrayLength(girlfriend_array), 0)
            },
            custom_selector_layout = ParsePath(json, "customSelectorLayout"),
            custom_folder = ParsePath(json, "customFolder"),
            custom_folder_gameplay = ParsePath(json, "inGameplayCustomFolder"),
            has_greetings = FS.FileExists(Funkin.WEEK_GREETINGS_FILE),
            songs_default_freeplay_host_icon_model = ParsePath(json, "songsDefaultFreeplayHostIconModel")
        };


        if (week_parsed.unlockables.boyfriend_models_size > 0 && week_parsed.unlockables.boyfriend_models == null) {
            week_parsed.unlockables.boyfriend_models_size = 0;
        }

        if (week_parsed.unlockables.girlfriend_models_size > 0 && week_parsed.unlockables.girlfriend_models == null) {
            week_parsed.unlockables.girlfriend_models_size = 0;
        }

        JSONParser.Destroy(json);
        return week_parsed;

L_failed:
        JSONParser.Destroy(json);
        return null;
    }

    public static string ParsePath(JSONToken json, string json_property_name) {
        string path = JSONParser.ReadString(json, json_property_name, null);
        if (path == null) return null;
        if (path.Length < 1) {
            Logger.Error($"weekenumerator_parse_path() invalid '{json_property_name}' found");
            //free(path);
            return null;
        }

        if (json_property_name == "freeplaySongFilename") {
            return SongPlayer.HelperGetTracksFullPath(path);
        }

        if (path.IndexOf(FS.CHAR_SEPARATOR_REJECT, 0) >= 0) {
            Logger.Error($"weekenumerator_parse_path() invalid char separator in: {path}");
            //free(path);
            return null;
        }

        if (path.StartsWithKDY(FS.CHAR_SEPARATOR.ToString(), 0) || path.StartsWithKDY(FS.NO_OVERRIDE_COMMON, 0)) {
            return path;// absolute path
        }

        return FS.GetFullPath(path);
    }

    public static WeekInfo.UnlockableCharacter[] ParseCharacters(JSONToken array_json) {
        int array_json_length = JSONParser.ReadArrayLength(array_json);
        if (array_json_length < 1) return null;

        WeekInfo.UnlockableCharacter[] array = new WeekInfo.UnlockableCharacter[array_json_length];

        for (int i = 0 ; i < array_json_length ; i++) {
            JSONToken array_item = JSONParser.ReadArrayItemObject(array_json, i);
            array[i] = new WeekInfo.UnlockableCharacter() {
                hide_if_locked = JSONParser.ReadBoolean(array_item, "hideIfLocked", false),
                unlock_directive = JSONParser.ReadString(array_item, "unlockDirectiveName", null),
                manifest = ParsePath(array_item, "manifest"),
                name = JSONParser.ReadString(array_item, "name", null),
            };
            if (StringUtils.IsEmpty(array[i].name) && StringUtils.IsEmpty(array[i].manifest)) {
                Logger.Warn("weekenumerator_parse_character() missing 'name' and/or 'manifest'");
                //for (; i >= 0; i--) {
                    //free(array[i].unlock_directive);
                    //free(array[i].name);
                    //free(array[i].manifest);
                //}
                //free(array);
                return null;
            }
        }

        return array;
    }

}
