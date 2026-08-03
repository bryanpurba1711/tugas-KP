#pragma once
#include <Arduino.h>
#include <SPI.h>

class IIS3DWB {
public:
    IIS3DWB(uint8_t csPin, SPIClass &spi = SPI);
    bool begin(uint8_t fullScaleG = 4);
    uint8_t whoAmI();
    bool dataReady();
    bool readSample(int16_t &x, int16_t &y, int16_t &z);
    void fifoEnableContinuous();
    uint16_t fifoCount();
    bool fifoReadSample(int16_t &x, int16_t &y, int16_t &z);
    void fifoReset();
    void powerDown();
    float toG(int16_t raw) const;
    float sensitivityGPerLsb() const;

private:
    uint8_t _cs;
    SPIClass &_spi;
    float _sensitivity_mg_per_lsb;
    SPISettings _spiSettings;
    uint8_t readReg(uint8_t reg);
    void writeReg(uint8_t reg, uint8_t val);
    void readRegs(uint8_t reg, uint8_t *buf, size_t len);
};

#define IIS3DWB_FIFO_CTRL1         0x07
#define IIS3DWB_FIFO_CTRL2         0x08
#define IIS3DWB_FIFO_CTRL3         0x09
#define IIS3DWB_FIFO_CTRL4         0x0A
#define IIS3DWB_WHO_AM_I           0x0F
#define IIS3DWB_WHO_AM_I_VAL       0x7B
#define IIS3DWB_CTRL1_XL           0x10
#define IIS3DWB_CTRL3_C            0x12
#define IIS3DWB_CTRL4_C            0x13
#define IIS3DWB_STATUS_REG         0x1E
#define IIS3DWB_OUTX_L_A           0x28
#define IIS3DWB_FIFO_STATUS1       0x3A
#define IIS3DWB_FIFO_STATUS2       0x3B
#define IIS3DWB_FIFO_DATA_OUT_TAG  0x78
#define IIS3DWB_SPI_READ           0x80
#define IIS3DWB_FIFO_TAG_ACCEL     0x02
