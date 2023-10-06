#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "serial/serial.h"
#include <QString>
#include <QFuture>
#include <QTimer>

enum E_DEVICE_TYPE {
    CNTR220,
    PROF130,
    KRUG,
};

enum E_DEVICE_SPEED{
    E_SPEED_025_MMS = 0,
    E_SPEED_050_MMS = 1,
    E_SPEED_100_MMS = 2,
    E_SPEED_200_MMS = 3
};

class Device: public QObject
{
    Q_OBJECT
public:
    static std::map <E_DEVICE_SPEED, QString> kSpeedMap;

    Device(QString port_address, E_DEVICE_TYPE type);
    Device();

    ~Device();

    bool connected();
    bool running();

    bool traceStarted();
    void start();
    void stop();
    long pointsReaded();
    static bool detect(std::string *);

    void mainThread();
    void limitTipAscendToCurrentLevel(bool v);
    QString address();
    void setType(int type);


public slots:
    void moveX(int val);
    void moveY(int val);
    void moveZ(int val);
    void moveXFixed(double val);
    void moveYFixed(double val);
    void moveZFixed(double val);
    void moveTipUp();
    void moveTipDown();
    void autoMaxCalib();

    void startTrace();
    void stopTrace();
    void setSpeed(int spd);

    void find();
    E_DEVICE_TYPE type();
private:


    serial::Serial *port_;
    E_DEVICE_TYPE device_type_;

    bool thread_running_;
    bool require_stop_;

    long pts_readed_;

    bool trace_started_;
    bool st_bytes_read_;

    bool limit_tip_acsend_;
    QByteArray tip_level_;

    QFuture<void> mainthread_ft_;
    QTimer * detection_timer_;
    std::string address_;
    inline QByteArray readOrDeviceLost(int bc);
    inline void writeOrDeviceLost(const std::string & data);

signals:
    void textSignal(const QString &s);
    void dataRecieved(double angle, uint xpos, uint alarm);
    void deviceLost();
    void deviceOpened();

};

#endif // DEVICE_H
