#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include <QObject>

#define PI 3.1415926
#define ZENITH -.83

class MyGeography : public QObject
{
    Q_OBJECT
public:
    explicit MyGeography(QObject *parent = 0);
    ~MyGeography();
    static  qreal GetDistance(double lat1, double lng1, double lat2, double lng2);
    static  qreal GetAzimuth(double lat1, double lng1, double lat2, double lng2);
    static  float calculateSunriseAsUTCHour(int year, int month, int day, float lat, float lng);
    static  float calculateSunsetAsUTCHour(int year, int month, int day, float lat, float lng);

signals:

public slots:

private:

};

#endif // GEOGRAPHY_H
