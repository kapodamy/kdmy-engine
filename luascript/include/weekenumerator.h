#ifndef __weekenumerator_h
#define __weekenumerator_h

typedef struct WeekInfoSong_t {
        const char* name;
        const char* freeplay_unlock_directive;
        const char* freeplay_song_filename;
        const char* freeplay_description;
        const bool freeplay_only;
} WeekInfoSong;

typedef struct WeekInfo_t {
    const char* name;
    const char* display_name;
    const char* description;
    const char* custom_folder;
    const char* unlock_directive;
    const char* emit_directive;
    const WeekInfoSong* songs;
    const int32_t songs_count;
} WeekInfo;

#endif