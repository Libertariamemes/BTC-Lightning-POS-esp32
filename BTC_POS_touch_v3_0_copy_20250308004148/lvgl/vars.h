#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_NONE
};

// Native global variables

extern float get_var_total_sats();
extern void set_var_total_sats(float value);
extern const char *get_var_sats_string_compose();
extern void set_var_sats_string_compose(const char *value);
extern float get_var_total_btc();
extern void set_var_total_btc(float value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/