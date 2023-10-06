#include "postprocessors.h"
#include <QDebug>

//======================================================================
IDataPostProcessor::IDataPostProcessor(const QString &name)
{
    name_ = name;
}

//======================================================================
DataMedianFilterProcessor::DataMedianFilterProcessor(int window):IDataPostProcessor("median"+QString::number(window))
{
    window_ = window;
}

//======================================================================
void DataMedianFilterProcessor::doPostProcessing(TData *src, TData *dst)
{
    auto y = (*src)[1];

    for (int i = window_/2; i < y.size()-window_/2; i++)
    {
        auto v = y.mid(i-window_/2, window_);
        std::vector<double> h(v.size()/2+1);
        std::partial_sort_copy(v.begin(), v.end(), h.begin(), h.end());
        double median = h.back();

        (*dst)[1][i] = median;
    }
}
//======================================================================
XZeronator::XZeronator():IDataPostProcessor("xzeronator")
{

}
//======================================================================
void XZeronator::doPostProcessing(TData *src, TData *dst)
{

    auto x = (*src)[0];
    auto min = *std::min(x.begin(), x.end());

    for (int i = 0; i < x.size(); i++)
    {
        (*dst)[0][i] = x.at(i) - min;
    }
}
