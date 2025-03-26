#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H
#include <lvgl.h>
#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"
#include <string.h>

extern float a;
extern float f;
extern double total_sats;
extern bool flag_decimal;
extern bool continua;
extern char texto_satoshi;
extern double cotacao;
extern double casa_decimal;
char sats_string_compose[100]="";
char BTC_USD[100];
char display_btc[100] = "";
extern bool b;

 
////////////////char-strings////////////////////


const char *get_var_btc_usd() {
    return BTC_USD;
                                          }

void set_var_btc_usd(const char *value) {
    strncpy(BTC_USD, value, sizeof(BTC_USD) / sizeof(char));
    BTC_USD[sizeof(BTC_USD) / sizeof(char) - 1] = '\0';
                                        }





const char *get_var_sats_string_compose() {
    return sats_string_compose;
                                          }

void set_var_sats_string_compose(const char *value) {
    strncpy(sats_string_compose, value, sizeof(sats_string_compose) / sizeof(char));
    
    sats_string_compose[sizeof(sats_string_compose) / sizeof(char) - 1] = '\0';
                                                      }




const char *get_var_display_btc() {
    return display_btc;
                                          }

void set_var_display_btc(const char *value) {
    strncpy(display_btc, value, sizeof(display_btc) / sizeof(char));
    display_btc[sizeof(display_btc) / sizeof(char) - 1] = 0;
                                            }


//////////////////////////////FLOATS/DOUBLES////////////////////////////////////
float get_var_a() {
    return a;
                  }

void set_var_a(float value) {
    a = value;
                            }




double get_var_total_sats() {
    return total_sats;
                            }

void set_var_total_sats(double value) {
    total_sats = value;
                                      }

/////////////////////////////eventos tela/////////////////////////////////////////
void action_tela_1(lv_event_t *e) {

  loadScreen(SCREEN_ID_MAIN);

                                  }


void action_tela_2(lv_event_t *e) {
 

  loadScreen(SCREEN_ID_CONFIRMA_QR);
  //create_LN_invoice();

                                  }



void action_tela3(lv_event_t *e) {

  loadScreen(SCREEN_ID_CONFIG_TELA);

                                  }



 void action_go_main_page(lv_event_t * e){
  continua=1;
  loadScreen(SCREEN_ID_MAIN);

}



 void action_configura_wifi(lv_event_t * e){
  b=1;////seta flag do comando de resetar spiffs
continua=1; 
}






#ifdef __cplusplus
extern "C" {
#endif
///////////////////////ACOES E TECLAS
void action_reset(lv_event_t *e) {
  b=1; ////seta flag do comando de resetar spiffs
  continua=1;               
                                  
                                  
                                  }
void action_serial_teste(lv_event_t *e) {

  a++;

                                          }
void action_gera_qr_dentro(lv_event_t *e) {
  f++;
  a++;
                                            }
void action_apaga_tecla(lv_event_t * e){
  total_sats=0;
  casa_decimal=1;
  flag_decimal=0;
   //sprintf(display_btc, "%.8f", total_sats); 
   display_btc[0]='\0';
   sats_string_compose[0]='\0';
   
                                        }


void action_numero_zero_tecla(lv_event_t *e){
  size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '0';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
    } 
    total_sats=(total_sats*10)+0;
    sprintf(display_btc, "%.8f", total_sats/100000000); 
 if(total_sats>1000000000){
  total_sats=1000000000;
  sats_string_compose[0]='\0';
  strcpy(sats_string_compose, "1000000000");
  sprintf(display_btc, "%.8f", total_sats/100000000); 

                          }
                                            }

void action_numero_um_tecla(lv_event_t *e){
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '1';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
                                              } 
    total_sats=(total_sats*10)+1;
    sprintf(display_btc, "%.8f", total_sats/100000000); 

 if(total_sats>1000000000){
  total_sats=1000000000;
  sats_string_compose[0]='\0';
  strcpy(sats_string_compose, "1000000000");
  sprintf(display_btc, "%.8f", total_sats/100000000); 
   

                          }
                                          }

void action_numero_dois_tecla(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '2';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+2;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_tres(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '3';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+3;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_quatro(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '4';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+4;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_cinco(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '5';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+5;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_seis(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '6';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+6;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_sete(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '7';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+7;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_oito(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '8';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
        total_sats=(total_sats*10)+8;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }

void action_numero_nove(lv_event_t *e){ 
    size_t len = strlen(sats_string_compose);
    if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '9';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
       } 
       if(!flag_decimal){
        total_sats=(total_sats*10)+9;
        sprintf(display_btc, "%.8f", total_sats/100000000); 
                      }else{
                        casa_decimal=casa_decimal/10;
                        total_sats=total_sats+(9*casa_decimal);

                        sprintf(display_btc, "%.8f", total_sats/*/100000000*/); 
                      }
        if(total_sats>1000000000){
        total_sats=1000000000;
       sats_string_compose[0]='\0';
       strcpy(sats_string_compose, "1000000000");
      sprintf(display_btc, "%.8f", total_sats/100000000); 
  
                                  }


                                              }


action_ponto_decimal(lv_event_t * e){
  if(!flag_decimal){
  size_t len = strlen(sats_string_compose);
  flag_decimal=1;
  if (len < sizeof(sats_string_compose) - 1) {
        sats_string_compose[len] = '.';  // Add '!' at the end
        sats_string_compose[len + 1] = '\0';  // Make sure the string is null-terminated
        //casa_decimal=1;
       } 
  }

}                                              
/////fim numeros 

void action_cotacao_usd_btc(lv_event_t *e){ 

      sprintf(BTC_USD, "%.2f", cotacao); 

                                                    }

                                                    





#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/
