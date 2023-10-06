#include "device/devicecontrolwidget.h"
#include "ui_devicecontrolwidget.h"
#include <QFileDialog>
#include "device.h"
#include <QDebug>
#include <QKeyEvent>
#include <QtMath>

#include "device/settings.h"

#include "dataproviders/devicedataprovider.h"

QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 );border-bottom-right-radius: 5px;border-bottom-left-radius: 5px;border: .px solid black;}";
QString safe= "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #78d,stop: 0.4999 #46a,stop: 0.5 #45a,stop: 1 #238 );border-bottom-right-radius: 7px;border-bottom-left-radius: 7px;border: 1px solid black;}";

const double kMinDevAngle = -7.5;
const double kMaxDevAngle = 9.5;

KeyReceiver::KeyReceiver(QMap <int, QAbstractButton *> map)
{
    map_ = map;
}

bool KeyReceiver::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);

        if (map_.contains(key->key()) && !key->isAutoRepeat())
        {
            qDebug() << "key pressed "<< key;
            map_[key->key()]->setDown(true);
            map_[key->key()]->pressed();
            return true;
        }
    }

    if (event->type()==QEvent::KeyRelease)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);

        if (map_.contains(key->key()) && !key->isAutoRepeat())
        {
            qDebug() << "key released "<< key;
            map_[key->key()]->setDown(false);
            map_[key->key()]->released();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

DeviceControlWidget::DeviceControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceControlWidget)
{
    ui->setupUi(this);

    ui->progressBar->setMinimum(kMinDevAngle*100);
    ui->progressBar->setMaximum(kMaxDevAngle*100);

    ui->speedComboBox->clear();
    ui->speedComboBox->addItem(Device::kSpeedMap[E_SPEED_025_MMS], QVariant(E_SPEED_025_MMS));
    ui->speedComboBox->addItem(Device::kSpeedMap[E_SPEED_050_MMS], QVariant(E_SPEED_050_MMS));
    ui->speedComboBox->addItem(Device::kSpeedMap[E_SPEED_100_MMS], QVariant(E_SPEED_100_MMS));
    ui->speedComboBox->addItem(Device::kSpeedMap[E_SPEED_200_MMS], QVariant(E_SPEED_200_MMS));
    ui->speedComboBox->setCurrentIndex(Settings::instance().movement.current_speed);



    QList <QDoubleSpinBox *> move_dbls_ctls;
    move_dbls_ctls << ui->xplusSpinBox
              << ui->yplusSpinBox
              << ui->zplusSpinBox
              << ui->xminusSpinBox
              << ui->yminusSpinBox
              << ui->zminusSpinBox;

    QList <double *> settings_ptrs;

    settings_ptrs << &Settings::instance().movement.xplus_step
                  << &Settings::instance().movement.yplus_step
                  << &Settings::instance().movement.zplus_step
                  << &Settings::instance().movement.zminus_step
                  << &Settings::instance().movement.yminus_step
                  << &Settings::instance().movement.xminus_step;


    for (auto ctl: move_dbls_ctls) ctl->setHidden(true);

    for(int i =0; i < move_dbls_ctls.size(); i++) move_dbls_ctls.at(i)->setValue(*settings_ptrs.at(i));

    for(int i =0; i < move_dbls_ctls.size(); i++) connect(move_dbls_ctls.at(i), qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double v) {
       *settings_ptrs.at(i) = v;
       Settings::instance().save();
    });

    connect(ui->userDefinedAxisStepCheckBox, &QCheckBox::stateChanged, this, [=](int state){
        Settings::instance().movement.user_defined_axis_step = state == Qt::CheckState::Checked;
        Settings::instance().save();
        for(auto ctl: move_dbls_ctls) ctl->setHidden(!Settings::instance().movement.user_defined_axis_step);
    });

    ui->speedComboBox->setCurrentIndex(Settings::instance().movement.current_speed);

    connect(ui->speedComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index){
        Settings::instance().movement.current_speed = index;
        Settings::instance().save();
    });

    ui->userDefinedAxisStepCheckBox->setChecked(Settings::instance().movement.user_defined_axis_step);

    connect(ui->tipRaiseLimitCheckBox, &QCheckBox::stateChanged, this, [&] (int state) {
        device_.limitTipAscendToCurrentLevel(state == Qt::Checked);
    });

    this->setEnabled(false);
    configerDeviceSignals();

}

KeyReceiver * DeviceControlWidget::getKeyEF()
{
    QMap <int, QAbstractButton *> btnmap;
    btnmap[Qt::Key_Left] = ui->xminusButton;
    btnmap[Qt::Key_Right] = ui->xplusButton;
    btnmap[Qt::Key_Up] = ui->yplusButton;
    btnmap[Qt::Key_Down] = ui->yminusButton;
    btnmap[Qt::Key_PageUp] = ui->pinupButton;
    btnmap[Qt::Key_PageDown] = ui->pindownButton;
    btnmap[Qt::Key_Space] = ui->stopButton;
    btnmap[Qt::Key_Home] = ui->zplusButton;
    btnmap[Qt::Key_End] = ui->zminusButton;
    btnmap[Qt::Key_Return] = ui->startTraceButton;

    KeyReceiver *kr = new KeyReceiver(btnmap);
    return kr;
}
void DeviceControlWidget::configerDeviceSignals()
{

    connect (&device_, &Device::deviceOpened, this, [=]() {

                emit statusBarText("Устройство " + device_.address() + " соединено");
                device_.setSpeed(E_SPEED_025_MMS);
                this->setEnabled(true);
        });

    connect (&device_, &Device::deviceLost, this, [=]() {
                emit statusBarText("Устройство не найдено");
                this->setEnabled(false);
                device_.find();
        });

    connect(ui->speedComboBox, SIGNAL(currentIndexChanged(int)), &device_, SLOT(setSpeed(int)));

    connect(ui->startTraceButton, &QPushButton::clicked, this, [=](){
        emit newTrace(new DeviceDataProvider(&device_), CONTOUR);
        ui->startTraceButton->setEnabled(false);
    });

    connect(ui->stopButton, &QPushButton::clicked, this, [=](){
        device_.stopTrace();
        ui->startTraceButton->setEnabled(true);
    });

    connect(ui->xplusButton, &QPushButton::pressed, this, [=](){
        if (ui->userDefinedAxisStepCheckBox->checkState() == Qt::Checked)
            device_.moveXFixed(ui->xplusSpinBox->value());
        else
            device_.moveX(1);
    });

    connect(ui->xminusButton, &QPushButton::pressed, this, [=](){
        if (ui->userDefinedAxisStepCheckBox->checkState() == Qt::Checked)
            device_.moveXFixed(-ui->xminusSpinBox->value());
        else
            device_.moveX(-1);
    });

    connect(ui->xplusButton, &QPushButton::released, this, [=](){
        device_.moveX(0);
    });
    connect(ui->xminusButton, &QPushButton::released, this, [=](){
        device_.moveX(0);
    });

    connect(ui->yplusButton, &QPushButton::pressed, this, [=](){
        if (ui->userDefinedAxisStepCheckBox->checkState() == Qt::Checked)
            device_.moveYFixed(ui->yplusSpinBox->value());
        else
            device_.moveY(1);
    });

    connect(ui->yminusButton, &QPushButton::pressed, this, [=](){
        if (ui->userDefinedAxisStepCheckBox->checkState() == Qt::Checked)
            device_.moveYFixed(-ui->yplusSpinBox->value());
        else
            device_.moveY(-1);
    });

    connect(ui->yplusButton, &QPushButton::released, this, [=](){
        device_.moveY(0);
    });

    connect(ui->yminusButton, &QPushButton::released, this, [=](){
        device_.moveY(0);
    });

    connect(ui->zplusButton, &QPushButton::pressed, this, [=](){
        if (ui->userDefinedAxisStepCheckBox->checkState() == Qt::Checked)
            device_.moveZFixed(ui->yplusSpinBox->value());
        else
            device_.moveZ(1);
    });

    connect(ui->zminusButton, &QPushButton::pressed, this, [=](){
        if (ui->userDefinedAxisStepCheckBox->checkState() == Qt::Checked)
            device_.moveZFixed(-ui->yplusSpinBox->value());
        else
            device_.moveZ(-1);
    });

    connect(ui->zplusButton, &QPushButton::released, this, [=](){
        device_.moveZ(0);
    });

    connect(ui->zminusButton, &QPushButton::released, this, [=](){
        device_.moveZ(0);
    });

    connect(ui->pinupButton, &QPushButton::clicked, this, [=](){
        device_.moveTipUp();
    });
    connect(ui->pindownButton, &QPushButton::clicked, this, [=](){
        device_.moveTipDown();
    });

    QTimer *notification_timer = new QTimer();
    connect(notification_timer, &QTimer::timeout, this, [&]() {
        if (device_.running())
            emit statusBarText("Устройство " + device_.address() + " считано " + QString::number(device_.pointsReaded()));
    });
    notification_timer->setInterval(1000);
    notification_timer->start();

    connect(&device_, &Device::dataRecieved, this, [=](double angle, uint x, uint alarm)
    {
        if (device_.type() == CNTR220)
        {

            ui->progressBar->setMinimum(kMinDevAngle*100);
            ui->progressBar->setMaximum(kMaxDevAngle*100);

            ui->progressBar->setValue(qRadiansToDegrees(angle)*100);

            if (alarm != 0)
            {
                ui->progressBar->setStyleSheet(danger);
                ui->startTraceButton->setEnabled(false);
            }
            else
            {
                ui->progressBar->setStyleSheet(safe);
                if (!device_.traceStarted())
                    ui->startTraceButton->setEnabled(true);
            }
        }

        if (device_.type() == PROF130)
        {
            //angle is 2bytes int
            //norm it to min & max

            ui->progressBar->setMinimum(0);
            ui->progressBar->setMaximum(65535);


            ui->progressBar->setValue(65535 - angle);

            if (alarm != 0)
            {
                ui->progressBar->setStyleSheet(danger);
                ui->startTraceButton->setEnabled(false);
            }
            else
            {
                ui->progressBar->setStyleSheet(safe);
                if (!device_.traceStarted())
                    ui->startTraceButton->setEnabled(true);
            }
        }
    });


    connect(ui->deviceType, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index){

        device_.setType(index);
        if (index == 1)
        {
            ui->pinupButton->hide();
            ui->xplusButton->hide();
            ui->xminusButton->hide();
            ui->zplusButton->hide();
            ui->zminusButton->hide();
            ui->automaxButton->hide();
            ui->yminusButton->hide();
            ui->yplusButton->hide();
            ui->tipRaiseLimitCheckBox->hide();
            ui->pindownButton->hide();
            ui->userDefinedAxisStepCheckBox->hide();
        }
        else
        {
            ui->pinupButton->show();
            ui->xplusButton->show();
            ui->xminusButton->show();
            ui->zplusButton->show();
            ui->zminusButton->show();
            ui->automaxButton->show();
            ui->yminusButton->show();
            ui->yplusButton->show();
            ui->tipRaiseLimitCheckBox->show();
            ui->pindownButton->show();
            ui->userDefinedAxisStepCheckBox->show();
        }

    });

}

DeviceControlWidget::~DeviceControlWidget()
{
    delete ui;
}
