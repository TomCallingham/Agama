/** \file   math_fit.h
    \brief  routines for fitting in one or many dimensions
    \date   2015
    \author Eugene Vasiliev
*/
#pragma once
#include "math_base.h"
#include "math_linalg.h"

namespace math{

/// \name ------ linear regression ------
///@{

/** perform a linear least-square fit (i.e., c * x + b = y).
    \param[in]  x  is the array of independent variables;
    \param[in]  y  is the array of dependent variables;
    \param[in]  w  is the optional array of weight coefficients (= inverse square error in y values),
                if set to NULL this means equal weights;
    \param[out] slope  stores the best-fit slope of linear regression;
    \param[out] intercept  stores the best-fit intercept (value at x=0);
    \param[out] rms  optionally stores the rms scatter (if not NULL).
*/
void linearFit(const std::vector<double>& x, const std::vector<double>& y, 
    const std::vector<double>* w, double& slope, double& intercept, double* rms=0);

/** perform a linear least-square fit without constant term (i.e., c * x = y).
    \param[in]  x  is the array of independent variables;
    \param[in]  y  is the array of dependent variables;
    \param[in]  w  is the optional array of weight coefficients (= inverse square error in y values),
                if set to NULL this means equal weights;
    \param[out] rms  optionally stores the rms scatter (if not NULL).
    \return  the best-fit slope of linear regression. 
*/
double linearFitZero(const std::vector<double>& x, const std::vector<double>& y, 
    const std::vector<double>* w, double* rms=0);

/** perform a multi-parameter linear least-square fit, i.e., solve the system of equations
    `X c = y`  in the least-square sense (using singular-value decomposition).
    \param[in]  coefs  is the matrix of coefficients (X) with M rows and N columns;
    \param[in]  rhs  is the the array of M values (y);
    \param[in]  w  is the optional array of weights (= inverse square error in y values), 
               if set to NULL this means equal weights;
    \param[out] result  stores the solution (array of N coefficients in the regression);
    \param[out] rms  optionally stores the rms scatter (if not NULL).
*/
void linearMultiFit(const Matrix<double>& coefs, const std::vector<double>& rhs, 
    const std::vector<double>* w, std::vector<double>& result, double* rms=0);

///@}
/// \name ------ multidimensional minimization -------
///@{

/** Prototype of a function of N>=1 variables that computes a vector of M>=1 values,
    and derivatives of these values w.r.t.the input variables (aka jacobian). */
class IFunctionNdimDeriv: public IFunctionNdim {
public:
    IFunctionNdimDeriv() {};
    virtual ~IFunctionNdimDeriv() {};

    /** evaluate the function and the derivatives.
        \param[in]  vars   is the N-dimensional point at which the function should be computed.
        \param[out] values is the M-dimensional array (possibly M=1) that will contain
                    the vector of function values.
        \param[out] derivs is the M-by-N matrix (M rows, N columns) of partial derivatives 
                    of the vector-valued function by the input variables;
                    if a NULL pointer is passed, this does not need to be computed,
                    otherwise the shape of matrix will be resized as needed
                    (i.e. one may pass a pointer to an empty matrix and it will be resized).
    */
    virtual void evalDeriv(const double vars[], double values[], Matrix<double>* derivs=0) const = 0;

    /** reimplement the evaluate function without derivatives */
    virtual void eval(const double vars[], double values[]) const {
        evalDeriv(vars, values);
    }
};

/** perform a multidimensional minimization of a function of N variables,
    using the Simplex algorithm of Nelder and Mead.
    \param[in]  F  is the function to be minimized (it may take N>=1 arguments 
    but must provide a single value);
    \param[in]  xinit  is the starting N-dimensional point for minimization;
    \param[in]  xstep  is the array of initial stepsizes in each of N dimensions;
    \param[in]  absToler  is the required tolerance on the location of minimum
    (average distance from the center of simplex to its corners):
    using a single control parameter demands that the input variables should be 
    scaled to comparable magnitudes;
    \param[in]  maxNumIter  is the upper limit on the number of iterations,
    the search is also terminated if (almost) no progress has been made for 
    10*N iterations;
    \param[out] result will contain the array of variables that minimize F.
    \returns  the number of iterations taken.
*/
int findMinNdim(const IFunctionNdim& F, const double xinit[], const double xstep[],
    const double absToler, const int maxNumIter, double result[]);

///@}
}  // namespace
