#ifndef _funkinsave_h
#define _funkinsave_h

#include <stdbool.h>
#include <stdlib.h>

#include "dataview_pack.h"
#include "float64.h"


#define SETTINGVALUE(KIND, VALUE) \
    (SettingValue) { .value_##KIND = VALUE }

#define funkinsave_set_setting_bool(ID, VALUE) funkinsave_set_setting(ID, SETTINGVALUE(bool, VALUE))
#define funkinsave_set_setting_int(ID, VALUE) funkinsave_set_setting(ID, SETTINGVALUE(int, VALUE))
#define funkinsave_set_setting_float(ID, VALUE) funkinsave_set_setting(ID, SETTINGVALUE(float, VALUE))
#define funkinsave_set_setting_byte(ID, VALUE) funkinsave_set_setting(ID, SETTINGVALUE(byte, VALUE))

#define funkinsave_get_setting_bool(ID, VALUE) funkinsave_get_setting(ID, SETTINGVALUE(bool, VALUE)).value_bool
#define funkinsave_get_setting_int(ID, VALUE) funkinsave_get_setting(ID, SETTINGVALUE(int, VALUE)).value_int
#define funkinsave_get_setting_float(ID, VALUE) funkinsave_get_setting(ID, SETTINGVALUE(float, VALUE)).value_float
#define funkinsave_get_setting_byte(ID, VALUE) funkinsave_get_setting(ID, SETTINGVALUE(byte, VALUE)).value_byte


typedef union Packed4bytes SettingValue;


extern bool funkinsave_has_changes;


int32_t funkinsave_read_from_vmu();
int32_t funkinsave_write_to_vmu();

int64_t funkinsave_get_week_score(const char* week_name, const char* week_difficult);
int32_t funkinsave_get_completed_count();
const char* funkinsave_get_last_played_week();
const char* funkinsave_get_last_played_difficult();

void funkinsave_create_unlock_directive(const char* name, float64 value);
void funkinsave_delete_unlock_directive(const char* name);
bool funkinsave_contains_unlock_directive(const char* name);
bool funkinsave_read_unlock_directive(const char* name, float64* value_output);

void funkinsave_set_last_played(const char* week_name, const char* difficulty_name);
void funkinsave_set_week_score(const char* week_name, const char* difficulty_name, bool only_if_best, int64_t score);

void funkinsave_set_setting(uint16_t setting_id, SettingValue setting_value);
SettingValue funkinsave_get_setting(uint16_t setting_id, SettingValue setting_value);

bool funkinsave_storage_set(const char* week_name, const char* name, const uint8_t* data, size_t data_size);
size_t funkinsave_storage_get(const char* week_name, const char* name, uint8_t** out_data);
void funkinsave_set_freeplay_score(const char* week_name, const char* difficulty_name, const char* song_name, bool only_if_best, int64_t score);
int64_t funkinsave_get_freeplay_score(const char* week_name, const char* difficulty_name, const char* song_name);

bool funkinsave_has_savedata_in_vmu(int8_t port, int8_t unit);
int32_t funkinsave_delete_from_vmu(int8_t port, int8_t unit);

void funkinsave_set_vmu(int8_t port, int8_t unit);
bool funkinsave_is_vmu_missing();
void funkinsave_pick_first_available_vmu();
bool funkinsave_get_vmu(int8_t* port, int8_t* unit);

#endif
