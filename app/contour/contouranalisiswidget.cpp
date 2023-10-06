#include "contouranalisiswidget.h"
#include "ui_contouranalisiswidget.h"
#include "qcustomplot/qcustomplot.h"
#include <QDebug>
#include <QMessageBox>
#include <memory>
#include <QMutexLocker>
#include <QCheckBox>
#include <QLinkedList>
#include "mainwindow.h"
#include <QTreeWidgetItem>
#include "reperpointcontroller.h"
#include "device/settings.h"

#include <math.h>
#include <nlopt.hpp>


const QString kContourWidgetName = "КОНТУР";

const double kScaleXFactor = 0.01;
const double kScaleYFactor = 0.01;


//======================================================================
MeasurementsManager::MeasurementsManager(QCustomPlot *plot):curr_type_(M_NONE)
{
    plot_ = plot;

    active_measurement = nullptr;
    rep_ctrl_ = new ReperPointController(plot_);

    connect(plot_, &QCustomPlot::mouseMove,rep_ctrl_, &ReperPointController::mouseMove);

    connect(plot_, &QCustomPlot::mousePress, this, [=](QMouseEvent *event)
    {
        qDebug() << "Total Measurement objects: "<<Measurement::objects_cnt();
        if (event->button() == Qt::LeftButton)
        {
             qDebug() << "Acitve messurement" << active_measurement;
            if (active_measurement && !active_measurement->isFinished())
            {
                qDebug() << "leftpress: "<<active_measurement->name() << active_measurement->isFinished();
                qDebug() << "Still active";
                return;
            }

            if (active_measurement && active_measurement->isFinished())
            {
                qDebug() << "actice measure finished";
                //check for object need deletion
                if (!measurements_.contains(active_measurement))
                    delete active_measurement;

                active_measurement = nullptr;

            }

            if (active_measurement && curr_type_ == M_NONE)
            {
                delete measurements_.takeLast();
                active_measurement = nullptr;
                qDebug() << "Still M_";
            }

            switch (curr_type_)
            {
                case M_VERTIAL_DISTANCE:
                    active_measurement = new VerticalDistanceMeasurment( plot_);
                    break;

                case M_HORISONTAL_DISTANCE:
                    active_measurement = new HorisontalDistanceMeasurment( plot_);
                    break;

                case M_DIRECT_DISTANCE:
                    active_measurement = new DirectDistanceMeasurement( plot_);
                    break;

                case M_RMSE:
                    active_measurement = new RMSEMeasurement( plot_);
                    break;

                case M_RDISPLERROR:
                    active_measurement = new RadiusDisplacementErrorMeasurement( plot_);
                    break;

                case M_LAY_FLAT:
                    active_measurement = new LayFlatMeasurement( plot_);
                    break;
                case M_MEAN_HEIGHT:
                    active_measurement = new MeanHeightMeasurement( plot_);
                    break;

                case M_ANGLE_REGULAR:
                    active_measurement = new RegularAngleMeasurement( plot_);
                    break;

                case M_ANGLE_HHORIZON:
                    active_measurement = new HorizontalAngleMeasurement( plot_);
                    break;
                case M_ANGLE_VHORIZON:
                    active_measurement = new VerticalAngleMeasurement( plot_);
                    break;
                case M_RADIUS_NORMAL:
                    active_measurement = new RegularRadiusMeasurement( plot_);
                    break;
                case M_RADIUS_ARCS:
                    active_measurement = new ArcsRadiusMeasurement( plot_);
                    break;
                case M_AVG_DIAMETER:
                    active_measurement = new AvgDiametertMeasurement( plot_);
                    break;

                break;
            }

            if(active_measurement && log_current_measurement_)
            {
                qDebug() << "new measurement " << active_measurement->name();

                measurements_.push_back(active_measurement);

                connect(active_measurement, &Measurement::measumentChanged, this, [=](){
                    emit measurementChanged(active_measurement);
                });

                emit measurementChanged(measurements_.last());
            }


            connect(active_measurement, &Measurement::graphDataChanged, this, [=](){
                plot->rescaleAxes();

                for (auto m : measurements_)
                {
                    m->calculateMeasurement();
                }

                plot->replot();
            });

            connect(active_measurement, &Measurement::reperPointChanged, rep_ctrl_, &ReperPointController::reperPointChanged);
            connect(active_measurement, &Measurement::reperPointRemoved, rep_ctrl_, &ReperPointController::reperPointRemoved);
            connect(active_measurement, &Measurement::statusText, this, [=](QString text) {
                emit statusText(text);
            });
        }
    });


}

//======================================================================
MeasurementsManager::~MeasurementsManager()
{

    for (auto m : measurements_)
    {
        if (m == active_measurement)
        {
            active_measurement = nullptr;
        }
        delete m;
    }

    for (auto m : not_loged_measurements_)
    {
        if (m == active_measurement)
        {
            active_measurement = nullptr;
        }
        delete m;
    }

    if (active_measurement){
        delete active_measurement;
    }

    delete rep_ctrl_;
}
//======================================================================
void MeasurementsManager::setActive(E_MEASUREMENTS m, bool log_measurement)
{
    if (curr_type_ != m && active_measurement && !active_measurement->isFinished())
    {
        qDebug() << "ask to delete active measurement";
        emit deleteMeasurement(active_measurement);        
        active_measurement = nullptr;
    }

    if (active_measurement)
    {
        qDebug() << "activate current measurement plot decorators";
        active_measurement->activatePlotDecorators();
    }

    curr_type_ = m;
    log_current_measurement_ = log_measurement;
    if (m == M_NONE)
    {
        //mainplot_->setInteractions(QCP::iSelectPlottables | QCP::iKeepSelections | QCP::iXRangeSelection );
        //mainplot_->setSelectionRectMode(QCP::srmSelectRealtime);




        plot_->setInteractions(plot_->interactions() | QCP::iRangeSelection);
        plot_->setXSelectionArrowVisibility(false);
        qDebug() << "removed iXRangeSelection modificator";
    }
    else
    {
        plot_->setInteractions(QCP::iRangeSelection);
        plot_->setXSelectionArrowVisibility(true);
        qDebug() << "add iXRangeSelection modificator";
    }

}

//======================================================================
QLinkedList<Measurement*>::iterator MeasurementsManager::begin()
{
    return measurements_.begin();
}

//======================================================================
QLinkedList<Measurement*>::iterator MeasurementsManager::end()
{
    return measurements_.end();
}

//======================================================================
Measurement *MeasurementsManager::findMeasurement(int mid)
{
    for (auto m: *this)
    {
        if (m->id() == mid) return m;
    }
    return nullptr;
}

//======================================================================
int MeasurementsManager::count()
{
    return measurements_.size();
}

//======================================================================
void MeasurementsManager::remove(int index)
{
   for (auto m: *this)
   {
       if (m->id() == index)
       {
           qDebug() << "mesman remove";
           measurements_.removeOne(m);
           if (active_measurement == m)
               active_measurement = nullptr;
           delete m;
           break;
       }
   }
}

#include<type_traits>
#include<utility>

template<typename Callable>
union storage
{
    storage() {}
    std::decay_t<Callable> callable;
};

template<int, typename Callable, typename Ret, typename... Args>
auto fnptr_(Callable&& c, Ret (*)(Args...))
{
    static bool used = false;
    static storage<Callable> s;
    using type = decltype(s.callable);

    if(used)
        s.callable.~type();
    new (&s.callable) type(std::forward<Callable>(c));
    used = true;

    return [](Args... args) -> Ret {
        return Ret(s.callable(std::forward<Args>(args)...));
    };
}

template<typename Fn, int N = 0, typename Callable>
Fn* fnptr(Callable&& c)
{
    return fnptr_<N>(std::forward<Callable>(c), (Fn*)nullptr);
}

//======================================================================
ContourAnalisisWidget::ContourAnalisisWidget(QWidget *parent, bool calibration_mode) :
    AnalisysWidget(kContourWidgetName),
    ui(new Ui::ContourAnalisisWidget),
    unit_multiplier_(1),
    calibration_mode_(calibration_mode),
    do_not_update_on_calib_set_(false),
    do_not_replot_on_calib_set_(false)

{
    ui->setupUi(this);

    //show angles in decimal degrees in calibration mode
    if (calibration_mode)
    {
        def_angles_in_deg_val_ = Settings::instance().view.angles_in_deg;
        Settings::instance().view.angles_in_deg = true;
    }

    createPlot();

    measurements_ = new MeasurementsManager(ui->plot);

    connect(measurements_, &MeasurementsManager::measurementChanged, this, [=](Measurement *m) {
        updateInfo(m);
    });

    connect(measurements_, &MeasurementsManager::deleteMeasurement, this, [=](Measurement *m) {
        deleteMeasurement(m->id());
    });

    connect(measurements_, &MeasurementsManager::statusText, this, [=](QString text) {
        ui->statusbar->showMessage(text);
    });

    ui->distmTree->setDefaultGroupName("ДЛИНЫ");
    ui->radTree->setDefaultGroupName("РАДИУСЫ");
    ui->angleTree->setDefaultGroupName("УГЛЫ");

    ui->heatmap->setVisible(false);


    connect(ui->distmTree, &MeasurementTreeWidget::deleteMeasurement, this, &ContourAnalisisWidget::deleteMeasurement);
    connect(ui->radTree, &MeasurementTreeWidget::deleteMeasurement, this, &ContourAnalisisWidget::deleteMeasurement);
    connect(ui->angleTree, &MeasurementTreeWidget::deleteMeasurement, this, &ContourAnalisisWidget::deleteMeasurement);

    connect(ui->distmTree, &MeasurementTreeWidget::itemSelectionChanged, this, &ContourAnalisisWidget::processMeasurementSelection);
    connect(ui->radTree, &MeasurementTreeWidget::itemSelectionChanged, this, &ContourAnalisisWidget::processMeasurementSelection);
    connect(ui->angleTree, &MeasurementTreeWidget::itemSelectionChanged, this, &ContourAnalisisWidget::processMeasurementSelection);

    setAxisSuffixesAndScaleData();

    if (calibration_mode_)
    {
        ui->partNameEdit->setVisible(false);
        ui->partNameLabel->setVisible(false);
        ui->radTree->setVisible(false);
        ui->angleTree->setVisible(false);

        ui->distmTree->setDefaultGroupName("ИЗМЕРЕНИЯ");
        ui->distmTree->setCalubrationView(true);

        ui->tipLengthSpinBox_3->setValue(Settings::instance().calibration.tool_length_nm);
        ui->headLengthSpinBox_3->setValue(Settings::instance().calibration.tool_head_lenght_nm);
        ui->headRadiusSpinBox_3->setValue(Settings::instance().calibration.tool_head_radius_nm);
        ui->headHorizonAngleSpinBox_3->setValue(Settings::instance().calibration.tool_zero_angle_deg);

        ui->headLengthSpinBox_3->setVisible(false);
        ui->headRadiusSpinBox_3->setVisible(false);
        ui->headLengthLable->setVisible(false);
        ui->headRadiusLable->setVisible(false);

        //disable actions not accessible in calibration mode
        ui->actionLayFlat->setVisible(false);
        ui->actionHHorizonAngle->setVisible(false);
        ui->actionVHorizonAngle->setVisible(false);
        ui->actionVerticalDistance->setVisible(false);
        ui->actionHorisontalDistance->setVisible(false);
        ui->actionMeanHeight->setVisible(false);
        ui->doOptimizationButton->setVisible(true);
        ui->heatmap->setVisible(true);

        ui->heatmap->axisRect()->setupFullAxesBox(true);
        ui->heatmap->xAxis->setLabel("Длина консоли");
        ui->heatmap->yAxis->setLabel("Угол горизонта");
        // set up the QCPColorMap:
        colorMap = new QCPColorMap(ui->heatmap->xAxis, ui->heatmap->yAxis);

        int nx = 20;
        int ny = 20;
        colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
        colorMap->data()->setRange(QCPRange(ui->tipLengthSpinBox_3->minimum(), ui->tipLengthSpinBox_3->maximum()),
                                   QCPRange(ui->headHorizonAngleSpinBox_3->minimum(), ui->headHorizonAngleSpinBox_3->maximum())); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions

        for (int x = 0; x < nx; x++) for(int y =0; y<ny; y++) colorMap->data()->setCell(x, y, 10);

        // add a color scale:
        QCPColorScale *colorScale = new QCPColorScale(ui->heatmap);

        ui->heatmap->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
        colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
        colorMap->setColorScale(colorScale); // associate the color map with the color scale
        colorMap->setGradient(QCPColorGradient::gpPolar);
        QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->heatmap);
        ui->heatmap->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
        colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
        colorScale->setDataScaleType( QCPAxis::ScaleType::stLogarithmic);
        ui->heatmap->xAxis->ticker()->setTickCount(3);
        ui->heatmap->rescaleAxes();

        colormapTracer = new QCPItemTracer(ui->heatmap);
        //colormapTracer->setGraph(colorMap);
        colormapTracer->setStyle(QCPItemTracer::tsCrosshair);
        colormapTracer->setPen(QPen(Qt::white));
        colormapTracer->setBrush(Qt::white);
        colormapTracer->setSize(7);
        colormapTracer->position->setCoords( Settings::instance().calibration.tool_length_nm,  Settings::instance().calibration.tool_zero_angle_deg );


        //make a fitness-function to minimize
        auto minfunc = [=] (const std::vector<double> &x, std::vector<double> &grad, void *my_func_data) {

            double tip_lenght = x[0];
            double zero_angle = x[1];
            colormapTracer->position->setCoords(tip_lenght, zero_angle);

            do_not_update_on_calib_set_ = true;
            ui->tipLengthSpinBox_3->setValue(tip_lenght + 1);
            do_not_update_on_calib_set_ = false;

            //on set will call a valueChangedSignal, that will calculate all measurements and populate a treeview
            ui->headHorizonAngleSpinBox_3->setValue(zero_angle + 1);

           //rms of errors
           QVector<double> errors = ui->distmTree->getErrors();

           qDebug() << errors;

           double rms = 0.0;
            double summ = 0;
           for (auto e : errors) {
               rms += e*e;
               summ += e;
           }
           rms /= errors.size();
           rms = sqrt(rms);

           int cellx,celly;

           colorMap->data()->coordToCell(tip_lenght, zero_angle, &cellx, &celly);

           colorMap->data()->setCell(cellx, celly, summ);

           colorMap->rescaleDataRange();

           ui->heatmap->replot();

           return summ;
        };

        connect(ui->heatmap, &QCustomPlot::mousePress, this, [=](QMouseEvent *event)
        {
            if (event->button() == Qt::LeftButton)
            {
                qDebug() << "Colormap press";
                double x,y;
                colorMap->pixelsToCoords(QPointF(event->pos()),x,y);
                std::vector<double> xx = {x,y};
                std::vector<double> g;

                minfunc(xx, g, NULL);
            }
        });


        connect(ui->buildHeatMapBtn, &QPushButton::pressed, this, [=]{

            if (measurements_->count() == 0)
            {
                QMessageBox::warning(this, "Ошибка", "Нет измерений для оптимизации!");
                return;
            }

            qDebug() << "Build heatmap";
            // now we assign some data, by accessing the QCPColorMapData instance of the color map:
            do_not_replot_on_calib_set_ = true;

            QProgressDialog progress("Вычисляю....", "Отмена", 0, nx*ny, this);
            progress.setWindowModality(Qt::WindowModal);
            progress.setWindowTitle("Расчет поверхности");

            double current_lenght = Settings::instance().calibration.tool_length_nm;
            double current_angle = Settings::instance().calibration.tool_zero_angle_deg;

            double x, y;
            int ct = 0;
            for (int xIndex=0; xIndex<nx; ++xIndex)
            {
              for (int yIndex=0; yIndex<ny; ++yIndex)
              {
                colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
                std::vector<double> xx = {x,y};
                std::vector<double> g;

                double r = minfunc(xx, g, NULL);

                progress.setValue(ct);
                ct++;
                if (progress.wasCanceled())
                {
                    std::vector<double> xx = {current_lenght,current_angle};
                    std::vector<double> g;

                    double r = minfunc(xx, g, NULL);

                    do_not_replot_on_calib_set_ = false;

                    ui->plot->replot();
                    return ;
                }

              }
            }
            std::vector<double> xx = {current_lenght,current_angle};
            std::vector<double> g;

            double r = minfunc(xx, g, NULL);

            do_not_replot_on_calib_set_ = false;
            ui->plot->replot();

        });

        connect(ui->doOptimizationButton, &QPushButton::pressed, this, [=]{

            if (measurements_->count() == 0)
            {
                QMessageBox::warning(this, "Ошибка", "Нет измерений для оптимизации!");
                return;
            }

            //create optimizer (without derevative: LN)
            nlopt::opt opt(nlopt::LN_COBYLA, 2);

            //wrap lambda to function ptr to use in nlopt
            auto fn = fnptr<double (const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)>(minfunc);

            //set bounds for variables
            std::vector<double> lb(2);
            lb[0] = ui->tipLengthSpinBox_3->minimum();
            lb[1] = ui->headHorizonAngleSpinBox_3->minimum();


            std::vector<double> ub(2);
            ub[0] = ui->tipLengthSpinBox_3->maximum();
            ub[1] = ui->headHorizonAngleSpinBox_3->maximum();
            opt.set_lower_bounds(lb);
            opt.set_upper_bounds(ub);

            //set minfunc
            opt.set_min_objective(fn, NULL);

            //set minimums
            opt.set_xtol_rel(1e-8);
            std::vector<double> x(2);

            //set predictions
            x[0] = Settings::instance().calibration.tool_length_nm;
            x[1] = Settings::instance().calibration.tool_zero_angle_deg;

            double minf;
            try{
                nlopt::result result = opt.optimize(x, minf);
                std::cout << "found minimum at f(" << x[0] << "," << x[1] << ") = " << result << std::endl;
            }
            catch(std::exception &e) {
                std::cout << "nlopt failed: " << e.what() << std::endl;
            }
         });

        connect(ui->tipLengthSpinBox_3, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ContourAnalisisWidget::calibrationDataValueChangedManually);
        connect(ui->headLengthSpinBox_3, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ContourAnalisisWidget::calibrationDataValueChangedManually);
        connect(ui->headRadiusSpinBox_3, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ContourAnalisisWidget::calibrationDataValueChangedManually);
        connect(ui->headHorizonAngleSpinBox_3, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ContourAnalisisWidget::calibrationDataValueChangedManually);
    }
    else
    {
        ui->groupBox->setVisible(false);
    }
}

//======================================================================
void ContourAnalisisWidget::setTraces(QVector <AbstractTrace *>t)
{
    this->traces = t;

    for (auto t : traces)
    {
        auto g = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);
        curves_.push_back(g);
        auto d = t->getLockedDataPtr();
        auto keys = d->at(0);
        auto values = d->at(1);
        for (int i =0; i < keys.size(); i++)
        {
            keys[i] *= unit_multiplier_;
            values[i] *= unit_multiplier_;
        }

        g->setData(keys, values);
        g->setPen(kPenDefaultBlue);
        QCPSelectionDecorator *dec = new QCPSelectionDecorator();
        dec->setPen(kPenSelection);
        g->setSelectionDecorator(dec);
    }

    QCustomPlot *p = this->ui->plot;
    ui->actionResetScale->trigger();
}

//======================================================================
void ContourAnalisisWidget::foreachGraph(std::function<void (QCPCurve *)> f)
{
    for (auto c: curves_)
    {
        f(c);
    }
}

//======================================================================
void disableAllActionsExceptSender(Ui_ContourAnalisisWidget* ui, QObject *sender)
{
    //cant be static cause many ui-widgets can be created
    QList <QObject*> actions =
    {
        ui->actionVerticalDistance,
        ui->actionHorisontalDistance,
        ui->actionDirectDistance,
        ui->actionRMSE,
        ui->actionRadiusError,
        ui->actionScaleX,
        ui->actionScaleY,
        ui->actionPAN,
        ui->actionRadius,
        ui->actionDirectAngle,
        ui->actionMultiRadius,
        ui->actionHHorizonAngle,
        ui->actionVHorizonAngle,
        ui->actionMeanHeight,
        ui->actionAvgDiameter
    };

    for (auto it : actions)
    {
        if (it == sender)
            continue;

        ((QAction *)it)->setChecked(false);
    }
}

//======================================================================
void ContourAnalisisWidget::createPlot()
{
    QCustomPlot *p = this->ui->plot;

    connect(ui->actionPAN, &QAction::toggled, this, [=](bool enabled) {
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
        foreachGraph([=](QCPCurve *g) {
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
                    foreachGraph([=](QCPCurve *g) {

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
                    foreachGraph([=](QCPCurve *g) {
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


    QList <QPair<QObject*, E_MEASUREMENTS>> actions =
    {
        {ui->actionVerticalDistance,    M_VERTIAL_DISTANCE},
        {ui->actionHorisontalDistance,  M_HORISONTAL_DISTANCE},
        {ui->actionDirectDistance,      M_DIRECT_DISTANCE},
        {ui->actionRMSE,                M_RMSE},
        {ui->actionRadiusError,         M_RDISPLERROR},
        {ui->actionRadius,              M_RADIUS_NORMAL},
        {ui->actionLayFlat,             M_LAY_FLAT},
        {ui->actionDirectAngle,         M_ANGLE_REGULAR},
        {ui->actionMultiRadius,         M_RADIUS_ARCS},
        {ui->actionHHorizonAngle,       M_ANGLE_HHORIZON},
        {ui->actionVHorizonAngle,       M_ANGLE_VHORIZON},
        {ui->actionMeanHeight,          M_MEAN_HEIGHT},
        {ui->actionAvgDiameter,          M_AVG_DIAMETER},
    };

    for (auto it : actions)
    {
        connect((QAction*)it.first, &QAction::toggled, this, [=](bool enable)
        {
            if (enable)
            {
                disableAllActionsExceptSender(ui, QObject::sender());

                measurements_->setActive(it.second, it.second != M_LAY_FLAT);
            }
            else
            {
                measurements_->setActive(M_NONE);
            }
        });
    }

    connect(ui->actionRST, &QAction::triggered, this, [=](){

        //grapsh and traces order is sync
        int i =0;
        for (auto t : traces)
        {
            auto g = curves_[i];
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
}



//======================================================================
void ContourAnalisisWidget::updateInfo(Measurement *m)
{
    if (measure_to_item.contains(m->id()))
    {
        ((MeasurementTreeWidget *) measure_to_item[m->id()]->treeWidget())->updateItem(measure_to_item[m->id()], m);
    }
    else
    {
        QTreeWidgetItem *item = nullptr;
        if (calibration_mode_)
        {
             item = ui->distmTree->addMeasurement(m);
        }
        else
        {
            switch (m->type())
            {
                case M_VERTIAL_DISTANCE:
                case M_HORISONTAL_DISTANCE:
                case M_DIRECT_DISTANCE:
                case M_MEAN_HEIGHT:
                case M_RMSE:
                case M_RDISPLERROR:
                case M_AVG_DIAMETER:
                    item = ui->distmTree->addMeasurement(m);
                    break;
                case M_ANGLE_REGULAR:
                case M_ANGLE_VHORIZON:
                case M_ANGLE_HHORIZON:
                    item = ui->angleTree->addMeasurement(m);
                    break;

                case M_RADIUS_ARCS:
                case M_RADIUS_NORMAL:
                    item = ui->radTree->addMeasurement(m);
                    break;

                default:
                    {
                        qDebug() << "error: do not know which measurement list to put item, use distances list";
                        item = ui->distmTree->addMeasurement(m);
                        break;
                    }
               }
        }

        measure_to_item[m->id()] = item;
    }

    ui->distmTree->recalcMeans();
    ui->angleTree->recalcMeans();
    ui->radTree->recalcMeans();
}

//======================================================================
void ContourAnalisisWidget::saveData()
{
    qDebug() << "Save countor analisis data";
}

//======================================================================
ContourAnalisisWidget::~ContourAnalisisWidget()
{
    delete ui;

    //do not delete trace, in calibration mode it is the original data
    if (!calibration_mode_)
        for (auto t: traces) delete t;

    delete measurements_;

    if (calibration_mode_)
    {
        Settings::instance().view.angles_in_deg = def_angles_in_deg_val_;
        Settings::instance().save();
    }
}

//======================================================================
void ContourAnalisisWidget::deleteMeasurement(int index)
{
    qDebug() << "Delete mesurement " << index << measure_to_item.contains(index);
    measurements_->remove(index);
    delete measure_to_item[index];
    measure_to_item.remove(index);
    ui->plot->replot();
}

//======================================================================
void ContourAnalisisWidget::processMeasurementSelection()
{
    qDebug() << "Selection processing";
    auto items = dynamic_cast <MeasurementTreeWidget *> (QObject::sender())->selectedItems();
    if (items.empty()) {qDebug() << "Selection empty"; return;}

    for (auto m: *measurements_)
    {
        if(m->id() == items.first()->data(0, Qt::UserRole).toInt())
            m->select(true);
        else
            m->select(false);
    }
    ui->plot->replot();
}

//======================================================================
void ContourAnalisisWidget::recalcAllMeasurements()
{
    qDebug() << "Recalc all measurements";
    setAxisSuffixesAndScaleData();
    for (auto m: *measurements_)
    {
        m->reloadSettings();
        m->calculateMeasurement();
        updateInfo(m);
    }
    ui->plot->replot();
}

void setMeasurementsGroupsAndIDs(MeasurementTreeWidget *w, MeasurementsManager *mgr, bool set)
{

    for (auto group: w->groups())
    {
        for (int measurement_index = 0; measurement_index < group.measurements.size(); measurement_index++)
        {
            MeasurementTreeWidget::SMeasurement tm = group.measurements[measurement_index];
            Measurement *measurement = mgr->findMeasurement(tm.id);
            if (measurement)
            {
                if (set)
                {
                    measurement->setNameText(QString::fromStdString(group.name) + " " + QString::number(measurement_index));
                    measurement->calculateMeasurement();
                    tm.item->setText(0, QString::number(measurement_index) + " " + tm.item->text(0));
                }
                else
                {
                    measurement->clearNameText();
                    measurement->calculateMeasurement();
                    tm.item->setText(0, measurement->name());
                }

            }
        }
    }

}
//======================================================================
void ContourAnalisisWidget::reportPrepare()
{
    setMeasurementsGroupsAndIDs(ui->distmTree, measurements_, true);
    setMeasurementsGroupsAndIDs(ui->angleTree, measurements_,  true);
    setMeasurementsGroupsAndIDs(ui->radTree, measurements_,  true);
    ui->plot->setXSelectionArrowVisibility(false);
    ui->plot->replot();
}

//======================================================================
QVector<QPixmap> ContourAnalisisWidget::getReportPixmaps() {

    QVector<QPixmap> r;
    r << ui->plot->toPixmap();
    return r;
}

//======================================================================
inja::json ContourAnalisisWidget::getReportData() {
    inja::json json;

    json["date"] = QDateTime::currentDateTime().toLocalTime().toString().toStdString();
    json["part_name"] = ui->partNameEdit->text().toStdString();
    json["controller_name"] = Settings::instance().controllers.at(Settings::instance().current_controller_index).toStdString();

    for (auto g: ui->distmTree->groups())
    {
        inja::json measurements= inja::json::array();
        for (auto m: g.measurements)
            measurements.push_back({{"name", m.name},{"value", m.value}});

        json["distances"].push_back({{"name", g.name},{"mean", g.mean}, {"measurements", measurements}});
    }
    for (auto g: ui->angleTree->groups())
    {
        inja::json measurements= inja::json::array();
        for (auto m: g.measurements)
            measurements.push_back({{"name", m.name},{"value", m.value}});

        json["angles"].push_back({{"name", g.name},{"mean", g.mean}, {"measurements", measurements}});
    }

    for (auto g: ui->radTree->groups())
    {
        inja::json measurements = inja::json::array();
        for (auto m: g.measurements)
            measurements.push_back({{"name", m.name},{"value", m.value}});

        json["rads"].push_back({{"name", g.name},{"mean", g.mean}, {"measurements", measurements}});
    }

    qDebug() << QString::fromStdString(json.dump());
    return json;
};

//======================================================================
void ContourAnalisisWidget::setAxisSuffixesAndScaleData()
{
    QString xunit,yunit;
    if (Settings::instance().view.axis_unit == ViewParameters::MM)
    {
        ui->plot->xAxis->ticker()->setSuffix(" мм");
        ui->plot->yAxis->ticker()->setSuffix(" мм");
        unit_multiplier_ = 1;
    }

    if (Settings::instance().view.axis_unit == ViewParameters::NM)
    {
        ui->plot->xAxis->ticker()->setSuffix(" нм");
        ui->plot->yAxis->ticker()->setSuffix(" нм");
        unit_multiplier_ = 1.0e6;
    }

    if (Settings::instance().view.axis_unit == ViewParameters::MK)
    {
        ui->plot->xAxis->ticker()->setSuffix(" мкм");
        ui->plot->yAxis->ticker()->setSuffix(" мкм");
        unit_multiplier_ = 1.0e4;
    }

    ui->actionRST->trigger();
}

//======================================================================
void ContourAnalisisWidget::reportFinished()
{
    setMeasurementsGroupsAndIDs(ui->distmTree, measurements_, false);
    setMeasurementsGroupsAndIDs(ui->angleTree, measurements_,  false);
    setMeasurementsGroupsAndIDs(ui->radTree, measurements_,  false);
    ui->plot->setXSelectionArrowVisibility(true);
    ui->plot->replot();
}

QString ContourAnalisisWidget::getReportTemplatePath()
{
    return "reports/contour.md";
}

//======================================================================
QString ContourAnalisisWidget::getReportReferencePath()
{
    return "reports/contour_ref.odt";
}


//CALIBRATION LOGIC
//======================================================================
void ContourAnalisisWidget::calibrationDataValueChangedManually(double val)
{

    if (QObject::sender() == ui->tipLengthSpinBox_3)
    {
        Settings::instance().calibration.tool_length_nm = val;
    }

    if (QObject::sender() == ui->headLengthSpinBox_3)
    {
        Settings::instance().calibration.tool_head_lenght_nm = val;
    }

    if (QObject::sender() == ui->headRadiusSpinBox_3)
    {
        Settings::instance().calibration.tool_head_radius_nm = val;
    }

    if (QObject::sender() == ui->headHorizonAngleSpinBox_3)
    {
        Settings::instance().calibration.tool_zero_angle_deg = val;
    }
    if (do_not_update_on_calib_set_)
        return;


    colormapTracer->position->setCoords( Settings::instance().calibration.tool_length_nm,  Settings::instance().calibration.tool_zero_angle_deg );


    for (int i = 0 ;i < traces.length(); i++)
    {

        traces[i]->resetPreProcessors();
        traces[i]->reProcessOriginalData();


         auto g = ui->plot->curve(i);
         auto d = traces[i]->getLockedDataPtr();
         auto keys = d->at(0);
         auto values = d->at(1);
         for (int i =0; i < keys.size(); i++)
         {
             keys[i] *= unit_multiplier_;
             values[i] *= unit_multiplier_;
         }
         g->setData(keys, values);
    }
     qDebug() << "Recalc all measurements";

     for (auto m: *measurements_)
     {
         m->reloadSettings();
         m->calculateMeasurement();

         updateInfo(m);
     }

     if (!do_not_replot_on_calib_set_)
     {
         if (ui->followCheckBox->checkState() == Qt::Checked)
             ui->actionResetScale->trigger();

         ui->plot->replot();
     }


}
