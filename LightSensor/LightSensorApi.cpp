#include "LightSensorApi.h"

#include <QDebug>
#include <chrono>
#include <random>
#include <thread>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// Регистры AS7341
#define REG_ENABLE 0x80
#define REG_ATIME 0x81
#define REG_WTIME 0x83
#define REG_ASTEP_L 0xCA
#define REG_ASTEP_H 0xCB
#define REG_CFG1 0xAA
#define REG_CFG6 0xAF
#define REG_STATUS 0x71
#define REG_CH0_DATA_L 0x95
#define REG_CH0_DATA_H 0x96
#define REG_CH1_DATA_L 0x97
#define REG_CH1_DATA_H 0x98
#define REG_CH2_DATA_L 0x99
#define REG_CH2_DATA_H 0x9A
#define REG_CH3_DATA_L 0x9B
#define REG_CH3_DATA_H 0x9C
#define REG_CH4_DATA_L 0x9D
#define REG_CH4_DATA_H 0x9E
#define REG_CH5_DATA_L 0x9F
#define REG_CH5_DATA_H 0xA0

// SMUX регистры и команды
#define SMUX_CFG_START 0x40
#define SMUX_CFG_END 0x4F
#define SMUX_CMD_REG 0xAF
#define SMUX_CMD_READ 1
#define SMUX_CMD_WRITE 2

// Предопределённые конфигурации SMUX для двух циклов (взяты из ams AS7341
// Arduino library) Цикл 0: F1, F2, F3, F4, NIR, CLEAR
static const uint8_t smuxConfigCycle0[16] = {0x00, 0x20, 0x10, 0x30, 0x01, 0x02,
                                             0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                             0x09, 0x0A, 0x0B, 0x0C};
// Цикл 1: F5, F6, F7, F8, FD, CLEAR
static const uint8_t smuxConfigCycle1[16] = {0x00, 0x20, 0x10, 0x30, 0x0D, 0x0E,
                                             0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
                                             0x15, 0x16, 0x17, 0x18};

LightSensorApi::LightSensorApi(QObject *parent)
    : QObject(parent),
      m_i2cFd(-1),
      m_integrationTimeMs(50),
      m_gainIndex(6),
      m_initialized(false),
      m_emulationMode(false) {
#ifdef Q_OS_WIN
    m_emulationMode = true;
#endif
}

LightSensorApi::~LightSensorApi() {
#ifdef Q_OS_LINUX
    if (m_i2cFd >= 0) close(m_i2cFd);
#endif
}

void LightSensorApi::setEmulationMode(bool enable) { m_emulationMode = enable; }

bool LightSensorApi::initialize() {
    if (m_emulationMode) {
        qDebug() << "LightSensorApi: running in emulation mode";
        m_initialized = true;
        return true;
    }

#ifdef Q_OS_LINUX
    m_i2cFd = open("/dev/i2c-1", O_RDWR);
    if (m_i2cFd < 0) {
        emit errorOccurred("Failed to open I2C bus /dev/i2c-1");
        return false;
    }
    if (ioctl(m_i2cFd, I2C_SLAVE, 0x39) < 0) {
        emit errorOccurred("Failed to set I2C address 0x39");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }

    // Включить питание датчика (PON=1)
    if (!writeRegister(REG_ENABLE, 0x01)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Настроить SMUX для первого цикла (записать конфигурацию в RAM)
    if (!writeSmuxConfig(smuxConfigCycle0)) return false;

    // Установить время интеграции и усиление
    setIntegrationTimeMs(m_integrationTimeMs);
    setGainByIndex(m_gainIndex);

    m_initialized = true;
    return true;
#else
    emit errorOccurred(
        "LightSensorApi not implemented for this OS (use emulation mode)");
    return false;
#endif
}

#ifdef Q_OS_LINUX
bool LightSensorApi::writeSmuxConfig(const uint8_t config[16]) {
    // Записываем 16 байт конфигурации в регистры SMUX
    for (int i = 0; i < 16; ++i) {
        if (!writeRegister(SMUX_CFG_START + i, config[i])) return false;
    }
    // Команда "записать конфигурацию из RAM в SMUX"
    if (!writeRegister(SMUX_CMD_REG, SMUX_CMD_WRITE)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return true;
}

bool LightSensorApi::switchSmuxCycle(int cycle) {
    const uint8_t *config = (cycle == 0) ? smuxConfigCycle0 : smuxConfigCycle1;
    return writeSmuxConfig(config);
}
#endif

void LightSensorApi::setIntegrationTimeMs(int ms) {
    m_integrationTimeMs = ms;
    if (m_emulationMode) return;
#ifdef Q_OS_LINUX
    // Расчёт ATIME и ASTEP для желаемого времени интеграции
    // Формула: tint = (ATIME+1)*(ASTEP+1)*2.78us
    // Выбираем ASTEP=599 (2.78us * 600 = 1.668ms на шаг)
    int aStep = 599;
    int aTime = (ms * 1000) / (2780) / (aStep + 1) - 1;
    if (aTime < 0) aTime = 0;
    if (aTime > 255) aTime = 255;
    writeRegister(REG_ATIME, aTime);
    writeRegister(REG_ASTEP_L, aStep & 0xFF);
    writeRegister(REG_ASTEP_H, (aStep >> 8) & 0xFF);
#endif
}

void LightSensorApi::setGainByIndex(int index) {
    m_gainIndex = index;
    if (m_emulationMode) return;
#ifdef Q_OS_LINUX
    static const uint8_t gainValues[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    if (index >= 0 && index <= 10) {
        writeRegister(REG_CFG1, gainValues[index]);
    }
#endif
}

bool LightSensorApi::readAllChannels(LightSensorData &data) {
    if (!m_initialized) return false;
    if (m_emulationMode) return emulationReadAllChannels(data);

#ifdef Q_OS_LINUX
    QVector<quint16> channels(11, 0);

    // ----- Цикл 1: F1, F2, F3, F4, NIR, CLEAR -----
    if (!switchSmuxCycle(0)) return false;
    if (!startMeasurement()) return false;
    if (!waitForDataReady(1000)) return false;

    uint8_t buf[12];
    if (!readBlock(REG_CH0_DATA_L, buf, 12)) return false;
    channels[CH_F1] = buf[0] | (buf[1] << 8);
    channels[CH_F2] = buf[2] | (buf[3] << 8);
    channels[CH_F3] = buf[4] | (buf[5] << 8);
    channels[CH_F4] = buf[6] | (buf[7] << 8);
    channels[CH_NIR] = buf[8] | (buf[9] << 8);
    channels[CH_CLEAR] = buf[10] | (buf[11] << 8);

    // ----- Цикл 2: F5, F6, F7, F8, FD, CLEAR -----
    if (!switchSmuxCycle(1)) return false;
    if (!startMeasurement()) return false;
    if (!waitForDataReady(1000)) return false;
    if (!readBlock(REG_CH0_DATA_L, buf, 12)) return false;
    channels[CH_F5] = buf[0] | (buf[1] << 8);
    channels[CH_F6] = buf[2] | (buf[3] << 8);
    channels[CH_F7] = buf[4] | (buf[5] << 8);
    channels[CH_F8] = buf[6] | (buf[7] << 8);
    channels[CH_FD] = buf[8] | (buf[9] << 8);
    // CLEAR усредняем с предыдущим значением
    channels[CH_CLEAR] = (channels[CH_CLEAR] + (buf[10] | (buf[11] << 8))) / 2;

    data.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();
    data.channels = channels;
    data.integrationTimeMs = m_integrationTimeMs;
    data.gainIndex = m_gainIndex;
    return true;
#else
    return false;
#endif
}

#ifdef Q_OS_LINUX
bool LightSensorApi::startMeasurement() {
    uint8_t ena = 0;
    if (!readRegister(REG_ENABLE, ena)) return false;
    ena |= (1 << 1);  // SP_EN
    return writeRegister(REG_ENABLE, ena);
}

bool LightSensorApi::waitForDataReady(int timeoutMs) {
    auto start = std::chrono::steady_clock::now();
    while (true) {
        uint8_t status;
        if (!readRegister(REG_STATUS, status)) return false;
        if (status & 0x01) return true;  // READY bit
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start)
                .count() > timeoutMs) {
            emit errorOccurred("Timeout waiting for data ready");
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

bool LightSensorApi::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    if (write(m_i2cFd, buf, 2) != 2) {
        emit errorOccurred(QString("Write reg 0x%1 failed").arg(reg, 2, 16));
        return false;
    }
    return true;
}

bool LightSensorApi::readRegister(uint8_t reg, uint8_t &value) {
    if (write(m_i2cFd, &reg, 1) != 1) return false;
    if (read(m_i2cFd, &value, 1) != 1) return false;
    return true;
}

bool LightSensorApi::readBlock(uint8_t reg, uint8_t *buffer, size_t len) {
    if (write(m_i2cFd, &reg, 1) != 1) return false;
    if (read(m_i2cFd, buffer, len) != (ssize_t)len) return false;
    return true;
}
#endif  // Q_OS_LINUX

// ---------- Эмуляция ----------
bool LightSensorApi::emulationReadAllChannels(LightSensorData &data) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<quint16> dist(100, 30000);

    QVector<quint16> channels(11);
    for (int i = 0; i < 11; ++i) channels[i] = dist(gen);
    data.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();
    data.channels = channels;
    data.integrationTimeMs = m_integrationTimeMs;
    data.gainIndex = m_gainIndex;
    return true;
}
