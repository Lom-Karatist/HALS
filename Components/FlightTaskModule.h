#ifndef FLIGHTTASKMODULE_H
#define FLIGHTTASKMODULE_H

#include <QObject>

class FlightTaskModule : public QObject {
    Q_OBJECT
public:
    explicit FlightTaskModule(QObject *parent = nullptr,
                              int ocSensorWidth = 1920,
                              int hsSensorHeight = 600);

    void setOcSensorWidth(int newOcSensorWidth);

    void setHsSensorHeight(int newHsSensorHeight);

public slots:
    void setAltitude(int altitude);

signals:
    void ocCharsWereUpdated(double fovMeters, double gsd);
    void hsCharsWereUpdated(double fovMeters, double gsd);

private:
    void recaculateCamerasChars();
    void recalculateSingleCameraChars(const int &fovDegree, int sensorSizePx,
                                      double &fovMeters, double &gsd);

    int m_flightAltitude;  //!< Высота полета в метрах

    int m_ocSensorWidth;
    int m_hsSensorHeight;

    const int m_ocFovWidthDegree =
        39;  //!< Размеры поля зрения по ширине в градусах (для обзорной камеры)
    const int m_hsFovWidthDegree =
        25;  //!< Размеры поля зрения по ширине в градусах (для сенсора
             //!< гиперспектрометра)
};

#endif  // FLIGHTTASKMODULE_H
