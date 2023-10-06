#ifndef ABSTRACTTRACE_H
#define ABSTRACTTRACE_H


#include <memory>
#include <QMutexLocker>
#include <QDateTime>
#include "helpers/datatypes.h"
#include "singlepointprocessors.h"
#include "postprocessors.h"

class AbstractTrace
{

public:
    AbstractTrace(int var_count);
    virtual ~AbstractTrace();
    int id() { return id_; }

public:
    std::shared_ptr< TData >  getLockedDataPtr();

    int getDimensions();
    void writeData(QVector<double> &data);

    QDateTime getCreationDate();
    QString getSourceName();
    void setSourceName(const QString &name);
    void addSinglePointProcessor(IDataSinglePointProcessor *p);
    void addPostProcessor(IDataPostProcessor *p);
    void clearPreProcessors();
    void clearPostProcessors();
    void doPostProcessing();
    void reProcessOriginalData();
    void resetPreProcessors();

    void cut(int s, int e);

protected:
    void doSinglePointPreprocessing(QVector<double> *);

private:
    QMutex mutex_;
    TData * data_raw_ = nullptr;
    TData * data_single_processed_ = nullptr;
    TData * data_post_processed_ = nullptr;
    int id_;
    QDateTime creation_date_;
    QString source_name_;

    QList <IDataSinglePointProcessor *> single_point_processors_;
    QList <IDataPostProcessor *> post_processors_;

    static int total_traces_count_;

};

#endif // ABSTRACTTRACE_H
