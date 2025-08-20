#include "BricMeasurementModel.h"


QVariant BricMeasurementModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    const int row = index.row();
    if (row < 0 || row >= m_measurements.size()) {
        return {};
    }
    const BricMeasurement& m = m_measurements.at(row);

    switch (role) {
    case TimestampRole: {
        return bricTimeToDateTime(m.primary.timestamp);
    }
    case TimestampStringRole: {
        return bricTimeToDateTime(m.primary.timestamp).toString(Qt::ISODateWithMs);
    }
    case DistanceMetersRole: {
        return m.primary.distanceMeters;
    }
    case AzimuthDegreesRole: {
        return m.primary.azimuthDegrees;
    }
    case InclinationDegreesRole: {
        return m.primary.inclinationDegrees;
    }
    case ReferenceIndexRole: {
        return static_cast<qulonglong>(m.metadata.referenceIndex);
    }
    case DipDegreesRole: {
        return m.metadata.dipDegrees;
    }
    case RollDegreesRole: {
        return m.metadata.rollDegrees;
    }
    case TemperatureCelsiusRole: {
        return m.metadata.temperatureCelsius;
    }
    case SamplesAveragedRole: {
        return static_cast<int>(m.metadata.samplesAveraged);
    }
    case MeasurementTypeRole: {
        return static_cast<int>(m.metadata.measurementType);
    }
    case Error1CodeRole: {
        return static_cast<int>(m.errors.first.code);
    }
    case Error1Data1Role: {
        return m.errors.first.data1;
    }
    case Error1Data2Role: {
        return m.errors.first.data2;
    }
    case Error2CodeRole: {
        return static_cast<int>(m.errors.second.code);
    }
    case Error2Data1Role: {
        return m.errors.second.data1;
    }
    case Error2Data2Role: {
        return m.errors.second.data2;
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> BricMeasurementModel::roleNames() const  {
    return {
        { TimestampRole,          QByteArrayLiteral("timestamp") },
        { TimestampStringRole,    QByteArrayLiteral("timestampString") },
        { DistanceMetersRole,     QByteArrayLiteral("distanceMeters") },
        { AzimuthDegreesRole,     QByteArrayLiteral("azimuthDegrees") },
        { InclinationDegreesRole, QByteArrayLiteral("inclinationDegrees") },
        { ReferenceIndexRole,     QByteArrayLiteral("referenceIndex") },
        { DipDegreesRole,         QByteArrayLiteral("dipDegrees") },
        { RollDegreesRole,        QByteArrayLiteral("rollDegrees") },
        { TemperatureCelsiusRole, QByteArrayLiteral("temperatureCelsius") },
        { SamplesAveragedRole,    QByteArrayLiteral("samplesAveraged") },
        { MeasurementTypeRole,    QByteArrayLiteral("measurementType") },
        { Error1CodeRole,         QByteArrayLiteral("error1Code") },
        { Error1Data1Role,        QByteArrayLiteral("error1Data1") },
        { Error1Data2Role,        QByteArrayLiteral("error1Data2") },
        { Error2CodeRole,         QByteArrayLiteral("error2Code") },
        { Error2Data1Role,        QByteArrayLiteral("error2Data1") },
        { Error2Data2Role,        QByteArrayLiteral("error2Data2") }
    };
}

void BricMeasurementModel::add(const BricMeasurement &measurement) {
    const int newRow = m_measurements.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_measurements.append(measurement);
    endInsertRows();
    emit countChanged();
}

void BricMeasurementModel::clear() {
    if (m_measurements.isEmpty()) {
        return;
    }
    beginResetModel();
    m_measurements.clear();
    endResetModel();
    emit countChanged();
}

QDateTime BricMeasurementModel::bricTimeToDateTime(const BricTime &t) const  {
    const QDate date(static_cast<int>(t.year),
                     static_cast<int>(t.month),
                     static_cast<int>(t.day));
    const QTime time(static_cast<int>(t.hours),
                     static_cast<int>(t.minutes),
                     static_cast<int>(t.seconds),
                     static_cast<int>(t.centiseconds) * 10);
    return QDateTime({date, time});
}
