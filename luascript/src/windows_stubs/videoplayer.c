#include "commons.h"
#include "videoplayer.h"

Sprite videoplayer_get_sprite(VideoPlayer videoplayer) {
    print_stub("videoplayer_get_sprite", "videoplayer=%p", videoplayer);
    return NULL;
}
void videoplayer_replay(VideoPlayer videoplayer) {
    print_stub("videoplayer_replay", "videoplayer=%p", videoplayer);
}
void videoplayer_play(VideoPlayer videoplayer) {
    print_stub("videoplayer_play", "videoplayer=%p", videoplayer);
}
void videoplayer_pause(VideoPlayer videoplayer) {
    print_stub("videoplayer_pause", "videoplayer=%p", videoplayer);
}
void videoplayer_stop(VideoPlayer videoplayer) {
    print_stub("videoplayer_stop", "videoplayer=%p", videoplayer);
}
void videoplayer_loop_enable(VideoPlayer videoplayer, bool enable) {
    print_stub("videoplayer_loop_enable", "videoplayer=%p enable=(bool)%i", videoplayer, enable);
}
void videoplayer_fade_audio(VideoPlayer videoplayer, bool in_or_out, float duration) {
    print_stub("videoplayer_fade_audio", "videoplayer=%p in_or_out=(bool)%i duration=%f", videoplayer, in_or_out, duration);
}
void videoplayer_set_volume(VideoPlayer videoplayer, float volume) {
    print_stub("videoplayer_set_volume", "videoplayer=%p volume=%f", videoplayer, volume);
}
void videoplayer_set_mute(VideoPlayer videoplayer, bool muted) {
    print_stub("videoplayer_set_mute", "videoplayer=%p muted=(bool)%i", videoplayer, muted);
}
void videoplayer_seek(VideoPlayer videoplayer, double timestamp) {
    print_stub("videoplayer_seek", "videoplayer=%p timestamp=%f", videoplayer, timestamp);
}
bool videoplayer_is_muted(VideoPlayer videoplayer) {
    print_stub("videoplayer_is_muted", "videoplayer=%p", videoplayer);
    return 0;
}
bool videoplayer_is_playing(VideoPlayer videoplayer) {
    print_stub("videoplayer_is_playing", "videoplayer=%p", videoplayer);
    return 0;
}
double videoplayer_get_duration(VideoPlayer videoplayer) {
    print_stub("videoplayer_get_duration", "videoplayer=%p", videoplayer);
    return 0;
}
double videoplayer_get_position(VideoPlayer videoplayer) {
    print_stub("videoplayer_get_position", "videoplayer=%p", videoplayer);
    return 0;
}
bool videoplayer_has_ended(VideoPlayer videoplayer) {
    print_stub("videoplayer_has_ended", "videoplayer=%p", videoplayer);
    return 0;
}
bool videoplayer_has_video_track(VideoPlayer videoplayer) {
    print_stub("videoplayer_has_video_track", "videoplayer=%p", videoplayer);
    return 0;
}
bool videoplayer_has_audio_track(VideoPlayer videoplayer) {
    print_stub("videoplayer_has_audio_track", "videoplayer=%p", videoplayer);
    return 0;
}
