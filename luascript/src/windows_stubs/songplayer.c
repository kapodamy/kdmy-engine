#include "commons.h"
#include "songplayer.h"

bool songplayer_changesong(SongPlayer songplayer, const char* src, bool prefer_no_copyright) {
    print_stub("songplayer_changesong", "songplayer=%p src=%s prefer_no_copyright=(bool)%i", songplayer, src, prefer_no_copyright);
    return 444;
}
void songplayer_play(SongPlayer songplayer, SongInfo_t* songinfo) {
    print_stub("songplayer_play", "songplayer=%p songinfo=%p", songplayer, songinfo);
}
void songplayer_pause(SongPlayer songplayer) {
    print_stub("songplayer_pause", "songplayer=%p", songplayer);
}
void songplayer_seek(SongPlayer songplayer, float timestamp) {
    print_stub("songplayer_seek", "songplayer=%p timestamp=%f", songplayer, timestamp);
}
float songplayer_get_duration(SongPlayer songplayer) {
    print_stub("songplayer_get_duration", "songplayer=%p", songplayer);
    return 123;
}
bool songplayer_is_completed(SongPlayer songplayer) {
    print_stub("songplayer_is_completed", "songplayer=%p", songplayer);
    return 999;
}
float songplayer_get_timestamp(SongPlayer songplayer) {
    print_stub("songplayer_get_timestamp", "songplayer=%p", songplayer);
    return 456;
}
void songplayer_mute_track(SongPlayer songplayer, bool vocals_or_instrumental, bool muted) {
    print_stub("songplayer_mute_track", "songplayer=%p vocals_or_instrumental=(bool)%i muted=(bool)%i", songplayer, vocals_or_instrumental, muted);
}
void songplayer_mute(SongPlayer songplayer, bool muted) {
    print_stub("songplayer_mute", "songplayer=%p muted=(bool)%i", songplayer, muted);
}
