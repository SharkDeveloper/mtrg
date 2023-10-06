#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "dataproviders/dataprovider.h"
#include "dataproviders/filedataprovider.h"

#include "contour/contouranalisiswidget.h"
#include "profile/profileanalisiswidget.h"
#include "circle/circularanalisiswidget.h"
#include "device/settingsdialog.h"
#include "device/calibrationdialog.h"

#include <QFileDialog>
#include "helpers/reportrenderer.h"
#include "device/settings.h"

const QString kOpenFileFilterAll = "Все файлы (*.*)";
const QString kOpenFileTitleAll = "Открыть что то непонятное";
const QString kOpenFileFilterProfile = "Данные профиля (*.prftrc)";
const QString kOpenFileTitleProfile = "Открыть профиль";
const QString kOpenFileFilterContour = "Данные контура (*.txt | *.igs | *.prf130)";
const QString kOpenFileTitleContour = "Открыть контур";
const QString kOpenFileFilterCircular = "Данные круга (*.cirtrc)";
const QString kOpenFileTitleCircular = "Открыть круг";
const QString kProfileWidgetTabName = "ПРОФИЛЬ";
const QString kContourAnalysTabName = "ИЗМЕРЕНИЯ #%1";

//======================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    profile_ = new ProfileAnalisisWidget(this);
    connect(this, &MainWindow::settingsChanged, profile_, &ProfileAnalisisWidget::settingsChanged);
    ui->tabWidget->addTab(profile_, kProfileWidgetTabName);

    connect(ui->actionOpenContour, &QAction::triggered, this, [=](){ openNewTrace(CONTOUR);} );

    //save data action for current widget (profile | contour | circle )
    connect(ui->actionSave, &QAction::triggered, this, [=](){
        dynamic_cast<AnalisysWidget*>(ui->tabWidget->currentWidget())->saveData();
    } );

    //profile is a default widget, that generates data
    //when dataread completed, it emits readyForMeasurement, and MainWindow should process it and start new widget for analisys of data
    connect(profile_, SIGNAL(readyForMeasurment(QVector <AbstractTrace*>)), this, SLOT(createMeasurmentWidget(QVector <AbstractTrace*>)));

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, [=](int index) {
        delete ui->tabWidget->widget(index);
        ui->tabWidget->removeTab(index);

    });

    ui->tabWidget->setTabsClosable(true);

    ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->deleteLater();
    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, 0);

    connect(ui->actionSettings, &QAction::triggered, this, [=](){

        QCoreApplication::instance()->removeEventFilter(profile_->deviceEventFilter());
        SettingsDialog dialog(this, profile_->traces());
        dialog.exec();
        emit settingsChanged();
        QCoreApplication::instance()->installEventFilter(profile_->deviceEventFilter());
    });


    connect(ui->actionPrint, &QAction::triggered, this, [=](){

        QCoreApplication::instance()->removeEventFilter(profile_->deviceEventFilter());
        bool ctrler_selected = false;

        QString ctrler = QInputDialog::getItem(this, "Контроллер", "Кто вы?", Settings::instance().controllers, Settings::instance().current_controller_index, false, &ctrler_selected);

        if(!ctrler_selected)
            return;

        Settings::instance().current_controller_index = Settings::instance().controllers.indexOf(ctrler);
        if (Settings::instance().current_controller_index == -1)
        {
            Settings::instance().controllers.push_back(ctrler);
            Settings::instance().current_controller_index = 0;
        }

        Settings::instance().save();

        QString f = QFileDialog::getSaveFileName(this, "Сохранить отчет", "/home/dpuzyrkov/projects/mtrlg/app/report.html", "HTML (*.html);;OpenDocument (*.odt)");
        if (!f.isEmpty())
        {
            AnalisysWidget *currw = (AnalisysWidget*) ui->tabWidget->currentWidget();
            currw->reportPrepare();
            ReportRenderer r(this, currw->getReportTemplatePath(), f, currw->getReportReferencePath(), currw->getReportPixmaps(), currw->getReportData());
            currw->reportFinished();
            QCoreApplication::instance()->installEventFilter(profile_->deviceEventFilter());
        }
    } );


    QCoreApplication::instance()->installEventFilter(profile_->deviceEventFilter());
}

//======================================================================
void MainWindow::createMeasurmentWidget(QVector <AbstractTrace*> t)
{
    //only contour measurements for now
    ContourAnalisisWidget *w = new ContourAnalisisWidget(this, false);
    ui->tabWidget->addTab(w, kContourAnalysTabName.arg(QString::number(ui->tabWidget->count()-1)));
    ui->tabWidget->setCurrentWidget(w);
    //do it last, to make widget and plots used to new gomentry
    w->setTraces(t);
    connect(this, &MainWindow::settingsChanged, w, &ContourAnalisisWidget::recalcAllMeasurements);
}

//======================================================================
void MainWindow::openNewTrace(E_ANALISIS_TYPE type)
{
    //ask a profile widget to read new data from file
    //with specific type
    //QString f = "/home/dpuzyrkov/projects/mtrlg/data/crystall/weeel.iges.igs";
    QString f = openFile(type);
    if (!f.isEmpty())
    {
        DataProvider *p = new FileDataProvider(f);
        profile_->setDataProvider(p);
        profile_->obtainData(type);
    }
}

//======================================================================
QString MainWindow::openFile(E_ANALISIS_TYPE type)
{
    QString filter = kOpenFileFilterAll;
    QString text = kOpenFileTitleAll;

    switch (type) {

    case E_ANALISIS_TYPE::CONTOUR:
        filter = kOpenFileFilterContour;
        text = kOpenFileTitleContour;
        break;
    case E_ANALISIS_TYPE::PROFILE:
        filter = kOpenFileFilterProfile;
        text = kOpenFileTitleProfile;
        break;
    case E_ANALISIS_TYPE::CIRCULAR:
        filter = kOpenFileFilterCircular;
        text = kOpenFileTitleCircular;
        break;
    }
    return QFileDialog::getOpenFileName(this, text, "/home/dpuzyrkov/projects/mtrlg/data", filter);
}

//======================================================================
MainWindow::~MainWindow()
{
    delete ui;
}

