#ifndef ERRORMEASUREMENT_H
#define ERRORMEASUREMENT_H

#include "measurement.h"

class ErrorMeasurement: public Measurement
{
public:
    ErrorMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count);
    ~ErrorMeasurement();
protected:
    void setErrorGraphData(QVector<double> x, QVector<double> y, double height, Algos::RMS &rms, QVector2D ntml);
    QString name() override;

    QCPCurve *err_curve;
    QCPItemLine *err_zero_axis_line;
    void setCalculatedValue(double value);
    double value_;
    QString name_;

};

class RMSEMeasurement: public ErrorMeasurement
{
public:
    RMSEMeasurement(QCustomPlot *p);

    ~RMSEMeasurement();

    QString value() override;
    double  dvalue() override;

    void select(bool v) override;

    QString typestring() override;

    void reloadSettings() override;

    void activatePlotDecorators() override;

    double guessOptimizationWeight() override;

protected:
    void setMarkerPosition(int mid, QVector2D &pos_start, QVector2D &pos_end);
    void setArrowPosition(QVector2D &pos_start, QVector2D &pos_end);

    void setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment);

    void showEverything(bool v);
    void showMarker(int id, bool v);

    void showText( bool v);
    void calculateMeasurement() override;

    QString unit_;
private:

    void initGraphics();

    QCPItemLine *marker_[2];

    QCPItemText *text_;

    bool lable_positioning_;
};


class RadiusDisplacementErrorMeasurement: public ErrorMeasurement
{
public:

    RadiusDisplacementErrorMeasurement(QCustomPlot *p);

    ~RadiusDisplacementErrorMeasurement();

    QString value() override;
    double dvalue() override;
    void select(bool v) override;
    QString typestring() override;
    void reloadSettings() override;
    void activatePlotDecorators() override;
    void calculateMeasurement() override;
    double guessOptimizationWeight() override;


protected:
    void setMarkerPosition(QVector2D &pos_start, QVector2D &pos_end);
    void setCirclePosition(const QPointF &center, double raduis);
    void setCalculatedRadius(double value);
    void setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment);

    void showEverything(bool v);

    void showCircle( bool v);
    void showText( bool v);

    void calcValueAndGraphics(const QPointF &center, double radius, double rmse);


private:

    void initGraphics();


    QCPItemEllipse *ellipse_;
    QCPItemText *text_;


    double value_;


    bool lable_positioning_;
    QString name_;
    QString unit_;

};

#endif // ERRORMEASUREMENT_H
