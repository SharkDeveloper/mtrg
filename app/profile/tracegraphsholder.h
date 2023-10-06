#ifndef TRACEGRAPHSHOLDER_H
#define TRACEGRAPHSHOLDER_H

#include "../qcustomplot/qcustomplot.h"
#include "traces/abstracttrace.h"
#include <QObject>

//framed marker on x-axis rect for selection
class XAxisMarker : public QObject
{
    Q_OBJECT
public:
    explicit XAxisMarker(QCPAxis *parentAxis);
    virtual ~XAxisMarker();
    // setters:
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setText(const QString &text);
    void updatePosition(double xvalue, double yvalue);
    void setVisible(bool val);

    double getX();
    double getY();
signals:
public slots:
protected:
    QCPAxis *axis_;

    QCPItemStraightLine *line_;
    QCPItemText *lable_;
    double xval;
    double yval;
};


class TraceGraphsHolder: public QObject
{

Q_OBJECT

public:
    typedef QString (TraceGraphsHolder::*ValGetter)();

    //a parameter struct
    struct TraceParam {

        enum EParamType { //list of all parametes can be calculated for trace
                E_PARAM_TRACE_LEN,
                E_PARAM_TRACE_MEASURMENTS,
                E_PARAM_TRACE_STEP,
                E_PARAM_RMS,
                E_PARAM_RMR,
                E_PARAM_RMS_1ST_ORDER_ANGLE,
                E_PARAM_DELTA_M_X,
                E_PARAM_DELTA_M_Y,
                E_PARAM_RA,
                //put here all other
                E_PARAM_PARAMS_END
            };

        QString name;
        ValGetter getter;

    public:
        TraceParam(const QString &name,  ValGetter getter) : name(name), getter(getter){};
        TraceParam(){};
    };

    private:
        static QMap <TraceParam::EParamType, TraceParam> params_data_;

    public:

        static QString paramName(TraceParam::EParamType param);
        QString paramValue(TraceParam::EParamType param);

        AbstractTrace *trace;
        QVector <QCustomPlot *> plots_;
        QCPCurve *main;
        QCPCurve *curve;

        QCPGraph *rms_fit;
        QCPItemStraightLine *rms_line[3]; //for all plots
        QCPItemStraightLine *rmr_line[3]; //for all plots

        XAxisMarker *xmarker_tag[2];

        QCPBars *hist;
        QCPGraph *rmr;

        TraceGraphsHolder();
        TraceGraphsHolder(int id, AbstractTrace *trace, QCustomPlot *main, QCustomPlot *hist, QCustomPlot *rmr);
        ~TraceGraphsHolder();
        void select();
        void deselect();

        void calculateGraphs(bool fitting_line, bool fit_data);
        void resetData();
        void calculateRMS();

        void calculateHsitAndRMR();

        void showPoints(bool enabled);
        void setVisible(bool visible);
        void setMainSelectionMode(QCP::SelectionType smode);
        bool isVisible();
        void setHistogramDP(int dp_percents);

        void setRMRLine(double x, double y);
        void setRMSLine(bool is_linear);
        void cutSelection();

        void scaleData(const QString unit, bool recalc = true);

        QString getTraceLengthString();
        QString getTraceStepString();
        QString getRMSString();
        QString getRMRString();
        QString getRMSFirstOrderStrings();
        QString getTraceMeasurmentsCountString();
        QString getDMXString();
        QString getDMYString();
        QString getRaString();

        TData currentData();
        double unit_multiplier() { return unit_multiplier_; }
        int id();
    private:
        QVector <double> main_data_keys;
        QVector <double> main_data_values;

        QVector <double> rms_data_keys;
        QVector <double> rms_data_values;

        //#1d data: histogram and rmr
        QVector <double> rmr_data_values;
        QVector <double> hist_data_values;

        double rms_;
        double rms_fo_angle_;
        double rmr_yval_;
        double rmr_xval_;
        double ra_val_;

        int dp_percents_;
        int rmr_percents_;

        bool rms_linear_;
        bool rmr_set_;

        bool markers_set_;

        int id_; //can be differ with trace_->id cause of traces id are uniq, and holder can be deleted and recreated with the same value
        QString unit_;
        double unit_multiplier_;


        bool last_fit_line_enabled_;
        bool last_fit_data_enabled_;

public slots:
        void graphSelectionChanged(const QCPDataSelection &selection);

        friend TraceParam;
};

Q_DECLARE_METATYPE(TraceGraphsHolder *);

#endif // TRACEGRAPHSHOLDER_H
