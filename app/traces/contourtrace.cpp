#include "contourtrace.h"
#include <QtMath>
#include "device/settings.h"
#include <QDebug>
//======================================================================
//A container for contour-trace (x,y,a)
//======================================================================
ContourTrace::ContourTrace(): AbstractTrace(3)
{

};

//======================================================================
//Clone data
//======================================================================
ContourTrace::ContourTrace(TData data): AbstractTrace(3)
{
    clearPreProcessors();
    clearPostProcessors();

    for (int i =0; i < data[0].length(); i++)
    {
        QVector<double> dpoint;
        dpoint << data[0][i] <<  data[1][i] << data[2][i];
        writeData(dpoint);
    }
}
