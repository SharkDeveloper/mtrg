#ifndef ANALISYSWIDGET_H
#define ANALISYSWIDGET_H

#include <QWidget>
#include "dataproviders/dataprovider.h"
#include <QMainWindow>
#include "../inja/include/inja/inja.hpp"

class AnalisysWidget: public QMainWindow
{
    Q_OBJECT

public:
    AnalisysWidget(const QString &readable_name);
    QString readableName();
    DataProvider * provider();
    void setDataProvider(DataProvider *p);
    bool obtainData(E_ANALISIS_TYPE type);
    ~AnalisysWidget();

    virtual AbstractTrace *getNewTrace() { return nullptr; };
    virtual QVector<QPixmap> getReportPixmaps() = 0;

    virtual void reportPrepare() = 0;
    virtual inja::json getReportData() = 0;
    virtual QString getReportTemplatePath() = 0;
    virtual QString getReportReferencePath() = 0;
    virtual void reportFinished() = 0;

public slots:
    virtual void saveData() = 0;
    virtual void dataReadStarted() {};
    virtual void dataBatchReady(int /*from*/, int /*count*/) {};
    virtual void dataReadFinished() {};
    virtual void dataReadError(QString /*error*/) {};





private:
    QString readable_name_;
    DataProvider *provider_ = nullptr;
    E_ANALISIS_TYPE type_;
};

#endif // ANALISYSWIDGET_H
