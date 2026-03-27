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
volatile bool cancel_verification_requested = false;
//unsigned long last_check_time = 0;
const unsigned long CHECK_INTERVAL = 3000; // 3 seconds



/* --- API CONFIGURATION --- */
// NOTE: These should be stored in a secrets file for public repositories
const char* api_key = "3e97fe7f-44be-4fec-939c-e59a9342703b";//dont abuse this key, but use if you need
const char* api_url = "https://rest.coinapi.io/v1/exchangerate/BTC/USD";
//const char* lnbits_url = "https://uvlnbits.libertariamemes.com.br/api/v1/payments";
//const char* lnbits_X_api_key = "make your key at uvlnbits.libertariamemes.com.br";
//const char* lnbits_api_key = "make your key at uvlnbits.libertariamemes.com.br";
// Agora são Strings mutáveis na RAM
String lnbits_url = "http://34.226.196.147:5004/api/v1/payments";
String lnbits_api_key = "2b273ca3d3a84db4a3857ff54776aa96"; 
String lnbits_X_api_key = "2b273ca3d3a84db4a3857ff54776aa96";





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
  // Priority 1: Virtual Touch
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

  // Priority 2: Physical Touch
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
/* --- MAIN SETUP --- */
void setup() {
  Serial.begin(115200);
  delay(500); // Aguarda a serial estabilizar para debug inicial
  
  // 1. Inicia SPIFFS com tentativa de montagem limpa
  if(!SPIFFS.begin(true)){
    Serial.println("Erro ao montar SPIFFS");
  } else {
    // Carrega configurações salvas, se existirem
    if(SPIFFS.exists("/config.txt")){
      File f = SPIFFS.open("/config.txt", "r");
      if(f && f.size() > 10){ 
        f.setTimeout(500); // Proteção contra leitura infinita se o arquivo estiver corrompido
        
        String url_raw = f.readStringUntil('\n');
        String key_raw = f.readStringUntil('\n');
        
        url_raw.trim();
        key_raw.trim();
        
        // Sanity Check: Só aplica se a URL parecer legítima (evita injetar lixo na RAM)
        if(url_raw.startsWith("http")) {
            lnbits_url = url_raw;
            lnbits_api_key = key_raw;
            lnbits_X_api_key = lnbits_api_key;
            Serial.println(">>> SPIFFS: Configs aplicadas com sucesso.");
        } else {
            Serial.println(">>> SPIFFS: Conteudo invalido detectado. Mantendo padrao.");
        }
        f.close();
      }
    }
    // O PASSO CRUCIAL: Desmontar a SPIFFS antes de iniciar o display/touch
    // para liberar o barramento e evitar o erro de spinlock
    SPIFFS.end(); 
  }

  // 2. Delay de estabilização do hardware (Dá tempo para o controlador de Flash "relaxar")
  delay(200); 
  yield();

  Serial.println("Bitcoin POS Terminal - Active");

  // 3. Inicialização de Periféricos (Display e Touch)
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
    // Configuração de WiFi via Serial (Formato: W:SSID,SENHA)
    else if (input.startsWith("W:")) {
      int commaIndex = input.indexOf(',');
      if (commaIndex != -1) {
        String new_ssid = input.substring(2, commaIndex);
        String new_pass = input.substring(commaIndex + 1);
        
        new_ssid.trim();
        new_pass.trim();

        Serial.printf("Tentando conectar ao WiFi: %s\n", new_ssid.c_str());
        
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 100);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.println("Conectando via Serial...");
        tft.println(new_ssid);

        WiFi.disconnect(); // Desconecta da rede atual
        WiFi.begin(new_ssid.c_str(), new_pass.c_str());

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 25) {
          delay(500);
          Serial.print(".");
          attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nConectado com sucesso!");
          tft.println("OK! Conectado.");
          delay(1000);
          displayBitcoinPrice(); // Atualiza o preço com a nova rede
          ui_init(); // Volta para a tela principal
        } else {
          Serial.println("\nFalha na conexao. Verifique os dados.");
          tft.fillScreen(TFT_RED);
          tft.setCursor(20, 120);
          tft.println("ERRO WIFI");
          delay(2000);
          ui_init();
        }       
        }
    }
else if (input.startsWith("U:")) {
      int commaIndex = input.indexOf(',');
      if (commaIndex != -1) {
        // Interrompe qualquer polling para evitar crash de memória
        waiting_for_payment = false; 

        lnbits_url = input.substring(2, commaIndex);
        lnbits_api_key = input.substring(commaIndex + 1);
        
        lnbits_url.trim();
        lnbits_api_key.trim();
        lnbits_X_api_key = lnbits_api_key;

        Serial.println("\n>>> CONFIGURAÇÃO ATUALIZADA <<<");
        Serial.print("URL: "); Serial.println(lnbits_url);
        Serial.print("KEY: "); Serial.println(lnbits_api_key);

        tft.fillScreen(TFT_BLUE);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, 80);
        tft.setTextSize(2);
        tft.println("URL & KEY");
        tft.println("UPDATED OK");
        
        delay(1500);
        ui_init(); 
      } else {
        Serial.println("Erro: Use U:url,key");
      }
    }
    else if (input.equalsIgnoreCase("S:")) {
      Serial.println("\n========= POS STATUS =========");
      Serial.print("WiFi:  "); Serial.println(WiFi.status() == WL_CONNECTED ? "CONECTADO" : "DESCONECTADO");
      Serial.print("URL:   "); Serial.println(lnbits_url);
      Serial.print("Key:   "); Serial.println(lnbits_api_key);
      Serial.print("Sats:  "); Serial.println(total_sats);
      Serial.print("Pagando? "); Serial.println(waiting_for_payment ? "SIM" : "NAO");
      Serial.println("==============================");
    }
    // Comando R: Reiniciar o ESP32
    else if (input.equalsIgnoreCase("R:")) {
      Serial.println("Reiniciando sistema...");
      delay(500);
      ESP.restart();
    }

    // Comando Z: Salvar URL, KEY e WiFi na memória permanente
// Comando Z: Salvar URL, KEY e WiFi na memória permanente
    else if (input.equalsIgnoreCase("Z:")) {
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(10, 100);
      tft.setTextSize(2);
      tft.println("GRAVANDO FLASH...");
      tft.println("NAO DESLIGUE");
      
      hardSafeSave(); // Chama o travamento e reinicia o chip
    }
    // Comando Q: Reiniciar sem gravar (limpeza de RAM)
    else if (input.equalsIgnoreCase("Q:")) {
      Serial.println("\n[!] REINICIANDO SISTEMA (LIMPEZA DE RAM)...");
      
      // Feedback visual rápido
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(10, 100);
      tft.setTextColor(TFT_WHITE);
      tft.println("REINICIANDO...");
      
      delay(500);
      ESP.restart();
    }
    // Comando L: Listar arquivos na SPIFFS
    else if (input.equalsIgnoreCase("L:")) {
      Serial.println("\n--- ARQUIVOS NA SPIFFS ---");
      if (!SPIFFS.begin(true)) {
        Serial.println("Erro ao acessar SPIFFS");
      } else {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file) {
          Serial.print("FILE: ");
          Serial.print(file.name());
          Serial.print(" | SIZE: ");
          Serial.println(file.size());
          file = root.openNextFile();
        }
        SPIFFS.end();
      }
      Serial.println("--------------------------");
    }

    // Comando F: Verificar espaço e saúde da memória
    else if (input.equalsIgnoreCase("F:")) {
      if (!SPIFFS.begin(true)) {
        Serial.println("Erro ao acessar SPIFFS");
      } else {
        size_t total = SPIFFS.totalBytes();
        size_t used = SPIFFS.usedBytes();
        Serial.println("\n--- SAUDE DA MEMORIA FLASH ---");
        Serial.printf("Total: %d bytes\n", total);
        Serial.printf("Usado: %d bytes\n", used);
        Serial.printf("Livre: %d bytes\n", total - used);
        Serial.printf("Uso: %.2f%%\n", (float)used / total * 100);
        
        if (used > (total * 0.8)) {
          Serial.println("[AVISO] Memoria acima de 80%! Risco de lentidao.");
        } else {
          Serial.println("[OK] Espaço em disco saudavel.");
        }
        SPIFFS.end();
      }
      Serial.println("------------------------------");
    }
    // Comando HELP: Listar todos os comandos disponíveis
    else if (input.equalsIgnoreCase("HELP:") || input.equalsIgnoreCase("?")) {
      Serial.println("\n========= BITCOIN POS - HELP =========");
      Serial.println("F:       -> Saude da memoria Flash (SPIFFS)");
      Serial.println("I:amount -> Gera invoice remota (Ex: I:100)");
      Serial.println("L:       -> Listar arquivos salvos na Flash");
      Serial.println("P:       -> Forcar verificacao de pagamento manual");
      Serial.println("Q:       -> Reiniciar sistema (Limpa RAM, nao salva)");
      Serial.println("R:       -> Reiniciar ESP32");
      Serial.println("S:       -> Status atual (WiFi, URL, Key, Sats)");
      Serial.println("T:x,y    -> Emular toque na tela (Ex: T:120,200)");
      Serial.println("U:url,key-> Atualizar URL/Key na RAM");
      Serial.println("W:ssid,pw-> Conectar a nova rede WiFi");
      Serial.println("Z:       -> SALVAR tudo na Flash (Blindado)");
      Serial.println("HELP:    -> Mostra esta lista de ajuda");
      Serial.println("========================================");
    }

  }
  ///calcel payment//
  if (waiting_for_payment) {
  check_cancel_touch();
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
    Serial.println(">>> [ERRO] Sem conexao WiFi!");
    return;
  }

  Serial.println("\n--- Iniciando Geracao de Invoice ---");
  Serial.printf("Endpoint: %s\n", lnbits_url.c_str());

  HTTPClient http;
  http.begin(lnbits_url.c_str());
  http.addHeader("X-Api-Key", lnbits_api_key.c_str());
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(15000); // 15 segundos de timeout

  // Montando o JSON
  String postData = "{\"out\": false, \"amount\":" + String((int)total_sats) + ", \"memo\": \"POS-ESP32\"}";
  Serial.printf("Dados Enviados: %s\n", postData.c_str());

  int httpCode = http.POST(postData);

  if (httpCode > 0) {
    Serial.printf("HTTP Response Code: %d\n", httpCode);
    String payload = http.getString();
    
    if (httpCode == 200 || httpCode == 201) {
      Serial.println("Sucesso! Analisando JSON de resposta...");
      
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.printf("Falha no Parse do JSON: %s\n", error.c_str());
        Serial.println("Payload recebido foi: " + payload);
      } else if (doc.containsKey("payment_request")) {
        String invoice = doc["payment_request"];
        invoice.toCharArray(QR_LN_invoice, 3000);
        current_payment_hash = doc["payment_hash"].as<String>();
        
        Serial.println("Invoice gerada com sucesso!");
        Serial.printf("Hash: %s\n", current_payment_hash.c_str());
        waiting_for_payment = true;
        check_count = 0;
      } else {
        Serial.println("Erro: JSON nao contem 'payment_request'.");
        Serial.println("Payload: " + payload);
      }
    } else {
      Serial.println(">>> [FALHA SERVIDOR] <<<");
      Serial.printf("Mensagem do Servidor: %s\n", payload.c_str());
    }
  } else {
    Serial.printf(">>> [ERRO DE CONEXAO] <<<\n");
    Serial.printf("Causa: %s\n", http.errorToString(httpCode).c_str());
    
    // Verificacao extra de DNS/IP
    if (httpCode == -1) {
      Serial.println("Dica: O servidor pode estar inacessivel ou o IP esta incorreto.");
    }
  }
  http.end();
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

////// cancel logic/////
void check_cancel_touch() {
  if (!waiting_for_payment) return;

  tp.read();
  if (tp.isTouched) {
    int x = tp.points[0].x;
    if (x > (TFT_HOR_RES / 2)) {
      Serial.println(">>> TOUCH CANCEL DETECTED (RIGHT HALF) <<<");
      cancel_verification_requested = true;
    }
  }
}




/* --- PAYMENT VERIFICATION --- */
/* --- PAYMENT VERIFICATION --- */
void verify_payment() {
  // 🔴 Immediate cancel
  if (cancel_verification_requested) {
    Serial.println(">>> PAYMENT CHECK CANCELLED BY TOUCH <<<");
    cancel_verification_requested = false;
    waiting_for_payment = false;
    current_payment_hash = "";

    tft.fillScreen(TFT_YELLOW);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 120);
    tft.println("PAYMENT CANCELED");
    delay(2000);

    ui_init();
    return;
  }

  if (!waiting_for_payment || current_payment_hash == "" || WiFi.status() != WL_CONNECTED) return;

  // Manual Serial Override
  if (Serial.available() > 0) {
    char incoming = Serial.read();
    if (incoming == 'c' || incoming == 'C' || incoming == 'x' || incoming == 'X') {
      Serial.println("\n >>> MANUAL CANCELLATION VIA SERIAL <<<");
      waiting_for_payment = false;
      current_payment_hash = "";

      tft.fillScreen(TFT_YELLOW);
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(20, 120);
      tft.println("PAYMENT CANCELED");
      delay(500);

      //ui_init();
      return;
    }
  }

  check_count++;
  Serial.printf("\n--- Checking Payment Status (%d/40) ---\n", check_count);

  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(30);

  HTTPClient http;
  http.begin(client, "https://libertariamemes.com.br/check_payment");
  http.setTimeout(15000);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-POS-Terminal");

  String jsonReq = "{\"payment_hash\":\"" + current_payment_hash + "\"}";
  Serial.print("Checking Hash: "); Serial.println(current_payment_hash);

  // 🧠 Keep LVGL alive + avoid watchdog
  lv_timer_handler();
  yield();
  delay(1);

  int httpCode = http.POST(jsonReq);

  // 🧠 Keep LVGL alive again
  lv_timer_handler();
  yield();
  delay(1);

  if (httpCode == 200) {
    String payload = http.getString();

    static DynamicJsonDocument doc(2048);
    static DynamicJsonDocument lnbitsResult(1024);
    doc.clear();
    lnbitsResult.clear();

    deserializeJson(doc, payload);

    if (doc["success"] == true && doc.containsKey("stdout")) {
      String stdoutStr = doc["stdout"];
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

  // 🔴 Timeout
  if (check_count >= 40) {
    Serial.println("Polling Timeout Reached.");
    waiting_for_payment = false;
    current_payment_hash = "";

    tft.fillScreen(TFT_YELLOW);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(40, 120);
    tft.println("PAYMENT TIMEOUT");
    delay(2000);

    //ui_init();
  }
}

void hardSafeSave() {
  Serial.println("\n[!] BLOQUEIO TOTAL: MODO SEGURANCA...");
  
  // 1. Desliga tudo para liberar o barramento Flash
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  continua = false; 
  waiting_for_payment = false;
  delay(500);

  // 2. TENTA MONTAR A SPIFFS PARA ESCRITA (CRUCIAL)
  if (!SPIFFS.begin(true)) {
    Serial.println("[ERRO] Falha ao montar SPIFFS para gravacao!");
  } else {
    Serial.println("[!] Gravando arquivo...");
    File f = SPIFFS.open("/config.txt", "w");
    if (f) {
      // Usando print simples para evitar caracteres de controle estranhos
      f.print(lnbits_url);
      f.print("\n");
      f.print(lnbits_api_key);
      f.print("\n");
      
      f.flush(); 
      f.close();
      Serial.println("[OK] Arquivo salvo e fechado.");
    } else {
      Serial.println("[ERRO] Nao foi possivel criar o arquivo config.txt");
    }
    
    SPIFFS.end(); // Fecha a partição com segurança
    Serial.println("[!] SPIFFS desmontada com seguranca.");
  }

  delay(1000);
  Serial.println("[!] REBOOT...");
  ESP.restart();
}
