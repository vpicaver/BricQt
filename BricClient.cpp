#include "BricClient.h"

//Qt includes
#include <QLowEnergyController>
#include <QLowEnergyService>

BricClient::BricClient(QObject *parent)
    : QObject(parent)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BricClient::deviceFound);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BricClient::scanFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BricClient::scanError);
}

void BricClient::startScan() {
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(8000);
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BricClient::writeLastTime(const BricTime &t) {
    if (!m_measureSvc) {
        return;
    }
    const auto chr = m_measureSvc->characteristic(QBluetoothUuid(kLastTimeChr));
    if (!chr.isValid()) {
        return;
    }
    m_measureSvc->writeCharacteristic(chr, makeLastTimeWrite(t), QLowEnergyService::WriteWithResponse);
}

void BricClient::sendCommand(const QByteArray &asciiLowercase) {
    if (!m_ctrlSvc) {
        return;
    }
    const auto chr = m_ctrlSvc->characteristic(QBluetoothUuid(kDeviceCtrlChr));
    if (!chr.isValid()) {
        return;
    }
    QByteArray payload = asciiLowercase.left(20);
    m_ctrlSvc->writeCharacteristic(chr, payload, QLowEnergyService::WriteWithResponse);
}

void BricClient::deviceFound(const QBluetoothDeviceInfo &bluetoothInfo) {
    if (!(bluetoothInfo.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) {
        return;
    }
    if (!bluetoothInfo.name().startsWith(QStringLiteral("BRIC5_"))) {
        return;
    }

    qDebug() << "Decive found:" << bluetoothInfo.deviceUuid() << bluetoothInfo.name();

    m_discoveryAgent->stop();

    m_controller = QLowEnergyController::createCentral(bluetoothInfo, this);
    connect(m_controller, &QLowEnergyController::connected, this, [this]() {
        emit info(QStringLiteral("Connected"));
        m_controller->discoverServices();
    });
    connect(m_controller, &QLowEnergyController::disconnected, this, [this]() {
        emit disconnected();
    });
    connect(m_controller, qOverload<QLowEnergyController::Error>(&QLowEnergyController::errorOccurred),
            this, [this](QLowEnergyController::Error e) {
                emit info(QStringLiteral("Controller error %1").arg(int(e)));
            });
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &BricClient::serviceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &BricClient::servicesDone);

    m_controller->connectToDevice();
}

void BricClient::scanFinished() {
    emit info(QStringLiteral("Scan finished"));
}

void BricClient::scanError(QBluetoothDeviceDiscoveryAgent::Error e) {
    emit info(QStringLiteral("Scan error %1").arg(int(e)));
}

void BricClient::serviceDiscovered(const QBluetoothUuid &uuid) {
    if (uuid == QBluetoothUuid(kMeasSyncSvc)) {
        m_measureSvc = m_controller->createServiceObject(uuid, this);
    } else if (uuid == QBluetoothUuid(kDeviceCtrlSvc)) {
        m_ctrlSvc = m_controller->createServiceObject(uuid, this);
    } else if (uuid == QBluetoothUuid(kBatterySvc) || uuid == QBluetoothUuid(kDeviceInfoSvc)) {
        // optional
    }
}

void BricClient::servicesDone() {
    if (m_measureSvc) {
        setupMeasureService();
    }
    if (m_ctrlSvc) {
        connect(m_ctrlSvc, &QLowEnergyService::stateChanged, this, [](QLowEnergyService::ServiceState s) {
            Q_UNUSED(s);
        });
        m_ctrlSvc->discoverDetails();
    }
    emit connected();
}

void BricClient::onCharacteristicChanged(const QLowEnergyCharacteristic &chr, const QByteArray &value) {
    const auto uuid16 = chr.uuid().toUInt16();
    if (uuid16 == kMeasPrimaryChr) {
        m_pending.primary = parseMeasurementPrimary(value);
        m_gotPrimary = true;
    } else if (uuid16 == kMeasMetaChr) {
        m_pending.metadata = parseMeasurementMetadata(value);
        m_gotMeta = true;
    } else if (uuid16 == kMeasErrorsChr) {
        m_pending.errors = parseMeasurementErrors(value);
        m_gotErrors = true;
    }

    if (m_gotPrimary && m_gotMeta && m_gotErrors) {
        emit measurementReady(m_pending);
        m_pending = BricMeasurement{};
        m_gotPrimary = m_gotMeta = m_gotErrors = false;
    }
}

void BricClient::setupMeasureService() {
    connect(m_measureSvc, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState s) {
        if (s == QLowEnergyService::RemoteServiceDiscovered) {
            subscribeIndication(QBluetoothUuid(kMeasPrimaryChr));
            subscribeIndication(QBluetoothUuid(kMeasMetaChr));
            subscribeIndication(QBluetoothUuid(kMeasErrorsChr));
        }
    });
    connect(m_measureSvc, &QLowEnergyService::characteristicChanged,
            this, &BricClient::onCharacteristicChanged);
    m_measureSvc->discoverDetails();
}

void BricClient::subscribeIndication(const QBluetoothUuid &uuid) {
    const auto chr = m_measureSvc->characteristic(uuid);
    if (!chr.isValid()) {
        emit info(QStringLiteral("Characteristic %1 missing").arg(uuid.toString(QUuid::WithoutBraces)));
        return;
    }
    const auto ccc = chr.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!ccc.isValid()) {
        emit info(QStringLiteral("CCC missing for %1").arg(uuid.toString(QUuid::WithoutBraces)));
        return;
    }
    // 0x0002 = Indications, 0x0001 = Notifications
    m_measureSvc->writeDescriptor(ccc, QByteArray::fromHex("0200"));
}
