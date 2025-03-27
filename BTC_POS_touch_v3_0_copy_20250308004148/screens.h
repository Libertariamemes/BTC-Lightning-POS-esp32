#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *config_wifi;
    lv_obj_t *config_tela;
    lv_obj_t *confirma_qr;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *preco_btc;
    lv_obj_t *obj2;
    lv_obj_t *confirma;
    lv_obj_t *m2;
    lv_obj_t *m2_1;
    lv_obj_t *m2_2;
    lv_obj_t *m2_3;
    lv_obj_t *m2_4;
    lv_obj_t *m2_5;
    lv_obj_t *m2_6;
    lv_obj_t *botao_ponto_decimal;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_CONFIG_WIFI = 2,
    SCREEN_ID_CONFIG_TELA = 3,
    SCREEN_ID_CONFIRMA_QR = 4,
};

void create_screen_main();
void tick_screen_main();

void create_screen_config_wifi();
void tick_screen_config_wifi();

void create_screen_config_tela();
void tick_screen_config_tela();

void create_screen_confirma_qr();
void tick_screen_confirma_qr();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/