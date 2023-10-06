#include "profileanalisiswidget.h"
#include "ui_profileanalisiswidget.h"
#include "traces/contourtrace.h"
#include "helpers/constants.h"
#include <QtGlobal>
#include <QHBoxLayout>
#include "device/settings.h"

//======================================================================
const QString kProfileAnalisisWidgetName = "ПРОФИЛЬ";
const QString kTraceNameTemplate = "Трасса #%1 = %3";
const QString kTraceInfoTemplate = "Трасса (%1 измерений с шагом %2 мкм)";
const QString kDeleteTraceString = "Удалить трассу";

//======================================================================
void foreachGraph(QCustomPlot *p, std::function<void (int index, QCPGraph *)> f)
{
    for (int i = 0; i < p->graphCount(); i++)
    {
        f(i, p->graph(i));
    }
}

//GOST 25142-82
//======================================================================
//* fill tree-view for each trace with calculated params
//======================================================================
void updateTraceListParameters(QTreeWidget *list, TraceGraphsHolder *holder)
{
    auto rootitem = list->topLevelItem(holder->id());

    if (rootitem == nullptr)
        return;

    rootitem->setText(2, holder->paramValue(TraceGraphsHolder::TraceParam::E_PARAM_RMR));
    rootitem->setText(3, holder->paramValue(TraceGraphsHolder::TraceParam::E_PARAM_DELTA_M_X));
    rootitem->setText(4, holder->paramValue(TraceGraphsHolder::TraceParam::E_PARAM_DELTA_M_Y));
    rootitem->setText(5, holder->paramValue(TraceGraphsHolder::TraceParam::E_PARAM_RA));

    for (int i = TraceGraphsHolder::TraceParam::E_PARAM_TRACE_LEN; i < TraceGraphsHolder::TraceParam::E_PARAM_PARAMS_END; i++)
    {
        auto child = rootitem->child(i);
        if (child)
            child->setText(1, holder->paramValue((TraceGraphsHolder::TraceParam::EParamType)i));
    }

    list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//======================================================================
ProfileAnalisisWidget::ProfileAnalisisWidget(QWidget *parent) :
    AnalisysWidget(kProfileAnalisisWidgetName),
    ui(new Ui::ProfileAnalisisWidget),
    data_read_in_progress(false)
{
    ui->setupUi(this);

    //ask mainwindow to create new analisys widget
    connect(ui->actionToMeasurement, &QAction::triggered, this, [=]()
    {
        //do nothing if there no traces opened
        if (graphs_.isEmpty())
            return;

        //clone visible traces (that are check in tree-view)
        QVector <AbstractTrace *> traces;
        for (auto g : graphs_)
        {
            if(g->isVisible())
            {
                traces.push_back(new ContourTrace(g->currentData())); //creates a new ContourTrace with all modifications
            }
        }

        if (traces.isEmpty())
            return;

        //from now, new widget manages this traces, no need to delete them.
        emit readyForMeasurment(traces);
    });

    //serve events from device-control widget (new trace button)
    connect(ui->deviceControlWidget, &DeviceControlWidget::newTrace, this, [=](DataProvider *p, E_ANALISIS_TYPE type) {
        setDataProvider(p);
        obtainData(type);
    });

    //init graphics
    configurePlots();
    mainplot_->replot();

    connect(ui->actionPAN, &QAction::toggled, this, [=](bool enabled)
    {
        if (enabled)
        {
            ui->actionMarkers->setChecked(false);
            ui->actionSelectionZoom->setChecked(false);
            mainplot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            mainplot_->setSelectionRectMode(QCP::srmNone);
        }

        if (enabled)
        {
            mainplot_->setCursor(Qt::OpenHandCursor);
        }
        else
        {
            mainplot_->unsetCursor();
        }

    });

    //reset action just resets the view
    connect(ui->actionReset, &QAction::triggered, this,  [=]() {
        mainplot_->rescaleAxes(true);
        mainplot_->replot();
        qDebug() << "reset action";

    });

    //custom context menu for traces list
    ui->tracesList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tracesList,&QTreeWidget::customContextMenuRequested,this, [=](const QPoint & pos){
        QTreeWidget *tree = ui->tracesList;

        QTreeWidgetItem *nd = tree->itemAt( pos );

        //check if any item clicked
        if (!nd) {return;}

        //check if clicked item is toplevel
        if(nd->parent()) {return;}

        QAction *deleteTraceAction = new QAction(kDeleteTraceString, this);
        connect(deleteTraceAction, &QAction::triggered, this, [=](){
            int index = nd->data(0,Qt::UserRole).toInt();

            qDebug() << "remove item from tree" << index;

            //delete holders
            delete graphs_[index];

            //delete traces (holder not own them)
            delete traces_[index];

            traces_.remove(index);
            graphs_.remove(index);

            int pos = ui->tracesList->indexOfTopLevelItem(nd);
            if (pos == -1)
                pos = ui->tracesList->indexOfTopLevelItem(nd->parent());

            delete tree->takeTopLevelItem(pos);
        });

        QMenu menu(this);
        menu.addAction(deleteTraceAction);

        QPoint pt(pos);
        menu.exec( tree->mapToGlobal(pos) );
    });

    //process user selection in tree-view
    connect(ui->tracesList, &QTreeWidget::itemSelectionChanged, this, [=]()
    {
        //deselect all graphs
        for (auto g: graphs_)
        {
            g->deselect();
        }

        //select required
        for (auto i:  ui->tracesList->selectedItems())
        {
            //find top-levl item (it's index is graph (and trace) index)
            int pos = ui->tracesList->indexOfTopLevelItem(i);
            if (pos == -1)
                pos = ui->tracesList->indexOfTopLevelItem(i->parent());
            auto item = ui->tracesList->topLevelItem(pos);
            graphs_[item->data(0,Qt::UserRole).toInt()]->select();
        }

        mainplot_->replot();

    });

    //if a check was pressed in tree-view
    connect(ui->tracesList, &QTreeWidget::itemChanged, this, [=](QTreeWidgetItem *item, int column){

        if (column == 0)
        {
            int index = ui->tracesList->topLevelItem( ui->tracesList->indexOfTopLevelItem(item))->data(0, Qt::UserRole).toInt();
            graphs_[index]->setVisible(item->checkState(0) == Qt::Checked);
        }

        mainplot_->replot();
    });

    connect(ui->actionPTS, &QAction::toggled, this, [=](bool enabled) {

        for (auto graph : graphs_)
        {
            graph->showPoints(enabled);
        }

        mainplot_->replot();
    });


    connect(ui->actionRLine, &QAction::toggled, this, [=](bool enabled) {
        if (!enabled)
        {
            ui->actionRFit->setChecked(false);
        }

        for (auto graph : graphs_)
        {
            if (graph->isVisible())
            {
                graph->calculateGraphs(enabled, false);
                ui->actionRFit->setEnabled(enabled);
            }
        }

        mainplot_->replot();
    });


    connect(ui->actionRFit, &QAction::toggled, this, [=](bool enabled) {
        for (auto graph : graphs_)
        {
            if (graph->isVisible())
                graph->calculateGraphs(true, enabled);
        }

        mainplot_->rescaleAxes(true);
        mainplot_->replot();
    });

    connect(ui->actionSelectionZoom, &QAction::toggled, this, [=](bool enabled) {
        if (enabled)
        {
            ui->actionPAN->setChecked(false);
            ui->actionMarkers->setChecked(false);
            mainplot_->setSelectionRectMode(QCP::srmZoom);
        }
        else
        {
            mainplot_->setSelectionRectMode(QCP::srmNone);
        }
    });

    connect(ui->actionMarkers, &QAction::toggled, this, [=](bool enabled) {

        if (enabled)
        {
            ui->actionCut->setEnabled(true);
            ui->actionPAN->setChecked(false);
            ui->actionSelectionZoom->setChecked(false);

            mainplot_->setInteractions(QCP::iSelectPlottables | QCP::iKeepSelections | QCP::iRangeSelection );
            mainplot_->setSelectionRectMode(QCP::srmSelectRealtime);


            for (auto g: graphs_) g->setMainSelectionMode(QCP::stDataRange);
        }
        else
        {
            mainplot_->setInteractions(QCP::iSelectPlottables);
            mainplot_->setSelectionRectMode(QCP::srmNone);
            ui->actionCut->setEnabled(false);
        }
    });

    connect(ui->actionCut, &QAction::triggered, this, [=]() {
        for(auto g: graphs_)
        {
            g->cutSelection(); //will cut only if selection presents in graphholder
            mainplot_->rescaleAxes(true);
            mainplot_->replot();
        }
    });

    //sync Y axis for hist & rmr plots with main
    connect(ui->plotMain->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotHist->yAxis, SLOT(setRange(QCPRange)));
    connect(ui->plotMain->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotRMR->yAxis, SLOT(setRange(QCPRange)));

    connect(ui->plotMain, &QCustomPlot::afterReplot, this, [=] {
        recalcParamPlots(QCPRange());
    });


    //rmr line processing
    connect(ui->plotRMR, &QCustomPlot::mousePress, this, [=](QMouseEvent *event){
        if (event->button() != Qt::LeftButton) return;

        double y = ui->plotRMR->yAxis->pixelToCoord(event->y());
        double x = ui->plotRMR->xAxis->pixelToCoord(event->x());

        QCPGraph *g = (QCPGraph*)ui->plotRMR->plottableAt(event->pos(), true);
        if (g)
        {
            TraceGraphsHolder * holder = qvariant_cast<TraceGraphsHolder*>( g->property("gholder"));
            holder->setRMRLine(x, y);
            ui->plotMain->replot();
            updateTraceListParameters(ui->tracesList, holder);
        }
    });

    ui->tracesList->setFont(kFontFixed);


    //combine histogramm density widget and put it to toolbar
    QWidget *w = new QWidget(ui->toolBar);
    QHBoxLayout *l = new QHBoxLayout(w);
    w->setLayout(l);
    QLabel *lb = new QLabel(ui->toolBar);
    QSpinBox *spinbox = new QSpinBox(ui->toolBar);
    lb->setText("Плотность гистограммы:");
    l->addWidget(lb);
    l->addWidget(spinbox);
    spinbox->setRange(1,50);
    spinbox->setValue(10);
    spinbox->setSuffix("%");
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    median_filter_cb_  = new QCheckBox("Медианный фильтр");
    median_filter_cb_->setCheckState(Qt::Checked);

    median_filter_sb_  = new QSpinBox();
    median_filter_sb_->setMinimum(3);
    median_filter_sb_->setMaximum(50);
    median_filter_sb_->setSuffix(" точек");

    ui->toolBar->addWidget(spacer);
    ui->toolBar->addWidget(median_filter_cb_);
    ui->toolBar->addWidget(median_filter_sb_);
    ui->toolBar->addWidget(w);

    //histogram densyty spinner
    connect(spinbox,  static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int val){
        for (auto g : graphs_)
        {
            g->setHistogramDP(val);
        }
        mainplot_->replot(); //it does histogram replot
    });

    connect(median_filter_sb_,  static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int val){

        for (auto graph : graphs_)
        {
            graph->trace->clearPostProcessors();
            graph->trace->addPostProcessor(new DataMedianFilterProcessor(val));
            graph->trace->addPostProcessor(new XZeronator());
            graph->trace->doPostProcessing();
            graph->resetData();
            mainplot_->replot();
        }
    });

    //histogram densyty spinner
    connect(median_filter_cb_,  &QCheckBox::stateChanged, this, [=](int val){
        if (val == Qt::Checked)
        {
            median_filter_sb_->setEnabled(true);
            //do median for traces
            for (auto graph : graphs_)
            {
                graph->trace->clearPostProcessors();
                graph->trace->addPostProcessor(new DataMedianFilterProcessor(median_filter_sb_->value()));
                graph->trace->addPostProcessor(new XZeronator());
                graph->trace->doPostProcessing();
                graph->resetData();
            }
            mainplot_->replot();
        }
        else
        {
            //undo median for traces
            median_filter_sb_->setEnabled(false);
            for (auto graph : graphs_)
            {
                graph->trace->clearPostProcessors();
                graph->trace->doPostProcessing();
                graph->resetData();
            }
            mainplot_->replot();
        }
    });


    connect(ui->deviceControlWidget, &DeviceControlWidget::statusBarText, this, [=](QString text) {
       ui->statusBar->showMessage(text);
       ui->statusBar->addWidget(new QLabel(this));
    });

    device_evfilter_ = ui->deviceControlWidget->getKeyEF();

    setAxisSuffixesAndScaleData();
}

//======================================================================
void ProfileAnalisisWidget::setAxisSuffixesAndScaleData()
{
    QString unit;
    if (Settings::instance().view.axis_unit == ViewParameters::MM)
    {
        ui->plotMain->xAxis->ticker()->setSuffix(" мм");
        ui->plotMain->yAxis->ticker()->setSuffix(" мм");
        unit = "mm";
    }

    if (Settings::instance().view.axis_unit == ViewParameters::NM)
    {
        ui->plotMain->xAxis->ticker()->setSuffix(" нм");
        ui->plotMain->yAxis->ticker()->setSuffix(" нм");
        unit = "nm";
    }

    if (Settings::instance().view.axis_unit == ViewParameters::MK)
    {
        ui->plotMain->xAxis->ticker()->setSuffix(" мкм");
        ui->plotMain->yAxis->ticker()->setSuffix(" мкм");
        unit = "mkm";
    }


    ui->plotHist->xAxis->ticker()->setSuffix("%");
    ui->plotRMR->xAxis->ticker()->setSuffix("%");

    for (auto g: graphs_)
    {
        g->scaleData(unit);
    }

    ui->actionReset->trigger();
}

//======================================================================
void ProfileAnalisisWidget::settingsChanged()
{
    setAxisSuffixesAndScaleData();
}

//======================================================================
KeyReceiver *ProfileAnalisisWidget::deviceEventFilter()
{
    return device_evfilter_;
}

//======================================================================
void ProfileAnalisisWidget::recalcParamPlots(QCPRange range)
{
    if (data_read_in_progress) return;

    for(auto g : graphs_)
    {
        if (g->isVisible()) //recalc this trash only for visible chatrs for performance
        {
            g->calculateHsitAndRMR();
            updateTraceListParameters(ui->tracesList, g);
        }
    }

    histplot_->xAxis->rescale(true);
    histplot_->replot();
    rmrplot_->xAxis->rescale(true);
    rmrplot_->replot();
}

//======================================================================
void ProfileAnalisisWidget::configurePlots()
{
    mainplot_ = ui->plotMain;
    histplot_ = ui->plotHist;
    rmrplot_  = ui->plotRMR;
    mainplot_->setFont(kFontFixed);
    histplot_->setFont(kFontFixed);
    rmrplot_->setFont(kFontFixed);

    ui->plotMain->plotLayout()->clear();
    ui->plotHist->plotLayout()->clear();
    ui->plotRMR->plotLayout()->clear();

    QCPAxisRect *wideAxisRect = new QCPAxisRect(ui->plotMain);
    wideAxisRect->setupFullAxesBox(true);
    wideAxisRect->axis(QCPAxis::atRight, 0)->setVisible(false);

    QCPAxisRect *wideAxisRectHist = new QCPAxisRect(ui->plotHist);
    wideAxisRectHist->setupFullAxesBox(true);
    wideAxisRectHist->axis(QCPAxis::atLeft, 0)->setVisible(false);
    wideAxisRectHist->axis(QCPAxis::atRight, 0)->setVisible(false);

    QCPAxisRect *wideAxisRectRMR = new QCPAxisRect(ui->plotRMR);
    wideAxisRectRMR->setupFullAxesBox(true);
    wideAxisRectRMR->axis(QCPAxis::atLeft, 0)->setVisible(false);
    wideAxisRectRMR->axis(QCPAxis::atRight, 0)->setVisible(true);

    ui->plotMain->plotLayout()->addElement(0, 0, wideAxisRect); // insert axis rect in first row
    ui->plotMain->xAxis->setNumberFormat("f");
    ui->plotMain->xAxis->setNumberPrecision(3);

    ui->plotMain->yAxis->setNumberFormat("f");
    ui->plotMain->yAxis->setNumberPrecision(3);
    ui->plotMain->yAxis2->setNumberFormat("f");
    ui->plotMain->yAxis2->setNumberPrecision(3);

    ui->plotHist->plotLayout()->addElement(0, 0, wideAxisRectHist); // insert axis rect in first row
    ui->plotHist->yAxis->setNumberFormat("f");
    ui->plotHist->yAxis->setNumberPrecision(3);
    ui->plotHist->yAxis2->setNumberFormat("f");
    ui->plotHist->yAxis2->setNumberPrecision(3);

    ui->plotRMR->plotLayout()->addElement(0, 0, wideAxisRectRMR); // insert axis rect in first row
    ui->plotRMR->yAxis->setNumberFormat("f");
    ui->plotRMR->yAxis->setNumberPrecision(3);
    ui->plotRMR->yAxis2->setNumberFormat("f");
    ui->plotRMR->yAxis2->setNumberPrecision(3);
    ui->plotRMR->setCursor(Qt::CrossCursor);


    //TODO: add right-axis values on RMR plot

}

//======================================================================
void addNewTraceToList(QTreeWidget *list, AbstractTrace *trace)
{
    QVariant var;
    var.setValue(trace->id());
    qDebug() << "New trace id" << trace->id();
    auto item = new QTreeWidgetItem(list);
    item->setData(0,Qt::UserRole,var);
    item->setText(0, "Трасса #"+QString::number(trace->id()));
    item->setText(1, trace->getCreationDate().toString());

    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    item->setTextAlignment(0, Qt::AlignLeft);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setTextAlignment(2, Qt::AlignRight);
    item->setTextAlignment(3, Qt::AlignRight);
    item->setTextAlignment(4, Qt::AlignRight);

    for (int i = TraceGraphsHolder::TraceParam::E_PARAM_TRACE_LEN; i < TraceGraphsHolder::TraceParam::E_PARAM_PARAMS_END; i++)
    {
        QTreeWidgetItem *child = new QTreeWidgetItem();
        child->setText(0, TraceGraphsHolder::paramName((TraceGraphsHolder::TraceParam::EParamType) i));
        child->setText(1, " ");
        item->addChild(child);
    }
}

//======================================================================
AbstractTrace * ProfileAnalisisWidget::getNewTrace()
{
    //create new trace container
    //will fill it in dataprovider
    ContourTrace * t = new ContourTrace();
    traces_[t->id()] = t;

    if (median_filter_cb_->checkState() == Qt::Checked) //
        t->addPostProcessor(new DataMedianFilterProcessor(median_filter_sb_->value()));
    t->addPostProcessor(new XZeronator());
    return t;
}

//======================================================================
void ProfileAnalisisWidget::dataBatchReady(int from, int count)
{
    //got new batch of data from dataprovider
    AbstractTrace *trace = dynamic_cast<DataProvider*>(QObject::sender())->getTrace();

    //mutex trace
    auto data = trace->getLockedDataPtr();

    //realtime show new data
    auto keys = data->at(0).mid(from, count);
    auto values = data->at(1).mid(from,count);

    for (int i = 0; i < keys.size(); i ++)
    {
        keys[i] = keys[i] * graphs_[trace->id()]->unit_multiplier();
        values[i] = values[i] * graphs_[trace->id()]->unit_multiplier();
    }

    graphs_[trace->id()]->main->addData(keys, values);
    graphs_[trace->id()]->main->rescaleAxes(true);

    mainplot_->rescaleAxes();
    mainplot_->replot();

    //mutex will be freed on exit of this function
}

//======================================================================
void ProfileAnalisisWidget::dataReadStarted()
{
    //dataprovider reports it is ready for dataread
    //create new graph
    AbstractTrace * trace = dynamic_cast<DataProvider*>(QObject::sender())->getTrace();
    graphs_[trace->id()] = new TraceGraphsHolder(traces_.size()-1,
                        trace,
                        mainplot_, histplot_, rmrplot_);
    QString unit;
    if (Settings::instance().view.axis_unit == ViewParameters::MM)
    {
        unit = "mm";
    }

    if (Settings::instance().view.axis_unit == ViewParameters::NM)
    {
        unit = "nm";
    }

    if (Settings::instance().view.axis_unit == ViewParameters::MK)
    {
        unit = "mkm";
    }

    graphs_[trace->id()]->scaleData(unit, false);

    data_read_in_progress = true; //should be set here, cause of tracesList change make mainplot replot

    addNewTraceToList(ui->tracesList, dynamic_cast<DataProvider*>(QObject::sender())->getTrace());
}

//======================================================================
void calculateAllGraphs(TraceGraphsHolder *h, bool pts, bool rline, bool rfit)
{
    h->calculateGraphs(rline, rfit);
    h->showPoints(pts);
}

//======================================================================
void ProfileAnalisisWidget::dataReadFinished()
{
    //update whole readed graph with final data

    AbstractTrace *trace = dynamic_cast<DataProvider*>(QObject::sender())->getTrace();

    calculateAllGraphs(graphs_[trace->id()],
                        ui->actionPTS->isChecked(),
                        ui->actionRLine->isChecked(),
                        ui->actionRFit->isChecked());
    data_read_in_progress = false;
    settingsChanged();
    updateTraceListParameters(ui->tracesList, graphs_[trace->id()]);

    mainplot_->rescaleAxes(true);
    mainplot_->replot();
    //do not need update other plots due to it connected to mainplot afterReplot
}

//======================================================================
void ProfileAnalisisWidget::dataReadError(QString error)
{
    data_read_in_progress = false;
    QMessageBox::warning(this, "Error", error);
}

//======================================================================
void ProfileAnalisisWidget::saveData()
{
    //write all traces to different files
    for (auto g : graphs_)
    {
        QString fname = QFileDialog::getSaveFileName(this,  g->trace->getSourceName(), "/home/dpuzyrkov/projects/mtrlg_src/Программа/Контурограф/Измерения/Профили", "Text files (*.txt);");
        if (fname.isEmpty())
            continue;

        QFile f(fname);
        f.open(QFile::WriteOnly);

        if (!f.isOpen())
            continue;

        auto data = g->trace->getLockedDataPtr();
        for (int i =0; i < (*data)[0].length(); i++)
        {
            //data in MM in graph, convert to nm
            QString dataw = QString::number((*data)[0][i]*1000) + "\t" + QString::number((*data)[1][i]*1000) + "\t" + QString::number((*data)[2][i]) + "\n";
            f.write(dataw.toUtf8());
        }
        f.close();
    }
}


//======================================================================
ProfileAnalisisWidget::~ProfileAnalisisWidget()
{
    delete ui;
}

//======================================================================
QVector<QPixmap> ProfileAnalisisWidget::getReportPixmaps()
{
    QVector <QPixmap> r;
    r << mainplot_->toPixmap();

    for (auto g: graphs_)
    {
        g->setVisible(false);
    }

    for (auto g: graphs_)
    {
        g->setVisible(true);
        r << mainplot_->toPixmap();
        g->setVisible(false);
    }

    for (auto g: graphs_)
    {
        g->setVisible(true);
    }

    return  r;
}

//======================================================================
void ProfileAnalisisWidget::reportPrepare()
{

}

//======================================================================
inja::json ProfileAnalisisWidget::getReportData()
{
    inja::json data;
    data["controller_name"] = Settings::instance().controllers.at(Settings::instance().current_controller_index).toStdString();
    data["date"] = QDateTime::currentDateTime().toLocalTime().toString().toStdString();
    for (auto g: graphs_)
    {
        inja::json trace;
        trace["name"] = g->trace->getSourceName().toStdString();


        for (int i = TraceGraphsHolder::TraceParam::E_PARAM_TRACE_LEN; i < TraceGraphsHolder::TraceParam::E_PARAM_PARAMS_END; i++)
        {
            inja::json param;
            param["name"] = g->paramName((TraceGraphsHolder::TraceParam::EParamType)i).toStdString();
            param["value"] = g->paramValue((TraceGraphsHolder::TraceParam::EParamType)i).toStdString();
            trace["params"].push_back(param);
        }

        data["traces"].push_back(trace);
    }
    return data;
}

//======================================================================
QString ProfileAnalisisWidget::getReportTemplatePath()
{
    return "reports/profile.md";
}

//======================================================================
QString ProfileAnalisisWidget::getReportReferencePath()
{
    return "reports/profile_ref.odt";
}

//======================================================================
void ProfileAnalisisWidget::reportFinished()
{

}

//======================================================================
QList <AbstractTrace *> ProfileAnalisisWidget::traces()
{
    return traces_.values();
}
