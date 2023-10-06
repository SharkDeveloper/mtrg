#ifndef LAYFLATMEASUREMENT_H
#define LAYFLATMEASUREMENT_H

#include "measurement.h"
#include "math/algos.h"

class LayFlatMeasurement: public Measurement
{
public:
    LayFlatMeasurement(QCustomPlot *p);
    ~LayFlatMeasurement();



    QString name() override;
    QString value() override;
    double  dvalue() override;
    void select(bool v) override;
    QString typestring() override;
    void activatePlotDecorators() override;
private:

    void calculateMeasurement() override;


    Algos::RMS rms_;

private slots:
    void graphSelectionChanged(const QCPDataSelection &);
};


#endif // LAYFLATMEASUREMENT_H
