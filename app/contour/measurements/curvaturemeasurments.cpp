#include "curvaturemeasurments.h"

#include <iterator>
#include <type_traits>
#include "math/algos.h"
#include "device/settings.h"

const QString kRegRadiusName = "Радиус";
const QString kArcRadiusName = "Радиус (A)";

//======================================================================
CurvatureMeasurement::CurvatureMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count):Measurement(type, p, 1), current_selection(0),

    lable_positioning_(false),
    name_(name)
{
    initGraphics();

    addConnection(connect(plot(), &QCustomPlot::mouseRelease, this, [=](QMouseEvent *event)
    {
        if (event->button() != Qt::LeftButton)
            return;

        if (lable_positioning_)
        {
           qDebug() << "Mouse Release lable!";
           finish();
           emit statusText(CurvatureMeasurement::name() + ": " + value());
           plot()->setXSelectionArrowVisibility(true);
           return;
        }
        else
        {
            if (selectionCount() < selections_count)
            {
                nextSelection();
            }
            else
            {
                stopSelection();
                emit statusText("Расположите значение");
                lable_positioning_ = true;
                plot()->setXSelectionArrowVisibility(false);
                plot()->replot();
            }
        }

    }));

    plot()->setInteractions(QCP::iSelectPlottables |  QCP::iRangeSelection | QCP::iRangeZoom  );
    plot()->setSelectionRectMode(QCP::srmSelectRealtime);

    for (int i = 0; i < plot()->curveCount(); i++)
    {
        plot()->curve(i)->setSelectable(QCP::SelectionType::stDataRange);
    }

    reloadSettings();

}


//======================================================================
void CurvatureMeasurement::activatePlotDecorators()
{
    plot()->setXSelectionArrowVisibility(true);
}

//======================================================================
void CurvatureMeasurement::reloadSettings()
{
    switch (Settings::instance().view.axis_unit)
    {
        case ViewParameters::MM:
            unit_ = "мм";
            break;
        case ViewParameters::NM:
            unit_ = "нм";
            break;
        case ViewParameters::MK:
            unit_ = "мкм";
            break;
    }
}

//======================================================================
QString CurvatureMeasurement::typestring()
{
    return  "Радиусы";
}

//======================================================================
void CurvatureMeasurement::initGraphics()
{
    marker_ = new QCPItemLine(plot());
    marker_->setPen(kPenDefault);
    marker_->setHead(QCPLineEnding::esSpikeArrow);

    ellipse_ = new QCPItemEllipse(plot());
    ellipse_->setPen(kPenDefault);

    text_ = new QCPItemText(plot());
    text_->setFont(kFontFixed);
    text_->setLayer("overlay");


    ellipse_->setLayer("overlay");

    marker_->setLayer("overlay");

    showEverything(false);
    select(true);
}

//======================================================================
void CurvatureMeasurement::select(bool v)
{
    const QPen p = v ? kPenActive : kPenDefault;

    marker_->setPen(p);
    ellipse_->setPen(p);
}

//======================================================================
CurvatureMeasurement::~CurvatureMeasurement()
{
    plot()->removeItem(marker_);
    plot()->removeItem(text_);
    plot()->removeItem(ellipse_);
}



//======================================================================
QString CurvatureMeasurement::name()
{
    return name_;
}

//======================================================================
QString CurvatureMeasurement::value()
{
    if (selection(0))
    {
        return QString::number(value_, 'f', 4) + unit_;
    }
    else
        return kUnknownValue;
}

//======================================================================
double CurvatureMeasurement::dvalue()
{
    if (selection(0))
    {
        return value_;
    }
    else
        return 0;
}

//======================================================================
void CurvatureMeasurement::setMarkerPosition(QVector2D &pos_start, QVector2D &pos_end)
{
    setQCPLinePositionVector(marker_, pos_start, pos_end);
}

//======================================================================
void CurvatureMeasurement::setCalculatedRadius(double value)
{
    value_ = value;
}

//======================================================================
void CurvatureMeasurement::setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment)
{
    text_->setText(name_text_ + "\n" + text);
    text_->position->setCoords(position.toPointF());
    text_->setRotation(rot_deg);
    text_->setPositionAlignment(alignment);
}

//======================================================================
void CurvatureMeasurement::showEverything(bool v)
{
    marker_->setVisible(v);
    ellipse_->setVisible(v);
    text_->setVisible(v);
}


//======================================================================
void CurvatureMeasurement::showMarker(bool v)
{
    marker_->setVisible(v);
}

//======================================================================
void CurvatureMeasurement::showCircle(bool v)
{
    ellipse_->setVisible(v);
}

//======================================================================
void CurvatureMeasurement::showText(bool v)
{
    text_->setVisible(v);
}

//======================================================================
void CurvatureMeasurement::setCirclePosition(const QPointF &center, double radius)
{
    ellipse_->topLeft->setCoords(center - QPointF(radius, radius));
    ellipse_->bottomRight->setCoords(center + QPointF(radius, radius));
}

//======================================================================
void CurvatureMeasurement::calcValueAndGraphics(const QPointF &center, double radius)
{
    QPointF cursor_pos;

    //cursor pos calcs with respect to first selection start
    if (allSelectionsValid())
    {
        auto it_start = selection(0)->it_begin();
        cursor_pos = QPointF(it_start->mainKey() + rel_cursor_pos().x(), it_start->mainValue() +  rel_cursor_pos().y());
    }

    setReper(0, center);

    //direct dir-vectors to mouse position according to intersections
    QVector2D cursor_dir(cursor_pos-center);
    cursor_dir.normalize();

    setCalculatedRadius(radius);
    setCirclePosition(center, radius);
    setTextParams(value(), QVector2D(cursor_pos), 0, Qt::AlignBottom|Qt::AlignHCenter );

    QVector2D marker_start(center);
    QVector2D marker_end(center + (cursor_dir*radius).toPointF());
    setMarkerPosition(marker_start, marker_end);

    showEverything(true);

    emit statusText(name() + ": " + value());

}

//======================================================================
//**
//======================================================================
RegularRadiusMeasurement::RegularRadiusMeasurement(QCustomPlot *p) : CurvatureMeasurement(M_RADIUS_NORMAL, p, kRegRadiusName, 1)
{

}


//======================================================================
void RegularRadiusMeasurement::calculateMeasurement()
{

    if (!allSelectionsValid())
        return;

    auto it_start = selection(0)->it_begin();
    auto it_end = selection(0)->it_end();

    if (it_start == it_end)
        return;

    QPointF center;
    double rad;

    Algos::hyperfitCircle(it_start, it_end, center, rad);

    calcValueAndGraphics(center, rad);
}


//======================================================================
//**
//======================================================================
ArcsRadiusMeasurement::ArcsRadiusMeasurement( QCustomPlot *p) : CurvatureMeasurement(M_RADIUS_ARCS, p, kArcRadiusName, 2)
{

}

//======================================================================
void ArcsRadiusMeasurement::calculateMeasurement()
{
    if (!allSelectionsValid() || selectionCount() != 2)
    {
        emit statusText("Выделите две арки");
        return;
    }


    auto it_start1 = selection(0)->it_begin();
    auto it_end1 =  selection(0)->it_end();

    auto it_start2 = selection(1)->it_begin();
    auto it_end2 =  selection(1)->it_end();

    QPointF center;
    double rad;

    Algos::hyperfitCircle2Arcs(it_start1, it_end1,
                               it_start2, it_end2,
                          center, rad);

    calcValueAndGraphics(center, rad);

}
