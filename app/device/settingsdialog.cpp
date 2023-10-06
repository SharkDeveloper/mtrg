#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "settings.h"
#include "QDoubleValidator"
#include "usersdialog.h"

#include "QSettings"

#include <QInputDialog>
#include <QMessageBox>
#include <QLayout>
#include "calibrationdialog.h"
#include "contour/contouranalisiswidget.h"

SettingsDialog::SettingsDialog(QWidget *parent, QList <AbstractTrace *> traces) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->headLengthSpinBox->setVisible(false);
    //ui->headRadiusSpinBox->setVisible(false);

    ui->headLengthLable->setVisible(false);
    //ui->headRadiusLable->setVisible(false);

    reload();
    connect(this, &QDialog::accepted, this, [=]() {

        Settings::instance().calibration.tool_length_nm = ui->tipLengthSpinBox->value();
        Settings::instance().calibration.tool_head_lenght_nm = ui->headLengthSpinBox->value();
        Settings::instance().calibration.tool_head_radius_nm = ui->headRadiusSpinBox->value();
        Settings::instance().calibration.tool_zero_angle_deg = ui->headHorizonAngleSpinBox->value();

        Settings::instance().device.tool_max_angle_deg = ui->maxToolAngleSpinBox->value();
        Settings::instance().device.tool_min_angle_deg = ui->minToolAngleSpinBox->value();
        Settings::instance().device.length_per_pulse_x_nm = ui->lengthByImpulseXSpinBox->value();
        Settings::instance().device.length_per_pulse_angle_nm = ui->lengthByImpulseYSpinBox->value();
        Settings::instance().device.mount_point_radius_mm = ui->mountPointRadiusSpinBox->value();

        Settings::instance().view.axis_unit = (ViewParameters::E_AXIS_UNIT) ui->axisUnit->currentIndex();

        Settings::instance().view.angles_in_deg = ui->anglesInDoubleCB->isChecked();
        Settings::instance().save();
    });

    connect(ui->openUsersDialog, &QPushButton::clicked, this, [=](){
        bool ok;
            QString text = QInputDialog::getText(this, tr("Сервис-пароль"),
                                                 tr("Сервис-пароль:"), QLineEdit::Normal,
                                                 "", &ok);

            if (ok && !text.isEmpty() && text == "1")
                UsersDialog(this).exec();
    });

    connect(ui->calibrationButton, &QPushButton::pressed, this, [=](){


        if (traces.length() == 0)
        {
            QMessageBox::critical(this, "Ошибка", "Нет трассы для калибровки. Откройте или запишите хотя бы одну трассу");
            return;
        }

        QDialog *d = new QDialog(this);
        d->setMinimumWidth(1024);
        QVBoxLayout *l = new QVBoxLayout();
        ContourAnalisisWidget *w = new ContourAnalisisWidget(d, true);
        w->setTraces(traces.toVector());
        l->addWidget(w);
        d->setLayout(l);
        d->setWindowTitle("Калибровка");
        d->exec();
        reload();
        delete d;
    });

}

void SettingsDialog::reload()
{

    ui->tipLengthSpinBox->setValue(Settings::instance().calibration.tool_length_nm);
    ui->headLengthSpinBox->setValue(Settings::instance().calibration.tool_head_lenght_nm);
    ui->headRadiusSpinBox->setValue(Settings::instance().calibration.tool_head_radius_nm);
    ui->headHorizonAngleSpinBox->setValue(Settings::instance().calibration.tool_zero_angle_deg);

    ui->maxToolAngleSpinBox->setValue(Settings::instance().device.tool_max_angle_deg);
    ui->minToolAngleSpinBox->setValue(Settings::instance().device.tool_min_angle_deg);

    ui->lengthByImpulseXSpinBox->setValue(Settings::instance().device.length_per_pulse_x_nm);
    ui->mountPointRadiusSpinBox->setValue(Settings::instance().device.mount_point_radius_mm);
    ui->lengthByImpulseYSpinBox->setValue(Settings::instance().device.length_per_pulse_angle_nm);

    ui->anglesInDoubleCB->setChecked(Settings::instance().view.angles_in_deg);
    ui->axisUnit->setCurrentIndex(Settings::instance().view.axis_unit);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
