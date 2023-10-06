#include "errormeasurement.h"

#include "math/algos.h"
#include "device/settings.h"


const QString kRMSEName = "Ср. откл. от прямой";

//======================================================================
ErrorMeasurement::ErrorMeasurement(E_MEASUREMENTS type, QCustomPlot *p, const QString &name, int selections_count = 1) : Measurement(type, p, selections_count), name_(name)
{
    err_curve = new QCPCurve(p->xAxis, p->yAxis);
    err_zero_axis_line = new QCPItemLine(p);
    err_zero_axis_line->setPen(kPenMarker);
    err_curve->setSelectable(QCP::SelectionType::stNone);
}

//======================================================================
void ErrorMeasurement::setCalculatedValue(double value)
{
    value_ = value;
}

//======================================================================
QString ErrorMeasurement::name()
{
    return name_;
}

//======================================================================
ErrorMeasurement::~ErrorMeasurement()
{
   // delete err_graph_;
    plot()->removeItem(err_zero_axis_line);
    plot()->removePlottable(err_curve);
    //cause crash on QPlot destroy;
}

//======================================================================
void ErrorMeasurement::setErrorGraphData(QVector<double> x, QVector<double> erry, double height, Algos::RMS &rms, QVector2D nrml)
{

    //normilize error y to height
    {
        double miny = *std::min_element(erry.constBegin(), erry.constEnd());
        double maxy = *std::max_element(erry.constBegin(), erry.constEnd());

        for (int i = 0; i< erry.length(); i++)
            erry[i] = (erry[i]/(maxy-miny))*height;
    }

    //generate a line, parralel to rms with an offset=(offset/2-error_val)
    QVector<double> nx,ny;
    nrml.normalize();

    {
       for (int i =0; i < x.length(); i++)
       {
           double py = rms.fit(x[i]) + nrml.y()*(height/2.0-erry[i]);
           double px = x[i] + nrml.x()*(height/2.0-erry[i]);

           nx.push_back(px);
           ny.push_back(py);
       }
    }

    //set zero-y axis line
    QVector2D pos_start;
    pos_start.setX(x[0] + nrml.x()*(height/2.0));
    pos_start.setY(rms.fit(x[0]) + nrml.y()*(height/2.0));

    QVector2D pos_end;
    pos_end.setX(x[x.length()-1] + nrml.x()*(height/2.0));
    pos_end.setY(rms.fit(x[x.length()-1]) + nrml.y()*(height/2.0));

    setQCPLinePositionVector(err_zero_axis_line, pos_start, pos_end);
    err_curve->setData(nx,ny);

}

//======================================================================
RMSEMeasurement::RMSEMeasurement(QCustomPlot *p) : ErrorMeasurement(M_RMSE, p, kRMSEName, 1),
    lable_positioning_(false)
{

    initGraphics();

    addConnection(connect(plot(), &QCustomPlot::mouseRelease, this, [=](QMouseEvent *event)
    {

        if (event->button() != Qt::LeftButton)
            return;

        if (lable_positioning_)
        {
           plot()->setXSelectionArrowVisibility(true);
           finish();
           emit statusText(name() + ": " + value());
        }
        else if (selection(0))
        {
            stopSelection();
            emit statusText("Расположите значение");
            lable_positioning_=true;
            plot()->setXSelectionArrowVisibility(false);
            plot()->replot();
        }

    }));

    plot()->setInteractions(QCP::iSelectPlottables |  QCP::iRangeSelection | QCP::iRangeZoom   );
    plot()->setSelectionRectMode(QCP::srmSelectRealtime);

    for (int i = 0; i < plot()->curveCount(); i++)
    {
        plot()->curve(i)->setSelectable(QCP::SelectionType::stDataRange);
    }
    reloadSettings();

    emit statusText(name() + ": выделите участок измерения");
}

//======================================================================
void RMSEMeasurement::reloadSettings()
{
    switch (Settings::instance().view.axis_unit)
    {
        case ViewParameters::MM: //if axis in mm -> we use 1e3 modifier, so it is mk
            unit_ = "мк";
            break;
        case ViewParameters::NM: //leave as it is
            unit_ = "нм";
            break;
        case ViewParameters::MK: //if axis in MK -> we use 1e3 modifier, so it is NM
            unit_ = "нм";
            break;
    }
}

//======================================================================
void RMSEMeasurement::activatePlotDecorators()
{
    plot()->setXSelectionArrowVisibility(true);
}

//======================================================================
QString RMSEMeasurement::typestring(){
    return "Отклонения";
}

//======================================================================
void RMSEMeasurement::initGraphics()
{
    marker_[0] = new QCPItemLine(plot());
    marker_[1] = new QCPItemLine(plot());

    marker_[0]->setPen(kPenDefault);
    marker_[1]->setPen(kPenDefault);


    text_ = new QCPItemText(plot());
    text_->setFont(kFontFixed);

    text_->setLayer("overlay");

    marker_[0]->setLayer("overlay");
    marker_[1]->setLayer("overlay");

    select(true);
    showEverything(false);
}

//======================================================================
RMSEMeasurement::~RMSEMeasurement()
{
    plot()->removeItem(marker_[0]);
    plot()->removeItem(marker_[1]);

    plot()->removeItem(text_);
}

//======================================================================
void RMSEMeasurement::select(bool v)
{
    const QPen p = v ? kPenActive : kPenDefault;

    marker_[0]->setPen(p);
    marker_[1]->setPen(p);

}


//======================================================================
QString RMSEMeasurement::value()
{
    if (selectionCount()>0)
    {
        return QString::number(dvalue(), 'f', 4) + unit_;
    }
    else
    {
        return kUnknownValue;
    }
}

//======================================================================
double RMSEMeasurement::dvalue()
{
    if (selectionCount()>0)
    {
        if (Settings::instance().view.axis_unit != ViewParameters::NM)
            return value_*10e3;
        else
            return value_;
    }
    else
        return 0;
}

//======================================================================
void RMSEMeasurement::setMarkerPosition(int id, QVector2D &pos_start, QVector2D &pos_end)
{
    setQCPLinePositionVector(marker_[id], pos_start, pos_end);
}

//======================================================================
void RMSEMeasurement::setArrowPosition(QVector2D &pos_start, QVector2D &pos_end)
{
    //setQCPLinePositionVector(arrow_, pos_start, pos_end);
}


//======================================================================
void RMSEMeasurement::setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment)
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
void RMSEMeasurement::showEverything(bool v)
{
    marker_[0]->setVisible(v);
    marker_[1]->setVisible(v);

    text_->setVisible(v);
}


//======================================================================
void RMSEMeasurement::showMarker(int id, bool v)
{
    marker_[id]->setVisible(v);
}


//======================================================================
void RMSEMeasurement::showText(bool v)
{
    text_->setVisible(v);
}


//======================================================================
void RMSEMeasurement::calculateMeasurement()
{
    if (!selection(0) || !selection(0)->valid())
        return;

    plot()->setXSelectionArrowVisibility(false);

    int xs = selection(0)->begin;
    int xe = selection(0)->end - 1;
    QCPCurve *graph = selection(0)->parent;

    QVector2D marker1_pos(DATA_X(graph, xs), DATA_Y(graph, xs));
    QVector2D marker2_pos(DATA_X(graph, xe), DATA_Y(graph, xe));
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

    setArrowPosition(arrow1_pos, arrow2_pos);

    //calculate RootMeanSq error
    auto it_start = selection(0)->it_begin();
    auto it_end =  selection(0)->it_end();
    if (it_start == it_end)
        return;

    qDebug() << "DataPt count " << (int)(it_end-it_start);

    //calculate the line
    Algos::RMS rms = Algos::linearRMS(it_start, it_end);

    QVector<double> ex, ey;

    //calculate error (summ difference of rms & real data)
    {
        double squared_summ = 0;
        for (auto i = it_start; i != it_end; i++)
        {
            double real_x = i->mainKey();
            double real_y = i->mainValue();

            double line_y = rms.fit(real_x);

            squared_summ += (real_y-line_y)*(real_y-line_y);
            ex.push_back(real_x);
            ey.push_back((real_y-line_y)*1e3);
        }
        qDebug() << "Summt " << squared_summ;

        setCalculatedValue( (qSqrt(squared_summ) / (it_end - it_start)));
        setErrorGraphData(ex,ey,  offset, rms, nrml);
    }

    if (plane < 0)
        setTextParams(value(), arrow2_pos - (arrow2_pos - arrow1_pos)/2, getAngleToHorizon_deg(arrow1_pos, arrow2_pos), Qt::AlignTop|Qt::AlignHCenter);

    else
        setTextParams(value(), arrow2_pos - (arrow2_pos - arrow1_pos)/2, getAngleToHorizon_deg(arrow1_pos, arrow2_pos), Qt::AlignBottom|Qt::AlignHCenter);


    emit statusText(RMSEMeasurement::name() + ": " + value());
    showEverything(true);
}

//======================================================================
double RMSEMeasurement::guessOptimizationWeight()
{
    return 5;
}

const QString kRDispErrorName = "Cр. откл. от радиуса";

//======================================================================
RadiusDisplacementErrorMeasurement::RadiusDisplacementErrorMeasurement(QCustomPlot *p):ErrorMeasurement(M_RDISPLERROR, p, kRDispErrorName, 1),
    lable_positioning_(false)

{
    initGraphics();

    addConnection(connect(plot(), &QCustomPlot::mouseRelease, this, [=](QMouseEvent *event)
    {

        if (event->button() != Qt::LeftButton)
            return;

        if (lable_positioning_)
        {
            plot()->setXSelectionArrowVisibility(true);
            finish();
            qDebug() << "Curvature finish";
        }
        else if (selectionCount() == 1)
        {
            stopSelection();
            lable_positioning_=true;
            plot()->setXSelectionArrowVisibility(false);
            plot()->replot();
        }

    }));

    plot()->setInteractions(QCP::iSelectPlottables |  QCP::iRangeSelection |QCP::iRangeZoom  );
    plot()->setSelectionRectMode(QCP::srmSelectRealtime);

    for (int i = 0; i < plot()->curveCount(); i++)
    {
        plot()->curve(i)->setSelectable(QCP::SelectionType::stDataRange);
    }
    reloadSettings();
}

//======================================================================
void RadiusDisplacementErrorMeasurement::activatePlotDecorators()
{
    plot()->setXSelectionArrowVisibility(true);
}

//======================================================================
void RadiusDisplacementErrorMeasurement::reloadSettings()
{
    switch (Settings::instance().view.axis_unit)
    {
        case ViewParameters::MM: //if axis in mm -> we use 1e3 modifier, so it is mk
            unit_ = "мк";
            break;
        case ViewParameters::NM: //leave as it is
            unit_ = "нм";
            break;
        case ViewParameters::MK: //if axis in MK -> we use 1e3 modifier, so it is NM
            unit_ = "нм";
            break;
    }
}

//======================================================================
QString RadiusDisplacementErrorMeasurement::typestring()
{
    return  "Отклонения";
}

//======================================================================
void RadiusDisplacementErrorMeasurement::initGraphics()
{

    ellipse_ = new QCPItemEllipse(plot());
    ellipse_->setPen(kPenDefault);

    text_ = new QCPItemText(plot());
    text_->setFont(kFontFixed);
    text_->setLayer("overlay");
    ellipse_->setLayer("overlay");


    showEverything(false);
    select(true);
}

//======================================================================
void RadiusDisplacementErrorMeasurement::select(bool v)
{
    const QPen p = v ? kPenActive : kPenDefault;


    ellipse_->setPen(p);
}

//======================================================================
RadiusDisplacementErrorMeasurement::~RadiusDisplacementErrorMeasurement()
{

    plot()->removeItem(text_);
    plot()->removeItem(ellipse_);
}

//======================================================================
QString RadiusDisplacementErrorMeasurement::value()
{
    if (selection(0))
    {
        return QString::number(dvalue(), 'f', 4) + unit_;
    }
    else
        return kUnknownValue;
}

//======================================================================
double RadiusDisplacementErrorMeasurement::dvalue()
{
    if (selection(0))
    {
        if (Settings::instance().view.axis_unit != ViewParameters::NM)
            return value_*10e3;
        else
            return value_;
    }
    else
        return 0;
}

//======================================================================
void RadiusDisplacementErrorMeasurement::setMarkerPosition(QVector2D &pos_start, QVector2D &pos_end)
{
   // setQCPLinePositionVector(marker_, pos_start, pos_end);
}

//======================================================================
void RadiusDisplacementErrorMeasurement::setCalculatedRadius(double value)
{
    value_ = value;
}

//======================================================================
void RadiusDisplacementErrorMeasurement::setTextParams(const QString &text, const QVector2D &position, double rot_deg, Qt::Alignment alignment)
{
    text_->setText(name_text_ + "\n" + text);
    text_->position->setCoords(position.toPointF());
    text_->setRotation(rot_deg);
    text_->setPositionAlignment(alignment);
}

//======================================================================
void RadiusDisplacementErrorMeasurement::showEverything(bool v)
{

    ellipse_->setVisible(v);
    text_->setVisible(v);
}



//======================================================================
void RadiusDisplacementErrorMeasurement::showCircle(bool v)
{
    ellipse_->setVisible(v);
}

//======================================================================
void RadiusDisplacementErrorMeasurement::showText(bool v)
{
    text_->setVisible(v);
}

//======================================================================
void RadiusDisplacementErrorMeasurement::setCirclePosition(const QPointF &center, double radius)
{
    ellipse_->topLeft->setCoords(center - QPointF(radius, radius));
    ellipse_->bottomRight->setCoords(center + QPointF(radius, radius));
}

//======================================================================
void RadiusDisplacementErrorMeasurement::calcValueAndGraphics(const QPointF &center, double radius, double rmse)
{
    QPointF cursor_pos;

    //cursor pos calcs with respect to first selection start
    if (selectionCount() > 0)
    {
        auto it_start = selection(0)->parent->data()->at(selection(0)->begin);
        cursor_pos = QPointF(it_start->mainKey() + rel_cursor_pos().x(), it_start->mainValue() +  rel_cursor_pos().y());
    }

    setReper(0, center);

    //direct dir-vectors to mouse position according to intersections
    QVector2D cursor_dir(cursor_pos-center);
    cursor_dir.normalize();

    setCalculatedRadius(rmse);
    qDebug() << "Circled rmse "<<rmse;
    setCirclePosition(center, radius);
    setTextParams(value(), QVector2D(cursor_pos), 0, Qt::AlignBottom|Qt::AlignHCenter );

    QVector2D marker_start(center);
    QVector2D marker_end(center + (cursor_dir*radius).toPointF());
    setMarkerPosition(marker_start, marker_end);

    showEverything(true);

    emit statusText(name() + ": " + value());

}

//======================================================================
void RadiusDisplacementErrorMeasurement::calculateMeasurement()
{
    if (!selection(0))
        return;

    double rad;
    QPointF center;



    auto it_start = selection(0)->it_begin();
    auto it_end =  selection(0)->it_end();

    if (it_start == it_end)
        return;

    Algos::hyperfitCircle(it_start, it_end, center, rad);
    Algos::RMS rms = Algos::linearRMS(it_start, it_end);


    QVector<double> ex, ey;
    double squared_summ = 0;
    for (auto i = it_start; i != it_end; i++)
    {
        double real_x = i->mainKey();
        double real_y = i->mainValue();

        double dist = qSqrt((real_x - center.x())*(real_x - center.x()) + (real_y-center.y())*(real_y-center.y())) - rad;
        squared_summ += dist*dist;

        ex.push_back(real_x);
        ey.push_back(dist);
    }

    calcValueAndGraphics(center, rad,  (qSqrt(squared_summ) / (it_end - it_start)));

    //calculate the line

    QVector2D marker1_pos(selection(0)->data_begin());
    QVector2D marker2_pos(selection(0)->data_end());
    QVector2D nrml = getNormalVector(marker1_pos, marker2_pos);

    setErrorGraphData(ex,ey,  -rad, rms, nrml);
}


//======================================================================
double RadiusDisplacementErrorMeasurement::guessOptimizationWeight()
{
    return 5;
}

