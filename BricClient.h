#ifndef BRICCLIENT_H
#define BRICCLIENT_H

//Qt includes
#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyCharacteristic>
#include <QIODevice>
#include <QDataStream>
class QLowEnergyController;
class QLowEnergyService;

// Standard services
static constexpr quint16 kDeviceInfoSvc   = 0x180A;
static constexpr quint16 kBatterySvc      = 0x180F;

// BRIC custom services
static constexpr quint16 kMeasSyncSvc     = 0x58D0;
static constexpr quint16 kMeasPrimaryChr  = 0x58D1; // Indicate
static constexpr quint16 kMeasMetaChr     = 0x58D2; // Indicate
static constexpr quint16 kMeasErrorsChr   = 0x58D3; // Indicate
static constexpr quint16 kLastTimeChr     = 0x58D4; // Read/Write

static constexpr quint16 kDeviceCtrlSvc   = 0x58E0;
static constexpr quint16 kDeviceCtrlChr   = 0x58E1; // Read/Write (ASCII cmd)

struct BricTime {
    quint16 year {0};
    quint8 month {0};
    quint8 day {0};
    quint8 hours {0};
    quint8 minutes {0};
    quint8 seconds {0};
    quint8 centiseconds {0};
};

struct BricPrimary {
    BricTime timestamp {};
    float distanceMeters {static_cast<float>(qQNaN())};
    float azimuthDegrees {static_cast<float>(qQNaN())};
    float inclinationDegrees {static_cast<float>(qQNaN())};
};

struct BricMetadata {
    quint32 referenceIndex {0};
    float dipDegrees {static_cast<float>(qQNaN())};
    float rollDegrees {static_cast<float>(qQNaN())};
    float temperatureCelsius {static_cast<float>(qQNaN())};
    quint16 samplesAveraged {0};
    quint8 measurementType {0};
};

struct BricErrorPair {
    quint8 code {0};
    float data1 {0};
    float data2 {0};
};

struct BricErrors {
    BricErrorPair first {};
    BricErrorPair second {};
};

struct BricMeasurement {
    BricPrimary primary {};
    BricMetadata metadata {};
    BricErrors errors {};
};

namespace detail {
template <typename T>
T readLE(QDataStream& ds) {
    T v{};
    ds.readRawData(reinterpret_cast<char*>(&v), sizeof(T));
    return v; // QDataStream respects byte order we set below
}


// QDataStream on QByteArray with LittleEndian
inline auto mkLE(const QByteArray& b) {
    auto ds = std::make_unique<QDataStream>(b);
    ds->setByteOrder(QDataStream::LittleEndian);
    ds->setFloatingPointPrecision(QDataStream::SinglePrecision);
    return ds;
}
}


inline BricPrimary parseMeasurementPrimary(const QByteArray& bytes) {
    auto ds = detail::mkLE(bytes);
    BricPrimary out;
    out.timestamp.year        = detail::readLE<quint16>(*ds);
    out.timestamp.month       = detail::readLE<quint8>(*ds);
    out.timestamp.day         = detail::readLE<quint8>(*ds);
    out.timestamp.hours       = detail::readLE<quint8>(*ds);
    out.timestamp.minutes     = detail::readLE<quint8>(*ds);
    out.timestamp.seconds     = detail::readLE<quint8>(*ds);
    out.timestamp.centiseconds= detail::readLE<quint8>(*ds);
    out.distanceMeters        = detail::readLE<float>(*ds);
    out.azimuthDegrees        = detail::readLE<float>(*ds);
    out.inclinationDegrees    = detail::readLE<float>(*ds);
    return out;
}

inline BricMetadata parseMeasurementMetadata(const QByteArray& bytes) {
    auto ds = detail::mkLE(bytes);
    BricMetadata out;
    out.referenceIndex        = detail::readLE<quint32>(*ds);
    out.dipDegrees            = detail::readLE<float>(*ds);
    out.rollDegrees           = detail::readLE<float>(*ds);
    out.temperatureCelsius    = detail::readLE<float>(*ds);
    out.samplesAveraged       = detail::readLE<quint16>(*ds);
    out.measurementType       = detail::readLE<quint8>(*ds);
    return out;
}

inline BricErrors parseMeasurementErrors(const QByteArray& bytes) {
    auto ds = detail::mkLE(bytes);
    BricErrors out;
    out.first.code  = detail::readLE<quint8>(*ds);
    out.first.data1 = detail::readLE<float>(*ds);
    out.first.data2 = detail::readLE<float>(*ds);
    out.second.code = detail::readLE<quint8>(*ds);
    out.second.data1= detail::readLE<float>(*ds);
    out.second.data2= detail::readLE<float>(*ds);
    return out;
}

inline QByteArray makeLastTimeWrite(const BricTime& t) {
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << t.year << t.month << t.day << t.hours << t.minutes << t.seconds << t.centiseconds;
    b.resize(20); // pad to 20 bytes per spec
    return b;
}

class BricClient final : public QObject {
    Q_OBJECT
public:
    explicit BricClient(QObject* parent = nullptr);

    Q_INVOKABLE void startScan();

    void writeLastTime(const BricTime& t);

    void sendCommand(const QByteArray& asciiLowercase);

signals:
    void connected();
    void disconnected();
    void measurementReady(const BricMeasurement& m);
    void info(const QString& message);

private slots:
    void deviceFound(const QBluetoothDeviceInfo& info);

    void scanFinished();

    void scanError(QBluetoothDeviceDiscoveryAgent::Error e);

    void serviceDiscovered(const QBluetoothUuid& uuid);

    void servicesDone();

    void onCharacteristicChanged(const QLowEnergyCharacteristic& chr, const QByteArray& value);

private:
    void setupMeasureService();

    void subscribeIndication(const QBluetoothUuid& uuid);

private:
    QBluetoothDeviceDiscoveryAgent* m_discoveryAgent {nullptr};
    QLowEnergyController* m_controller {nullptr};
    QLowEnergyService* m_measureSvc {nullptr};
    QLowEnergyService* m_ctrlSvc {nullptr};

    BricMeasurement m_pending {};
    bool m_gotPrimary {false};
    bool m_gotMeta {false};
    bool m_gotErrors {false};
};


#endif // BRICCLIENT_H
