#ifndef DISTANSEMEASUREMENTS_H
#define DISTANSEMEASUREMENTS_H

#include "measurement.h"

//======================================================================
class DataPoint
{
    public:
        size_t index;
        QCPCurve *parent;
        DataPoint(size_t index, QCPCurve *parent) : index(index), parent(parent) {};
        DataPoint() : index(0), parent(nullptr) {};

        QPointF data() { return QPointF(DATA_X(parent, index), DATA_Y(parent, index)); }

        bool valid() { return parent; }
};


class DistanceMeasurement: public Measurement
{
public:
    DistanceMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count = 1);

    ~DistanceMeasurement();

    QString name() override;
    QString value() override;
    double  dvalue() override;
    void select(bool v) override;
    QString typestring() override;
    void reloadSettings() override;
    void activatePlotDecorators() override;
    virtual DataPoint getCurrentMeasurementPoint() = 0;

protected:
    void setMarkerPosition(int mid, QVector2D &pos_start, QVector2D &pos_end);
    void setArrowPosition(QVector2D &pos_start, QVector2D &pos_end);
    void setCalculatedValue(double value);
    void setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment);

    void showEverything(bool v);
    void showMarker(int id, bool v);
    void showArrow( bool v);
    void showText( bool v);
    QString unit_;
    //points to calc distance between

    void setP1(DataPoint p);
    void setP2(DataPoint p);

    DataPoint *P1();
    DataPoint *P2();
    int current_selection;
private:

    void initGraphics();
    DataPoint *p1;
    DataPoint *p2;

    //virtual void calculateMeasurement() = 0;

    QCPItemLine *marker_[2];
    QCPItemLine *arrow_;
    QCPItemText *text_;


    double value_;



    bool lable_positioning_;
    QString name_;

};

class DirectDistanceMeasurement: public DistanceMeasurement
{

public:
    DirectDistanceMeasurement(QCustomPlot *p);
    void calculateMeasurement() override;
    DataPoint getCurrentMeasurementPoint() override;
};

class HorisontalDistanceMeasurment: public DistanceMeasurement
{

public:
    HorisontalDistanceMeasurment(QCustomPlot *p);
    void calculateMeasurement() override;
    DataPoint getCurrentMeasurementPoint() override;
};


class VerticalDistanceMeasurment: public DistanceMeasurement
{
public:
    VerticalDistanceMeasurment(QCustomPlot *p);
    void calculateMeasurement() override;
    DataPoint getCurrentMeasurementPoint() override;
};

class MeanHeightMeasurement: public DistanceMeasurement
{
public:
    MeanHeightMeasurement(QCustomPlot *p);
    void calculateMeasurement() override;
    DataPoint getCurrentMeasurementPoint() override;
};


class AvgDiametertMeasurement: public DistanceMeasurement
{
public:
    AvgDiametertMeasurement(QCustomPlot *p);
    void calculateMeasurement() override;
    DataPoint getCurrentMeasurementPoint() override;
};

#endif // DISTANSEMEASUREMENTS_H
