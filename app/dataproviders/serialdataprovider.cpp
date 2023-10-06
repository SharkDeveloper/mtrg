#include "serialdataprovider.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QFileInfo>
#include <QtEndian>
#include <QtMath>
#include "traces/abstracttrace.h"
const int kBatchNotificationSize = 200;

//======================================================================
SerialDataProvider::SerialDataProvider(const QString &path, int baud):
    DataProvider(path),
    path(path),
    baud(baud)
{

}

//======================================================================
void SerialDataProvider::prepareTrace()
{
    getTrace()->setSourceName(QFileInfo(path).fileName());

    getTrace()->clearPreProcessors();


    getTrace()->addSinglePointProcessor(new YFromAnglePointProcessor());
    getTrace()->addSinglePointProcessor(new MultiplicationProcessor(10.0e-6, {0, 1})); //convert nm to mm
}

//======================================================================
bool SerialDataProvider::obtainData()
{
    qDebug() << "SerialDataProvider obtain data from " << path;


    QFile f(path);

    if (f.open(QFile::ReadOnly) && !f.isOpen())
    {
        qDebug() << "Can't open file " << path;
        emit dataReadError("Can't open file ");
        return false;
    }

    //wait for ST bytes (means start)
    while (!f.atEnd())
    {
        //data in hex, read 4 bytes
        QByteArray bytes = f.read(2);
        if (bytes == "ST")
            break;
    }

    //start dataread
    long pts_readed = 0;
    while (!f.atEnd())
    {
        //read till "B"
        int bytes_skipped = 0;
        while (!f.atEnd())
        {
            QByteArray bytes = f.read(1);

            if (bytes[0] == 'B')
                break;
            else
                bytes_skipped++;

        }

        if (bytes_skipped != 0)
            qDebug() << "Skipped "<<bytes_skipped << " bytes till next data portion";

        //this is actual Y data
        QByteArray bytes_angle = f.read(4);
        QByteArray bytes_xpos = f.read(3);
        QByteArray bytes_r = f.read(1);
        QByteArray bytes_a = f.read(1);
        double angle = qFromLittleEndian<quint32>(bytes_angle);//bytes_angle[3] + bytes_angle[1]*256 + bytes_angle[2]*256*256 + bytes_angle[3]*256*256*256;
        double y = 0;
        double xpos = pts_readed;
        int alarm_byte = bytes_a[0];

        //till here should be read 10 bytes B + YYYY + XXX + R + A

        if (alarm_byte != 0)
        {
           //emit dataReadError("Alarm!");
           qDebug() << "Alarm byte "<<alarm_byte;
           continue;//return false;
        }

        QVector <double> data;
        data.push_back(xpos);
        data.push_back(y);
        data.push_back(angle);

        //y value is 0, but it is allright: Y value will be calculated
        //directly in ContourTrace, during preprocessing action.
        getTrace()->writeData(data);

        pts_readed ++;
        if (pts_readed % kBatchNotificationSize == 0){ emit dataBatchReady(pts_readed-kBatchNotificationSize, kBatchNotificationSize); }
    }

    return true;
}
