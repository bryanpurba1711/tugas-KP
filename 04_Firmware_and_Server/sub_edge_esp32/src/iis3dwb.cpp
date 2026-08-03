#include "iis3dwb.h"

IIS3DWB::IIS3DWB(uint8_t csPin, SPIClass &spi)
    : _cs(csPin), _spi(spi), _sensitivity_mg_per_lsb(0.122f),
      _spiSettings(8000000UL, MSBFIRST, SPI_MODE3) {}

void IIS3DWB::readRegs(uint8_t reg, uint8_t *buf, size_t len) {
    _spi.beginTransaction(_spiSettings);
    digitalWrite(_cs, LOW);
    _spi.transfer(reg | IIS3DWB_SPI_READ);
    for (size_t i = 0; i < len; ++i) buf[i] = _spi.transfer(0x00);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
}

uint8_t IIS3DWB::readReg(uint8_t reg) {
    uint8_t value = 0;
    readRegs(reg, &value, 1);
    return value;
}

void IIS3DWB::writeReg(uint8_t reg, uint8_t value) {
    _spi.beginTransaction(_spiSettings);
    digitalWrite(_cs, LOW);
    _spi.transfer(reg & 0x7F);
    _spi.transfer(value);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
}

uint8_t IIS3DWB::whoAmI() { return readReg(IIS3DWB_WHO_AM_I); }

bool IIS3DWB::begin(uint8_t fullScaleG) {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    delay(20);
    if (whoAmI() != IIS3DWB_WHO_AM_I_VAL) return false;

    writeReg(IIS3DWB_CTRL3_C, 0x01); // SW_RESET
    delay(20);
    writeReg(IIS3DWB_CTRL3_C, 0x44); // BDU + IF_INC

    uint8_t fsBits = 0b10;
    switch (fullScaleG) {
        case 2:  fsBits = 0b00; _sensitivity_mg_per_lsb = 0.061f; break;
        case 4:  fsBits = 0b10; _sensitivity_mg_per_lsb = 0.122f; break;
        case 8:  fsBits = 0b11; _sensitivity_mg_per_lsb = 0.244f; break;
        case 16: fsBits = 0b01; _sensitivity_mg_per_lsb = 0.488f; break;
        default: return false;
    }

    // IIS3DWB uses XL_EN[2:0]=101 for measurement mode; FS_XL in bits [3:2].
    const uint8_t ctrl1 = static_cast<uint8_t>((0b101 << 5) | (fsBits << 2));
    writeReg(IIS3DWB_CTRL1_XL, ctrl1);
    writeReg(IIS3DWB_CTRL4_C, 0x04); // disable I2C interface on IIS3DWB
    fifoReset();
    fifoEnableContinuous();
    return true;
}

bool IIS3DWB::dataReady() { return (readReg(IIS3DWB_STATUS_REG) & 0x01) != 0; }

bool IIS3DWB::readSample(int16_t &x, int16_t &y, int16_t &z) {
    uint8_t b[6];
    readRegs(IIS3DWB_OUTX_L_A, b, sizeof(b));
    x = static_cast<int16_t>((b[1] << 8) | b[0]);
    y = static_cast<int16_t>((b[3] << 8) | b[2]);
    z = static_cast<int16_t>((b[5] << 8) | b[4]);
    return true;
}

void IIS3DWB::fifoReset() {
    writeReg(IIS3DWB_FIFO_CTRL4, 0x00); // bypass
    writeReg(IIS3DWB_FIFO_CTRL1, 0x00);
    writeReg(IIS3DWB_FIFO_CTRL2, 0x00);
    delay(2);
}

void IIS3DWB::fifoEnableContinuous() {
    // BDR_XL_BATCH_[3:0]=1010 => 26.667 kHz accelerometer batching.
    writeReg(IIS3DWB_FIFO_CTRL3, 0x0A);
    // FIFO_MODE_[2:0]=110 => continuous mode.
    writeReg(IIS3DWB_FIFO_CTRL4, 0x06);
}

uint16_t IIS3DWB::fifoCount() {
    const uint8_t lo = readReg(IIS3DWB_FIFO_STATUS1);
    const uint8_t hi = readReg(IIS3DWB_FIFO_STATUS2);
    return static_cast<uint16_t>(((hi & 0x03) << 8) | lo);
}

bool IIS3DWB::fifoReadSample(int16_t &x, int16_t &y, int16_t &z) {
    if (fifoCount() == 0) return false;
    uint8_t b[7];
    readRegs(IIS3DWB_FIFO_DATA_OUT_TAG, b, sizeof(b));
    const uint8_t tag = static_cast<uint8_t>((b[0] >> 3) & 0x1F);
    if (tag != IIS3DWB_FIFO_TAG_ACCEL) return false;
    x = static_cast<int16_t>((b[2] << 8) | b[1]);
    y = static_cast<int16_t>((b[4] << 8) | b[3]);
    z = static_cast<int16_t>((b[6] << 8) | b[5]);
    return true;
}

void IIS3DWB::powerDown() {
    fifoReset();
    writeReg(IIS3DWB_CTRL1_XL, 0x00); // XL_EN=000
}

float IIS3DWB::toG(int16_t raw) const {
    return static_cast<float>(raw) * _sensitivity_mg_per_lsb / 1000.0f;
}

float IIS3DWB::sensitivityGPerLsb() const {
    return _sensitivity_mg_per_lsb / 1000.0f;
}
