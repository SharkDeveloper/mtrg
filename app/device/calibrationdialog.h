#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include "traces/abstracttrace.h"

namespace Ui {
class calibrationdialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget *parent, AbstractTrace *trace);
    ~CalibrationDialog();
private:
    void createPlot();
private:
    Ui::calibrationdialog *ui;

};

#endif // CALIBRATIONDIALOG_H
