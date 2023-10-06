#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "contour/contouranalisiswidget.h"
#include "profile/profileanalisiswidget.h"
#include "circle/circularanalisiswidget.h"
#include "dataproviders/dataprovider.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QString openFile(E_ANALISIS_TYPE type);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

public slots:

    void openNewTrace(E_ANALISIS_TYPE type);
    void createMeasurmentWidget(QVector <AbstractTrace *> t);

signals:
    void settingsChanged();

private:

    ProfileAnalisisWidget *profile_;


};
#endif // MAINWINDOW_H
