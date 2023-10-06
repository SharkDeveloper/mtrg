#ifndef ANGLEMEASUREMENT_H
#define ANGLEMEASUREMENT_H

#include "measurement.h"
#include <QObject>

class AngleMeasurement: public Measurement
{
public:

    AngleMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count = 1);

    ~AngleMeasurement();

    QString name() override;
    QString value() override;
    double dvalue() override;
    void select(bool v) override;
    void reloadSettings() override;
    QString typestring() override;
    void activatePlotDecorators() override;
    double guessRequestedValue() override;

protected:
    void setMarkerPosition(int mid, QVector2D &pos_start, QVector2D &pos_end);
    void setArcPosition(const QPointF &topleft, const QPointF &bottomright, int start_a, int span_a);
    void setCalculatedAngleDeg(double value);
    void setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment);

    void showEverything(bool v);
    void showMarker(int id, bool v);
    void showArc( bool v);
    void showText( bool v);

    void calcValueAndGraphics(const QPointF &intersection, QVector2D &dir1,  QVector2D &dir2);

private:

    void initGraphics();

    QCPItemLine *marker_[2];
    QCPItemArc *ellipse_;
    QCPItemText *text_;

    double value_;

    bool angle_in_doubles_;

    bool lable_positioning_;
    QString name_;

};

class RegularAngleMeasurement: public AngleMeasurement
{
    Q_OBJECT
public:
    RegularAngleMeasurement( QCustomPlot *p);
    void calculateMeasurement() override;

};

class HorizontalAngleMeasurement: public AngleMeasurement
{
    Q_OBJECT
public:
    HorizontalAngleMeasurement( QCustomPlot *p);
    void calculateMeasurement() override;
};

class VerticalAngleMeasurement: public AngleMeasurement
{
    Q_OBJECT
public:
    VerticalAngleMeasurement( QCustomPlot *p);
    void calculateMeasurement() override;
};

#endif // ANGLEMEASUREMENT_H
