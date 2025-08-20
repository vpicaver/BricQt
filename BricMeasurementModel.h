#ifndef BRICMEASUREMENTMODEL_H
#define BRICMEASUREMENTMODEL_H

//Qt includes
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDateTime>

//Our includes
#include "BricClient.h"

class BricMeasurementModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        TimestampRole = Qt::UserRole + 1,
        TimestampStringRole,
        DistanceMetersRole,
        AzimuthDegreesRole,
        InclinationDegreesRole,
        ReferenceIndexRole,
        DipDegreesRole,
        RollDegreesRole,
        TemperatureCelsiusRole,
        SamplesAveragedRole,
        MeasurementTypeRole,
        Error1CodeRole,
        Error1Data1Role,
        Error1Data2Role,
        Error2CodeRole,
        Error2Data1Role,
        Error2Data2Role
    };
    Q_ENUM(Role)

    explicit BricMeasurementModel(QObject* parent = nullptr)
        : QAbstractListModel(parent) {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return m_measurements.size();
    }

    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    // QML-callable API
    Q_INVOKABLE void add(const BricMeasurement& measurement);

    Q_INVOKABLE void clear();


    int count() const {
        return m_measurements.size();
    }

signals:
    void countChanged();

private:
    QList<BricMeasurement> m_measurements {};

    // ---- Helper to convert BRIC time to QDateTime ----
    QDateTime bricTimeToDateTime(const BricTime& t) const;
};
#endif // BRICMEASUREMENTMODEL_H
