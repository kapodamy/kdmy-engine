#include "commons.h"
#include "soundplayer.h"

void soundplayer_play(SoundPlayer soundplayer) {
    print_stub("soundplayer_play", "soundplayer=%p", soundplayer);
}
void soundplayer_pause(SoundPlayer soundplayer) {
    print_stub("soundplayer_pause", "soundplayer=%p", soundplayer);
}
void soundplayer_stop(SoundPlayer soundplayer) {
    print_stub("soundplayer_stop", "soundplayer=%p", soundplayer);
}
void soundplayer_loop_enable(SoundPlayer soundplayer, bool enable) {
    print_stub("soundplayer_loop_enable", "soundplayer=%p enable=(bool)%i", soundplayer, enable);
}
void soundplayer_fade(SoundPlayer soundplayer, bool in_or_out, float duration) {
    print_stub("soundplayer_fade", "soundplayer=%p in_or_out=(bool)%i duration=%f", soundplayer, in_or_out, duration);
}
void soundplayer_set_volume(SoundPlayer soundplayer, float volume) {
    print_stub("soundplayer_set_volume", "soundplayer=%p volume=%f", soundplayer, volume);
}
void soundplayer_set_mute(SoundPlayer soundplayer, bool muted) {
    print_stub("soundplayer_set_mute", "soundplayer=%p muted=(bool)%i", soundplayer, muted);
}
bool soundplayer_is_muted(SoundPlayer soundplayer) {
    print_stub("soundplayer_is_muted", "soundplayer=%p", soundplayer);
    return 123;
}
bool soundplayer_is_playing(SoundPlayer soundplayer) {
    print_stub("soundplayer_is_playing", "soundplayer=%p", soundplayer);
    return 456;
}
Fadding soundplayer_has_fading(SoundPlayer soundplayer) {
    print_stub("soundplayer_has_fading", "soundplayer=%p", soundplayer);
    return 0;
}
double soundplayer_get_duration(SoundPlayer soundplayer) {
    print_stub("soundplayer_get_duration", "soundplayer=%p", soundplayer);
    return 999;
}
double soundplayer_get_position(SoundPlayer soundplayer) {
    print_stub("soundplayer_get_position", "soundplayer=%p", soundplayer);
    return 100;
}
void soundplayer_seek(SoundPlayer soundplayer, double timestamp) {
    print_stub("soundplayer_get_position", "soundplayer=%p timestamp=%f", soundplayer, timestamp);
}
bool soundplayer_has_ended(SoundPlayer soundplayer) {
    print_stub("soundplayer_has_ended", "soundplayer=%p", soundplayer);
    return false;
}

