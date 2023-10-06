#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QObject>
#include "../thirdparty/qcustomplot/qcustomplot.h"
#include <QSharedPointer>
#include <QtMath>
#include "math/algos.h"
#include "helpers/constants.h"

enum E_MEASUREMENTS
{
    M_VERTIAL_DISTANCE,
    M_HORISONTAL_DISTANCE,
    M_DIRECT_DISTANCE,
    M_ANGLE_VHORIZON,
    M_ANGLE_HHORIZON,
    M_ANGLE_REGULAR,
    M_RADIUS_NORMAL,
    M_RADIUS_ARCS,
    M_LAY_FLAT,
    M_MEAN_HEIGHT,
    M_RMSE,
    M_RDISPLERROR,
    M_AVG_DIAMETER,
    M_NONE
};


//======================================================================
inline double DATA_X(QCPCurve* g, const int &index)
{
    return g->data()->at(index)->key;
}

//======================================================================
inline double DATA_Y(QCPCurve* g, const int &index)
{
    return g->data()->at(index)->value;
}

//======================================================================
class DataSelection
{
    public:
        size_t begin;
        size_t end;
        QCPCurve *parent;
        DataSelection(size_t begin, size_t end, QCPCurve *parent) : begin(begin), end(end), parent(parent) {};
        DataSelection() : begin(0), end(0), parent(nullptr) {};
        auto it_begin() { return parent->data()->at(begin); }
        auto it_end() { return parent->data()->at(end); }
        QPointF data_begin() { return QPointF(DATA_X(parent, begin), DATA_Y(parent, begin)); }
        QPointF data_end() { return QPointF(DATA_X(parent, end), DATA_Y(parent, end)); }

        bool valid() { return parent != nullptr; }
};


//======================================================================
inline QVector2D getNormalVector(const QVector2D &v1, const QVector2D &v2)
{
    return QVector2D(-((v2 - v1)).y(), ((v2 - v1).x())).normalized();
}

//======================================================================
inline double getAngleToHorizon_deg(const QVector2D &v1, const QVector2D &v2)
{
    return QLineF(v1.toPointF(), v2.toPointF()).angle();
}

//======================================================================
inline double getAngleForVecs_deg(const QVector2D &v1, const QVector2D &v2)
{
    return qAtan2(v2.y() - v1.y(), v2.x() - v1.x()) * 180/M_PI;
}

//======================================================================
inline double getAngleForRMS_deg(const Algos::RMS &v1, Algos::RMS &v2)
{
    return qAtan((v2.coef(1) - v1.coef(1))/(1+v2.coef(1)*v1.coef(1))) * 180/M_PI;
}

//======================================================================
inline void setQCPLinePositionVector (QCPItemLine *m, const QVector2D &start_pos, const QVector2D &end_pos)
{
    m->start->setCoords(start_pos.toPointF());
    m->end->setCoords(end_pos.toPointF());
}

class ReperPoint;
//======================================================================
class Measurement: public QObject
{
    Q_OBJECT;

public:

    Measurement(E_MEASUREMENTS type, QCustomPlot *p, int rep_count);
    ~Measurement();

    QCustomPlot *plot();

    int id();
    E_MEASUREMENTS type();


    bool isFinished();
    virtual QString name() = 0;
    virtual QString value() = 0;
    virtual double dvalue() = 0;
    virtual void calculateMeasurement() = 0;
    virtual QString typestring() = 0;
    static int objects_cnt() { return objects_cnt_; }
    void finish();

    virtual void select(bool v) = 0;
    virtual void reloadSettings() {};
    void setNameText(QString text);
    void clearNameText();
    virtual void activatePlotDecorators();

    virtual double guessRequestedValue();
    virtual double guessOptimizationWeight();
signals:
    void measumentChanged();
    void graphDataChanged();
    void reperPointChanged(ReperPoint *pt);
    void reperPointRemoved(ReperPoint *pt);
    void statusText(QString string);


protected:

    DataSelection * selection(size_t index);
    size_t  selectionCount();
    QVector <DataSelection> selections_;
    void changeSelection(size_t index, DataSelection s);
    void addSelection(DataSelection s);
    void nextSelection();
    void stopSelection();
    bool allSelectionsValid();


    QPointF rel_cursor_pos();
    QString name_text_;
    void addConnection(QMetaObject::Connection conn);
    void setReper(int id, QPointF pt);

    void deactivate();
    void removeConnection(const QMetaObject::Connection &conn);


private:


    QCustomPlot *plot_;

    QPointF relative_cursor_pos_;  //zero is selection start x & y values

    int id_;

    QList <QMetaObject::Connection> connections_;

    size_t current_selection_;

    static int objects_cnt_;
    static int static_index_;

    bool finished_;
    bool selection_stoped_;

    QList<ReperPoint*> repers_;
    E_MEASUREMENTS type_;

    void setReperCount(int val);

private slots:
    void graphSelectionChanged(const QCPDataSelection &sel);


};

class ReperPoint: public QObject {
    Q_OBJECT;
    QCPItemTracer * reper_;
    Measurement *m;
    QCustomPlot *plot_;


public:
    QPointF  getPosition();
    bool setActive(bool v);



    void setVisible(bool v);
    Measurement *measurement();

    ReperPoint(QCustomPlot *plot, Measurement *m);
    ~ReperPoint();

    void setPosition(QPointF pt);
};
#endif // MEASUREMENT_H
