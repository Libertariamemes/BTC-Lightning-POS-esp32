#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_serial_teste(lv_event_t * e);
extern void action_gera_qr_dentro(lv_event_t * e);
extern void action_tela_2(lv_event_t * e);
extern void action_tela_1(lv_event_t * e);
extern void action_numero_um_tecla(lv_event_t * e);
extern void action_apaga_tecla(lv_event_t * e);
extern void action_numero_zero_tecla(lv_event_t * e);
extern void action_numero_dois_tecla(lv_event_t * e);
extern void action_numero_tres(lv_event_t * e);
extern void action_numero_quatro(lv_event_t * e);
extern void action_numero_cinco(lv_event_t * e);
extern void action_numero_seis(lv_event_t * e);
extern void action_numero_oito(lv_event_t * e);
extern void action_numero_sete(lv_event_t * e);
extern void action_numero_nove(lv_event_t * e);
extern void action_reset(lv_event_t * e);
extern void action_tela3(lv_event_t * e);
extern void action_cotacao_usd_btc(lv_event_t * e);
extern void action_ponto_decimal(lv_event_t * e);
extern void action_go_main_page(lv_event_t * e);
extern void action_configura_wifi(lv_event_t * e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/