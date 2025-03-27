#include <WiFiManager.h>
#include <FS.h>
#include <SPIFFS.h>
#include <lvgl.h>
#include "TAMC_GT911.h"
#include <ui.h>
#include <string>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <actions.h>
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 21
#define TOUCH_RST 25
#define TOUCH_WIDTH 320
#define TOUCH_HEIGHT 240
TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#include "qrcoded.h"
#endif
#define TFT_HOR_RES 240
#define TFT_VER_RES 320
#define TFT_ROTATION LV_DISPLAY_ROTATION_0
/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

uint32_t draw_buf[DRAW_BUF_SIZE / 20];
////////afeta dirretamente a heap/dram max o acima

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

bool welcome_screen;
bool continua=1;
int t;
float a;
float f;
double total_sats = 0;
bool flag_decimal=0;
bool b = 0;
bool c = 0;
bool d = 0;
const char* api_key = "3e97fe7f-44be-4fec-939c-e59a9342703b";//// Replace with your CoinAPI key
const char* api_url = "https://rest.coinapi.io/v1/exchangerate/BTC/USD";
const char* lnbits_url ="https://empathicvenison9.lnbits.com/api/v1/payments?api-key=230bf714edbd4e678b9d714c1046c1f3";
const char* lnbits_X_api_key="99d7c9ffb3c844a7b84d46d3b96dccc1";
const char* lnbits_api_key="230bf714edbd4e678b9d714c1046c1f3";

double cotacao=123456.78;
double casa_decimal=1;
char jsonOutput[1024];
char QR_LN_invoice[3000];






/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  /*Copy `px map` to the `area`*/

  /*For example ("my_..." functions needs to be implemented by you)
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    my_set_window(area->x1, area->y1, w, h);
    my_draw_bitmaps(px_map, w * h);
     */

  /*Call it to tell LVGL you are ready*/
  lv_display_flush_ready(disp);
}

//static lv_disp_draw_buf_t draw_buf;
//static lv_color_t buf[ DISPLAY_BUF_SIZE];
/*
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ){
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}*/





/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  tp.read();
  //int32_t x, y;
  bool touched = tp.isTouched;
  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;

    data->point.x = tp.points[0].x;
    data->point.y = tp.points[0].y;
    /*x=tp.points[0].x;
          y=tp.points[0].y;*/
    /*
      Serial.println("  x: ");Serial.print(tp.points[0].x);
      Serial.println("  y: ");Serial.print(tp.points[0].y);

      Serial.println("  X: ");Serial.print(x);
      Serial.println("  Y: ");Serial.print(y);*/

    Serial.print("  xx: ");
    Serial.println(data->point.x);
    Serial.print("  yy: ");
    Serial.println(data->point.y);
  }
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void) {
  return millis();
}

void setup() {


  Serial.begin(115200);

  Serial.println("Point of sale BITCOIN");
  
  tp.begin();
  tp.setRotation(ROTATION_INVERTED);
  //b=0;
 // displayBitcoinPrice();//limite diario de 100 chamadas por dia por limitacao do provedor da API
  tft.fillScreen(TFT_BLACK);
  lv_init();
  tft.fillScreen(TFT_BLACK);
  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
  tft.fillScreen(TFT_BLACK);

  /* register print function for debugging */
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  lv_display_t *disp;
#if LV_USE_TFT_ESPI
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);

#else
  /*Else create a display yourself*/
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);


  tp.points[0].y = 0;
  tp.points[0].x = 0;
  welcome_screen = 0;


 tft.fillScreen(TFT_BLACK);
  ui_init();
  a = 0;
ui_tick();

  
  //text_total_sats='a';
}

void loop() {


  if(continua){
    ui_tick();
    lv_timer_handler();
  setup_wifi();}

//create_LN_invoice();
  //BTC_POS_MAINLOOP();

    ui_tick();
    lv_timer_handler(); /* let the GUI do its work */
  delay(5); /* let this time pass */
  if (c) {
    ui_tick();
    lv_timer_handler(); /* let the GUI do its work */
  }
  if (a) {
    create_LN_invoice();
    qrShowCodeLNURL();
     
    delay(100);
    delay(100);
    a = 0;
    ui_tick();

    Serial.println(total_sats);
    //total_sats=0;
  }
  if (b) {
    b = 0;
    c = 0;  //setup_wifi();
    d = 1;  //reset wifi



    // ESP.restart();
  }
if(f){
 // create_LN_invoice();
  f=0;
  
}



}







////////////////////////definicoes de funçoes//////////////

void displayBitcoinPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_url);
    http.addHeader("X-CoinAPI-Key", api_key);

    int httpCode = http.GET(); // Make the request

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

       cotacao = doc["rate"];
      String date = doc["time"];

    } else {
      Serial.println("Error on HTTP request");  
    }
    http.end(); // End the request
  }  
}

void create_LN_invoice() {/////////cria invoice/////////////////////////////////////////////////
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(lnbits_url);
    http.addHeader("X-API-KEY", lnbits_X_api_key);
    http.addHeader("api-key", lnbits_api_key);
    http.addHeader("Content-Type","application/json");
    //jsonOutput=   ;
Serial.println("Lnbits"); 

    //serializeJson(doc, jsonOutput);

    String Post_invoice_request;
    Post_invoice_request=
    "{\"unit\": \"sat\",\"internal\": false,\"out\": false,\"amount\": "+
    String(total_sats)+", \"memo\": \"qq coisa\",\"expiry\": 0,\"extra\": {},\"webhook\": \"string\",\"bolt11\": \"string\",\"lnurl_callback\": \"string\"}";


        int httpCode = http.POST(String(Post_invoice_request)); // Make the request

/*
    int httpCode = http.POST(String("{\"unit\": \"sat\",\"internal\": false,\"out\": false,\"amount\": 33, \"memo\": \"qq coisa\",\"expiry\": 0,\"extra\": {},\"webhook\": \"string\",\"bolt11\": \"string\",\"lnurl_callback\": \"string\"}")); // Make the request
*/

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

        String invoice_LNbits = doc["payment_request"];

        

        invoice_LNbits.toCharArray(QR_LN_invoice,3000);
        


      Serial.println(QR_LN_invoice);
      

    } else {
      Serial.println("Error on HTTP request");  
    }
    http.end(); // End the request
  }  
}




void qrShowCodeLNURL()  //message
{
 // tft.fillScreen(TFT_WHITE);
  //qrData.toUpperCase();
 // const char *qrDataChar = qrData.c_str();

  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(80)];
  
  String LN_invoice_QR_local=String(QR_LN_invoice);
  
/*
//lnbc800n1pnuukhvdqdw9cjqcm0d9ekznp4qta3jzrm4rvk0lk7phwvmquf98f660xqttnwecan06fazkymf6tgspp5gxesazhycd6q04vp9dejpg6dhfxuynn5equ7u7pae6zynnt3r22ssp50g0ldfcrj8w8tuhz04qsgslu3xuku4glylh43y343sq9sek4n0jq9qyysgqcqpcxqyz5vqrzjqw9fu4j39mycmg440ztkraa03u5qhtuc5zfgydsv6ml38qd4azymlapyqqqqqqqtxuqqqqlgqqqq86qqjqs5tjqu2tjdr6p638mn7znushr00wq7tm63cmrx9zzt2hh3mm25rk2sp09ckfy9pcv7p2ld8xp4prhedhn9xjq8h2ejckxg7gm76skssq90a9l0
*/
Serial.println(QR_LN_invoice);  
Serial.println(LN_invoice_QR_local[4]);  
if(!(LN_invoice_QR_local[4]=='\0')){ //cehcagem se nao conectou e se a string do invoice esta vazia
  qrcode_initText(&qrcoded, qrcodeData, 14, 0, QR_LN_invoice);  //qrDataChar 
  for (uint8_t y = 0; y < qrcoded.size; y++) {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcoded.size; x++) {
      if (qrcode_getModule(&qrcoded, x, y)) {
        tft.fillRect(35 + 2 * x, 55 + 2 * y, 2, 2, TFT_BLACK);  //28 55
      } else {
        tft.fillRect(35 + 2 * x, 55 + 2 * y, 2, 2, TFT_WHITE);  //
      }
    }
  }
  tft.setCursor(10, 220);
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  }
  //tft.println(message);
}


void setup_wifi() {
  if (!c) {  // c so roda uma unica vez se nao resetado pelo botao cfg
  tft.fillScreen(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 120);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.println("CONECTANDO.");
  delay(276);
  tft.setCursor(0, 120);
  tft.println("CONECTANDO..");
  delay(276);
  tft.setCursor(0, 120);
  tft.println("CONECTANDO...");
  tft.setCursor(0, 160);
    tft.println("60 sec para  ");
    tft.setCursor(0, 200);
  tft.println("reabrir      ");
  tft.setCursor(0, 240);
  tft.println("config wifi ");

  delay(276);

    WiFiManager wm;

wm.setTimeout(60);
//wm.resetSettings();

    if (d) {
      wm.resetSettings();  //apenas reseta manualmente pelo botao config
          // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
      d = 0;
 ESP.restart();



    }  ///reseta password

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap

    res = wm.autoConnect("AutoConnectAP", "password");  // password protected ap

    if (!res) {
      Serial.println("Failed to connect");
      msg_portal_cfg();
      
      // ESP.restart();


    } else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
      tft.setCursor(0, 120);
        //tft.println("CONECTANDO...")
        tft.println("     OK!!    ");
          tft.setCursor(0, 160);
    tft.println("  conectado!  ");
    tft.setCursor(0, 200);
  tft.println("            ");
  tft.setCursor(0, 240);
  tft.println("            ");
          delay(50);
      ui_init();
    }
    c = 1;
    displayBitcoinPrice();
    
  }
  continua=0;
}



void msg_portal_cfg(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 60);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.println("CONFIGURE");
  tft.println("PELO CELULAR");
  tft.println("CELULAR");
  tft.println("------------");
  tft.println("CONECTANDO ");
  tft.println("NA REDE ");
  tft.println("AutoConnectAP");
  tft.println("SENHA ");
  tft.println("password");
  }
