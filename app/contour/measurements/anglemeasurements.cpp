#include "anglemeasurements.h"

#include "math/algos.h"
#include "device/settings.h"

const qreal kArcMarkersOverlapMult = 1.1;
const QString kRegualrAngleName = "Угол";
const QString kVertAngleName = "Угол (В)";
const QString kHorAngleName = "Угол (Г)";

//======================================================================
AngleMeasurement::AngleMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count):Measurement(type, p, 1),
    lable_positioning_(false), ellipse_(nullptr), text_(nullptr),
    name_(name)
{
    initGraphics();
    addConnection(connect(plot(), &QCustomPlot::mouseRelease, this, [=](QMouseEvent *event)
    {
        if (event->button() != Qt::LeftButton)
            return;

        if (lable_positioning_)
        {
           finish();
           emit statusText(AngleMeasurement::name() + ": " + value());
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
void AngleMeasurement::activatePlotDecorators()
{
    plot()->setXSelectionArrowVisibility(true);
}


//======================================================================
QString AngleMeasurement::typestring()
{
    return "Углы";
}

//======================================================================
void AngleMeasurement::reloadSettings()
{
    angle_in_doubles_ = Settings::instance().view.angles_in_deg;
}

//======================================================================
void AngleMeasurement::initGraphics()
{
    marker_[0] = new QCPItemLine(plot());
    marker_[1] = new QCPItemLine(plot());

    marker_[0]->setPen(kPenDefault);
    marker_[1]->setPen(kPenDefault);

    ellipse_ = new QCPItemArc(plot());
    ellipse_->setPen(kPenDefault);

    text_ = new QCPItemText(plot());
    text_->setFont(kFontFixed);


    text_->setLayer("overlay");
    ellipse_->setLayer("overlay");
    marker_[0]->setLayer("overlay");
    marker_[1]->setLayer("overlay");

    showEverything(false);
    select(true);
}

//======================================================================
void AngleMeasurement::select(bool v)
{
    const QPen p = v ? kPenActive : kPenDefault;
    marker_[0]->setPen(p);
    marker_[1]->setPen(p);
    ellipse_->setPen(p);
}

//======================================================================
AngleMeasurement::~AngleMeasurement()
{
    plot()->removeItem(marker_[0]);
    plot()->removeItem(marker_[1]);
    plot()->removeItem(text_);
    plot()->removeItem(ellipse_);
}

//======================================================================
QString AngleMeasurement::name()
{
    return name_;
}

//======================================================================
QString AngleMeasurement::value()
{
    if (angle_in_doubles_)
        return QString::number(value_, 'f', 4) + "°";

    //switch the value to positive
    int degs =  floor(value_);

    double delta = value_ - degs;

    //gets minutes and seconds
    int mseconds = (int)floor(3600.0 * delta);
    int seconds = mseconds % 60;
    int minutes = (int)floor(mseconds / 60.0);

    if (selectionCount() > 0)
    {
        return QString::number(degs)+"° "+QString::number(minutes)+"' "+QString::number(seconds)+"''";
    }
    else
        return kUnknownValue;
}

//======================================================================
double AngleMeasurement::dvalue()
{
    if (selectionCount() > 0)
    {
        return value_;
    }
    else
        return 0;
}

//======================================================================
void AngleMeasurement::setMarkerPosition(int id, QVector2D &pos_start, QVector2D &pos_end)
{
    setQCPLinePositionVector(marker_[id], pos_start, pos_end);
}

//======================================================================
void AngleMeasurement::setCalculatedAngleDeg(double value)
{
    value_ = qAbs(value);
}

//======================================================================
void AngleMeasurement::setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment)
{
    text_->setText(name_text_ + "\n" + text);
    text_->position->setCoords(position.toPointF());
    text_->setRotation(rot_deg);
    text_->setPositionAlignment(alignment);
}

//======================================================================
void AngleMeasurement::showEverything(bool v)
{
    marker_[0]->setVisible(v);
    marker_[1]->setVisible(v);
    ellipse_->setVisible(v);
    text_->setVisible(v);
}

//======================================================================
void AngleMeasurement::showMarker(int id, bool v)
{
    marker_[id]->setVisible(v);
}

//======================================================================
void AngleMeasurement::showArc(bool v)
{
    ellipse_->setVisible(v);
}

//======================================================================
void AngleMeasurement::showText(bool v)
{
    text_->setVisible(v);
}

//======================================================================
void AngleMeasurement::setArcPosition(const QPointF &topleft, const QPointF &bottomright, int start_a, int span_a)
{
    ellipse_->topLeft->setCoords(topleft);
    ellipse_->bottomRight->setCoords(bottomright);
    ellipse_->mStartAngle = start_a;
    ellipse_->mSpanAngle = span_a;
}

//======================================================================
void AngleMeasurement::calcValueAndGraphics(const QPointF &intersection, QVector2D &dir1, QVector2D &dir2)
{
    QPointF cursor_pos;

    //cursor pos calcs with respect to first selection start
    if (selectionCount() >= 1 && selection(0)->valid())
    {
        auto it_start = selection(0)->parent->data()->at(selection(0)->begin);
        cursor_pos = QPointF(it_start->mainKey() + rel_cursor_pos().x(), it_start->mainValue() +  rel_cursor_pos().y());
    }

    //calculate radius with respect to cursor distance to intersection point
    qreal radius = (QVector2D(intersection) - QVector2D(cursor_pos.x(), cursor_pos.y())).length();

    //calculate unit vectors for both rms

    setReper(0, intersection);

    //direct dir-vectors to mouse position according to intersections
    QVector2D cursor_dir(cursor_pos-intersection);
    cursor_dir.normalize();

    if ( QVector2D::dotProduct(dir1, cursor_dir) < 0)
    {
        dir1 *= -1;
    }

    if ( QVector2D::dotProduct(dir2, cursor_dir) < 0)
    {
        dir2 *= -1;
    }

    QLineF horizon(0,0, 1,0);
    QLineF v1(QPointF(0,0), dir1.toPointF());
    QLineF v2(QPointF(0,0), dir2.toPointF());

    qreal start_angle = v1.angleTo(horizon);
    qreal angle = v1.angleTo(v2);

    if (angle > 180)
        angle = -v2.angleTo(v1);

    setCalculatedAngleDeg(angle);
    setArcPosition(intersection - QPointF(radius,radius), intersection + QPointF(radius,radius), start_angle*16, -angle*16);
    setTextParams(value(), QVector2D(cursor_pos), 0, Qt::AlignBottom|Qt::AlignHCenter );

    QVector2D marker0_start(intersection - (dir1*radius*0.2).toPointF());
    QVector2D marker1_start(intersection - (dir2*radius*0.2).toPointF());
    QVector2D marker0_end(intersection + (dir1 * radius * kArcMarkersOverlapMult).toPointF());
    QVector2D marker1_end(intersection + (dir2 * radius * kArcMarkersOverlapMult).toPointF());

    setMarkerPosition(0, marker0_start, marker0_end);
    setMarkerPosition(1, marker1_start, marker1_end);

    showEverything(true);
    emit statusText(name()+": " + value());
}

double closest(double val, QVector<double> arr) {

    int min = arr[0];
    int closest = val;

    for (double v : arr) {
        double diff = qAbs(v - val);

        if (diff < min) {
            min = diff;
            closest = v;
        }
    }

    return closest;
}
//======================================================================
double AngleMeasurement::guessRequestedValue()
{
    return closest(dvalue(), {45,90,135,180});
}

//======================================================================
//**
//======================================================================
RegularAngleMeasurement::RegularAngleMeasurement(QCustomPlot *p) : AngleMeasurement(M_ANGLE_REGULAR, p, kRegualrAngleName, 2)
{

}


//======================================================================
void RegularAngleMeasurement::calculateMeasurement()
{

    //rms's are line equasions y = ax+c & y = bx+d
    //calculate intersection point

    if (selectionCount() <= 1)
    {
        emit statusText("Выделите два отрезка данных");
        return;
    }

    if (!allSelectionsValid())
    {
        emit statusText("Выделите два отрезка данных");
        return;
    }

    qDebug() << selection(0);
    qDebug() << selection(0)->parent;
    qDebug() << selection(0)->parent->data();

    auto it_start = selection(0)->parent->data()->at(selection(0)->begin);
    auto it_end =  selection(0)->parent->data()->at(selection(0)->end);
    auto rms1 = Algos::linearRMS(it_start, it_end);

    auto it_start2 = selection(1)->parent->data()->at(selection(1)->begin);
    auto it_end2 =  selection(1)->parent->data()->at(selection(1)->end);
    auto rms2 = Algos::linearRMS(it_start2, it_end2);



    qreal a = rms1.coef(1);
    qreal c = rms1.coef(0);
    qreal b = rms2.coef(1);
    qreal d = rms2.coef(0);

    qreal x_i = (d-c)/(a-b);
    qreal y_i = a*((d-c)/(a-b)) + c;

    QPointF intersection(x_i, y_i);

    //10 can be critical to precision
    QVector2D dir_vec0(10,a*10);
    QVector2D dir_vec1(10,b*10);

    dir_vec0.normalize();
    dir_vec1.normalize();

    calcValueAndGraphics(intersection, dir_vec0, dir_vec1);
}


//======================================================================
//**
//======================================================================
HorizontalAngleMeasurement::HorizontalAngleMeasurement(QCustomPlot *p) : AngleMeasurement(M_ANGLE_HHORIZON, p, kHorAngleName, 1)
{

}

//======================================================================
void HorizontalAngleMeasurement::calculateMeasurement()
{
    if (selectionCount() < 1 || !allSelectionsValid())
        return;

    auto it_start = selection(0)->parent->data()->at(selection(0)->begin);
    auto it_end =  selection(0)->parent->data()->at(selection(0)->end);

    auto rms = Algos::linearRMS(it_start, it_end);
    qreal a = rms.coef(1);

    int xe = selection(0)->end - 1;
    QPointF intersection(DATA_X(selection(0)->parent, xe), rms.fit(DATA_X(selection(0)->parent, xe)));

    //+10 can be critical to precision
    QVector2D dir_vec0(10,a*10);
    QVector2D dir_vec1(10,0);

    dir_vec0.normalize();
    dir_vec1.normalize();

    calcValueAndGraphics(intersection, dir_vec0, dir_vec1);
}

//======================================================================
//**
//======================================================================
VerticalAngleMeasurement::VerticalAngleMeasurement(QCustomPlot *p) : AngleMeasurement(M_ANGLE_VHORIZON, p, kVertAngleName, 1)
{

}

//======================================================================
void VerticalAngleMeasurement::calculateMeasurement()
{
    if (selectionCount() < 1 || !allSelectionsValid())
        return;

    auto it_start = selection(0)->parent->data()->at(selection(0)->begin);
    auto it_end =  selection(0)->parent->data()->at(selection(0)->end);
    auto rms = Algos::linearRMS(it_start, it_end);
    qreal a = rms.coef(1);

    int xe = selection(0)->end - 1;
    QPointF intersection(DATA_X(selection(0)->parent, xe), rms.fit(DATA_X(selection(0)->parent, xe)));

    //+10 can be critical to precision
    QVector2D dir_vec0(10,a*10);
    QVector2D dir_vec1(0,10);

    dir_vec0.normalize();
    dir_vec1.normalize();

    calcValueAndGraphics(intersection, dir_vec0, dir_vec1);
}

