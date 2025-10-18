#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

#define BUZZER_PIN 14       // D5 = GPIO14
#define LED_PIN 2           // D4 = GPIO2 (onboard LED, active LOW)
#define THRESHOLD 500       // Higher = less sensitive (filter small bumps)
#define SAMPLE_COUNT 100    // Baseline calibration samples
#define STABLE_COUNT 10     // Quake ends after 10 stable readings
#define ALERT_DURATION 4000 // Total alert duration (ms)
#define PULSE_INTERVAL 40   // Pulse on/off interval (ms)
#define CONFIRM_COUNT 5     // Require consecutive strong shakes
#define FILTER_SIZE 10      // Moving average filter size
#define NOISE_FLOOR 50      // Ignore raw vibrations below this
#define RETRY_INTERVAL 5000 // Retry reconnect every 5s
#define COOLDOWN_TIME 2000  // Time after alert where detection is ignored
#define RESET_TIMEOUT 60000 // 60s no valid data = auto reset
#define DAILY_RESET 86400000UL // 24h auto reset (ms)

MPU6050 mpu;

// Baseline values
long ax0, ay0, az0;

// Moving average buffer
long filterBuffer[FILTER_SIZE];
int filterIndex = 0;
long filterSum = 0;

bool quakeActive = false;
int stableCounter = 0;

// Smoothed vibration value
long vibration = 0;

// Alert timer
unsigned long alertStart = 0;
bool alerting = false;

// Pulse timer
unsigned long lastPulse = 0;
bool buzzerOn = false;

// Consecutive trigger counter
int triggerCount = 0;

// Sensor OK flag
bool sensorOK = false;

// Last alert end time
unsigned long lastAlertEnd = 0;

// Last retry time
unsigned long lastRetry = 0;

// Failsafe: last good read
unsigned long lastGoodRead = 0;

// Daily reset reference
unsigned long bootTime = 0;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  Serial.println("Initializing MPU6050...");
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed! System in safe mode (no buzzer).");
    sensorOK = false;
    lastGoodRead = millis();
    bootTime = millis();
    return;
  }

  Serial.println("MPU6050 connected");
  sensorOK = true;


  Serial.println("Calibrating... Keep device still");
  long axSum = 0, aySum = 0, azSum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    if (abs(ax) > 32000 || abs(ay) > 32000 || abs(az) > 32000) continue;

    axSum += ax;
    aySum += ay;
    azSum += az;
    delay(10);
  }
  ax0 = axSum / SAMPLE_COUNT;
  ay0 = aySum / SAMPLE_COUNT;
  az0 = azSum / SAMPLE_COUNT;

  for (int i = 0; i < FILTER_SIZE; i++) filterBuffer[i] = 0;

  Serial.print("Baseline -> AX=");
  Serial.print(ax0);
  Serial.print(" AY=");
  Serial.print(ay0);
  Serial.print(" AZ=");
  Serial.println(az0);
  Serial.println("System ready");

  lastGoodRead = millis();
  bootTime = millis();
}


void loop() {
  if (!sensorOK) {
    if (millis() - lastRetry > RETRY_INTERVAL) {
      Serial.println("Retrying MPU6050...");
      mpu.initialize();
      if (mpu.testConnection()) {
        Serial.println("Reconnected!");
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
    Serial.println("Lost connection to MPU6050! Stopping buzzer.");
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, HIGH);
    sensorOK = false;
    return;
  }
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  lastGoodRead = millis();

  long dx = ax - ax0;
  long dy = ay - ay0;
  long dz = az - az0;
  long rawVibration = sqrt((dx * dx) + (dy * dy) + (dz * dz));

  if (rawVibration < NOISE_FLOOR) rawVibration = 0;

  filterSum -= filterBuffer[filterIndex];
  filterBuffer[filterIndex] = rawVibration;
  filterSum += rawVibration;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  vibration = filterSum / FILTER_SIZE;

  Serial.print(millis() / 1000.0, 2);
  Serial.print("s | Vibration=");
  Serial.print(vibration);
  int barLength = map(vibration, 0, 2000, 0, 50); 
  Serial.print(" | ");
  for (int i = 0; i < barLength; i++) Serial.print("-");
  if (alerting) Serial.print(" ALERT");
  Serial.println();

  if (!alerting && millis() - lastAlertEnd < COOLDOWN_TIME) {
    delay(50);
    return;
  }

  if (vibration > THRESHOLD) {
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
  } else {
    triggerCount = 0;
  }

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
      Serial.println("Alert finished, entering cooldown");
    }
  }

  if (vibration <= THRESHOLD) {
    if (quakeActive) {
      stableCounter++;
      if (stableCounter > STABLE_COUNT) {
        quakeActive = false;
        Serial.println("Earthquake ended");
      }
    }
  }

  // AUTO-RESET FAILSAFE
  if (millis() - lastGoodRead > RESET_TIMEOUT) {
    Serial.println("No valid data too long! Restarting...");
    delay(500);
    ESP.restart();
  }

  // DAILY AUTO-RESET
  if (millis() - bootTime > DAILY_RESET) {
    Serial.println("Daily scheduled reset...");
    delay(500);
    ESP.restart();
  }

  delay(50);
}
