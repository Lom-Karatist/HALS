#include "LightSensorApi.h"

#include <QDateTime>
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

// SMUX конфигурации (взяты из успешно работающей библиотеки Adafruit AS7341)
const uint8_t LightSensorApi::SMUX_CONFIG_CYCLE0[16] = {
    0x30, 0x31, 0x32, 0x33, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

const uint8_t LightSensorApi::SMUX_CONFIG_CYCLE1[16] = {
    0x10, 0x11, 0x12, 0x13, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

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
        qDebug() << "LightSensorApi: emulation mode enabled";
        m_initialized = true;
        return true;
    }

#ifdef Q_OS_LINUX
    qDebug() << "LightSensorApi: opening /dev/i2c-1";
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

    // 1. Проверка ID датчика
    uint8_t id = 0;
    if (!readRegister(REG_ID, id)) {
        emit errorOccurred("Failed to read ID register");
        return false;
    }
    qDebug() << "\t\tAS7341 ID = 0x" << Qt::hex << id << Qt::dec;
    if (id != 0x39 && id != 0x40) {
        emit errorOccurred(QString("Invalid AS7341 ID: 0x%1").arg(id, 2, 16));
        return false;
    }

    // 2. Включение питания (PON)
    if (!writeRegister(REG_ENABLE, 0x01)) {
        emit errorOccurred("Failed to set PON");
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    qDebug() << "\t\tPON set, delay 10ms";

    // 3. Выбор банка SMUX (регистр 0xEF = 0x00)
    // В даташите прямо не описан, но требуется для работы SMUX
    if (!writeRegister(0xEF, 0x00)) {
        emit errorOccurred("Failed to select SMUX bank");
        return false;
    }
    qDebug() << "\t\tSMUX bank selected (0xEF=0x00)";

    // 4. Настройка SMUX для цикла 0
    if (!writeSmuxConfig(SMUX_CONFIG_CYCLE0)) {
        emit errorOccurred("Failed to write SMUX config cycle 0");
        return false;
    }

    // 5. Включение SMUX (бит SMUXEN в ENABLE)
    uint8_t enable = 0;
    if (!readRegister(REG_ENABLE, enable)) return false;
    enable |= (1 << 4);  // SMUXEN
    if (!writeRegister(REG_ENABLE, enable)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // 6. Установка времени интеграции и усиления
    setIntegrationTimeMs(m_integrationTimeMs);
    setGainByIndex(m_gainIndex);

    qDebug() << "LightSensorApi: initialized successfully";
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
    // Запись 16 байт в SMUX регистры
    qDebug() << "\t\tWriting SMUX config, cycle"
             << (config == SMUX_CONFIG_CYCLE0 ? 0 : 1);

    for (int i = 0; i < 16; ++i) {
        if (!writeRegister(0x40 + i, config[i])) return false;
        qDebug() << "  reg 0x" << Qt::hex << (0x40 + i) << " = 0x" << config[i];
    }

    // Команда "записать конфигурацию из RAM в SMUX"
    uint8_t cmd = 2;  // SMUX_CMD = 2 (Write)
    if (!writeRegister(REG_SMUX_CMD, cmd)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    qDebug() << "\t\tSMUX_CMD write issued, delay 20ms";

    return true;
}

bool LightSensorApi::switchSmuxCycle(int cycle) {
    // Выбор банка SMUX (один раз уже сделан, но повторим для надёжности)
    if (!writeRegister(0xEF, 0x00)) return false;

    const uint8_t *config =
        (cycle == 0) ? SMUX_CONFIG_CYCLE0 : SMUX_CONFIG_CYCLE1;
    return writeSmuxConfig(config);
}
#endif

void LightSensorApi::setIntegrationTimeMs(int ms) {
    qDebug() << "\t\tsetIntegrationTimeMs:" << ms << "ms";
    m_integrationTimeMs = ms;
    if (m_emulationMode) return;
#ifdef Q_OS_LINUX
    int aStep = 599;                         // (ASTEP+1) = 600
    double stepTimeUs = 2.78 * (aStep + 1);  // 2.78 µs * 600 = 1668 µs
    double tintUs = ms * 1000.0;  // желаемое время в микросекундах
    int aTime = static_cast<int>(tintUs / stepTimeUs) - 1;
    if (aTime < 0) aTime = 0;
    if (aTime > 255) aTime = 255;

    qDebug() << "  ATIME =" << aTime << ", ASTEP =" << aStep;

    writeRegister(REG_ATIME, aTime);
    writeRegister(REG_ASTEP_L, aStep & 0xFF);
    writeRegister(REG_ASTEP_H, (aStep >> 8) & 0xFF);
#endif
}

void LightSensorApi::setGainByIndex(int index) {
    qDebug() << "setGainByIndex:" << index << "->"
             << (index == 0 ? 0.5 : (1 << (index - 1))) << "x";

    m_gainIndex = index;
    if (m_emulationMode) return;
#ifdef Q_OS_LINUX
    // Значения gain из даташита: 0=0.5x, 1=1x, 2=2x, 3=4x, 4=8x, 5=16x,
    // 6=32x, 7=64x, 8=128x, 9=256x, 10=512x
    if (index >= 0 && index <= 10) {
        writeRegister(REG_CFG1, static_cast<uint8_t>(index));
    }
#endif
}

bool LightSensorApi::readAllChannels(LightSensorData &data) {
    if (!m_initialized) return false;
    if (m_emulationMode) return emulationReadAllChannels(data);

#ifdef Q_OS_LINUX
    QVector<quint16> channels(11, 0);

    // ----- Цикл 0: F1, F2, F3, F4, NIR, CLEAR -----
    if (!switchSmuxCycle(0)) return false;
    if (!startMeasurement()) return false;
    if (!waitForDataReady(1000)) return false;

    uint8_t buf[12];
    if (!readBlock(REG_CH0_L, buf, 12)) return false;
    channels[CH_F1] = buf[0] | (buf[1] << 8);
    channels[CH_F2] = buf[2] | (buf[3] << 8);
    channels[CH_F3] = buf[4] | (buf[5] << 8);
    channels[CH_F4] = buf[6] | (buf[7] << 8);
    channels[CH_NIR] = buf[8] | (buf[9] << 8);
    channels[CH_CLEAR] = buf[10] | (buf[11] << 8);

    // ----- Цикл 1: F5, F6, F7, F8, FD, CLEAR -----
    if (!switchSmuxCycle(1)) return false;
    if (!startMeasurement()) return false;
    if (!waitForDataReady(1000)) return false;
    if (!readBlock(REG_CH0_L, buf, 12)) return false;
    channels[CH_F5] = buf[0] | (buf[1] << 8);
    channels[CH_F6] = buf[2] | (buf[3] << 8);
    channels[CH_F7] = buf[4] | (buf[5] << 8);
    channels[CH_F8] = buf[6] | (buf[7] << 8);
    channels[CH_FD] = buf[8] | (buf[9] << 8);
    // CLEAR усредняем с предыдущим значением
    quint16 clear2 = buf[10] | (buf[11] << 8);
    channels[CH_CLEAR] = (channels[CH_CLEAR] + clear2) / 2;

    data.dateTime =
        QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.z");
    data.channels = channels;
    data.integrationTimeMs = m_integrationTimeMs;
    data.gainIndex = m_gainIndex;
    qDebug() << "Channels:"
             << "F1=" << channels[CH_F1] << "F2=" << channels[CH_F2]
             << "F3=" << channels[CH_F3] << "F4=" << channels[CH_F4]
             << "F5=" << channels[CH_F5] << "F6=" << channels[CH_F6]
             << "F7=" << channels[CH_F7] << "F8=" << channels[CH_F8]
             << "NIR=" << channels[CH_NIR] << "CLEAR=" << channels[CH_CLEAR]
             << "FD=" << channels[CH_FD];
    return true;
#else
    return false;
#endif
}

#ifdef Q_OS_LINUX
bool LightSensorApi::startMeasurement() {
    qDebug() << "\t\tstartMeasurement: setting SP_EN";
    uint8_t enable;
    readRegister(REG_ENABLE, enable);
    enable |= (1 << 1);
    bool ok = writeRegister(REG_ENABLE, enable);
    qDebug() << "  new ENABLE = 0x" << Qt::hex << enable;
    return ok;
}

bool LightSensorApi::waitForDataReady(int timeoutMs) {
    qDebug() << "waitForDataReady: timeout" << timeoutMs << "ms";
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
    qDebug() << "\tI2C WRITE: reg 0x" << Qt::hex << reg << " = 0x" << value;

    uint8_t buf[2] = {reg, value};
    if (write(m_i2cFd, buf, 2) != 2) {
        emit errorOccurred(QString("Write reg 0x%1 failed").arg(reg, 2, 16));
        return false;
    }
    qDebug() << "  -> OK";
    return true;
}

bool LightSensorApi::readRegister(uint8_t reg, uint8_t &value) {
    qDebug() << "I2C READ : reg 0x" << Qt::hex << reg;

    if (write(m_i2cFd, &reg, 1) != 1) return false;
    if (read(m_i2cFd, &value, 1) != 1) return false;
    qDebug() << "  -> value = 0x" << Qt::hex << value;
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
    data.dateTime =
        QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.z");
    data.channels = channels;
    data.integrationTimeMs = m_integrationTimeMs;
    data.gainIndex = m_gainIndex;
    return true;
}
