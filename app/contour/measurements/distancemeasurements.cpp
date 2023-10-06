#include "distancemeasurements.h"
#include "math/algos.h"
#include "device/settings.h"


const QString kDirectDistanseName = "Прямая";
const QString kHorisontalDistanseName = "Длина";
const QString kVerticalDistanseName = "Высота";
const QString kMeanHeightName = "Ср. Высота";

//======================================================================
DistanceMeasurement::DistanceMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count):Measurement(type, p, 2),
    lable_positioning_(false),
    name_(name),
    current_selection(0),
    p1(nullptr),
    p2(nullptr)
{
    initGraphics();

    //distance measurement are available between multiple curves on plot
    //mouse press was processed in MeasurementController, so
    //update p1 on creation.
    //
    // use mouse release to find second measurement point
    // and mousemove to handle changes...

    //in some cases, first we need to select a range on one curve, then select range on second curve
    //and calc firts and second points for this ranges, this will be handled in subclasses.


    //if one selection - use start point
    //end point will be found in mouse release
    if (selections_count == 1)
    {
        setP1(DataPoint(plot()->getXSelectionArrowPointIndex(), plot()->getXSelectionArrowPointCurve()));
        setP2(DataPoint(plot()->getXSelectionArrowPointIndex(), plot()->getXSelectionArrowPointCurve()));
    }

    auto mouse_move_connection = connect(plot(), &QCustomPlot::mouseMove, this, [=](QMouseEvent *event)
    {
        if (!isFinished() && !lable_positioning_)
        {

            if (selections_count > 1 && current_selection == 0)
            {
                //update p1 in case of multiple selection on first selection
                setP1(getCurrentMeasurementPoint());

            }
            else
            {
                setP2(getCurrentMeasurementPoint());
            }
        }
    });

    addConnection(mouse_move_connection);

    addConnection(connect(plot(), &QCustomPlot::mouseRelease, this, [=](QMouseEvent *event)
    {
        if (event->button() != Qt::LeftButton)
            return;

        if (lable_positioning_)
        {
           finish();
           plot()->setXSelectionArrowVisibility(true);
           emit statusText(DistanceMeasurement::name() + ": " + value());
           return;
        }
        else
        {
            if (selections_count > 1 && current_selection < selections_count - 1)
            {
                setP1(getCurrentMeasurementPoint());
                current_selection++;
                nextSelection();
            }
            else
            {
                setP2(getCurrentMeasurementPoint());
                stopSelection();
                emit statusText("Расположите значение");
                plot()->setXSelectionArrowVisibility(false);
                lable_positioning_ = true;
                plot()->replot();
            }
            //removeConnection(mouse_move_connection);
        }

    }));

    reloadSettings();
}

//======================================================================
void DistanceMeasurement::reloadSettings()
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

//======================================================================s
void DistanceMeasurement::setP1(DataPoint p)
{
    if (!p1)
        p1 = new DataPoint(p);
    else
        *p1 = p;

}

//======================================================================
void DistanceMeasurement::setP2(DataPoint p)
{    
    if (!p2)
        p2 = new DataPoint(p);
    else
        *p2 = p;

}

//======================================================================
DataPoint *DistanceMeasurement::P1()
{
    if (!p1)
        return p2;

    if (!p2)
        return p1;

    return p1->data().x() > p2->data().x() ? p2 : p1;
}

//======================================================================
DataPoint *DistanceMeasurement::P2()
{
    if (!p2)
        return p1;
    if (!p1)
        return p2;

    return p1->data().x() > p2->data().x() ? p1 : p2;
}

//======================================================================
void DistanceMeasurement::activatePlotDecorators()
{
    plot()->setXSelectionArrowVisibility(true);
}

//======================================================================
QString DistanceMeasurement::typestring(){
    return "Длины";
}

//======================================================================
void DistanceMeasurement::initGraphics()
{
    marker_[0] = new QCPItemLine(plot());
    marker_[1] = new QCPItemLine(plot());

    marker_[0]->setPen(kPenDefault);
    marker_[1]->setPen(kPenDefault);

    arrow_  = new QCPItemLine(plot());
    arrow_->setHead(QCPLineEnding(QCPLineEnding::esLineArrow));
    arrow_->setTail(QCPLineEnding(QCPLineEnding::esLineArrow));
    arrow_->setPen(kPenDefault);

    text_ = new QCPItemText(plot());
    text_->setFont(kFontFixed);


    text_->setLayer("overlay");
    arrow_->setLayer("overlay");
    marker_[0]->setLayer("overlay");
    marker_[1]->setLayer("overlay");

    select(true);
    showEverything(false);
}

//======================================================================
DistanceMeasurement::~DistanceMeasurement()
{
    plot()->removeItem(marker_[0]);
    plot()->removeItem(marker_[1]);
    plot()->removeItem(arrow_);
    plot()->removeItem(text_);
    delete p1;
    delete p2;
}

//======================================================================
void DistanceMeasurement::select(bool v)
{
    const QPen p = v ? kPenActive : kPenDefault;

    marker_[0]->setPen(p);
    marker_[1]->setPen(p);
    arrow_->setPen(p);
}

//======================================================================
QString DistanceMeasurement::name()
{
    return name_;
}

//======================================================================
QString DistanceMeasurement::value()
{
    return QString::number(value_, 'f', 4) + unit_;
}

//======================================================================
double DistanceMeasurement::dvalue()
{
   return value_;
}

//======================================================================
void DistanceMeasurement::setMarkerPosition(int id, QVector2D &pos_start, QVector2D &pos_end)
{
    setQCPLinePositionVector(marker_[id], pos_start, pos_end);
}

//======================================================================
void DistanceMeasurement::setArrowPosition(QVector2D &pos_start, QVector2D &pos_end)
{
    setQCPLinePositionVector(arrow_, pos_start, pos_end);
}

//======================================================================
void DistanceMeasurement::setCalculatedValue(double value)
{
    value_ = value;
}

//======================================================================
void DistanceMeasurement::setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment)
{
    if (alignment & Qt::AlignBottom)
        text_->setText(name_text_ + "\n" + text);
    else
        text_->setText(text + "\n" + name_text_);
    text_->position->setCoords(position.toPointF());
    text_->setRotation(rot_deg);
    text_->setPositionAlignment(alignment);
}

//======================================================================
void DistanceMeasurement::showEverything(bool v)
{
    marker_[0]->setVisible(v);
    marker_[1]->setVisible(v);
    arrow_->setVisible(v);
    text_->setVisible(v);
}


//======================================================================
void DistanceMeasurement::showMarker(int id, bool v)
{
    marker_[id]->setVisible(v);
}

//======================================================================
void DistanceMeasurement::showArrow(bool v)
{
    arrow_->setVisible(v);
}

//======================================================================
void DistanceMeasurement::showText(bool v)
{
    text_->setVisible(v);
}

//======================================================================
//**
//======================================================================
DirectDistanceMeasurement::DirectDistanceMeasurement(QCustomPlot *p) : DistanceMeasurement(M_DIRECT_DISTANCE, p, kDirectDistanseName)
{
    emit statusText(DistanceMeasurement::name() + ": выделите участок измерения");
}

//======================================================================
DataPoint DirectDistanceMeasurement::getCurrentMeasurementPoint()
{
    return DataPoint(plot()->getXSelectionArrowPointIndex(), plot()->getXSelectionArrowPointCurve());
}

//======================================================================
void DirectDistanceMeasurement::calculateMeasurement()
{
    //p1 & p2 always present
    QVector2D marker1_pos(P1()->data().x(), P1()->data().y());
    QVector2D marker2_pos(P2()->data().x(), P2()->data().y());
    QVector2D nrml = getNormalVector(marker1_pos, marker2_pos);

    //cursor position is relative to selection start values
    QVector2D cursor(marker1_pos.x() + rel_cursor_pos().x(), marker1_pos.y() + rel_cursor_pos().y());

    //position of cursor with respect the data line
    double plane = QVector2D::dotProduct(nrml, (cursor-marker1_pos));

    //cursor distance to data line
    double offset = cursor.distanceToLine(marker1_pos, (marker2_pos-marker1_pos).normalized());

    if (plane < 0)
        offset *= -1;

    QVector2D arrow1_pos = marker1_pos + offset*nrml;
    QVector2D arrow2_pos = marker2_pos + offset*nrml;

    setMarkerPosition(0, marker1_pos, arrow1_pos);
    setMarkerPosition(1, marker2_pos, arrow2_pos);

    setReper(0, marker1_pos.toPointF());
    setReper(1, marker2_pos.toPointF());

    setArrowPosition(arrow1_pos, arrow2_pos);

    setCalculatedValue( (marker2_pos-marker1_pos).length() );

    if (plane < 0)
        setTextParams(value(), arrow2_pos - (arrow2_pos - arrow1_pos)/2, getAngleToHorizon_deg(arrow1_pos, arrow2_pos), Qt::AlignTop|Qt::AlignHCenter);

    else
        setTextParams(value(), arrow2_pos - (arrow2_pos - arrow1_pos)/2, getAngleToHorizon_deg(arrow1_pos, arrow2_pos), Qt::AlignBottom|Qt::AlignHCenter);


    emit statusText(DistanceMeasurement::name() + ": " + value());
    showEverything(true);
}


//======================================================================
HorisontalDistanceMeasurment::HorisontalDistanceMeasurment(QCustomPlot *p) : DistanceMeasurement(M_HORISONTAL_DISTANCE, p, kHorisontalDistanseName)
{

}

//======================================================================
DataPoint HorisontalDistanceMeasurment::getCurrentMeasurementPoint()
{
    return DataPoint(plot()->getXSelectionArrowPointIndex(), plot()->getXSelectionArrowPointCurve());
}

//======================================================================
void HorisontalDistanceMeasurment::calculateMeasurement()
{
    QVector2D marker1_pos(P1()->data().x(), P1()->data().y());
    QVector2D marker2_pos(P2()->data().x(), P2()->data().y());

    double xdelta = fabs((marker2_pos - marker1_pos).x());

    //cursor position is relative to selection start values
    double calculated_y_pos = marker1_pos.y() + rel_cursor_pos().y();

    QVector2D arrow_pos_start(marker1_pos.x(), calculated_y_pos );
    QVector2D arrow_pos_end(marker2_pos.x(), calculated_y_pos);

    setMarkerPosition(0, marker1_pos, arrow_pos_start);
    setMarkerPosition(1, marker2_pos, arrow_pos_end);
    setArrowPosition(arrow_pos_start, arrow_pos_end);

    setReper(0, marker1_pos.toPointF());
    setReper(1, marker2_pos.toPointF());

    setCalculatedValue( xdelta );

    if (calculated_y_pos < marker1_pos.y())
        setTextParams(value(), arrow_pos_end - (arrow_pos_end - arrow_pos_start)/2, 0, Qt::AlignTop|Qt::AlignHCenter );

    else
        setTextParams(value(), arrow_pos_end - (arrow_pos_end - arrow_pos_start)/2, 0, Qt::AlignBottom|Qt::AlignHCenter);

    showEverything(true);
    emit statusText(DistanceMeasurement::name() + ": " + value());
}

//======================================================================
// **
//======================================================================
VerticalDistanceMeasurment::VerticalDistanceMeasurment(QCustomPlot *p) : DistanceMeasurement(M_VERTIAL_DISTANCE, p, kVerticalDistanseName )
{

}

//======================================================================
DataPoint VerticalDistanceMeasurment::getCurrentMeasurementPoint()
{
    return DataPoint(plot()->getXSelectionArrowPointIndex(), plot()->getXSelectionArrowPointCurve());
}

//======================================================================
void VerticalDistanceMeasurment::calculateMeasurement()
{

    //if (!graph())
    //    return;
    //plot()->setXSelectionArrowVisibility(false);
    //int xs = selection().dataRange().begin();
    //int xe = selection().dataRange().end();

    QVector2D marker1_pos(P1()->data().x(), P1()->data().y());
    QVector2D marker2_pos(P2()->data().x(), P2()->data().y());

    double ydelta = fabs((marker2_pos - marker1_pos).y());

    //cursor position is relative to selection start values
    double calculated_x_pos = marker1_pos.x() + rel_cursor_pos().x();

    QVector2D arrow_pos_start(calculated_x_pos, marker1_pos.y());
    QVector2D arrow_pos_end(calculated_x_pos, marker2_pos.y());

    setMarkerPosition(0, marker1_pos, arrow_pos_start);
    setMarkerPosition(1, marker2_pos, arrow_pos_end);
    setArrowPosition(arrow_pos_start, arrow_pos_end);
    setReper(0, marker1_pos.toPointF());
    setReper(1, marker2_pos.toPointF());
    setCalculatedValue(ydelta);


    if (calculated_x_pos < marker1_pos.x())
        setTextParams(value(), arrow_pos_end - (arrow_pos_end - arrow_pos_start)/2, 90, Qt::AlignTop|Qt::AlignHCenter );
    else
        setTextParams(value(), arrow_pos_end - (arrow_pos_end - arrow_pos_start)/2, 90, Qt::AlignBottom|Qt::AlignHCenter );

    showEverything(true);
    emit statusText(DistanceMeasurement::name() + ": " + value());
}

//======================================================================
//**
//======================================================================
MeanHeightMeasurement::MeanHeightMeasurement(QCustomPlot *p) : DistanceMeasurement(M_MEAN_HEIGHT, p, kMeanHeightName, 2)
{

    plot()->setInteractions(QCP::iSelectPlottables |  QCP::iRangeSelection | QCP::iRangeZoom  );
    plot()->setSelectionRectMode(QCP::srmSelectRealtime);

    for (int i = 0; i < plot()->curveCount(); i++)
    {
        plot()->curve(i)->setSelectable(QCP::SelectionType::stDataRange);
    }
    setP1(getCurrentMeasurementPoint());
}

//======================================================================
DataPoint MeanHeightMeasurement::getCurrentMeasurementPoint()
{
    return DataPoint();
}

//======================================================================
void MeanHeightMeasurement::calculateMeasurement()
{

    if (!allSelectionsValid())
    {
        emit statusText("Выделите две области для расчета");
        return;
    }

    if (selectionCount() > 0 )
    {
        double cursor_pos = selection(0)->data_begin().x() + rel_cursor_pos().x();

        QList<QVector2D> arrow_pos;


        auto it_start = selection(0)->it_begin();
        auto it_end =  selection(0)->it_end();

        double start_x = it_start->mainKey() + (it_end->mainKey()-it_start->mainKey()) / 2.0;
        double start_y = Algos::linearRMS(it_start, it_end).fit(start_x);

        double end_x = cursor_pos;
        double end_y = start_y;

        QVector2D marker_pos_start(start_x, start_y);
        QVector2D marker_pos_end(end_x, end_y);

        setMarkerPosition(0, marker_pos_start, marker_pos_end);
        setReper(0, marker_pos_start.toPointF());
        arrow_pos.push_back(marker_pos_end);
        showMarker(0, true);

        if (selectionCount() == 2)
        {

            auto it_start = selection(1)->it_begin();
            auto it_end =  selection(1)->it_end();

            double start_x = it_start->mainKey() + (it_end->mainKey()-it_start->mainKey()) / 2.0;
            double start_y = Algos::linearRMS(it_start, it_end).fit(start_x);

            double end_x = cursor_pos;
            double end_y = start_y;

            QVector2D marker2_pos_start(start_x, start_y);
            QVector2D marker2_pos_end(end_x, end_y);

            setMarkerPosition(1, marker2_pos_start, marker2_pos_end);
            setReper(1, marker2_pos_start.toPointF());
            arrow_pos.push_back(marker2_pos_end);
            showMarker(1, true);

            QVector2D arrow_pos_start = arrow_pos[0];
            QVector2D arrow_pos_end = arrow_pos[1];
            setArrowPosition(arrow_pos_start, arrow_pos_end);
            setCalculatedValue(fabs(arrow_pos_end.y() - arrow_pos_start.y()));
            setTextParams(value(), arrow_pos_end - (arrow_pos_end - arrow_pos_start)/2, 90, Qt::AlignBottom|Qt::AlignHCenter );
            showEverything(true);
            emit statusText(DistanceMeasurement::name() + ": " + value());
        }
    }
}

//======================================================================
//**
//======================================================================
AvgDiametertMeasurement::AvgDiametertMeasurement(QCustomPlot *p) : DistanceMeasurement(M_AVG_DIAMETER, p, kMeanHeightName, 2)
{

    plot()->setInteractions(QCP::iSelectPlottables |  QCP::iRangeSelection | QCP::iRangeZoom  );
    plot()->setSelectionRectMode(QCP::srmSelectRealtime);

    for (int i = 0; i < plot()->curveCount(); i++)
    {
        plot()->curve(i)->setSelectable(QCP::SelectionType::stDataRange);
    }
    setP1(getCurrentMeasurementPoint());
}

//======================================================================
DataPoint AvgDiametertMeasurement::getCurrentMeasurementPoint()
{
    return DataPoint();
}

//======================================================================
void AvgDiametertMeasurement::calculateMeasurement()
{

    if (!allSelectionsValid())
    {
        emit statusText("Выделите две области для расчета");
        return;
    }

    if (selectionCount() > 0 )
    {
        double cursor_pos = selection(0)->data_begin().x() + rel_cursor_pos().x();

        QList<QVector2D> arrow_pos;


        auto it_start = selection(0)->it_begin();
        auto it_end =  selection(0)->it_end();

        auto rms = Algos::linearRMS(it_start, it_end);
        double start_x = it_start->mainKey();
        double start_y = rms.fit(start_x);

        double end_x = it_end->mainKey();
        double end_y = rms.fit(end_x);;

        QVector2D marker_pos_start(start_x, start_y);
        QVector2D marker_pos_end(end_x, end_y);

        setMarkerPosition(0, marker_pos_start, marker_pos_end);
        setReper(0, marker_pos_start.toPointF());

        QVector2D pp(cursor_pos, rms.fit(cursor_pos));
        arrow_pos.push_back(pp);
        showMarker(0, true);

        if (selectionCount() == 2)
        {

            auto it_start = selection(1)->it_begin();
            auto it_end =  selection(1)->it_end();


            auto rms = Algos::linearRMS(it_start, it_end);
            double start_x = it_start->mainKey();
            double start_y = rms.fit(start_x);

            double end_x = it_end->mainKey();
            double end_y = rms.fit(end_x);;

            QVector2D marker2_pos_start(start_x, start_y);
            QVector2D marker2_pos_end(end_x, end_y);

            setMarkerPosition(1, marker2_pos_start, marker2_pos_end);
            setReper(1, marker2_pos_start.toPointF());
            QVector2D pp(cursor_pos, rms.fit(cursor_pos));
            arrow_pos.push_back(pp);
            showMarker(1, true);

            QVector2D arrow_pos_start = arrow_pos[0];
            QVector2D arrow_pos_end = arrow_pos[1];
            setArrowPosition(arrow_pos_start, arrow_pos_end);
            setCalculatedValue(fabs(arrow_pos_end.y() - arrow_pos_start.y()));
            setTextParams(value(), arrow_pos_end - (arrow_pos_end - arrow_pos_start)/2, 90, Qt::AlignBottom|Qt::AlignHCenter );
            showEverything(true);
            emit statusText(DistanceMeasurement::name() + ": " + value());
        }
    }
}

