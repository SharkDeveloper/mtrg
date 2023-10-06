#ifndef CIRCULARANALISISWIDGET_H
#define CIRCULARANALISISWIDGET_H

#include <QWidget>
#include "helpers/analisyswidget.h"
#include "dataproviders/dataprovider.h"

namespace Ui {
class CircularAnalisisWidget;
}

class CircularAnalisisWidget :  public AnalisysWidget
{
    Q_OBJECT

public:
    explicit CircularAnalisisWidget(QWidget *parent, DataProvider * data_provider);
    ~CircularAnalisisWidget();

    AbstractTrace * getNewTrace() override { return nullptr; };

public slots:
    void dataBatchReady(int /*from*/, int /*count*/) override
    {

    }
    void dataReadStarted() override{

    }
    void dataReadFinished() override{

    }
    void dataReadError(QString /*error*/) override
    {

    }
private:
    Ui::CircularAnalisisWidget *ui;
};

#endif // CIRCULARANALISISWIDGET_H
