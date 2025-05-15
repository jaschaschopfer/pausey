// ===== File: Pausey.ino =====
// Complete, self‑contained sketch – May 2025
// -----------------------------------------
//  • 10:51   Push‑Up challenge
//  • 10:55   Stretch challenge
//  • 11:00   Dance challenge
// -----------------------------------------

#include <Arduino.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL6180X.h>
#include <WiFi.h>
#include <Ticker.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>

// ───────────────────────────────── CONFIGURATION ─────────────────────────────
// Wi‑Fi credentials
#define WIFI_SSID        "tinkergarden"
#define WIFI_PASSWORD    "strenggeheim"

// Unique device ID
#define DEVICE_ID        "PAUSEY-DEV-007"

// I²C pins (OLED + VL6180X)
const uint8_t PIN_I2C_SDA = 21;
const uint8_t PIN_I2C_SCL = 22;

// Piezo + button
const uint8_t PIN_BUTTON = 0;   // onboard BOOT button on many ESP32 dev boards
const uint8_t PIN_PIEZO  = 2;

// Beeps
#define BEEP_FREQ_INVITE   1000      // Hz   – short invite jingle
#define BEEP_FREQ_EXERCISE  440      // Hz   – feedback beeps
#define BEEP_DURATION_MS    120      // ms   – uniform duration

// Push‑Up (VL6180X distance sensor)
#define PUSHUP_MAX_DURATION_SEC   ( 5 * 60)
#define PUSHUP_MAX_SCORE          10        // 10 perfect reps

// Stretch (vibration‑switch)
const uint8_t PIN_VIBRATION_SENSOR = 19;
#define VIBRATION_DEBOUNCE_MS      50
#define STRETCH_MAX_DURATION_SEC   ( 2 * 60)
#define STRETCH_THRESH_LOW_SEC     60
#define STRETCH_THRESH_MED_SEC     90
#define STRETCH_THRESH_HIGH_SEC   120
#define STRETCH_SCORE_MAX          10.0f

// Dance (NeoPixel ring)
const uint8_t PIN_NEOPIXEL   = 18;
const uint8_t NEOPIXEL_COUNT = 12;
#define DANCE_MAX_DURATION_SEC  ( 2 * 60)
#define DANCE_SCORE_MAX         10.0f
#define DANCE_BPM               60        // beats per minute for ticker

// OLED
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64

// ───────────────────────────── GLOBAL OBJECTS ───────────────────────────────
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel strip(NEOPIXEL_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_VL6180X  vlSensor;   // distance sensor for push‑ups

enum GameType { GAME_PUSHUP, GAME_STRETCH, GAME_DANCE };

// Time synchronisation & tickers
static Ticker timeSyncTicker;   // every 12 h SNTP sync
static Ticker danceTicker;      // dedicated beat ticker for dance

// ──────────────────────────── FUNCTION PROTOTYPES ───────────────────────────
void connectWiFi();
void syncTimeSNTP();
void initDisplay();
void showIdleFace();
void showInvite(GameType game);
void showResult(float score);

void initPushUp();  uint8_t runPushUpExercise();
void initStretch();
float runStretchExercise();
void initDance();
float runDanceExercise();

// ───────────────────────────── SETUP & LOOP ───────────────────────────────
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- Pausey Start ---");
  connectWiFi();
  initDisplay();
  initPushUp();
  initStretch();
  initDance();
}

void loop() {
  // current seconds since midnight
  time_t now = time(nullptr);
  struct tm ti;  localtime_r(&now, &ti);
  int secs = ti.tm_hour * 3600 + ti.tm_min * 60 + ti.tm_sec;

  // ───── daily schedule (10:51, 10:55, 11:00) ─────
  const int TIMES[] = {
    00 * 3600 + 51 * 60,    // 10:51   Push‑Up
    00 * 3600 + 55 * 60,    // 10:55   Stretch
    01 * 3600               // 11:00   Dance
  };
  static uint8_t nextGame = 0;      // 0,1,2

  // Reset pointer at midnight
  if (secs < TIMES[0] && nextGame != 0) nextGame = 0;

  // Kick off upcoming game
  if (nextGame < 3 && secs >= TIMES[nextGame]) {
    GameType game = static_cast<GameType>(nextGame);

    showInvite(game);
    tone(PIN_PIEZO, BEEP_FREQ_INVITE, BEEP_DURATION_MS);
    // Wait for user to press button to start exercise
    while (digitalRead(PIN_BUTTON) == HIGH) {
      delay(50);
    }
    noTone(PIN_PIEZO);
    delay(1000);  // wait 1 second before starting exercise
    // Signal game start: display message and three short beeps
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Game started");
    display.display();
    for (int i = 0; i < 3; i++) {
      tone(PIN_PIEZO, BEEP_FREQ_EXERCISE, BEEP_DURATION_MS);
      delay(BEEP_DURATION_MS + 50);
    }
    // keep "Game started" on screen until exercise ends

    float score = 0;
    switch (game) {
      case GAME_PUSHUP:  score = runPushUpExercise(); break;
      case GAME_STRETCH: score = runStretchExercise(); break;
      case GAME_DANCE:   score = runDanceExercise();   break;
    }

    showResult(score);
    // Signal game end: two long beeps
    for (int i = 0; i < 2; i++) {
      tone(PIN_PIEZO, BEEP_FREQ_INVITE, BEEP_DURATION_MS * 4);
      delay(BEEP_DURATION_MS * 4 + 100);
    }
    // Send score JSON to webserver
    {
      HTTPClient http;
      http.begin("http://pausey.danocreations.ch/etl/load.php");
      http.addHeader("Content-Type", "application/json");
      String gameType;
      switch (game) {
        case GAME_PUSHUP:  gameType = "pushup";  break;
        case GAME_STRETCH: gameType = "stretch"; break;
        case GAME_DANCE:   gameType = "dance";   break;
      }
      String payload = String("{\"device_id\":\"") + DEVICE_ID + "\",\"game_type\":\"" + gameType + "\",\"score\":" + String(score, 1) + "}";
      int httpCode = http.POST(payload);
      if (httpCode > 0) {
        Serial.printf("POST code: %d", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          Serial.println("Score sent successfully");
        }
      } else {
        Serial.printf("POST failed, error: %s", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
    // hold the result on screen for 30 seconds
    delay(30000);

    showIdleFace();
    delay(500);
    nextGame++;
  }

  delay(200);
}

// ─────────────────────────────── UTILITIES ─────────────────────────────────
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting Wi‑Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.'); delay(500);
  }
  Serial.println(" connected");
  syncTimeSNTP();
  timeSyncTicker.attach(43200, syncTimeSNTP);   // twice a day
}

void syncTimeSNTP() {
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");  // CET +1, DST +1
  time_t now;  do { now = time(nullptr); delay(200);} while (now < 100000);
  struct tm ti;  localtime_r(&now, &ti);
  char buf[32];  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ti);
  Serial.print("SNTP time: "); Serial.println(buf);
}

void initDisplay() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed – halting");
    while (true) delay(1000);
  }
  display.clearDisplay(); display.display();
  showIdleFace();
}

// ───────────────────────── SCREEN HELPERS ───────────────────────────────────
void showIdleFace() {
  display.clearDisplay();
  display.fillCircle(32, 24, 4, SSD1306_WHITE);   // eyes
  display.fillCircle(96, 24, 4, SSD1306_WHITE);
    // inverted smile (frown)
  for (int i = 0; i <= 32; i += 2) {
    float angle = (float)i / 32.0f * PI;
    int x = 48 + i;
    int y = 50 + (int)(4.0f * sin(angle));  // inverted curve
    display.drawPixel(x, y, SSD1306_WHITE);
  }
  display.display();
}

void showInvite(GameType game) {
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE); display.setCursor(0,0);
  switch (game) {
    case GAME_PUSHUP:  display.println("Time for Push‑Ups!");  break;
    case GAME_STRETCH: display.println("Time for Stretch!");   break;
    case GAME_DANCE:   display.println("Time for Dance!");     break;
  }
  display.display();
}

void showResult(float score) {
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20); display.println("You scored");
  display.setCursor(0,35);
  if (fabsf(score - roundf(score)) < 0.01f)  display.print(static_cast<int>(score));
  else                                         display.print(score, 1);
  display.println(" P");
  display.display();
}

// ──────────────────────────── PUSH‑UP GAME ─────────────────────────────────
void initPushUp() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);        // distance sensor on same bus
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_PIEZO,  OUTPUT);
  if (!vlSensor.begin(&Wire)) {
    Serial.println("VL6180X init failed – halting"); while (true) delay(1000);
  }
}

uint8_t runPushUpExercise() {
  Serial.println("Push‑Up: GO");
  uint8_t reps = 0; bool wasDown = false;
  const unsigned long start = millis();
  const unsigned long timeout = PUSHUP_MAX_DURATION_SEC * 1000UL;

  while (millis() - start < timeout) {
    bool isDown = (vlSensor.readRange() < 100);   // <10 cm

    if (wasDown && !isDown) {           // completed one rep
      reps++; tone(PIN_PIEZO, BEEP_FREQ_EXERCISE, BEEP_DURATION_MS);
      Serial.printf("Rep %u\n", reps);
      if (reps >= PUSHUP_MAX_SCORE) break;
    }
    wasDown = isDown;
    if (digitalRead(PIN_BUTTON) == LOW) break;      // abort
    delay(50);
  }
  noTone(PIN_PIEZO);
  Serial.printf("Push‑Up finished: %u reps\n", reps);
  return reps;   // 0‑10 gives the score directly
}

// ───────────────────────────── STRETCH GAME ───────────────────────────────
volatile bool vibDetected = false;
volatile unsigned long lastVibMs = 0;

void IRAM_ATTR onVibration() {
  unsigned long now = millis();
  if (now - lastVibMs >= VIBRATION_DEBOUNCE_MS) {
    vibDetected = true;
    lastVibMs = now;
  }
}

void initStretch() {
  pinMode(PIN_VIBRATION_SENSOR, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_PIEZO, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_VIBRATION_SENSOR), onVibration, RISING);
}

float runStretchExercise() {
  Serial.println("Stretch: HOLD the pose… press button to abort");
  vibDetected = false;
  unsigned long start = millis();
  lastVibMs = start;
  unsigned long timeout = STRETCH_MAX_DURATION_SEC * 1000UL;

  while (millis() - start < timeout) {
    if (digitalRead(PIN_BUTTON) == LOW) {
      Serial.println("Stretch aborted by button");
      break;
    }
    if (millis() - lastVibMs >= 5000) {
      Serial.println("Stretch aborted: no vibration for 5 seconds");
      break;
    }
    delay(10);
  }
  tone(PIN_PIEZO, BEEP_FREQ_EXERCISE, BEEP_DURATION_MS);

  unsigned long end = millis();
  float sec = fminf((end - start) / 1000.0f, (float)STRETCH_MAX_DURATION_SEC);
  float score;
  if (sec < STRETCH_THRESH_LOW_SEC) score = 0.0f;
  else if (sec < STRETCH_THRESH_MED_SEC) score = 5.0f;
  else if (sec < STRETCH_THRESH_HIGH_SEC) score = 7.5f;
  else score = 10.0f;

  Serial.printf("Stretch held %.1f s -> %.1f P\n", sec, score);
  return score;
}

// ────────────────────────────── DANCE GAME ─────────────────────────────────
volatile bool danceBeat = false;
uint8_t        danceIdx  = 0;

void IRAM_ATTR onDanceTick() { danceBeat = true; }

void initDance() {
  strip.begin(); strip.show();  // clear
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_PIEZO,  OUTPUT);
}

float runDanceExercise() {
  Serial.println("Dance: START – follow the lights");
  danceBeat = false; danceIdx = 0;
  const unsigned long start = millis();
  const unsigned long timeout = DANCE_MAX_DURATION_SEC * 1000UL;

  danceTicker.attach(60.0 / DANCE_BPM, onDanceTick);

  while (millis() - start < timeout) {
    if (danceBeat) {
      strip.clear();
      // choose one of four groups at random
      uint8_t group = random(0, 4);
      switch (group) {
        case 0: // top: 11, 0, 1
          strip.setPixelColor(11, strip.Color(255,255,255));
          strip.setPixelColor(0,  strip.Color(255,255,255));
          strip.setPixelColor(1,  strip.Color(255,255,255));
          break;
        case 1: // right: 2, 3, 4
          strip.setPixelColor(2,  strip.Color(255,255,255));
          strip.setPixelColor(3,  strip.Color(255,255,255));
          strip.setPixelColor(4,  strip.Color(255,255,255));
          break;
        case 2: // bottom: 5, 6, 7
          strip.setPixelColor(5,  strip.Color(255,255,255));
          strip.setPixelColor(6,  strip.Color(255,255,255));
          strip.setPixelColor(7,  strip.Color(255,255,255));
          break;
        case 3: // left: 8, 9, 10
          strip.setPixelColor(8,  strip.Color(255,255,255));
          strip.setPixelColor(9,  strip.Color(255,255,255));
          strip.setPixelColor(10, strip.Color(255,255,255));
          break;
      }
      strip.show();
      tone(PIN_PIEZO, BEEP_FREQ_EXERCISE, BEEP_DURATION_MS);
      danceIdx++; danceBeat = false;
    }
    if (digitalRead(PIN_BUTTON) == LOW) { Serial.println("Dance aborted"); break; }
  }

  danceTicker.detach(); noTone(PIN_PIEZO);
  strip.clear(); strip.show();

  const float sec = fminf((millis() - start) / 1000.0f, (float)DANCE_MAX_DURATION_SEC);
  const float score = (sec / DANCE_MAX_DURATION_SEC) * DANCE_SCORE_MAX;
  Serial.printf("Dance %.1f s → %.1f P\n", sec, score);
  return score;
}
