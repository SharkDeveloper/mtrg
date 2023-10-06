#include "circularanalisiswidget.h"
#include "ui_circularanalisiswidget.h"

static QString kAnalisysWidgetName = "КРУГ";

//======================================================================
CircularAnalisisWidget::CircularAnalisisWidget(QWidget * /*parent*/, DataProvider * /*data_provider*/) :
    AnalisysWidget(kAnalisysWidgetName),
    ui(new Ui::CircularAnalisisWidget)
{
    ui->setupUi(this);
}

//======================================================================
CircularAnalisisWidget::~CircularAnalisisWidget()
{
    delete ui;
}
