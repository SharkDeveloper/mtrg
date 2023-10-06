#ifndef ALGOS_H
#define ALGOS_H

#include "../qcustomplot/qcustomplot.h"
#include <iterator>
class Algos
{
public:
    Algos();

    class CoeffFitter {
    public:
        CoeffFitter(const QVector<double> &);
        CoeffFitter();
        virtual double fit(double val) = 0;
        bool isValid();
        double coef(int pos) const ;

        protected:
        QVector<double>    coeffs_;
    };

    class RMS : public CoeffFitter {
    public:
        RMS(const QVector<double> &);
        RMS();
        double fit(double val) override;
    };

    class LSQ : public CoeffFitter{
    public:
        LSQ(const QVector<double> &);
        LSQ();
        double fit(double val) override;
    };

    static RMS linearRMS( QCPGraphDataContainer::const_iterator start,
                           QCPGraphDataContainer::const_iterator end);
    static RMS linearRMS( const QCPCurveData* start,
                          const QCPCurveData *end);

    static LSQ linearLSQ( QCPGraphDataContainer::const_iterator start,
                           QCPGraphDataContainer::const_iterator end);

    static LSQ linearLSQ( const QCPCurveData* start,
                          const QCPCurveData *end);


    static void hyperfitCircle( QCPGraphDataContainer::const_iterator start,
                                QCPGraphDataContainer::const_iterator end, QPointF &center, double &rad);
    static void hyperfitCircle(const QCPCurveData * start,
                               const QCPCurveData * end, QPointF &center, double &rad);


    static void hyperfitCircle2Arcs( QCPGraphDataContainer::const_iterator start1,
                                QCPGraphDataContainer::const_iterator end1,
                                     QCPGraphDataContainer::const_iterator start2,
                                                                     QCPGraphDataContainer::const_iterator end2, QPointF &center, double &rad);


    static void hyperfitCircle2Arcs( const QCPCurveData *start1,
                                const QCPCurveData * end1,
                                     const QCPCurveData * start2,
                                                                     const QCPCurveData * end2, QPointF &center, double &rad);
};


void testnlopt();
#endif // ALGOS_H
