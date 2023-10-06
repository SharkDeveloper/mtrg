#include "layflatmeasurement.h"
#include "math/polinomialregression.h"

const QString kLayFlatMeasurementName  = "Выравнивание";
QPointF rotate_point(float cx, float cy, float angle, QPointF p){

     return QPointF(cos(angle) * (p.x() - cx) - sin(angle) * (p.y() - cy) + cx,
                  sin(angle) * (p.x() - cx) + cos(angle) * (p.y() - cy) + cy);
}

//======================================================================
LayFlatMeasurement::LayFlatMeasurement(QCustomPlot *p) : Measurement(M_LAY_FLAT, p, 0)
{
    addConnection(connect(plot(), &QCustomPlot::mouseRelease, this, [=](QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            stopSelection();
            qDebug() << "Left release" << name();

            if (!rms_.isValid())
            {
                qDebug() << "Invalid RMS";
                return;
            }

            qreal angle = QLineF(0, rms_.fit(0), 1, rms_.fit(1)).angleTo(QLineF (0, 0, 1, 0));

            if (angle > 180)
                angle = -QLineF (0, 0, 1, 0).angleTo(QLineF(0, rms_.fit(0), 1, rms_.fit(1)));

            angle = qDegreesToRadians(-angle);

            QPointF center_rot = selection(0)->data_begin();

            //modify plot
            for (int i =0; i< plot()->curveCount(); i++)
            {
                for (auto it  = plot()->curve(i)->data()->begin(); it != plot()->curve(i)->data()->end(); it++)
                {
                    // translate point back to origin:
                    QPointF rot = rotate_point(center_rot.x(),center_rot.y(), angle, QPointF(it->key, it->value));
                    it->key = rot.x();
                    it->value = rot.y();
                }
            }

            finish();

            emit graphDataChanged();
            emit statusText("Укладка готова: [" + QString::number(rms_.coef(0)) + ", " + QString::number(rms_.coef(1))+"]");
        }
    }));

    plot()->setInteractions(QCP::iSelectPlottables |  QCP::iRangeSelection | QCP::iRangeZoom );
    plot()->setSelectionRectMode(QCP::srmSelectRealtime);


    for (int i = 0; i < plot()->curveCount(); i++)
    {
        plot()->curve(i)->setSelectable(QCP::SelectionType::stDataRange);
    }

}

void LayFlatMeasurement::activatePlotDecorators()
{

    plot()->setXSelectionArrowVisibility(false);
}

//======================================================================
LayFlatMeasurement::~LayFlatMeasurement()
{
    qDebug() << "Lay flat destructor";
}

//======================================================================
void LayFlatMeasurement::calculateMeasurement()
{
    if (!allSelectionsValid())
        return;

    auto it_start = selection(0)->it_begin();
    auto it_end = selection(0)->it_end();

    qDebug() << it_start << it_end;

    if (it_start == it_end)
        return;

    rms_ = Algos::linearRMS(it_start, it_end);
}

//======================================================================
QString LayFlatMeasurement::name()
{
    return kLayFlatMeasurementName;
}

//======================================================================
QString LayFlatMeasurement::value()
{
    return kUnknownValue;
}
//======================================================================
void LayFlatMeasurement::select(bool v)
{

}

//======================================================================
double LayFlatMeasurement::dvalue()
{
    return 0;
}

//======================================================================
QString LayFlatMeasurement::typestring(){
    return  "";
}
