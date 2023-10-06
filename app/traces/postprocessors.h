#ifndef POSTPROCESSORS_H
#define POSTPROCESSORS_H

#include "helpers/datatypes.h"

class IDataPostProcessor {
public:
    IDataPostProcessor (const QString &name);
    virtual void doPostProcessing(TData *src, TData *dst){};  //implement for preprocessing
    QString name() { return name_; }
    virtual ~IDataPostProcessor() {};
private:
    QString name_;
};

class DataMedianFilterProcessor: public IDataPostProcessor {
public:
    DataMedianFilterProcessor(int window);
    ~DataMedianFilterProcessor(){};
    void doPostProcessing(TData *src, TData *dst) override;  //implement for preprocessing
private:
    int window_;
};

class XZeronator: public IDataPostProcessor {
public:
    XZeronator();
    ~XZeronator(){};
    void doPostProcessing(TData *src, TData *dst) override;  //implement for preprocessing
private:
};


#endif // POSTPROCESSORS_H
