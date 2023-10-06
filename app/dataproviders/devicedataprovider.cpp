#include "devicedataprovider.h"

#include <QDebug>
#include <QFile>
#include <QThread>
#include <QFileInfo>
#include <QtEndian>
#include <QtMath>
#include "traces/abstracttrace.h"
#include "device/settings.h"
const int kBatchNotificationSize = 200;

//======================================================================
DeviceDataProvider::DeviceDataProvider(Device *d):
    DataProvider(d->address()),
    device(d)

{
    pts_readed_ = 0;

    conn = connect(device, &Device::dataRecieved, this, [&](double angle, uint xpos, uint alarm) {
        if (!device->traceStarted())
        {
            return;
        }
        QVector <double> data;

        data.push_back(xpos);
        data.push_back(angle);
        data.push_back(angle);

        getTrace()->writeData(data);

        pts_readed_ ++;

        if (pts_readed_ % kBatchNotificationSize == 0){ emit dataBatchReady(pts_readed_-kBatchNotificationSize, kBatchNotificationSize); }

    });

}

//======================================================================
DeviceDataProvider::~DeviceDataProvider()
{
    obtaining_data_ = false;
}

//======================================================================
void DeviceDataProvider::prepareTrace()
{
    getTrace()->setSourceName(device->address());
    getTrace()->clearPreProcessors();

    if (device->type() == CNTR220)
    {
        getTrace()->addSinglePointProcessor(new YFromAnglePointProcessor());
        getTrace()->addSinglePointProcessor(new MultiplicationProcessor(10.0e-6, {0, 1})); //convert nm to mm
    }
    if (device->type() == PROF130 || device->type() == KRUG)
    {
        qDebug() << "No points processors for PROF130 & KRUG";
    }

}

//======================================================================
bool DeviceDataProvider::obtainData()
{
    obtaining_data_ = true;
    qDebug() << "DeviceDataProvider obtain data";


    device->startTrace();
    //have to block this thread cause obtainData should be blocking
    qDebug() << "waiting for trace finish";
    while (device->traceStarted() && obtaining_data_)
    {
        QThread::msleep(500);
    }

    QObject::disconnect(conn);
    qDebug() << "device data provider trace finished";

    return true;
}
