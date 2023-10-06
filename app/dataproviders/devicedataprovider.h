#ifndef DEVICEDATAPROVIDER_H
#define DEVICEDATAPROVIDER_H

#include "dataprovider.h"
#include "device/device.h"

#include "QString"
#include <QVector>

class DeviceDataProvider : public DataProvider
{
public:
    DeviceDataProvider(Device *d);
    ~DeviceDataProvider();
private:
    bool obtainData();
    void prepareTrace() override;
    Device *device;
    long pts_readed_;
    QMetaObject::Connection conn;
    bool obtaining_data_;
};


#endif // DEVICEDATAPROVIDER_H
