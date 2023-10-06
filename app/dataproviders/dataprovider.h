#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QString>
#include <QObject>
#include <QVector>
#include "helpers/datatypes.h"
#include "traces/abstracttrace.h"

class DataProvider: public QObject
{
    Q_OBJECT

public:
    DataProvider(const QString & name);
    ~DataProvider();
    QString readableName();

    void run(AbstractTrace *out_trace);

    bool dataObtainedSuccessfully();

    AbstractTrace *getTrace();
    virtual void prepareTrace() = 0;

private:
    virtual bool obtainData();
    void obtainDataWrapper();

private:
    QString name;
    bool data_obtained;
    AbstractTrace *out_trace = nullptr;

signals:
    void dataReadStarted();
    void dataBatchReady(int from, int count);
    void dataReadFinished();
    void dataReadError(QString s);

};

#endif // DATAPROVIDER_H
