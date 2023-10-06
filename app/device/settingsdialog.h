#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "traces/abstracttrace.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent, QList <AbstractTrace *> traces);
    void reload();
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
