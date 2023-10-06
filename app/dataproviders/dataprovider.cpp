#include "dataprovider.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

//======================================================================
DataProvider::DataProvider(const QString &name): QObject(),
  data_obtained(false)
{
    this->name = name;
}

//======================================================================
QString DataProvider::readableName()
{
    return name;
}

//======================================================================
void DataProvider::obtainDataWrapper()
{
    prepareTrace();
    emit dataReadStarted();
    data_obtained = obtainData();
    getTrace()->doPostProcessing();
    emit dataReadFinished();
}

//======================================================================
bool DataProvider::obtainData()
{
    qDebug() << "Empty DataProvider obtain data";
    emit dataBatchReady(0,0);
    return true;
}

//======================================================================
void DataProvider::run(AbstractTrace *trace)
{
    assert(trace != nullptr);

    this->out_trace = trace;

    QtConcurrent::run(this, &DataProvider::obtainDataWrapper);
}

//======================================================================
bool DataProvider::dataObtainedSuccessfully()
{
    return data_obtained;
}

//======================================================================
AbstractTrace *DataProvider::getTrace()
{
    return out_trace;
}


//======================================================================
DataProvider::~DataProvider()
{
    qDebug() << "Data provider destroyd (but data not!) "<<readableName();
}





