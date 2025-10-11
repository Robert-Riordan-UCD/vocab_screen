#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

const char *ssid = SSID;
const char *password = PASSWORD;

NetworkServer server(80);
String serverName = "http://192.168.2.8:5000/";

#define HOLD_TIME 30000
#define SCREEN_WIDTH 16
#define SCREEN_REFRESH_RATE 500

#define SHOW_BTN D0
#define SUCCESS_BTN D1

// Address, characters per line, lines
LiquidCrystal_I2C lcd(0x27, 16, 2);

char dutch[SCREEN_WIDTH] = "Geen wifi";
char english[SCREEN_WIDTH] = "No wifi";
bool show_eng = false;
bool update_screen = true;
bool success = false;
unsigned long next_word_time = 0;


void connect_to_wifi() {
  delay(5);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void request_random_word() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverName + "/api/random_word";
    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      // Serial.print("HTTP Response code: ");
      // Serial.println(httpResponseCode);
      String payload = http.getString();
      
      // assuming payload = "{"dutch":"<DUTCH WORD>","english":"<ENGLISH WORD>"}"
      // It aint pretty but it works
      int dutch_offset = 10;
      int i = 0;
      for (i; i < SCREEN_WIDTH; i++) {
        if (payload[i+dutch_offset] == '"') {
          dutch[i] = 0;
          break;
        }
        dutch[i] = payload[i+dutch_offset];
      }

      int english_offset = dutch_offset + 13 + i;
      for (i = 0; i < SCREEN_WIDTH; i++) {
        if (payload[i+english_offset] == '"') {
          english[i] = 0;
          break;
        }
        english[i] = payload[i+english_offset];
      }
      
      // Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();

    show_eng = false;
    update_screen = true;
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void send_success() {
    if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverName + "/api/success?word=" + dutch + ':' + english; // + <DUTCH>:<ENGLISH>
    serverPath.replace(' ', '-');
    Serial.println(serverPath);    

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET();
    Serial.println(httpResponseCode);
  } else {
    Serial.println("WiFi Disconnected");
  }

  success = false;
}

void IRAM_ATTR isr_show() {
  show_eng = true;
  update_screen = true;
}

void IRAM_ATTR isr_success() {
  Serial.println("SUCCESS");
  if (show_eng) {
    success = true;
  }
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  connect_to_wifi();

  attachInterrupt(SHOW_BTN, isr_show, FALLING);
  attachInterrupt(SUCCESS_BTN, isr_success, FALLING);
}

void loop() {
  if (next_word_time < millis()) {
    if (success) {
      send_success();
    }
    request_random_word();
    next_word_time = millis() + HOLD_TIME;
  }

  if (update_screen) {
    update_screen = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dutch);
    if (show_eng) {
      lcd.setCursor(0, 1);
      lcd.print(english);
    }
  }
}
