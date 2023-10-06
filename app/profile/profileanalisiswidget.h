#ifndef PROFILEANALISISWIDGET_H
#define PROFILEANALISISWIDGET_H

#include <QWidget>
#include "helpers/analisyswidget.h"
#include "dataproviders/dataprovider.h"
#include "../qcustomplot/qcustomplot.h"
#include "device/devicecontrolwidget.h"

#include "tracegraphsholder.h"

#include "QMainWindow"
namespace Ui {

class ProfileAnalisisWidget;
}

class ProfileAnalisisWidget : public AnalisysWidget
{
    Q_OBJECT

public:
    explicit ProfileAnalisisWidget(QWidget *parent = nullptr);
    KeyReceiver *deviceEventFilter();

    QVector<QPixmap> getReportPixmaps() override;

    void reportPrepare() override;
    inja::json getReportData() override;
    QString getReportTemplatePath() override;
    QString getReportReferencePath() override;
    void reportFinished() override;
    ~ProfileAnalisisWidget();
    QList <AbstractTrace *> traces();

private:
    Ui::ProfileAnalisisWidget *ui;
    QMap<int, AbstractTrace*> traces_; //list of all traces
    QCustomPlot *mainplot_;
    QCustomPlot *histplot_;
    QCustomPlot *rmrplot_;
    QCheckBox *median_filter_cb_;
    QSpinBox *median_filter_sb_;
    QMap <int, TraceGraphsHolder *> graphs_; //list of all helpers for each trace

    AbstractTrace * getNewTrace() override; //override analisyswidget method for new trace generation

    bool data_read_in_progress;
    KeyReceiver *device_evfilter_;


public slots:
    void dataBatchReady(int from, int count) override;
    void dataReadStarted() override;
    void dataReadFinished() override;
    void dataReadError(QString error) override;
    void saveData() override;
    void recalcParamPlots(QCPRange range); //recalculate hist&rmr plots
    void settingsChanged();

private:

    void configurePlots();
    void setAxisSuffixesAndScaleData();


signals:
    void readyForMeasurment(QVector <AbstractTrace*> traces);
};



#endif // PROFILEANALISISWIDGET_H
