#include "abstracttrace.h"
#include <memory>
#include <QDebug>
#include "device/settings.h"
//======================================================================
//* Abstract trace
//* implements default obtaining and storage capabilities
//======================================================================
int AbstractTrace::total_traces_count_ = 0;

AbstractTrace::AbstractTrace(int var_count):
    id_(total_traces_count_),
    creation_date_(QDateTime::currentDateTime())
{
    qDebug() << "AbstractTrace ctr";
    data_raw_ = new TData();
    data_single_processed_ = new TData();
    data_post_processed_ = new TData();

    //new containers for values (var_cont == demensions)
    for (int i = 0; i < var_count; i++)
    {
        data_raw_->append(QVector<double>());
        data_single_processed_->append(QVector<double>());
        data_post_processed_->append(QVector<double>());
    }

    //init calibration settings
    CalibrationData();
    total_traces_count_ ++;
}

//======================================================================
AbstractTrace::~AbstractTrace()
{
    qDebug() << "Abstract trace "<< getSourceName() << "destructor";
    delete data_single_processed_;
    delete data_post_processed_;
    delete data_raw_;
}

//======================================================================
//* get mutexed ptr to data
//* this method will lock a procees if some thread already obtained this ptr and not freed it
//======================================================================
std::shared_ptr< TData > AbstractTrace::getLockedDataPtr()
{
    //i don't know how it works, some magick from stackowerflow.com
    auto counter = std::make_shared<QMutexLocker>(&mutex_);
    std::shared_ptr<TData> result{ counter, data_post_processed_ };
    return result;
}

//======================================================================
// * Cut a dataarray from s index to e index and reprocess data
//======================================================================
void AbstractTrace::cut(int start_index, int end_index)
{
    for (int i = 0; i < getDimensions(); i++)
    {
       (*data_raw_)[i] = data_raw_->at(i).mid(start_index, end_index-start_index);
    }
    reProcessOriginalData();
}

//======================================================================
int AbstractTrace::getDimensions()
{
    return data_post_processed_->length();
}

//======================================================================
//* append a single datapoint (x,y,z,...) to data array
//======================================================================
void AbstractTrace::writeData(QVector<double> &data)
{
    for (int i = 0; i < getDimensions(); i++)
    {
        data_raw_->operator[](i).push_back(data[i]);
    }

    doSinglePointPreprocessing(&data);

    for (int i = 0; i < getDimensions(); i++)
    {
        data_single_processed_->operator[](i).push_back(data[i]);
        getLockedDataPtr()->operator[](i).push_back(data[i]);
    }
}

//======================================================================
void AbstractTrace::reProcessOriginalData()
{
    //do this before cleare,  cause of getDimensions uses data_post_processed_ array for detection
    int dims = getDimensions();

    data_single_processed_->clear();
    data_post_processed_->clear();

    for (int i = 0; i < dims; i++)
    {
        data_single_processed_->append(QVector<double>());
        data_post_processed_->append(QVector<double>());
    }

    for (int i = 0; i < data_raw_->operator[](0).length(); i++)
    {
        QVector <double> data;
        for (int j = 0; j < dims; j++) data << data_raw_->operator[](j)[i];

        doSinglePointPreprocessing(&data);

        for (int j = 0; j < dims; j++)
        {
            data_single_processed_->operator[](j).push_back(data[j]);
            getLockedDataPtr()->operator[](j).push_back(data[j]);
        }
    }

    doPostProcessing();
}

//======================================================================
QDateTime AbstractTrace::getCreationDate()
{
    return creation_date_;
}

//======================================================================
QString AbstractTrace::getSourceName()
{
    return source_name_;
}

//======================================================================
void AbstractTrace::setSourceName(const QString &name)
{
    this->source_name_ = name;
}

//======================================================================
void AbstractTrace::doSinglePointPreprocessing(QVector<double> *data)
{
    for (auto &p: single_point_processors_)
    {
        p->doSinglePointConversion(data);
    }
}

//======================================================================
void AbstractTrace::doPostProcessing()
{

    auto data = getLockedDataPtr();
    for (auto &p: post_processors_)
    {
        qDebug() << "Do post-procesing with ["<<p->name() << "]";
        p->doPostProcessing(data_single_processed_, data.get());
    }

    if (post_processors_.length() == 0)
    {
        qDebug() << "Set original data with no postproc";
        std::copy(data_single_processed_->begin(), data_single_processed_->end(), data->begin());
    }
}

//======================================================================
void AbstractTrace::addSinglePointProcessor(IDataSinglePointProcessor *p)
{
    single_point_processors_.append(p);
}

//======================================================================
void AbstractTrace::addPostProcessor(IDataPostProcessor *p)
{
    qDebug() << "Added pproc: " << p->name() << post_processors_.size();
    post_processors_.append(p);
}

//======================================================================
void AbstractTrace::clearPreProcessors()
{
    qDebug() << "Clearing pre-processors for trace " << this->id();
    qDebug() << "There were" << single_point_processors_.size();
    for (auto s: single_point_processors_) delete s;
    single_point_processors_.clear();
}

//======================================================================
void AbstractTrace::resetPreProcessors()
{
    for (auto s: single_point_processors_) s->reset();
}

//======================================================================
void AbstractTrace::clearPostProcessors()
{
    qDebug() << "Clearing post-processors for trace " << this->id();
    qDebug() << "There were" << post_processors_.size();
    for (auto p: post_processors_) delete p;
    post_processors_.clear();
}
