#ifndef FILEDATAPROVIDER_H
#define FILEDATAPROVIDER_H

#include "dataprovider.h"
#include "QString"
#include <QVector>

class FileDataProvider : public DataProvider
{
public:
    FileDataProvider(const QString &file);
private:

    bool importContourFile(QString file);
    bool importProfileFile(QString file);
    bool importIGSFile(QString file);

    QString filename;
    bool obtainData();
    void prepareTrace() override;
};

#endif // FILEDATAPROVIDER_H
