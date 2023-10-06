#ifndef CONTOURANALISISWIDGET_H
#define CONTOURANALISISWIDGET_H

#include <QMainWindow>
#include "helpers/analisyswidget.h"
#include "dataproviders/dataprovider.h"
#include <traces/abstracttrace.h>
#include "../qcustomplot/qcustomplot.h"
#include <QLinkedList>

#include <memory>
#include <QMutex>
#include "reperpointcontroller.h"
#include "measurements/measurement.h"
#include "measurements/distancemeasurements.h"
#include "measurements/anglemeasurements.h"
#include "measurements/layflatmeasurement.h"
#include "measurements/errormeasurement.h"
#include "measurements/curvaturemeasurments.h"


namespace Ui {
class ContourAnalisisWidget;
}

//======================================================================
class MeasurementsManager: public QObject {
    Q_OBJECT;
public:

public:
    MeasurementsManager(QCustomPlot *plop);
    ~MeasurementsManager();
    void setActive(E_MEASUREMENTS m, bool log_measurement = true);
    int count();

    QLinkedList<Measurement*>::iterator begin();
    QLinkedList<Measurement*>::iterator end();

    Measurement *findMeasurement(int mid);

    void remove(int index);




signals:
    void measurementChanged(Measurement *);
    void deleteMeasurement(Measurement *);
    void statusText(QString status);

private:
    QLinkedList <Measurement*> measurements_;
    QLinkedList <Measurement*> not_loged_measurements_;

    Measurement *active_measurement;
    ReperPointController *rep_ctrl_;
    E_MEASUREMENTS curr_type_;
    bool log_current_measurement_;

    QCustomPlot *plot_;
};

//======================================================================
class ContourAnalisisWidget : public AnalisysWidget
{
    Q_OBJECT

public:
    explicit ContourAnalisisWidget(QWidget *parent, bool calibration_mode);
    ~ContourAnalisisWidget();
    void setTraces(QVector <AbstractTrace *>t);

    void reportPrepare() override;
    QVector<QPixmap> getReportPixmaps() override;
    inja::json getReportData() override;
    QString getReportTemplatePath() override;
    QString getReportReferencePath() override;
    void reportFinished() override;



private:
    Ui::ContourAnalisisWidget *ui;
    QVector <AbstractTrace*>  traces;
    MeasurementsManager *measurements_;
    void setAxisSuffixesAndScaleData();

    double unit_multiplier_;
    bool calibration_mode_;
    bool def_angles_in_deg_val_;
    bool do_not_update_on_calib_set_;
    bool do_not_replot_on_calib_set_;
    QCPColorMap *colorMap;
    QCPItemTracer *colormapTracer;
    QVector <QCPCurve*> curves_;


private slots:
    void processMeasurementSelection();


private:
    void createPlot();
    void updateInfo(Measurement *m);
    void foreachGraph(std::function< void(QCPCurve*) > f);

    QMap <int, QTreeWidgetItem *> measure_to_item;

public slots:
    void saveData() override;
    void deleteMeasurement(int index);
    void recalcAllMeasurements();

private slots:
    void calibrationDataValueChangedManually(double val);
};

#endif // CONTOURANALISISWIDGET_H
