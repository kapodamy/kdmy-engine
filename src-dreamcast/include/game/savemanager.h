#ifndef _savemanager_h
#define _savemanager_h

#include <stdbool.h>
#include <stdint.h>


typedef struct SaveManager_s* SaveManager;


SaveManager savemanager_init(bool save_only, int32_t error_code);
void savemanager_destroy(SaveManager* savemanager);

void savemanager_show(SaveManager savemanager);
int32_t savemanager_should_show(bool attempt_to_save_or_load);
void savemanager_check_and_save_changes();
bool savemanager_is_running_without_savedata();
void savemanager_change_actions(SaveManager savemanager, bool save_only, bool allow_delete, bool no_leave_confirm);

#endif
