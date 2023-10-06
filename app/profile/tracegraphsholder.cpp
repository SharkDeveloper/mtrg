#include "tracegraphsholder.h"
#include "math/polinomialregression.h"
#include "math/histogram.hpp"
#include "math/algos.h"
#include <QObject>
#include <functional>
#include <QtConcurrent/QtConcurrent>
#include "helpers/constants.h"
#include "device/settings.h"


const QPen kPenRMS = QPen(QBrush(Qt::red), 1, Qt::DotLine);
const QPen kPenRMR = QPen(QBrush(Qt::blue), 1, Qt::DotLine);
//const QBrush kBrushRMR = QBrush(QColor(0, 0, 255, 20));

XAxisMarker::XAxisMarker(QCPAxis *parentAxis) : QObject(parentAxis),
    axis_(parentAxis), xval(0), yval(0)
{
    line_ = new QCPItemStraightLine(parentAxis->parentPlot());
    line_->setPen(kPenMarker);
    line_->setVisible(false);

    lable_ = new QCPItemText(parentAxis->parentPlot());
    lable_->setVisible(false);
    lable_->setLayer("overlay");
    lable_->setClipToAxisRect(false);
    lable_->setPadding(QMargins(3, -1, 3, -1));
    lable_->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    lable_->setFont(kFontFixed);
    lable_->position->setTypeX(QCPItemPosition::ptPlotCoords);//QCPItemPosition::ptPlotCoords
    lable_->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
    lable_->setSelectable(false);
}

XAxisMarker::~XAxisMarker()
{
    qDebug() << "XTag destroy";
    axis_->parentPlot()->removeItem(line_);
    axis_->parentPlot()->removeItem(lable_);
}

void XAxisMarker::setPen(const QPen &pen)
{
  lable_->setPen(pen);
}
void XAxisMarker::setBrush(const QBrush &brush)
{
  lable_->setBrush(brush);
}
void XAxisMarker::setText(const QString &text)
{
  lable_->setText(text);
}
void XAxisMarker::updatePosition(double xvalue,  double yvalue)
{
    line_->point1->setCoords(xvalue, 0);
    line_->point2->setCoords(xvalue, 1);
    lable_->position->setCoords(xvalue,0);
    this->xval = xvalue;
    this->yval = yvalue;
}

double XAxisMarker::getX()
{
    return xval;
}

double XAxisMarker::getY()
{
    return yval;
}

void XAxisMarker::setVisible(bool val)
{
    line_->setVisible(val);
    lable_->setVisible(val);
}

enum GraphType{
    P_MAIN = 0,
    P_HIST,
    P_RMR
};

QMap<TraceGraphsHolder::TraceParam::EParamType, TraceGraphsHolder::TraceParam> TraceGraphsHolder::params_data_ =
{
    {TraceGraphsHolder::TraceParam::E_PARAM_TRACE_LEN,
        TraceGraphsHolder::TraceParam("Длинна", &TraceGraphsHolder::getTraceLengthString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_TRACE_MEASURMENTS,
        TraceGraphsHolder::TraceParam("Количество точек", &TraceGraphsHolder::getTraceMeasurmentsCountString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_TRACE_STEP,
        TraceGraphsHolder::TraceParam("Шаг измерений", &TraceGraphsHolder::getTraceStepString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_RMS,
        TraceGraphsHolder::TraceParam("RMS", &TraceGraphsHolder::getRMSString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_RMR,
        TraceGraphsHolder::TraceParam("RMR", &TraceGraphsHolder::getRMRString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_RMS_1ST_ORDER_ANGLE,
        TraceGraphsHolder::TraceParam("RMS(1-O) *", &TraceGraphsHolder::getRMSFirstOrderStrings)},

    {TraceGraphsHolder::TraceParam::E_PARAM_DELTA_M_X,
        TraceGraphsHolder::TraceParam("dMX", &TraceGraphsHolder::getDMXString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_DELTA_M_Y,
        TraceGraphsHolder::TraceParam("dMY", &TraceGraphsHolder::getDMYString)},

    {TraceGraphsHolder::TraceParam::E_PARAM_RA,
        TraceGraphsHolder::TraceParam("Ra", &TraceGraphsHolder::getRaString)},
};

//======================================================================
QString TraceGraphsHolder::paramName(TraceGraphsHolder::TraceParam::EParamType param)
{
    return params_data_[param].name;
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))
//======================================================================
QString TraceGraphsHolder::paramValue(TraceGraphsHolder::TraceParam::EParamType param)
{
    return CALL_MEMBER_FN(*this, params_data_[param].getter)();
}


//======================================================================
TraceGraphsHolder::TraceGraphsHolder(int id, AbstractTrace *trace, QCustomPlot *mainp, QCustomPlot *histp, QCustomPlot *rmrp):
    trace(trace),
    dp_percents_(10),
    rmr_percents_(1),
    rmr_set_(false),
    markers_set_(false),
    id_(id),
    unit_("мм"),
    last_fit_line_enabled_(false),
    last_fit_data_enabled_(false),
    unit_multiplier_(1)
{
    plots_ << mainp << histp << rmrp;
    hist = new QCPBars(plots_[P_HIST]->yAxis, plots_[P_HIST]->xAxis);
    rmr = new QCPGraph(plots_[P_RMR]->yAxis, plots_[P_RMR]->xAxis);

    main = new QCPCurve(plots_[P_MAIN]->xAxis, plots_[P_MAIN]->yAxis);

    curve = new QCPCurve(plots_[P_MAIN]->xAxis, plots_[P_MAIN]->yAxis);

    rms_fit = plots_[P_MAIN]->addGraph();
    rms_fit->setSelectable(QCP::SelectionType::stNone);
    rms_fit->setVisible(false);

    for (int i = P_MAIN; i <= P_RMR; i++)
    {
        rms_line[i] = new QCPItemStraightLine(plots_[i]);
        rms_line[i]->setSelectable(QCP::SelectionType::stNone);
        rms_line[i]->setVisible(false);
        rms_line[i]->setPen(kPenRMS);

        rmr_line[i] = new QCPItemStraightLine(plots_[i]);
        rmr_line[i]->setSelectable(QCP::SelectionType::stNone);
        rmr_line[i]->setVisible(false);
        rmr_line[i]->setPen(kPenRMR);
    }

    QVariant v;
    v.setValue(this);

    rmr->setProperty("gholder", v);
    hist->setProperty("gholder", v);
    main->setProperty("gholder", v);

    main->setPen(kPenDefault);
    rmr->setPen(kPenDefault);

    rms_fit->setPen(kPenRMS);
    rms_fit->setSelectable(QCP::stNone);
    xmarker_tag[0] = new XAxisMarker(main->valueAxis());
    xmarker_tag[1] = new XAxisMarker(main->valueAxis());
    xmarker_tag[1]->setPen(kPenMarker);
    xmarker_tag[0]->setPen(kPenMarker);

    connect(main, SIGNAL(selectionChanged(const QCPDataSelection &)), this, SLOT(graphSelectionChanged(const QCPDataSelection &)));
    setMainSelectionMode(QCP::stDataRange);
}
//======================================================================
TraceGraphsHolder::~TraceGraphsHolder()
{

    delete xmarker_tag[0];
    delete xmarker_tag[1];

    //plots_[P_MAIN]->removeGraph(main);

    delete main;

    plots_[P_HIST]->removePlottable(hist);
    plots_[P_RMR]->removeGraph(rmr);
    plots_[P_MAIN]->removePlottable(rms_fit);

    for (int i = P_MAIN; i <= P_RMR; i++)
    {
        plots_[i]->removeItem(rmr_line[i]);
        plots_[i]->removeItem(rms_line[i]);
    }

    qDebug() << "TraceGraphHolder destructor done";
}

//======================================================================
void TraceGraphsHolder::graphSelectionChanged(const QCPDataSelection &selection)
{

    qDebug() << "selection change";

    if (selection.dataPointCount() == 0)
    {
        xmarker_tag[1]->setVisible(false);
        xmarker_tag[0]->setVisible(false);
        markers_set_ = false;
        return;
    }

    if (selection.dataRange().isValid())
    {
        double sx,sy;
        double ex,ey;

        //should use main_data_keys/vals instead of plot->data()
        //plot free it sometimese
        sx = main_data_keys.at(selection.dataRange().begin());
        sy = main_data_values.at(selection.dataRange().begin());
        ex = main_data_keys.at(selection.dataRange().end() - 1);
        ey = main_data_values.at(selection.dataRange().end() - 1);

        xmarker_tag[0]->updatePosition(sx, sy);
        xmarker_tag[1]->updatePosition(ex, ey);

        xmarker_tag[0]->setText(QString::number(sx));
        xmarker_tag[1]->setText(QString::number(ex));

        markers_set_ = true;
        xmarker_tag[0]->setVisible(true);
        xmarker_tag[1]->setVisible(true);
    }

    qDebug() << "selection change ended";
}

//======================================================================
void TraceGraphsHolder::select()
{
    main->setPen(kPenGraphSelected);
    hist->setPen(kPenGraphSelected);
    rmr->setPen(kPenGraphSelected);
}

//======================================================================
void TraceGraphsHolder::deselect()
{
    main->setPen(kPenDefault);
    hist->setPen(kPenDefault);
    rmr->setPen(kPenDefault);
}

double calcRaValue(QVector<double> & y, Algos::RMS rms)
{


    double sum = 0;
    for (int i = 0; i < y.length(); i++)
    {
        sum += fabs(rms.fit(y[i]) - y[i]);
    }

    return sum/y.length();
}

//======================================================================
void TraceGraphsHolder::calculateGraphs(bool fitting_line, bool fit_data)
{
    last_fit_line_enabled_ = fitting_line;
    last_fit_data_enabled_ = fit_data;



    auto data = trace->getLockedDataPtr();
    /*
    main_data_keys.clear();
    main_data_values.clear();

    for (auto i = main->data()->begin(); i!=main->data()->end(); i++)
    {
            main_data_keys.push_back(i->key);
            main_data_values.push_back(i->value);
    }*/

    //OR DO THE CUTTING FOR TRACE, NOT DATA
    main_data_keys =  data->at(0);
    main_data_values = data->at(1);

    for (int i =0; i < main_data_keys.size(); i++)
    {
        main_data_keys[i] *= unit_multiplier_;
        main_data_values[i] *= unit_multiplier_;
    }

    /*QVector<QCPCurveData> curvedata(data->at(0).size());
    for (int i =0; i < data->at(0).size(); i++)
    {
        curvedata[i] = QCPCurveData(i, data->at(0)[i], data->at(1)[i]);

    }
    curve->data()->set(curvedata,true);*/

    rms_data_keys.clear();
    rms_data_values.clear();
    rmr_data_values.clear();

    calculateRMS();

    if (fitting_line) //calc fit line
    {
        QVector<double> coeffs;
        PolynomialRegression().fitIt(main_data_keys, main_data_values, 2, coeffs);

        //same x-axis
        rms_data_keys = main_data_keys;

        for (int i = 0; i <  rms_data_keys.length(); i++)
        {
            //fit data by polynom
            rms_data_values.push_back(rms_data_keys[i]*rms_data_keys[i]*coeffs[2] + rms_data_keys[i]*coeffs[1] + coeffs[0]);
        }

        if (fit_data)
        {
            for (int i = 0; i <  main_data_keys.length(); i++)
            {
                //modfy main data to fit linear
                main_data_values[i] -= rms_data_values[i] - rms_;
            }
        }
    }

    //calculate RMS 1st order fit, and get an angle from start point to end point
    QVector<double> coeffs;
    if (PolynomialRegression().fitIt(main_data_keys, main_data_values, 1, coeffs))
    {
        QPointF start_point = QPointF(main_data_keys.first(), main_data_keys.first()*coeffs[1]+ coeffs[0]);
        QPointF end_point = QPointF(main_data_keys.last(), main_data_keys.last()*coeffs[1]+ coeffs[0]);

        QPointF delta = end_point - start_point;
        rms_fo_angle_ = atan2(delta.y(),  delta.x());
    }

    //swhow rms line
    if (!fitting_line || (fitting_line && fit_data))
    {
        setRMSLine(true);
    }
    else
    {
        setRMSLine(false);
    }

    //calculate GOST params
    ra_val_ = calcRaValue(main_data_values, Algos::RMS(coeffs));

    main->setData(main_data_keys, main_data_values);

}


//======================================================================
void TraceGraphsHolder::calculateRMS()
{
    double sum = 0;

    for (int i = 0; i < main_data_keys.length(); i++)
        sum += pow(main_data_values[i], 2);

    rms_ = sqrt(sum / main_data_values.length());
}


//======================================================================
double integrate( QCPGraphDataContainer::const_iterator start,  QCPGraphDataContainer::const_iterator end, double zero_y)
{
    double result = 0;
    double a = 0;
    double b = 0;
    double h = 0;
    double s = 0;
    for( auto i = start ; i <  end-1; i++)
    {
        //trapezoid formula
        if (i->mainValue() > zero_y)
        {
            a = i->mainValue();
            b = (i+1)->mainValue();

            h = (i+1)->mainKey()  - i->mainKey();

            s = h * (a + b) /2.0;

            result += fabs(s);
        }
    }

    return result;
}

double integrate( const QCPCurveData *start, const QCPCurveData *end, double zero_y)
{
    double result = 0;
    double a = 0;
    double b = 0;
    double h = 0;
    double s = 0;
    for( auto i = start ; i <  end-1; i++)
    {
        //trapezoid formula
        if (i->mainValue() > zero_y)
        {
            a = i->mainValue();
            b = (i+1)->mainValue();

            h = (i+1)->mainKey()  - i->mainKey();

            s = h * (a + b) /2.0;

            result += fabs(s);
        }
    }

    return result;
}

//======================================================================
void TraceGraphsHolder::calculateHsitAndRMR()
{
    //define ranges in view
    QCPRange x_range = plots_[P_MAIN]->xAxis->range();
    QCPRange y_range = plots_[P_MAIN]->yAxis->range();

    auto it_start = main->data()->findBegin(x_range.lower);
    auto it_end = main->data()->findEnd(x_range.upper);

    double maxy = it_start->mainValue();
    double miny = it_start->mainValue();
    int numpts = main_data_keys.length();


    //find min/max y values in region x_range and
    //calc 100% summ for histogram
    double fullsum = 0;
    for (auto i = it_start; i < it_end; i++)
    {
        if(i->mainValue() > y_range.lower && i->mainValue() < y_range.upper)
        {
            if (i->mainValue() > maxy)
                maxy = i->mainValue();

            if (i->mainValue() < miny)
                miny = i->mainValue();
        }

        fullsum += i->mainValue();
    }


    //calc y-range for hist and bin size and count
    double y_data_range_in_view = maxy - miny;
    double bin_size =  y_data_range_in_view * (dp_percents_ / 100.0);

    //do nothing if there is no data points
    if (bin_size == 0)
        return;

    int bin_count = y_data_range_in_view / bin_size;
    if (bin_count < 0 )
    {
        qDebug() << "Some strange data: no y data in view: "<<y_data_range_in_view;
        bin_count = 0;
    }
    //clear histogram data values
    hist_data_values.fill(0, bin_count);

    double curr_val = 0; //for optimisation
    int histpos = 0;

    for (auto i = it_start; i < it_end; i++)
    {
        if(i->mainValue() > y_range.lower && i->mainValue() < y_range.upper)
        {
            curr_val = i->mainValue() - miny;

            histpos = curr_val/bin_size;

            if (histpos >= bin_count) //special case border points
                histpos -=1;

            hist_data_values[histpos]+=1;
        }
    }

    //calculate histogram keys (y-axis)
    //normalize hist to actual percentage
    QVector <double> hist_keys(bin_count);
    for (int i = 0; i < bin_count; i++)
    {
         hist_keys[i]= (miny + bin_size/2 + i*(bin_size));
         hist_data_values[i] = (hist_data_values[i] / numpts) * 100.0;
    }


    //rmr dencity:
    int rmr_prec = 100;
    rmr_data_values.fill(0, rmr_prec);

    //calculate rmr data
    //hou much we accend from from min y for each iteration
    double step_size =  y_data_range_in_view / (double)rmr_prec;

    QVector<double> rmrkeys;
    rmrkeys.fill(0, rmr_prec);

    //max value of an integral to normalize to 100%
    double rmr_max_val = 0;

    //from 0 to 100% (from ymax to ymin step by step)
    //parallel integration
    QVector<QFuture <double>> futures(rmr_prec);

    for (int prc = 0; prc < rmr_prec; prc++)
    {
        futures[prc] = QtConcurrent::run([=](){

            return  integrate(it_start,
                      it_end,
                      prc*step_size + miny);
        });
    }

    for (int i =0; i < futures.length(); i++)
    {
        rmr_data_values[i] = futures[i].result();


        if (rmr_data_values[i] > rmr_max_val)
            rmr_max_val = rmr_data_values[i];

        //calc keys for every step (y-axis)
        rmrkeys[i] = miny + i*step_size;
    }

    //normalize rmr to actual percentage
    for (int i =0; i <rmr_data_values.length(); i++)
         rmr_data_values[i] =  (rmr_data_values[i] / rmr_max_val)*100;

    //set calculated data to graphs
    hist->setData(hist_keys, hist_data_values);
    hist->setWidth(bin_size);
    hist->setPen(Qt::NoPen);
    hist->setBrush(QColor(10, 140, 70, 160));

    rmr->setData(rmrkeys, rmr_data_values, true);
    //rmr->setBrush(kBrushRMR);

}

//======================================================================
void TraceGraphsHolder::showPoints(bool enabled)
{
    if (enabled)
    {
        main->setLineStyle(QCPCurve::lsNone);
        qDebug() << "Tool hed in mm " << Settings::instance().calibration.tool_head_radius_nm/1.0e6;
        main->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, 1));
    }

    else
    {
        main->setLineStyle(QCPCurve::lsLine);
        main->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
    }
}

//======================================================================
void TraceGraphsHolder::setVisible(bool visible)
{
    main->setVisible(visible);

    for(int i =0; i < 3; i++)
    {
        if (rms_linear_)
        {
            rms_line[i]->setVisible(visible);
        }

        if (rmr_set_)
        {
            rmr_line[i]->setVisible(visible);
        }
    }

    if (!rms_linear_)
    {
        rms_fit->setVisible(visible);
    }


    rmr->setVisible(visible);


    hist->setVisible(visible);

    if (markers_set_)
    {
        xmarker_tag[0]->setVisible(visible);
        xmarker_tag[1]->setVisible(visible);
    }
}

//======================================================================
void TraceGraphsHolder::setMainSelectionMode(QCP::SelectionType smode)
{
    main->setSelectable(smode);
}

//======================================================================
bool TraceGraphsHolder::isVisible()
{
    return main->visible();
}

//======================================================================
void TraceGraphsHolder::setHistogramDP(int dp_percents)
{
    this->dp_percents_ = dp_percents;
}

//======================================================================
void TraceGraphsHolder::setRMRLine(double y, double x)
{

   //find X val accordinng to X val
    double key = 0;
    double value = 0;

    double maxx = std::numeric_limits<double>::max();
    double maxy = std::numeric_limits<double>::max();

    QCPDataRange dataRange = rmr->data()->dataRange();
    QCPGraphDataContainer::const_iterator begin = rmr->data()->at(dataRange.begin());
    QCPGraphDataContainer::const_iterator end = rmr->data()->at(dataRange.end());

    int n = end-begin;
    if (n>0)
    {
        double *dx = new double[n];
        double *dy = new double[n];

        int index =0;
        for (QCPGraphDataContainer::const_iterator it=begin; it<end; it++)
        {
            dx[index] = qAbs(x - it->key);
            dy[index] = qAbs(y - it->value);
            if ((dx[index] < maxx) && (dy[index] < maxy))
            {
                key = it->key;
                value = it->value;

                maxx = dx[index];
                maxy = dy[index];
            }
            index++;
        }
        delete [] dy;
        delete [] dx;

    }
    for (int i =P_MAIN; i <= P_RMR; i++)
    {
        rmr_line[i]->point1->setCoords(0,key);
        rmr_line[i]->point2->setCoords(0.01,key);
        rmr_line[i]->setVisible(true);
        rmr_line[i]->setPen(kPenRMR);
        rmr_line[i]->setSelectable(false);
    }

    rmr_xval_ = value;
    rmr_yval_ = key;
    rmr_set_ = true;
}

//======================================================================
void TraceGraphsHolder::setRMSLine(bool is_linear)
{
    rms_linear_ = is_linear;

    if (!is_linear)
    {
        //show curve plot
        rms_fit->setData(rms_data_keys, rms_data_values);
        rms_fit->setVisible(true);
        rms_fit->setSelectable(QCP::SelectionType::stNone);

        //hide lines
        for (int i =P_MAIN; i <= P_RMR; i++)
        {
            rms_line[i]->setVisible(false);
            rms_line[i]->setSelectable(false);
        }

    }
    else
    {
        //hide curve plot
        rms_fit->setVisible(false);

        //show infinity horizontal lines
        for (int i =P_MAIN; i <= P_RMR; i++)
        {
            rms_line[i]->point1->setCoords(0, rms_);
            rms_line[i]->point2->setCoords(0.01, rms_);
            rms_line[i]->setVisible(true);
            rms_line[i]->setPen(kPenRMS);
            rms_line[i]->setSelectable(false);
        }
    }
}

//======================================================================
void TraceGraphsHolder::cutSelection()
{
    if (!main->selection().isEmpty() && main->selection().dataRange().isValid())
    {
        int xstart = main->selection().dataRange().begin();
        int xend = main->selection().dataRange().end()-1;
        trace->cut(xstart, xend);

        /*main_data_keys = main_data_keys.mid(xstart, xend-xstart);
        main_data_values = main_data_values.mid(xstart, xend-xstart);
        main->setData(main_data_keys, main_data_values);
        main->setSelection(QCPDataSelection());

        //trace->*/

        markers_set_=false;
        xmarker_tag[0]->setVisible(false);
        xmarker_tag[1]->setVisible(false);
        calculateGraphs(last_fit_line_enabled_, last_fit_data_enabled_);
    }
}

//======================================================================
TData TraceGraphsHolder::currentData()
{
    TData res;
    res.push_back(main_data_keys);
    res.push_back(main_data_values);
    res.push_back(trace->getLockedDataPtr()->at(2));
    return res;
}

//======================================================================
QString TraceGraphsHolder::getTraceLengthString()
{
    if (main_data_keys.size() > 0)
        return QString::number(main_data_keys.last() - main_data_keys.first(), 'f', 4) + unit_;
    else
        return "Неизвестно";
}

//======================================================================
QString TraceGraphsHolder::getTraceStepString()
{
    //calc avg step
    double sum = 0;
    for (int i = 0; i < main_data_keys.length()-1; i++)
    {
        sum += main_data_keys[i+1] - main_data_keys[i];
    }
    sum /= main_data_keys.length()-1;

    return QString::number(sum * 1000, 'f', 4) + "мкм";

}
//======================================================================
QString TraceGraphsHolder::getRMSString()
{
    return QString::number(rms_, 'f', 4) + unit_;
}
//======================================================================
QString TraceGraphsHolder::getRMRString()
{
    return QString::number(rmr_yval_, 'f', 4) + unit_ + " ("+QString::number(rmr_xval_, 'f', 4) + "%)";
}

//======================================================================
QString TraceGraphsHolder::getRMSFirstOrderStrings()
{
    return QString::number(rms_fo_angle_, 'f', 4) + "°";
}

//======================================================================
QString TraceGraphsHolder::getTraceMeasurmentsCountString()
{
 return QString::number(main_data_keys.length());
}

//======================================================================
QString TraceGraphsHolder::getDMXString()
{
 return  QString::number(xmarker_tag[1]->getX() - xmarker_tag[0]->getX(), 'f', 4) + unit_;
}

//======================================================================
QString TraceGraphsHolder::getDMYString()
{
 return QString::number(xmarker_tag[1]->getY() - xmarker_tag[0]->getY(), 'f', 4) + unit_;
}

//======================================================================
QString TraceGraphsHolder::getRaString()
{
 return QString::number(ra_val_, 'f', 4) + unit_;
}



//======================================================================
int TraceGraphsHolder::id()
{
    return id_;
}

//======================================================================
void TraceGraphsHolder::scaleData(const QString unit, bool recalc)
{
    //assume the data already in mm
    if (unit == "mm")
    {
        unit_ = "мм";
        unit_multiplier_ = 1;
    }

    //assume the data already in mm
    if (unit == "nm")
    {
        unit_ = "нм";
        unit_multiplier_ = 1e6;
    }

    if (unit == "mkm")
    {
        unit_ = "мкм";
        unit_multiplier_ = 1e4;
    }
    if (recalc)
        resetData();
}

//======================================================================
void TraceGraphsHolder::resetData()
{
    calculateGraphs(last_fit_line_enabled_, last_fit_data_enabled_);

}



