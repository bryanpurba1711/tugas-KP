#pragma once
#include <Arduino.h>

// Design baseline DB-001 Rev.1.0 — sensor node vibration + temperature.
#define NODE_ID                 "edge01"
#define FIRMWARE_VERSION        "1.0.0-revA"
#define PAYLOAD_SCHEMA_VERSION  "1.0"

#define MESH_PREFIX             "kp_motor_monitor_mesh"
#define MESH_PASSWORD           "CHANGE_ME_FOR_DEPLOYMENT"
#define MESH_PORT               5555

// IIS3DWB SPI
#define PIN_SPI_SCK             18
#define PIN_SPI_MISO            19
#define PIN_SPI_MOSI            23
#define PIN_IIS3DWB_CS           5
#define PIN_INT1                 4
#define IIS3DWB_SPI_HZ    8000000UL

// I2C MLX90614
#define PIN_I2C_SDA             21
#define PIN_I2C_SCL             22
#define MLX90614_I2C_ADDRESS  0x5A

// The GPIO drives the enable input of an external load switch/MOSF.
// Do not power both sensors directly from a GPIO.
#define PIN_SENSOR_POWER_EN     27
#define SENSOR_POWER_ACTIVE_LEVEL HIGH
#define SENSOR_POWER_SETTLE_MS  300

// Switched battery divider. Configure resistor ratio in hardware.
#define PIN_BATTERY_ADC         34
#define PIN_BATTERY_DIV_EN      25
#define BATTERY_DIV_ACTIVE_LEVEL HIGH
#define BATTERY_DIVIDER_RATIO  2.0f
#define ADC_REFERENCE_MV      3300.0f
#define ADC_FULL_SCALE        4095.0f

// Acquisition: 8192 samples at 26.667 kHz = 0.307 s, FFT resolution 3.255 Hz.
#define N_SAMPLES              8192
#define FS_HZ               26667.0f
#define ACCEL_FS_G                4
#define FFT_BAND_COUNT            5
static constexpr float FFT_BAND_LOW_HZ[FFT_BAND_COUNT]  = {10.0f, 100.0f, 500.0f, 1000.0f, 3000.0f};
static constexpr float FFT_BAND_HIGH_HZ[FFT_BAND_COUNT] = {100.0f, 500.0f, 1000.0f, 3000.0f, 6000.0f};
#define VELOCITY_LOW_HZ        10.0f
#define VELOCITY_HIGH_HZ     1000.0f

// Four-hour periodic condition-trending cycle.
#define CYCLE_SLEEP_SEC          14400UL
#define MESH_JOIN_TIMEOUT_MS       8000UL
#define ACK_TIMEOUT_MS             3000UL
#define SEND_RETRY_COUNT                 3
#define SEND_RETRY_BACKOFF_MS      1000UL
#define US_TO_S_FACTOR          1000000ULL

// 0 for real sensor hardware, 1 only for bench/simulation builds.
#ifndef USE_SIMULATED_SENSOR
#define USE_SIMULATED_SENSOR 0
#endif

#if USE_SIMULATED_SENSOR
#define SIM_BASE_FREQ_HZ 150.0f
#define SIM_NOISE_AMPL     0.05f
#endif
