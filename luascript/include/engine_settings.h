#ifndef __engine_settings_h
#define __engine_settings_h

typedef struct {
    bool gameplay_enabled_distractions;
    bool gameplay_enabled_flashinglights;
    bool inverse_strum_scroll;
    bool song_progressbar;
} EngineSettings_t;


static EngineSettings_t SETTINGS = {};

#endif