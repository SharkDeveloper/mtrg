#include "polinomialregression.h"

#include <vector>
#include <stdlib.h>
#include <cmath>
#include <stdexcept>

#include "qdebug.h"

PolynomialRegression::PolynomialRegression() {};

bool PolynomialRegression::fitIt(
  QCPGraphDataContainer::const_iterator start,
  QCPGraphDataContainer::const_iterator end,
  const int &             order,
  QVector<double> &     coeffs)
{
    // The size of xValues and yValues should be same

    int n = order;
    int np1 = n + 1;
    int np2 = n + 2;
    int tnp1 = 2 * n + 1;
    double tmp;

    // X = vector that stores values of sigma(xi^2n)
    std::vector<double> X(tnp1);
    for (int i = 0; i < tnp1; ++i) {
      X[i] = 0;
      for (auto j = start; j != end; ++j){
        X[i] += (double)pow(j->key, i);
      }
    }

    // a = vector to store final coefficients.
    std::vector<double> a(np1);

    // B = normal augmented matrix that stores the equations.
    std::vector<std::vector<double> > B(np1, std::vector<double> (np2, 0));

    for (int i = 0; i <= n; ++i)
      for (int j = 0; j <= n; ++j)
        B[i][j] = X[i + j];

    // Y = vector to store values of sigma(xi^n * yi)
    std::vector<double> Y(np1);
    for (int i = 0; i < np1; ++i) {
      Y[i] = (double)0;
      for (auto j = start; j != end; ++j) {
        Y[i] += (double)pow(j->key, i)*j->value;
      }
    }

    // Load values of Y as last column of B
    for (int i = 0; i <= n; ++i)
      B[i][np1] = Y[i];

    n += 1;
    int nm1 = n-1;

    // Pivotisation of the B matrix.
    for (int i = 0; i < n; ++i)
      for (int k = i+1; k < n; ++k)
        if (B[i][i] < B[k][i])
          for (int j = 0; j <= n; ++j) {
            tmp = B[i][j];
            B[i][j] = B[k][j];
            B[k][j] = tmp;
          }

    // Performs the Gaussian elimination.
    // (1) Make all elements below the pivot equals to zero
    //     or eliminate the variable.
    for (int i=0; i<nm1; ++i)
      for (int k =i+1; k<n; ++k) {
        double t = B[k][i] / B[i][i];
        for (int j=0; j<=n; ++j)
          B[k][j] -= t*B[i][j];         // (1)
      }

    // Back substitution.
    // (1) Set the variable as the rhs of last equation
    // (2) Subtract all lhs values except the target coefficient.
    // (3) Divide rhs by coefficient of variable being calculated.
    for (int i=nm1; i >= 0; --i) {
      a[i] = B[i][n];                   // (1)
      for (int j = 0; j<n; ++j)
        if (j != i)
          a[i] -= B[i][j] * a[j];       // (2)
      a[i] /= B[i][i];                  // (3)
    }

    coeffs.resize(a.size());
    for (size_t i = 0; i < a.size(); ++i)
      coeffs[i] = a[i];

    return true;
}

bool PolynomialRegression::fitIt(
  const QCPCurveData * start,
  const QCPCurveData * end,
  const int &             order,
  QVector<double> &     coeffs)
{
    // The size of xValues and yValues should be same

    int n = order;
    int np1 = n + 1;
    int np2 = n + 2;
    int tnp1 = 2 * n + 1;
    double tmp;

    // X = vector that stores values of sigma(xi^2n)
    std::vector<double> X(tnp1);
    for (int i = 0; i < tnp1; ++i) {
      X[i] = 0;
      for (auto j = start; j != end; ++j){
        X[i] += (double)pow(j->key, i);
      }
    }

    // a = vector to store final coefficients.
    std::vector<double> a(np1);

    // B = normal augmented matrix that stores the equations.
    std::vector<std::vector<double> > B(np1, std::vector<double> (np2, 0));

    for (int i = 0; i <= n; ++i)
      for (int j = 0; j <= n; ++j)
        B[i][j] = X[i + j];

    // Y = vector to store values of sigma(xi^n * yi)
    std::vector<double> Y(np1);
    for (int i = 0; i < np1; ++i) {
      Y[i] = (double)0;
      for (auto j = start; j != end; ++j) {
        Y[i] += (double)pow(j->key, i)*j->value;
      }
    }

    // Load values of Y as last column of B
    for (int i = 0; i <= n; ++i)
      B[i][np1] = Y[i];

    n += 1;
    int nm1 = n-1;

    // Pivotisation of the B matrix.
    for (int i = 0; i < n; ++i)
      for (int k = i+1; k < n; ++k)
        if (B[i][i] < B[k][i])
          for (int j = 0; j <= n; ++j) {
            tmp = B[i][j];
            B[i][j] = B[k][j];
            B[k][j] = tmp;
          }

    // Performs the Gaussian elimination.
    // (1) Make all elements below the pivot equals to zero
    //     or eliminate the variable.
    for (int i=0; i<nm1; ++i)
      for (int k =i+1; k<n; ++k) {
        double t = B[k][i] / B[i][i];
        for (int j=0; j<=n; ++j)
          B[k][j] -= t*B[i][j];         // (1)
      }

    // Back substitution.
    // (1) Set the variable as the rhs of last equation
    // (2) Subtract all lhs values except the target coefficient.
    // (3) Divide rhs by coefficient of variable being calculated.
    for (int i=nm1; i >= 0; --i) {
      a[i] = B[i][n];                   // (1)
      for (int j = 0; j<n; ++j)
        if (j != i)
          a[i] -= B[i][j] * a[j];       // (2)
      a[i] /= B[i][i];                  // (3)
    }

    coeffs.resize(a.size());
    for (size_t i = 0; i < a.size(); ++i)
      coeffs[i] = a[i];

    return true;
}


bool PolynomialRegression::fitIt(
  const QVector<double> & x,
  const QVector<double> & y,
  const int &               order,
  QVector<double> &       coeffs)
{
  // The size of xValues and yValues should be same
  if (x.size() != y.size()) {
    throw std::runtime_error( "The size of x & y arrays are different" );
    return false;
  }
  // The size of xValues and yValues cannot be 0, should not happen
  if (x.size() == 0 || y.size() == 0) {
    //throw std::runtime_error( "The size of x or y arrays is 0" );
    return false;
  }

  size_t N = x.size();
  int n = order;
  int np1 = n + 1;
  int np2 = n + 2;
  int tnp1 = 2 * n + 1;
  double tmp;

  // X = vector that stores values of sigma(xi^2n)
  std::vector<double> X(tnp1);
  for (int i = 0; i < tnp1; ++i) {
    X[i] = 0;
    for (int j = 0; j < N; ++j)
      X[i] += (double)pow(x[j], i);
  }

  // a = vector to store final coefficients.
  std::vector<double> a(np1);

  // B = normal augmented matrix that stores the equations.
  std::vector<std::vector<double> > B(np1, std::vector<double> (np2, 0));

  for (int i = 0; i <= n; ++i)
    for (int j = 0; j <= n; ++j)
      B[i][j] = X[i + j];

  // Y = vector to store values of sigma(xi^n * yi)
  std::vector<double> Y(np1);
  for (int i = 0; i < np1; ++i) {
    Y[i] = (double)0;
    for (int j = 0; j < N; ++j) {
      Y[i] += (double)pow(x[j], i)*y[j];
    }
  }

  // Load values of Y as last column of B
  for (int i = 0; i <= n; ++i)
    B[i][np1] = Y[i];

  n += 1;
  int nm1 = n-1;

  // Pivotisation of the B matrix.
  for (int i = 0; i < n; ++i)
    for (int k = i+1; k < n; ++k)
      if (B[i][i] < B[k][i])
        for (int j = 0; j <= n; ++j) {
          tmp = B[i][j];
          B[i][j] = B[k][j];
          B[k][j] = tmp;
        }

  // Performs the Gaussian elimination.
  // (1) Make all elements below the pivot equals to zero
  //     or eliminate the variable.
  for (int i=0; i<nm1; ++i)
    for (int k =i+1; k<n; ++k) {
      double t = B[k][i] / B[i][i];
      for (int j=0; j<=n; ++j)
        B[k][j] -= t*B[i][j];         // (1)
    }

  // Back substitution.
  // (1) Set the variable as the rhs of last equation
  // (2) Subtract all lhs values except the target coefficient.
  // (3) Divide rhs by coefficient of variable being calculated.
  for (int i=nm1; i >= 0; --i) {
    a[i] = B[i][n];                   // (1)
    for (int j = 0; j<n; ++j)
      if (j != i)
        a[i] -= B[i][j] * a[j];       // (2)
    a[i] /= B[i][i];                  // (3)
  }

  coeffs.resize(a.size());
  for (size_t i = 0; i < a.size(); ++i)
    coeffs[i] = a[i];

  return true;
}
