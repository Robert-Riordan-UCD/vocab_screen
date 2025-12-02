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
#define SHOW_BOTH_TIME 10000
#define SCROLL_TIME 1500

#define CHAR_PER_SCROLL 3

#define SCREEN_WIDTH 16
#define MAX_WORD_LENGTH 256

#define SHOW_BTN D0
#define SUCCESS_BTN D1

// Address, characters per line, lines
LiquidCrystal_I2C lcd(0x27, 16, 2);

char dutch[MAX_WORD_LENGTH] = "Geen wifi";
char english[MAX_WORD_LENGTH] = "No wifi";
bool show_nl = false;
bool show_eng = false;
uint8_t nl_length = 9;
uint8_t eng_length = 7;
bool update_screen = true;
bool success = false;
unsigned long next_word_time = 0;
unsigned long next_scroll_time = 0;
uint8_t nl_scroll_offset = 0;
uint8_t eng_scroll_offset = 0;


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
      for (i; i < MAX_WORD_LENGTH; i++) {
        if (payload[i+dutch_offset] == '"') {
          dutch[i] = 0;
          break;
        }
        dutch[i] = payload[i+dutch_offset];
      }
      nl_length = i;

      int english_offset = dutch_offset + 13 + i;
      for (i = 0; i < MAX_WORD_LENGTH; i++) {
        if (payload[i+english_offset] == '"') {
          english[i] = 0;
          break;
        }
        english[i] = payload[i+english_offset];
      }
      eng_length = i;
      
      // Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();

    if (random()%2) {
      show_nl = true;
      show_eng = false;
    } else {
      show_nl = false;
      show_eng = true;
    }
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
  if (show_eng && show_nl) {return;}
  show_eng = true;
  show_nl = true;
  update_screen = true;
  next_word_time = millis() + SHOW_BOTH_TIME;
}

void IRAM_ATTR isr_success() {
  if (show_eng && show_nl) {
    success = true;
  }
}

void scroll_word(char* word, int scroll_offset, int word_length, bool show, char* scrolled_word) {
  if (!show) {
    scrolled_word[0] = 0;
    return;
  }

  for (int i=0; i < SCREEN_WIDTH; i++) {
    if (word[i+scroll_offset] == 0) {
      scrolled_word[i] = 0;
      break;
    }
    scrolled_word[i] = word[i+scroll_offset];
  }
}

void display_word(char* dutch_to_show, char* english_to_show) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(dutch_to_show);

  lcd.setCursor(0, 1);
  lcd.print(english_to_show);
}

uint8_t update_scroll_offset(uint8_t current_offset, uint8_t word_length, bool show) {
  if (!show) {return 0;} // Not showing word
  if (word_length < SCREEN_WIDTH) {return 0;} // Word too short to scroll
  if (current_offset > word_length - SCREEN_WIDTH) { // Return to start of word
    update_screen = true;
    return 0;
  }
  update_screen = true;
  return current_offset + CHAR_PER_SCROLL;
}

void reset_globals() {
    nl_scroll_offset = 0;
    eng_scroll_offset = 0;
    next_scroll_time = millis() + SCROLL_TIME;
    next_word_time = millis() + HOLD_TIME;
    // Extra 2 seconds per offscreen character
    if (nl_length > SCREEN_WIDTH || eng_length > SCREEN_WIDTH) {
      next_word_time = millis() + HOLD_TIME + 2000*(max(nl_length, eng_length)-SCREEN_WIDTH);
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
    if (success) {send_success();}
    request_random_word();
    reset_globals();
  }

  if (next_word_time - SHOW_BOTH_TIME < millis() && (!show_nl || !show_eng)) {
    show_nl = true;
    show_eng = true;
    update_screen = true;
  }

  if (update_screen) {
    update_screen = false;
    char dutch_to_show[SCREEN_WIDTH];
    char english_to_show[SCREEN_WIDTH];
    scroll_word(dutch, nl_scroll_offset, nl_length, show_nl, dutch_to_show);
    scroll_word(english, eng_scroll_offset, eng_length, show_eng, english_to_show);
    display_word(dutch_to_show, english_to_show);
  }

  if (next_scroll_time < millis()) {
    nl_scroll_offset = update_scroll_offset(nl_scroll_offset, nl_length, show_nl);
    eng_scroll_offset = update_scroll_offset(eng_scroll_offset, eng_length, show_eng);

    next_scroll_time = millis() + SCROLL_TIME;
  }
}
