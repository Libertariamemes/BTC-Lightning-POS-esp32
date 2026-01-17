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
#include <WiFiClientSecure.h>

/* --- TOUCH SCREEN CONFIGURATION --- */
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

/* --- DISPLAY CONFIGURATION --- */
#define TFT_HOR_RES 240
#define TFT_VER_RES 320
#define TFT_ROTATION LV_DISPLAY_ROTATION_0
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

uint32_t draw_buf[DRAW_BUF_SIZE / 20];

/* --- VIRTUAL SERIAL TOUCH EMULATION --- */
int virtual_x = 0;
int virtual_y = 0;
bool is_virtual_pressed = false;
uint32_t virtual_touch_timeout = 0;

/* --- GLOBAL STATE VARIABLES --- */
bool welcome_screen;
bool continua = 1;
bool payload_flag;
int t;
float a;
float f;
double total_sats = 0;
bool flag_decimal = 0;
bool b = 0;
bool c = 0;
bool d = 0;

/* --- API CONFIGURATION --- */
// NOTE: These should be stored in a secrets file for public repositories
const char* api_key = "3e97fe7f-44be-4fec-939c-e59a9342703b";//dont abuse this key, but use if you need
const char* api_url = "https://rest.coinapi.io/v1/exchangerate/BTC/USD";
const char* lnbits_url = "https://uvlnbits.libertariamemes.com.br/api/v1/payments";
const char* lnbits_X_api_key = "make your key at uvlnbits.libertariamemes.com.br";
const char* lnbits_api_key = "make your key at uvlnbits.libertariamemes.com.br";
const char* VPS_libertariamemes_URL = "http://libertariamemes.com.br:8071/send";// phyton on flask on a vps, but you can use the lnbits api directly see the phyton on the other folder

/* --- DATA VARIABLES --- */
double cotacao = 123456.78;
double casa_decimal = 1;
char jsonOutput[1024];
char QR_LN_invoice[3000];

/* --- PAYMENT POLLING VARIABLES --- */
String current_payment_hash = "";
bool waiting_for_payment = false;
uint32_t last_check_time = 0;
int check_count = 0;

/* --- LVGL DISPLAY CALLBACKS --- */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  lv_display_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  // Priority 1: Check for Serial-based Virtual Touch
  if (is_virtual_pressed) {
    if (millis() > virtual_touch_timeout) {
      is_virtual_pressed = false;
      data->state = LV_INDEV_STATE_RELEASED;
    } else {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = virtual_x;
      data->point.y = virtual_y;
      return; 
    }
  }

  // Priority 2: Physical Hardware Touch (GT911)
  tp.read();
  if (!tp.isTouched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = tp.points[0].x;
    data->point.y = tp.points[0].y;
    Serial.printf("HW Touch: x:%d y:%d\n", data->point.x, data->point.y);
  }
}

static uint32_t my_tick(void) {
  return millis();
}

/* --- MAIN SETUP --- */
void setup() {
  Serial.begin(115200);
  Serial.println("Bitcoin POS Terminal - Active");
  
  tp.begin();
  tp.setRotation(ROTATION_INVERTED);
  
  tft.fillScreen(TFT_BLACK);
  lv_init();
  lv_tick_set_cb(my_tick);

  lv_display_t *disp;
#if LV_USE_TFT_ESPI
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);
#endif

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();
  a = 0;
  ui_tick();
}

/* --- MAIN LOOP --- */
void loop() {
  // Serial Command Interface
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Virtual Touch Emulation (Format: T:x,y)
    if (input.startsWith("T:")) { 
      int commaIndex = input.indexOf(',');
      if (commaIndex != -1) {
        virtual_x = input.substring(2, commaIndex).toInt();
        virtual_y = input.substring(commaIndex + 1).toInt();
        is_virtual_pressed = true;
        virtual_touch_timeout = millis() + 350; 
        Serial.printf("Emulating Touch at: %d, %d\n", virtual_x, virtual_y);
      }
    }
    // Remote Invoice Generation (Format: I:amount)
    else if (input.startsWith("I:")) {
      double requested_amount = input.substring(2).toDouble();
      if (requested_amount > 0) {
        total_sats = requested_amount;
        a = 1; // Trigger flag for processing
        Serial.printf("Remote Request: Generating invoice for %.0f sats...\n", total_sats);
      } else {
        Serial.println("Error: Invalid amount provided.");
      }
    }
    // Manual Payment Verification Trigger (Format: P:)
    else if (input.startsWith("P:")) {
      Serial.println("Manual payment check initiated...");
      verify_payment();
    }
  }

  // Handle Initial Configuration and WiFi
  if (continua) {
    ui_tick();
    lv_timer_handler();
    setup_wifi();
  }

  ui_tick();
  
  // Payment Polling: Checks every 5 seconds if waiting for payment
  if (waiting_for_payment && (millis() - last_check_time > 5000)) {
    last_check_time = millis();
    verify_payment();
  }
  
  lv_timer_handler();
  delay(5);

  // Invoice Generation Logic
  if (a) {
    Serial.println("Contacting LNbits for Bolt11 Invoice...");
    create_LN_invoice();
    
    if (QR_LN_invoice[0] != '\0') {
      qrShowCodeLNURL();
      Serial.println("Invoice rendered to display.");
      Serial.print("Bolt11 Data: ");
      Serial.println(QR_LN_invoice);
    } else {
      Serial.println("System Error: Invoice generation failed.");
    }
    
    a = 0; // Reset invoice trigger
    ui_tick();
  }

  if (b) { b = 0; c = 0; d = 1; }
  if (f) { f = 0; }
}

/* --- WIFI MANAGEMENT --- */
void setup_wifi() {
  if (!c) {
    tft.fillScreen(TFT_WHITE);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setCursor(0, 120);
    tft.println("CONNECTING...");

    WiFiManager wm;
    wm.setTimeout(60); 

    if (d) {
      wm.resetSettings();
      d = 0;
      ESP.restart();
    }

    bool res = wm.autoConnect("AutoConnectAP", "password");

    if (!res) {
      Serial.println("WiFi Portal Timeout. Attempting backup credentials...");
      tft.fillScreen(TFT_WHITE);
      tft.setCursor(0, 120);
      tft.println("FAILSAFE...");
      
      WiFi.begin("SILVA2", "silva@21");
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }

      if (WiFi.status() != WL_CONNECTED) {
        msg_portal_cfg();
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Network Connection Established.");
      tft.setCursor(0, 120);
      tft.println("     OK!!    ");
      tft.setCursor(0, 160);
      tft.println("  Connected!  ");
      delay(500);
      ui_init();
    }
    c = 1;
    displayBitcoinPrice();
  }
  continua = 0;
}

/* --- PRICE UPDATES --- */
void displayBitcoinPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_url);
    http.addHeader("X-CoinAPI-Key", api_key);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      cotacao = doc["rate"];
    }
    http.end();
  }  
}

/* --- LIGHTNING INVOICE CREATION --- */
void create_LN_invoice() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(">>> NETWORK ERROR: WiFi disconnected.");
    return;
  }

  int maxRetries = 2;
  bool success = false;
  QR_LN_invoice[0] = '\0'; 

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    WiFiClient stdClient;
    HTTPClient http;
    http.setTimeout(40000); 
    http.begin(VPS_libertariamemes_URL);
    http.addHeader("Content-Type", "application/json");

    // Memo 'zelokasso' kept as per project logic
    String Post_invoice_request = "{\"out\": false, \"amount\":" + String(total_sats) + ", \"memo\": \"zelokasso\"}";
    
    if (attempt > 1) {
      Serial.printf("Retrying Invoice Request... (Attempt %d/%d)\n", attempt, maxRetries);
    }

    int httpCode = http.POST(Post_invoice_request);

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument outer(3000);
      deserializeJson(outer, payload);

      String stdoutStr = outer["stdout"];
      if (stdoutStr.length() > 0) {
        DynamicJsonDocument inner(2048);
        deserializeJson(inner, stdoutStr);

        if (inner.containsKey("bolt11")) {
          String invoice_LNbits = inner["bolt11"];
          invoice_LNbits.toCharArray(QR_LN_invoice, 3000);

          current_payment_hash = inner["payment_hash"].as<String>(); 
          waiting_for_payment = true;
          check_count = 0; 
          success = true; 
        }
      }

      const char* stderrStr = outer["stderr"];
      if (!success && stderrStr && strlen(stderrStr) > 0) {
        Serial.printf("Attempt %d Server Error: %s\n", attempt, stderrStr);
      }
    } else {
      Serial.printf("Attempt %d Connection Failed: %s\n", attempt, http.errorToString(httpCode).c_str());
    }

    http.end();
    if (success) break;
    delay(1000); 
  }

  if (!success) {
    Serial.println(">>> CRITICAL ERROR: Could not generate invoice.");
  }
}

/* --- QR CODE RENDERING --- */
void qrShowCodeLNURL() {
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(80)];
  String LN_invoice_QR_local = String(QR_LN_invoice);
  
  if (!(LN_invoice_QR_local[4] == '\0')) {
    qrcode_initText(&qrcoded, qrcodeData, 14, 0, QR_LN_invoice);
    for (uint8_t y = 0; y < qrcoded.size; y++) {
      for (uint8_t x = 0; x < qrcoded.size; x++) {
        if (qrcode_getModule(&qrcoded, x, y)) {
          tft.fillRect(35 + 2 * x, 55 + 2 * y, 2, 2, TFT_BLACK);
        } else {
          tft.fillRect(35 + 2 * x, 55 + 2 * y, 2, 2, TFT_WHITE);
        }
      }
    }
  }
}

/* --- CAPTIVE PORTAL UI MESSAGE --- */
void msg_portal_cfg() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 60);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.println("CONFIGURE");
  tft.println("VIA PHONE");
  tft.println("------------");
  tft.println("SSID: AutoConnectAP");
  tft.println("PASS: password");
}

/* --- PAYMENT VERIFICATION --- */
void verify_payment() {
  if (current_payment_hash == "" || WiFi.status() != WL_CONNECTED) return;

  // Manual Serial Override
  if (Serial.available() > 0) {
    char incoming = Serial.read();
    if (incoming == 'c' || incoming == 'C' || incoming == 'x' || incoming == 'X') {
      Serial.println("\n >>> MANUAL CANCELLATION VIA SERIAL <<<");
      waiting_for_payment = false;
      current_payment_hash = "";
      tft.fillScreen(TFT_BLACK);
      ui_init(); 
      return; 
    }
  }

  check_count++;
  Serial.printf("\n--- Checking Payment Status (%d/40) ---\n", check_count);

  WiFiClientSecure client;
  client.setInsecure(); 
  client.setHandshakeTimeout(45); 

  HTTPClient http;
  
  // NOTE: Target URL without trailing slash as required by VPS redirect policy
  http.begin(client, "https://libertariamemes.com.br/check_payment"); // phyton on flask on a vps, but you can use the lnbits api directly see the phyton on the other folder
  http.setTimeout(15000); 
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-POS-Terminal");

  String jsonReq = "{\"payment_hash\":\"" + current_payment_hash + "\"}";
  Serial.print("Checking Hash: "); Serial.println(current_payment_hash);

  int httpCode = http.POST(jsonReq);

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    if (doc["success"] == true && doc.containsKey("stdout")) {
      String stdoutStr = doc["stdout"];
      DynamicJsonDocument lnbitsResult(1024);
      deserializeJson(lnbitsResult, stdoutStr);

      if (lnbitsResult["paid"] == true) {
        Serial.println("🎉 PAYMENT CONFIRMED!");
        tft.fillScreen(TFT_GREEN);
        tft.setTextColor(TFT_BLACK);
        tft.setCursor(40, 100);
        tft.setTextSize(3);
        tft.println("PAID OK");
        
        waiting_for_payment = false;
        current_payment_hash = "";
        return;
      } else {
        Serial.println("Status: Unpaid.");
      }
    }
  } else {
    Serial.printf("Request Failed: %s (%d)\n", http.errorToString(httpCode).c_str(), httpCode);
  }
  
  http.end();

  // Timeout logic after 40 attempts (~3.3 minutes at 5s intervals)
  if (check_count >= 40) {
    Serial.println("Polling Timeout Reached.");
    waiting_for_payment = false;
    current_payment_hash = "";
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(20, 120);
    tft.println("TIMEOUT");
    delay(5000);
    ui_init(); 
  }
}
