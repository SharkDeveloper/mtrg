#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QDockWidget>
#include <dataproviders/serialdataprovider.h>
#include "device.h"
#include <QTimer>
#include <QAbstractButton>
#include <QKeyEvent>

namespace Ui {
class DeviceControlWidget;
}
class KeyReceiver;

class DeviceControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceControlWidget(QWidget *parent = nullptr);
    KeyReceiver * getKeyEF();
    ~DeviceControlWidget();

signals:
    void newTrace(DataProvider *p, E_ANALISIS_TYPE type);
    void statusBarText(QString txt);
private:
    Ui::DeviceControlWidget *ui;

    Device  device_;
    QTimer * detection_timer_;

    void configerDeviceSignals();

};

class KeyReceiver : public QObject
{
    Q_OBJECT
public:
    KeyReceiver(QMap <int, QAbstractButton *> map);
protected:
    QMap <int, QAbstractButton *> map_;
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif // DEVICECONTROLWIDGET_H
