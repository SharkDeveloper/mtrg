#include "algos.h"
#include "polinomialregression.h"
#include <math.h>
#include <nlopt.hpp>


Algos::Algos(){}
Algos::CoeffFitter::CoeffFitter(const QVector<double> &coeffs) : coeffs_(coeffs){}
Algos::CoeffFitter::CoeffFitter(){}
bool Algos::CoeffFitter::isValid() {return coeffs_.length() != 0; };
double Algos::CoeffFitter::coef(int pos) const {return coeffs_[pos];} ;
Algos::RMS::RMS(const QVector<double> &coefs) : CoeffFitter(coefs) {};
Algos::RMS::RMS() {};
Algos::LSQ::LSQ(const QVector<double> &coefs) : CoeffFitter(coefs) {};
Algos::LSQ::LSQ() {};

double Algos::RMS::fit(double val) {

    return coeffs_[1]*val + coeffs_[0];
};

double Algos::LSQ::fit(double val) {

    return coeffs_[1]*val + coeffs_[0];
};


Algos::RMS Algos::linearRMS( const QCPCurveData * start,
                      const QCPCurveData * end)
{
    double sumx = 0;
    double sumy = 0;
    double sumx2 = 0;
    double sumxy = 0;
    int n = 0;
    for (auto i = start; i != end; i++)
    {
        sumx += i->key;
        sumy += i->value;
        sumx2 += i->key * i->key;
        sumxy += i->key * i->value;
        n++;
    }

    QVector<double> coeffs;

    coeffs.resize(2);
    coeffs[1] = (n*sumxy - (sumx*sumy)) / (n*sumx2 - sumx*sumx);
    coeffs[0] = (sumy - coeffs[1]*sumx) / n;

    return RMS(coeffs);
}

Algos::RMS Algos::linearRMS( QCPGraphDataContainer::const_iterator start,
                       QCPGraphDataContainer::const_iterator end)
{    
    double sumx = 0;
    double sumy = 0;
    double sumx2 = 0;
    double sumxy = 0;
    int n = 0;
    for (auto i = start; i != end; i++)
    {
        sumx += i->key;
        sumy += i->value;
        sumx2 += i->key * i->key;
        sumxy += i->key * i->value;
        n++;
    }

    QVector<double> coeffs;

    coeffs.resize(2);
    coeffs[1] = (n*sumxy - (sumx*sumy)) / (n*sumx2 - sumx*sumx);
    coeffs[0] = (sumy - coeffs[1]*sumx) / n;

    return RMS(coeffs);
}

Algos::LSQ Algos::linearLSQ( QCPGraphDataContainer::const_iterator start,
                       QCPGraphDataContainer::const_iterator end)
{

    QVector<double> coeffs;
    PolynomialRegression().fitIt(start, end, 2, coeffs);
    return LSQ(coeffs);
}

Algos::LSQ Algos::linearLSQ(const QCPCurveData * start,
                       const QCPCurveData * end)
{

    QVector<double> coeffs;
    PolynomialRegression().fitIt(start, end, 2, coeffs);
    return LSQ(coeffs);
}

void Algos::hyperfitCircle(  QCPGraphDataContainer::const_iterator start,
                             QCPGraphDataContainer::const_iterator end,
                             QPointF &center, double &rad)
{
    /*
          Circle fit to a given set of data points (in 2D)

          This is an algebraic fit based on the journal article

          A. Al-Sharadqah and N. Chernov, "Error analysis for circle fitting algorithms",
          Electronic Journal of Statistics, Vol. 3, pages 886-911, (2009)

          It is an algebraic circle fit with "hyperaccuracy" (with zero essential bias).
          The term "hyperaccuracy" first appeared in papers by Kenichi Kanatani around 2006


     data.n   - the number of data points
     data.X[] - the array of X-coordinates
     data.Y[] - the array of Y-coordinates

        Output:
                   circle - parameters of the fitting circle:

               circle.a - the X-coordinate of the center of the fitting circle
               circle.b - the Y-coordinate of the center of the fitting circle
               circle.r - the radius of the fitting circle
               circle.s - the root mean square error (the estimate of sigma)
               circle.j - the total number of iterations
     */



        int i, iter, IterMAX=99;

        double Xi,Yi,Zi;
        double Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
        double A0,A1,A2,A22;
        double Dy,xnew,x,ynew,y;
        double DET,Xcenter,Ycenter;


        // Compute x- and y- sample means
        double meanX=0., meanY=0.;
        int n = 0;
        for (auto i = start; i != end; i++)
        {
            meanX += i->mainKey();
            meanY += i->mainValue();
            n++;
        }

        meanX /= n;
        meanY /= n;

    //     computing moments

        Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

        for (auto i = start; i != end; i++)
        {
            Xi = i->mainKey() - meanX;   //  centered x-coordinates
            Yi = i->mainValue() - meanY;   //  centered y-coordinates
            Zi = Xi*Xi + Yi*Yi;

            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
        }

        Mxx /= n;
        Myy /= n;
        Mxy /= n;
        Mxz /= n;
        Myz /= n;
        Mzz /= n;

    //    computing the coefficients of the characteristic polynomial

        Mz = Mxx + Myy;
        Cov_xy = Mxx*Myy - Mxy*Mxy;
        Var_z = Mzz - Mz*Mz;

        A2 = 4*Cov_xy - 3*Mz*Mz - Mzz;
        A1 = Var_z*Mz + 4*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
        A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
        A22 = A2 + A2;

    //    finding the root of the characteristic polynomial
    //    using Newton's method starting at x=0
    //     (it is guaranteed to converge to the right root)

        for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
        {
            Dy = A1 + x*(A22 + 16.*x*x);
            xnew = x - y/Dy;
            if ((xnew == x)||(!std::isfinite(xnew))) break;
            ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
            if (fabs(ynew)>=fabs(y))  break;
            x = xnew;  y = ynew;
        }

    //    computing paramters of the fitting circle

        DET = x*x - x*Mz + Cov_xy;
        Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2.0;
        Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2.0;

    //       assembling the output

        center.setX(Xcenter + meanX);
        center.setY(Ycenter + meanY);
        rad = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz - x - x);
}

void Algos::hyperfitCircle( const  QCPCurveData* start,
                            const QCPCurveData* end,
                             QPointF &center, double &rad)
{
    /*
          Circle fit to a given set of data points (in 2D)

          This is an algebraic fit based on the journal article

          A. Al-Sharadqah and N. Chernov, "Error analysis for circle fitting algorithms",
          Electronic Journal of Statistics, Vol. 3, pages 886-911, (2009)

          It is an algebraic circle fit with "hyperaccuracy" (with zero essential bias).
          The term "hyperaccuracy" first appeared in papers by Kenichi Kanatani around 2006


     data.n   - the number of data points
     data.X[] - the array of X-coordinates
     data.Y[] - the array of Y-coordinates

        Output:
                   circle - parameters of the fitting circle:

               circle.a - the X-coordinate of the center of the fitting circle
               circle.b - the Y-coordinate of the center of the fitting circle
               circle.r - the radius of the fitting circle
               circle.s - the root mean square error (the estimate of sigma)
               circle.j - the total number of iterations
     */



        int i, iter, IterMAX=99;

        double Xi,Yi,Zi;
        double Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
        double A0,A1,A2,A22;
        double Dy,xnew,x,ynew,y;
        double DET,Xcenter,Ycenter;


        // Compute x- and y- sample means
        double meanX=0., meanY=0.;
        int n = 0;
        for (auto i = start; i != end; i++)
        {
            meanX += i->mainKey();
            meanY += i->mainValue();
            n++;
        }

        meanX /= n;
        meanY /= n;

    //     computing moments

        Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

        for (auto i = start; i != end; i++)
        {
            Xi = i->mainKey() - meanX;   //  centered x-coordinates
            Yi = i->mainValue() - meanY;   //  centered y-coordinates
            Zi = Xi*Xi + Yi*Yi;

            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
        }

        Mxx /= n;
        Myy /= n;
        Mxy /= n;
        Mxz /= n;
        Myz /= n;
        Mzz /= n;

    //    computing the coefficients of the characteristic polynomial

        Mz = Mxx + Myy;
        Cov_xy = Mxx*Myy - Mxy*Mxy;
        Var_z = Mzz - Mz*Mz;

        A2 = 4*Cov_xy - 3*Mz*Mz - Mzz;
        A1 = Var_z*Mz + 4*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
        A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
        A22 = A2 + A2;

    //    finding the root of the characteristic polynomial
    //    using Newton's method starting at x=0
    //     (it is guaranteed to converge to the right root)

        for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
        {
            Dy = A1 + x*(A22 + 16.*x*x);
            xnew = x - y/Dy;
            if ((xnew == x)||(!std::isfinite(xnew))) break;
            ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
            if (fabs(ynew)>=fabs(y))  break;
            x = xnew;  y = ynew;
        }

    //    computing paramters of the fitting circle

        DET = x*x - x*Mz + Cov_xy;
        Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2.0;
        Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2.0;

    //       assembling the output

        center.setX(Xcenter + meanX);
        center.setY(Ycenter + meanY);
        rad = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz - x - x);
}


void Algos::hyperfitCircle2Arcs(QCPGraphDataContainer::const_iterator start1,
                                QCPGraphDataContainer::const_iterator end1,
                                QCPGraphDataContainer::const_iterator start2,
                                QCPGraphDataContainer::const_iterator end2,
                                QPointF &center, double &rad)
{
    /*
          Circle fit to a given set of data points (in 2D)

          This is an algebraic fit based on the journal article

          A. Al-Sharadqah and N. Chernov, "Error analysis for circle fitting algorithms",
          Electronic Journal of Statistics, Vol. 3, pages 886-911, (2009)

          It is an algebraic circle fit with "hyperaccuracy" (with zero essential bias).
          The term "hyperaccuracy" first appeared in papers by Kenichi Kanatani around 2006


     data.n   - the number of data points
     data.X[] - the array of X-coordinates
     data.Y[] - the array of Y-coordinates

        Output:
                   circle - parameters of the fitting circle:

               circle.a - the X-coordinate of the center of the fitting circle
               circle.b - the Y-coordinate of the center of the fitting circle
               circle.r - the radius of the fitting circle
               circle.s - the root mean square error (the estimate of sigma)
               circle.j - the total number of iterations
     */



        int i, iter, IterMAX=99;

        double Xi,Yi,Zi;
        double Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
        double A0,A1,A2,A22;
        double Dy,xnew,x,ynew,y;
        double DET,Xcenter,Ycenter;


        // Compute x- and y- sample means
        double meanX=0., meanY=0.;
        int n = 0;
        for (auto i = start1; i != end1; i++)
        {
            meanX += i->mainKey();
            meanY += i->mainValue();
            n++;
        }

        for (auto i = start2; i != end2; i++)
        {
            meanX += i->mainKey();
            meanY += i->mainValue();
            n++;
        }

        meanX /= n;
        meanY /= n;

    //     computing moments

        Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

        for (auto i = start1; i != end1; i++)
        {
            Xi = i->mainKey() - meanX;   //  centered x-coordinates
            Yi = i->mainValue() - meanY;   //  centered y-coordinates
            Zi = Xi*Xi + Yi*Yi;

            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
        }

        for (auto i = start2; i != end2; i++)
        {
            Xi = i->mainKey() - meanX;   //  centered x-coordinates
            Yi = i->mainValue() - meanY;   //  centered y-coordinates
            Zi = Xi*Xi + Yi*Yi;

            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
        }

        Mxx /= n;
        Myy /= n;
        Mxy /= n;
        Mxz /= n;
        Myz /= n;
        Mzz /= n;

    //    computing the coefficients of the characteristic polynomial

        Mz = Mxx + Myy;
        Cov_xy = Mxx*Myy - Mxy*Mxy;
        Var_z = Mzz - Mz*Mz;

        A2 = 4*Cov_xy - 3*Mz*Mz - Mzz;
        A1 = Var_z*Mz + 4*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
        A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
        A22 = A2 + A2;

    //    finding the root of the characteristic polynomial
    //    using Newton's method starting at x=0
    //     (it is guaranteed to converge to the right root)

        for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
        {
            Dy = A1 + x*(A22 + 16.*x*x);
            xnew = x - y/Dy;
            if ((xnew == x)||(!std::isfinite(xnew))) break;
            ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
            if (fabs(ynew)>=fabs(y))  break;
            x = xnew;  y = ynew;
        }

    //    computing paramters of the fitting circle

        DET = x*x - x*Mz + Cov_xy;
        Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2.0;
        Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2.0;

    //       assembling the output

        center.setX(Xcenter + meanX);
        center.setY(Ycenter + meanY);
        rad = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz - x - x);
}


void Algos::hyperfitCircle2Arcs(const QCPCurveData * start1,
                                const QCPCurveData * end1,
                                const QCPCurveData * start2,
                                const QCPCurveData * end2,
                                QPointF &center, double &rad)
{
    /*
          Circle fit to a given set of data points (in 2D)

          This is an algebraic fit based on the journal article

          A. Al-Sharadqah and N. Chernov, "Error analysis for circle fitting algorithms",
          Electronic Journal of Statistics, Vol. 3, pages 886-911, (2009)

          It is an algebraic circle fit with "hyperaccuracy" (with zero essential bias).
          The term "hyperaccuracy" first appeared in papers by Kenichi Kanatani around 2006


     data.n   - the number of data points
     data.X[] - the array of X-coordinates
     data.Y[] - the array of Y-coordinates

        Output:
                   circle - parameters of the fitting circle:

               circle.a - the X-coordinate of the center of the fitting circle
               circle.b - the Y-coordinate of the center of the fitting circle
               circle.r - the radius of the fitting circle
               circle.s - the root mean square error (the estimate of sigma)
               circle.j - the total number of iterations
     */



        int i, iter, IterMAX=99;

        double Xi,Yi,Zi;
        double Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
        double A0,A1,A2,A22;
        double Dy,xnew,x,ynew,y;
        double DET,Xcenter,Ycenter;


        // Compute x- and y- sample means
        double meanX=0., meanY=0.;
        int n = 0;
        for (auto i = start1; i != end1; i++)
        {
            meanX += i->mainKey();
            meanY += i->mainValue();
            n++;
        }

        for (auto i = start2; i != end2; i++)
        {
            meanX += i->mainKey();
            meanY += i->mainValue();
            n++;
        }

        meanX /= n;
        meanY /= n;

    //     computing moments

        Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

        for (auto i = start1; i != end1; i++)
        {
            Xi = i->mainKey() - meanX;   //  centered x-coordinates
            Yi = i->mainValue() - meanY;   //  centered y-coordinates
            Zi = Xi*Xi + Yi*Yi;

            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
        }

        for (auto i = start2; i != end2; i++)
        {
            Xi = i->mainKey() - meanX;   //  centered x-coordinates
            Yi = i->mainValue() - meanY;   //  centered y-coordinates
            Zi = Xi*Xi + Yi*Yi;

            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
        }

        Mxx /= n;
        Myy /= n;
        Mxy /= n;
        Mxz /= n;
        Myz /= n;
        Mzz /= n;

    //    computing the coefficients of the characteristic polynomial

        Mz = Mxx + Myy;
        Cov_xy = Mxx*Myy - Mxy*Mxy;
        Var_z = Mzz - Mz*Mz;

        A2 = 4*Cov_xy - 3*Mz*Mz - Mzz;
        A1 = Var_z*Mz + 4*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
        A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
        A22 = A2 + A2;

    //    finding the root of the characteristic polynomial
    //    using Newton's method starting at x=0
    //     (it is guaranteed to converge to the right root)

        for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
        {
            Dy = A1 + x*(A22 + 16.*x*x);
            xnew = x - y/Dy;
            if ((xnew == x)||(!std::isfinite(xnew))) break;
            ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
            if (fabs(ynew)>=fabs(y))  break;
            x = xnew;  y = ynew;
        }

    //    computing paramters of the fitting circle

        DET = x*x - x*Mz + Cov_xy;
        Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2.0;
        Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2.0;

    //       assembling the output

        center.setX(Xcenter + meanX);
        center.setY(Ycenter + meanY);
        rad = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz - x - x);
}


//minimum optimisation test
double myfunc(unsigned n, const double *x, double *grad, void *my_func_data)
{
    if (grad) {
        grad[0] = 0.0;
        grad[1] = 0.5 / sqrt(x[1]);
    }
    return sqrt(x[1]);
}

typedef struct {
    double a, b;
} my_constraint_data;

double myconstraint(unsigned n, const double *x, double *grad, void *data)
{
    my_constraint_data *d = (my_constraint_data *) data;
    double a = d->a, b = d->b;
    if (grad) {
        grad[0] = 3 * a * (a*x[0] + b) * (a*x[0] + b);
        grad[1] = -1.0;
    }
    return ((a*x[0] + b) * (a*x[0] + b) * (a*x[0] + b) - x[1]);
 }

void testnlopt()
{
    double lb[2] = { -HUGE_VAL, 0 }; /* lower bounds */
    nlopt_opt opt;
    opt = nlopt_create(NLOPT_LD_MMA, 2); /* algorithm and dimensionality */
    nlopt_set_lower_bounds(opt, lb);
    nlopt_set_min_objective(opt, myfunc, NULL);
    my_constraint_data data[2] = { {2,0}, {-1,1} };
    nlopt_add_inequality_constraint(opt, myconstraint, &data[0], 1e-8);
    nlopt_add_inequality_constraint(opt, myconstraint, &data[1], 1e-8);
    nlopt_set_xtol_rel(opt, 1e-4);
    double x[2] = { 1.234, 5.678 };  /* `*`some` `initial` `guess`*` */
    double minf; /* `*`the` `minimum` `objective` `value,` `upon` `return`*` */
    if (nlopt_optimize(opt, x, &minf) < 0) {
        printf("nlopt failed!\n");
    }
    else {
        printf("found minimum at f(%g,%g) = %0.10g\n", x[0], x[1], minf);
    }
    nlopt_destroy(opt);
}


