#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

#define BUZZER_PIN 14          // D5 = GPIO14
#define LED_PIN 2              // D4 = GPIO2 (onboard LED, active LOW)
#define THRESHOLD 450          // Higher = less sensitive
#define CONFIRM_COUNT 2        // Require consecutive strong shakes
#define FILTER_SIZE 15         // Moving average filter size
#define SAMPLE_COUNT 100       // Baseline calibration samples
#define STABLE_COUNT 10        // Quake ends after 10 stable readings
#define ALERT_DURATION 4000    // Total alert duration (ms)
#define PULSE_INTERVAL 40      // Pulse on/off interval (ms)
#define NOISE_FLOOR 60         // Ignore tiny raw vibrations
#define RETRY_INTERVAL 5000    // Retry reconnect every 5s
#define COOLDOWN_TIME 1000     // Cooldown after alert
#define RESET_TIMEOUT 60000    // 60s no valid data = auto reset
#define DAILY_RESET 86400000UL // 24h auto reset (ms)
#define MIN_DURATION 1200      // *** Anti-knock: vibration must last 1.2s ***

MPU6050 mpu;

long ax0, ay0, az0;
long filterBuffer[FILTER_SIZE];
int filterIndex = 0;
long filterSum = 0;

bool quakeActive = false;
int stableCounter = 0;
long vibration = 0;

unsigned long alertStart = 0;
bool alerting = false;

unsigned long lastPulse = 0;
bool buzzerOn = false;

int triggerCount = 0;
bool sensorOK = false;

unsigned long lastAlertEnd = 0;
unsigned long lastRetry = 0;
unsigned long lastGoodRead = 0;
unsigned long bootTime = 0;

// Anti-knock timers
unsigned long knockStart = 0;
bool possibleQuake = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Initializing MPU6050...");
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed! Safe mode.");
    sensorOK = false;
    lastGoodRead = millis();
    bootTime = millis();
    return;
  }

  Serial.println("MPU6050 connected");
  sensorOK = true;

  long axSum = 0, aySum = 0, azSum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    if (abs(ax) > 32000 || abs(ay) > 32000 || abs(az) > 32000) continue;
    axSum += ax; aySum += ay; azSum += az;
    delay(10);
  }
  ax0 = axSum / SAMPLE_COUNT;
  ay0 = aySum / SAMPLE_COUNT;
  az0 = azSum / SAMPLE_COUNT;

  for (int i = 0; i < FILTER_SIZE; i++) filterBuffer[i] = 0;

  Serial.println("System ready");
  lastGoodRead = millis();
  bootTime = millis();
}

void loop() {
  if (!sensorOK) {
    if (millis() - lastRetry > RETRY_INTERVAL) {
      mpu.initialize();
      if (mpu.testConnection()) {
        sensorOK = true;
        lastGoodRead = millis();
      }
      lastRetry = millis();
    }
    delay(200);
    return;
  }

  int16_t ax, ay, az, gx, gy, gz;
  if (!mpu.testConnection()) {
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, HIGH);
    sensorOK = false;
    return;
  }
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  lastGoodRead = millis();

  long dx = ax - ax0, dy = ay - ay0, dz = az - az0;
  long rawVibration = sqrt((dx * dx) + (dy * dy) + (dz * dz));
  if (rawVibration < NOISE_FLOOR) rawVibration = 0;

  filterSum -= filterBuffer[filterIndex];
  filterBuffer[filterIndex] = rawVibration;
  filterSum += rawVibration;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  vibration = filterSum / FILTER_SIZE;

  if (!alerting && millis() - lastAlertEnd < COOLDOWN_TIME) return;

  // -------- Anti-knock detection --------
  if (vibration > THRESHOLD) {
    if (!possibleQuake) {
      possibleQuake = true;
      knockStart = millis();  // start timer
    }
    // if vibration lasts longer than MIN_DURATION, confirm quake
    if (millis() - knockStart >= MIN_DURATION) {
      triggerCount++;
      if (triggerCount >= CONFIRM_COUNT && !alerting) {
        quakeActive = true;
        stableCounter = 0;
        alerting = true;
        alertStart = millis();
        lastPulse = millis();
        buzzerOn = false;
        digitalWrite(LED_PIN, LOW);
        Serial.println("*** EARTHQUAKE DETECTED ***");
      }
    }
  } else {
    possibleQuake = false;  // reset if it was only a knock
    triggerCount = 0;
  }

  // -------- Alert handling --------
  if (alerting) {
    if (millis() - lastPulse >= PULSE_INTERVAL) {
      lastPulse = millis();
      buzzerOn = !buzzerOn;
      if (buzzerOn) tone(BUZZER_PIN, 2000);
      else noTone(BUZZER_PIN);
    }
    if (millis() - alertStart >= ALERT_DURATION) {
      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, HIGH);
      alerting = false;
      lastAlertEnd = millis();
      Serial.println("Alert finished, cooldown...");
    }
  }

  // Quake ends
  if (vibration <= THRESHOLD) {
    if (quakeActive) {
      stableCounter++;
      if (stableCounter > STABLE_COUNT) {
        quakeActive = false;
        Serial.println("Earthquake ended");
      }
    }
  }

  if (millis() - lastGoodRead > RESET_TIMEOUT) ESP.restart();
  if (millis() - bootTime > DAILY_RESET) ESP.restart();

  delay(50);
}
