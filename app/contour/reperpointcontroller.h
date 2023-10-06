#ifndef REPERPOINTCONTROLLER_H
#define REPERPOINTCONTROLLER_H

#include <QObject>
#include "../qcustomplot/qcustomplot.h"
#include "measurements/measurement.h"



class ReperPointController: public QObject
{
    Q_OBJECT;
public:
    ReperPointController(QCustomPlot *plt);


private:
    QCustomPlot *plot_;
    QMap <ReperPoint *, ReperPoint *> repers;

public slots:
    void reperPointChanged(ReperPoint *p);
    void reperPointRemoved(ReperPoint *p);
    void mouseMove(QMouseEvent *e);
};

#endif // REPERPOINTCONTROLLER_H
