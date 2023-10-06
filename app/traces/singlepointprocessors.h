#ifndef SINGLEPOINTPROCESSORS_H
#define SINGLEPOINTPROCESSORS_H

#include <QVector>

class IDataSinglePointProcessor {
public:
    virtual void doSinglePointConversion(QVector<double> *){};  //implement for preprocessing
    virtual void reset() {};
};

//==========================================================================================================
class YFromAnglePointProcessor : public IDataSinglePointProcessor
{
    public:
        YFromAnglePointProcessor();
        void doSinglePointConversion(QVector<double> *) override;
        void reset() override;

    private:
        double tool_len;
        double head_len;
        double head_radius;
        double zero_angle;
};

//==========================================================================================================
class MultiplicationProcessor : public IDataSinglePointProcessor
{
    public:
        MultiplicationProcessor(double mult, std::initializer_list<int> indexes) ;
        void doSinglePointConversion(QVector<double> *) override;
        void reset() override;

    private:
        double mult_;
        std::vector<int> indeces_;
};

//==========================================================================================================
class NormalisationProcessor : public IDataSinglePointProcessor
{
    public:
        NormalisationProcessor(double lower, double upper, std::initializer_list<int> indeces);
        void doSinglePointConversion(QVector<double> *) override;
        void reset() override;

    private:
        double lower_;
        double upper_;
        std::vector<int> indeces_;
};


//==========================================================================================================
#endif // SINGLEPOINTPROCESSORS_H
