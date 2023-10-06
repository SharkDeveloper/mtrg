#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"

CalibrationDialog::CalibrationDialog(QWidget *parent, AbstractTrace *trace) :
    QDialog(parent),
    ui(new Ui::calibrationdialog)
{
    ui->setupUi(this);
}

//======================================================================
void CalibrationDialog::createPlot()
{
    QCustomPlot *p = this->ui->plot;

    /*connect(ui->actionPAN, &QAction::toggled, this, [=](bool enabled) {
        if (enabled)
        {
            disableAllActionsExceptSender(ui, QObject::sender());

            p->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            p->setSelectionRectMode(QCP::srmNone);
            p->setCursor(Qt::OpenHandCursor);
        }
        else
        {
            p->unsetCursor();
        }
    });

    connect(ui->actionResetScale, &QAction::triggered, this,  [=]() {
        foreachGraph([=](QCPGraph *g) {
            qDebug() << "=============+!";;
            g->keyAxis()->rescale();
            g->valueAxis()->rescale();
            g->valueAxis()->setScaleRatio(g->keyAxis(), 1 );

        });
        p->replot();

    });

    connect(ui->actionScaleX, &QAction::toggled, this,  [=](bool enable) {
        if (enable)
        {
            disableAllActionsExceptSender(ui, QObject::sender());
        }

        static auto conn = std::make_shared<QMetaObject::Connection>();

        if (enable)
        {
            ui->plot->setInteractions(QCP::iRangeDrag);
            p->setCursor(Qt::SizeVerCursor);
            *conn = connect(p, &QCustomPlot::mouseWheel, this, [=](QWheelEvent *event) {
                    foreachGraph([=](QCPGraph *g) {

                        const double wheelSteps = event->delta()/120.0; // a single step delta is +/-120 usually
                        const double factor = qPow(g->keyAxis()->axisRect()->rangeZoomFactor(g->keyAxis()->orientation()), wheelSteps);

                        g->keyAxis()->scaleRange(factor);

                    });
                    p->replot();
            });
        }
        else
        {
            p->unsetCursor();
            disconnect(*conn);
        }
    });

    connect(ui->actionScaleY, &QAction::toggled, this,  [=](bool enable) {
        if (enable)
        {
            disableAllActionsExceptSender(ui, QObject::sender());
        }

        static auto conn = std::make_shared<QMetaObject::Connection>();
        if (enable)
        {
            p->setCursor(Qt::SizeVerCursor);
            ui->plot->setInteractions(QCP::iRangeDrag);
            *conn = connect(p, &QCustomPlot::mouseWheel, this, [=](QWheelEvent *event) {
                    foreachGraph([=](QCPGraph *g) {
                        const double wheelSteps = event->delta()/120.0; // a single step delta is +/-120 usually
                        const double factor = qPow(g->valueAxis()->axisRect()->rangeZoomFactor(g->valueAxis()->orientation()), wheelSteps);

                        g->valueAxis()->scaleRange(factor);

                    });
                    p->replot();
            });
        }
        else
        {
            p->unsetCursor();
            disconnect(*conn);
        }
    });
*/

  /*
    connect(ui->actionRST, &QAction::triggered, this, [=](){

        //grapsh and traces order is sync
        int i =0;
        for (auto t : traces)
        {
            auto g = ui->plot->graph(i);
            auto d = t->getLockedDataPtr();
            auto keys = d->at(0);
            auto values = d->at(1);
            for (int i =0; i < keys.size(); i++)
            {
                keys[i] *= unit_multiplier_;
                values[i] *= unit_multiplier_;
            }
            g->setData(keys, values);

            i++;
        }

        for (auto m : *measurements_)
        {
            m->calculateMeasurement();
        }

        QCustomPlot *p = this->ui->plot;
        ui->actionResetScale->trigger();
        ui->actionLayFlat->setChecked(false);
    } );

    //undo flatting if button was unchecked
    connect(ui->actionLayFlat, &QAction::toggled, this, [=](bool val){
        if (!val)
            ui->actionRST->trigger();
    } );
    */
}


CalibrationDialog::~CalibrationDialog()
{
    delete ui;
}
