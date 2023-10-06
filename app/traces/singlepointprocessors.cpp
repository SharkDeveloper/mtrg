#include "singlepointprocessors.h"
#include "device/settings.h"
#include <QtMath>

//==========================================================================================================
YFromAnglePointProcessor::YFromAnglePointProcessor()
{
    reset();
};

//==========================================================================================================
void YFromAnglePointProcessor::reset()
{
    tool_len = Settings::instance().calibration.tool_length_nm;
    head_len = Settings::instance().calibration.tool_head_lenght_nm;
    head_radius = Settings::instance().calibration.tool_head_radius_nm;
    zero_angle = qDegreesToRadians(Settings::instance().calibration.tool_zero_angle_deg);
}


//==========================================================================================================
void YFromAnglePointProcessor::doSinglePointConversion(QVector<double> *data)
{
    //data: 0 - x, 1 - y, 2 - z (angle)
    //calibration reinited in ctr of abstractrace
    (*data)[1] = tool_len * sin((*data)[2] - zero_angle);
    (*data)[0] = (*data)[0] + tool_len * (1.0 - cos((*data)[2] - zero_angle));
}


//==========================================================================================================
MultiplicationProcessor::MultiplicationProcessor(double mult, std::initializer_list<int> indeces)
{
    mult_ = mult;
    indeces_ = indeces;
}

//==========================================================================================================
void MultiplicationProcessor::reset()
{

}

//==========================================================================================================
void MultiplicationProcessor::doSinglePointConversion(QVector<double> *data)
{
    for (auto i: indeces_)
    {
        (*data)[i] *= mult_;
    }
}



//==========================================================================================================
NormalisationProcessor::NormalisationProcessor(double lower, double upper, std::initializer_list<int> indeces)
{
    lower_ = lower;
    upper_ = upper;
    indeces_ = indeces;
}

//==========================================================================================================
void NormalisationProcessor::reset()
{

}

//==========================================================================================================
void NormalisationProcessor::doSinglePointConversion(QVector<double> *data)
{
    for (auto i: indeces_)
    {
        (*data)[i] = (((*data)[i]) / 65535.0)/100000 ;//to mkm?;
    }
}

