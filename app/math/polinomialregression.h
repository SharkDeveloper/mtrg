#ifndef POLINOMIALREGRESSION_H
#define POLINOMIALREGRESSION_H

#include <QVector>
#include "../qcustomplot/qcustomplot.h"

class PolynomialRegression {
  public:

    PolynomialRegression();
    virtual ~PolynomialRegression(){};

    bool fitIt(
      const QVector<double> & x,
      const QVector<double> & y,
      const int &             order,
      QVector<double> &     coeffs);


    bool fitIt(
      QCPGraphDataContainer::const_iterator start,
      QCPGraphDataContainer::const_iterator end,
      const int &             order,
      QVector<double> &     coeffs);

    bool fitIt(
      const QCPCurveData* start,
      const QCPCurveData * end,
      const int &             order,
      QVector<double> &     coeffs);
};


#endif // POLINOMIALREGRESSION_H
