#ifndef SERIALDATAPROVIDER_H
#define SERIALDATAPROVIDER_H


#include "dataprovider.h"

#include "QString"
#include <QVector>

class SerialDataProvider : public DataProvider
{
public:
    SerialDataProvider(const QString &path, int baud);
private:

    QString path;
    int baud;
    bool obtainData();
    void prepareTrace() override;
};

#endif // SERIALDATAPROVIDER_H
