#ifndef CURVATUREMEASURMENTS_H
#define CURVATUREMEASURMENTS_H

#include "measurement.h"
#include "qcustomplot/qcustomplot.h"
#include <QObject>

class CurvatureMeasurement: public Measurement
{
public:

    CurvatureMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count = 1);

    ~CurvatureMeasurement();

    QString name() override;
    QString value() override;
    double dvalue() override;
    void select(bool v) override;
    QString typestring() override;
    void reloadSettings() override;
    void activatePlotDecorators() override;

protected:
    void setMarkerPosition(QVector2D &pos_start, QVector2D &pos_end);
    void setCirclePosition(const QPointF &center, double raduis);
    void setCalculatedRadius(double value);
    void setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment);

    void showEverything(bool v);
    void showMarker(bool v);
    void showCircle( bool v);
    void showText( bool v);

    void calcValueAndGraphics(const QPointF &center, double radius);


    void initGraphics();

    QCPItemLine *marker_;
    QCPItemEllipse *ellipse_;
    QCPItemText *text_;

    double value_;


    bool lable_positioning_;
    QString name_;
    QString unit_;
    int current_selection;

};

class RegularRadiusMeasurement: public CurvatureMeasurement
{
    Q_OBJECT
public:
    RegularRadiusMeasurement(QCustomPlot *p);
    void calculateMeasurement() override;

};

class ArcsRadiusMeasurement: public CurvatureMeasurement
{
    Q_OBJECT
public:
    ArcsRadiusMeasurement(QCustomPlot *p);
    void calculateMeasurement() override;
};

#endif // CURVATUREMEASURMENTS_H
