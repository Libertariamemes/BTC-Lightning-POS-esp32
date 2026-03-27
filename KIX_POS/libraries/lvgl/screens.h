#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *confirma_qr;
    lv_obj_t *obj0;
    lv_obj_t *confirma;
    lv_obj_t *m2;
    lv_obj_t *m2_1;
    lv_obj_t *m2_2;
    lv_obj_t *m2_3;
    lv_obj_t *m2_4;
    lv_obj_t *m2_5;
    lv_obj_t *m2_6;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_CONFIRMA_QR = 2,
};

void create_screen_main();
void tick_screen_main();

void create_screen_confirma_qr();
void tick_screen_confirma_qr();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/