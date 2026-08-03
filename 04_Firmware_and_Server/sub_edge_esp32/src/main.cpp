#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <Adafruit_MLX90614.h>
#include "config.h"
#include "iis3dwb.h"
#include "dsp_processing.h"

Scheduler scheduler;
painlessMesh mesh;
IIS3DWB vibrationSensor(PIN_IIS3DWB_CS);
Adafruit_MLX90614 temperatureSensor;
DspProcessor dsp;

RTC_DATA_ATTR uint32_t sequenceNumber = 0;
static volatile bool ackReceived = false;
static volatile uint32_t ackSequence = 0;
static int16_t rawX[N_SAMPLES];
static int16_t rawY[N_SAMPLES];
static int16_t rawZ[N_SAMPLES];

struct TemperatureReading {
    float objectC = NAN;
    float ambientC = NAN;
    bool valid = false;
};

void setSensorPower(bool enabled) {
    pinMode(PIN_SENSOR_POWER_EN, OUTPUT);
    digitalWrite(PIN_SENSOR_POWER_EN,
                 enabled ? SENSOR_POWER_ACTIVE_LEVEL : !SENSOR_POWER_ACTIVE_LEVEL);
    if (enabled) delay(SENSOR_POWER_SETTLE_MS);
}

uint16_t readBatteryMillivolts() {
    pinMode(PIN_BATTERY_DIV_EN, OUTPUT);
    digitalWrite(PIN_BATTERY_DIV_EN, BATTERY_DIV_ACTIVE_LEVEL);
    delay(3);
    uint32_t total = 0;
    for (uint8_t i = 0; i < 16; ++i) total += analogRead(PIN_BATTERY_ADC);
    digitalWrite(PIN_BATTERY_DIV_EN, !BATTERY_DIV_ACTIVE_LEVEL);
    const float adc = total / 16.0f;
    return static_cast<uint16_t>((adc / ADC_FULL_SCALE) * ADC_REFERENCE_MV * BATTERY_DIVIDER_RATIO);
}

#if USE_SIMULATED_SENSOR
void acquireSimulated() {
    for (uint16_t i = 0; i < N_SAMPLES; ++i) {
        const float t = i / FS_HZ;
        const float base = 0.25f * sinf(2.0f * PI * SIM_BASE_FREQ_HZ * t);
        const float harmonic = 0.08f * sinf(2.0f * PI * 2.0f * SIM_BASE_FREQ_HZ * t);
        const float noise = SIM_NOISE_AMPL * (random(-1000, 1001) / 1000.0f);
        rawX[i] = static_cast<int16_t>((base + noise) / 0.000122f);
        rawY[i] = static_cast<int16_t>((0.7f * base + harmonic + noise) / 0.000122f);
        rawZ[i] = static_cast<int16_t>((0.4f * base + noise) / 0.000122f);
    }
}
#endif

bool acquireVibration() {
#if USE_SIMULATED_SENSOR
    acquireSimulated();
    return true;
#else
    vibrationSensor.fifoReset();
    vibrationSensor.fifoEnableContinuous();
    uint16_t count = 0;
    const uint32_t deadline = millis() + 2500;
    while (count < N_SAMPLES && millis() < deadline) {
        int16_t x, y, z;
        if (vibrationSensor.fifoReadSample(x, y, z)) {
            rawX[count] = x; rawY[count] = y; rawZ[count] = z;
            ++count;
        } else {
            delayMicroseconds(20);
        }
    }
    return count == N_SAMPLES;
#endif
}

TemperatureReading readTemperature() {
    TemperatureReading r;
    r.ambientC = temperatureSensor.readAmbientTempC();
    r.objectC = temperatureSensor.readObjectTempC();
    r.valid = isfinite(r.ambientC) && isfinite(r.objectC) &&
              r.ambientC > -40.0f && r.ambientC < 125.0f &&
              r.objectC > -70.0f && r.objectC < 380.0f;
    return r;
}

void addAxis(JsonObject axis, const AxisFeatures &f) {
    axis["accel_rms_g"] = f.acceleration_rms_g;
    axis["peak_g"] = f.peak_g;
    axis["crest_factor"] = f.crest_factor;
    axis["kurtosis"] = f.kurtosis;
    axis["dominant_freq_hz"] = f.dominant_freq_hz;
    axis["velocity_rms_mm_s"] = f.velocity_rms_mm_s;
    JsonArray bands = axis.createNestedArray("band_rms_g");
    for (uint8_t i = 0; i < FFT_BAND_COUNT; ++i) bands.add(f.band_rms_g[i]);
}

void onMeshReceive(uint32_t, String &message) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, message)) return;
    if (strcmp(doc["type"] | "", "ack") != 0) return;
    if (strcmp(doc["node_id"] | "", NODE_ID) != 0) return;
    const uint32_t seq = doc["seq"] | 0;
    if (seq == sequenceNumber) {
        ackSequence = seq;
        ackReceived = true;
    }
}

bool joinMesh() {
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &scheduler, MESH_PORT);
    mesh.onReceive(&onMeshReceive);
    const uint32_t deadline = millis() + MESH_JOIN_TIMEOUT_MS;
    while (millis() < deadline) {
        mesh.update();
        if (!mesh.getNodeList().empty()) return true;
        delay(10);
    }
    return false;
}

bool sendWithAck(const String &payload) {
    for (uint8_t attempt = 1; attempt <= SEND_RETRY_COUNT; ++attempt) {
        ackReceived = false;
        mesh.sendBroadcast(payload);
        const uint32_t deadline = millis() + ACK_TIMEOUT_MS;
        while (millis() < deadline) {
            mesh.update();
            if (ackReceived && ackSequence == sequenceNumber) return true;
            delay(5);
        }
        delay(SEND_RETRY_BACKOFF_MS * attempt);
    }
    return false;
}

[[noreturn]] void sleepNow() {
    setSensorPower(false);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(CYCLE_SLEEP_SEC) * US_TO_S_FACTOR);
    Serial.flush();
    esp_deep_sleep_start();
    while (true) {}
}

void setup() {
    Serial.begin(115200);
    delay(100);
    ++sequenceNumber;
    analogReadResolution(12);
    setSensorPower(true);
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_IIS3DWB_CS);
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    bool vibrationReady = true;
#if !USE_SIMULATED_SENSOR
    vibrationReady = vibrationSensor.begin(ACCEL_FS_G);
#endif
    const bool temperatureReady = temperatureSensor.begin(MLX90614_I2C_ADDRESS, &Wire);
    if (!vibrationReady) Serial.println("[ERR] IIS3DWB tidak terdeteksi");
    if (!temperatureReady) Serial.println("[ERR] MLX90614 tidak terdeteksi");

    const bool acquisitionOk = vibrationReady && acquireVibration();
    const TemperatureReading temp = temperatureReady ? readTemperature() : TemperatureReading{};
    AxisFeatures fx, fy, fz;
    if (acquisitionOk) {
        const float scale =
#if USE_SIMULATED_SENSOR
            0.000122f;
#else
            vibrationSensor.sensitivityGPerLsb();
#endif
        fx = dsp.process(rawX, N_SAMPLES, FS_HZ, scale);
        fy = dsp.process(rawY, N_SAMPLES, FS_HZ, scale);
        fz = dsp.process(rawZ, N_SAMPLES, FS_HZ, scale);
    }
#if !USE_SIMULATED_SENSOR
    vibrationSensor.powerDown();
#endif
    setSensorPower(false);

    StaticJsonDocument<4096> doc;
    doc["schema_version"] = PAYLOAD_SCHEMA_VERSION;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["type"] = "condition_features";
    doc["node_id"] = NODE_ID;
    doc["seq"] = sequenceNumber;
    doc["battery_mv"] = readBatteryMillivolts();
    doc["sample_rate_hz"] = FS_HZ;
    doc["sample_count"] = N_SAMPLES;
    doc["acquisition_valid"] = acquisitionOk;
    JsonObject t = doc.createNestedObject("temperature");
    t["valid"] = temp.valid;
    if (temp.valid) {
        t["object_c"] = temp.objectC;
        t["ambient_c"] = temp.ambientC;
        t["delta_c"] = temp.objectC - temp.ambientC;
    }
    JsonObject vib = doc.createNestedObject("vibration");
    addAxis(vib.createNestedObject("x"), fx);
    addAxis(vib.createNestedObject("y"), fy);
    addAxis(vib.createNestedObject("z"), fz);
    doc["alarm_state"] = "UNCONFIGURED_BASELINE";

    String payload;
    serializeJson(doc, payload);
    const bool joined = joinMesh();
    bool delivered = false;
    if (joined) delivered = sendWithAck(payload);
    Serial.printf("[RESULT] joined=%d delivered=%d bytes=%u\n", joined, delivered, payload.length());
    sleepNow();
}

void loop() {}
