/** \file galPot.h
    \brief Walter Dehnen's GalaxyPotential code

Copyright Walter Dehnen, 1996-2005 
e-mail:   walter.dehnen@astro.le.ac.uk 
address:  Department of Physics and Astronomy, University of Leicester 
          University Road, Leicester LE1 7RH, United Kingdom 

Put into the Torus code (with a minimum of fuss) by Paul McMillan, Oxford 2010
email: p.mcmillan1@physics.ox.ac.uk

Modifications by Eugene Vasiliev, June 2015


The method, explained in Dehnen & Binney (1998, MNRAS, 294, 429) and based 
on the approach of Kuijken & Dubinski (1994, MNRAS, 269, 13), is applicable 
to any disk density profile which is separable in cylindrical coordinates.

Let the density profile of the disk be

\f$  \rho_d(R,z) = f(R) h(z)  \f$,  

and let H(z) be the second integral of h(z) over z.
Then the potential of the disk can be written as a sum of 'main' and 'residual' parts:

\f$  \Phi(R,z) = 4\pi f(r) H(z) + \Phi_{res}  \f$,

where the argument of f is spherical rather than cylindrical radius, 
and the residual potential is generated by the following density profile:

\f$  \rho_{res} = [f(R)-f(r)] h(z) - f''(r) H(z) - 2 f'(r) [H(z) + z H'(z)]/r  \f$.

This residual potential is not strongly confined to the disk plane, and can be 
efficiently approximated by a multipole expanion, which, in turn, is represented 
by a two-dimensional quintic spline in (R,z) plane.

The original GalaxyPotential uses this method for any combination of disk components 
and additional, possibly flattened spheroidal components: the residual density of all 
disks and the entire density of spheroids serves as the source to the Multipole potential 
approximation.

In the present modification, the GalaxyPotential class is replaced by a more generic Composite 
potential, which contains one Multipole potential and possibly several DiskAnsatz components.
The latter come in pairs with DiskResidual density components, so that the sum of densities 
in each pair equals the input density profile of that disk model. 
A composite density model with all DiskResidual and all SpheroidDensity components is used 
to initialize the Multipole potential. Of course this input may be generalized to contain 
other density components, and the Composite potential may also contain some other potential 
models apart from DiskAnsatz and Multipole. 

For compatibility with the original implementation, an utility function `createGalaxyPotential`
is provided, which takes the name of parameter file and the Units object as parameters.
*/

#pragma once
#include "potential_base.h"
#include <vector>

namespace potential{

/// parameters that describe a disk component
struct DiskParam{
    double surfaceDensity;      ///< surface density normalisation Sigma_0 [Msun/kpc^2]
    double scaleLength;         ///< scale length R_d [kpc]
    double scaleHeight;         ///< scale height h [kpc]: 
    ///< For h<0 an isothermal (sech^2) profile is used, for h>0 an exponential one, 
    ///< and for h=0 the disk is infinitesimal thin
    double innerCutoffRadius;   ///< if nonzero, specifies the radius of a hole at the center R_0
    double modulationAmplitude; ///< a term eps*cos(R/R_d) is added to the exponent
};

/// parameters describing a spheroidal component
struct SphrParam{
    double densityNorm;         ///< density normalization rho_0 [Msun/kpc^3] 
    double axisRatio;           ///< axis ratio q (z/R)
    double gamma;               ///< inner power slope gamma 
    double beta;                ///< outer power slope beta 
    double scaleRadius;         ///< transition radius r_0 [kpc] 
    double outerCutoffRadius;   ///< outer cut-off radius r_t [kpc] 
};

/** Specification of a disk density profile separable in R and z requires two auxiliary function,
    f(R) and H(z)  (the former essentially describes the surface density of the disk,
    and the latter is the second antiderivative of vertical density profile h(z) ).
    They are used by both DiskAnsatz potential and DiskResidual density classes.
    In the present implementation they are the same as in GalPot:

    \f$  f(R) = \Sigma_0  \exp [ -R_0/R - R/R_d + \epsilon \cos(R/R_d) ]  \f$,

    \f$  h(z) = \delta(z)                 \f$  for  h=0, or 
    \f$  h(z) = 1/(2 h)  * exp(-|z/h|)    \f$  for  h>0, or
    \f$  h(z) = 1/(4|h|) * sech^2(|z/2h|) \f$  for  h<0.

    The corresponding second antiderivatives of h(z) are given in Table 2 of Dehnen&Binney 1998.
*/

/** helper routine to create an instance of radial density function */
const math::IFunction* createRadialDiskFnc(const DiskParam& params);

/** helper routine to create an instance of vertical density function */
const math::IFunction* createVerticalDiskFnc(const DiskParam& params);

/** Residual density profile of a disk component (eq.9 in Dehnen&Binney 1998) */
class DiskResidual: public BaseDensity {
public:
    DiskResidual (const DiskParam& params) : 
        BaseDensity(), 
        radial_fnc  (createRadialDiskFnc(params)),
        vertical_fnc(createVerticalDiskFnc(params)) {};
    ~DiskResidual() { delete radial_fnc; delete vertical_fnc; }
    virtual SymmetryType symmetry() const { return ST_AXISYMMETRIC; }
    virtual const char* name() const { return myName(); };
    static const char* myName() { return "DiskResidual"; };
private:
    const math::IFunction* radial_fnc;    ///< function describing radial dependence of surface density
    const math::IFunction* vertical_fnc;  ///< function describing vertical density profile
    virtual double density_cyl(const coord::PosCyl &pos) const;
    virtual double density_car(const coord::PosCar &pos) const
    {  return density_cyl(coord::toPosCyl(pos)); }
    virtual double density_sph(const coord::PosSph &pos) const
    {  return density_cyl(coord::toPosCyl(pos)); }
};

/** Part of the disk potential provided analytically as  4\pi f(r) H(z) */
class DiskAnsatz: public BasePotentialCyl {
public:
    DiskAnsatz (const DiskParam& params) : 
        BasePotentialCyl(),
        radial_fnc  (createRadialDiskFnc(params)),
        vertical_fnc(createVerticalDiskFnc(params)) {};
    ~DiskAnsatz() { delete radial_fnc; delete vertical_fnc; }
    virtual SymmetryType symmetry() const { return ST_AXISYMMETRIC; }
    virtual const char* name() const { return myName(); };
    static const char* myName() { return "DiskAnsatz"; };
private:
    const math::IFunction* radial_fnc;    ///< function describing radial dependence of surface density
    const math::IFunction* vertical_fnc;  ///< function describing vertical density profile
    /** Compute _part_ of disk potential: f(r)*H(z) */
    virtual void eval_cyl(const coord::PosCyl &pos,
        double* potential, coord::GradCyl* deriv, coord::HessCyl* deriv2) const;
    virtual double density_cyl(const coord::PosCyl &pos) const;
};

/** Two-power-law spheroidal density profile with optional cutoff and flattening 
    along the minor axis.
    The density is given by
    \f$  \rho(R,z) = \rho_0  (r/r_0)^{-\gamma} (1+r/r_0)^{\gamma-\beta} \exp[ -(r/r_{cut})^2],
    r = \sqrt{ R^2 + z^2/q^2 }  \f$.
*/
class SpheroidDensity: public BaseDensity{
public:
    SpheroidDensity (const SphrParam &_params);
    virtual SymmetryType symmetry() const { 
        return params.axisRatio==1?ST_SPHERICAL:ST_AXISYMMETRIC; }
    virtual const char* name() const { return myName(); };
    static const char* myName() { return "TwoPowerLawSpheroid"; };
private:
    SphrParam params;
    virtual double density_cyl(const coord::PosCyl &pos) const;
    virtual double density_car(const coord::PosCar &pos) const
    {  return density_cyl(coord::toPosCyl(pos)); }
    virtual double density_sph(const coord::PosSph &pos) const
    {  return density_cyl(coord::toPosCyl(pos)); }
};

/** Multipole expansion for axisymmetric potentials, generated from a given axisymmetric 
    density profile (which may well be an instance of a CompositeDensity class).
*/
class Multipole: public BasePotentialCyl{
private:
    int    K[2];  // dimensions of 2d spline
    double Rmin, Rmax, gamma, beta, Phi0;
    double lRmin, lRmax, g2;
    double lzmin, lzmax, tg3, g3h;
    double *logr; 
    double *X[2], **Y[3], **Z[4];
    void   AllocArrays();
    void   setup(const BaseDensity& source_density,
                 const double r_min, const double r_max,
                 const double gamma, const double beta);
    virtual const char* name() const { return myName(); };
    static const char* myName() { return "AxisymmetricMultipole"; };
public:
    /** Compute the potential using the multi expansion and approximate it 
        by a two-dimensional spline in (R,z) plane. 
        \param[in]  source_density  is the density model that serves as an input 
                    to the potential approximation, a std::runtime_error exception 
                    is raised if it is not axisymmetric;
        \param[in]  r_min, r_max  give the radial grid extent;
        \param[in]  num_grid_points   is the size of logarithmic spline grid in R;
        \param[in]  gamma  is the power-law index of density extrapolation at small r;
        \param[in]  beta   is the slope of density profile at large radii;
    */
    Multipole (const BaseDensity& source_density,
               const double r_min, const double r_max,
               const int num_grid_points,
               const double gamma, const double beta);
    ~Multipole();
    virtual SymmetryType symmetry() const { return ST_AXISYMMETRIC; }
private:
    virtual void eval_cyl(const coord::PosCyl &pos,
        double* potential, coord::GradCyl* deriv, coord::HessCyl* deriv2) const;
};

/** Construct a CompositeCyl potential consisting of a Multipole and a number of DiskAnsatz 
    components, using the provided arrays of parameters for disks and spheroids
*/
const potential::BasePotential* createGalaxyPotential(
    const std::vector<DiskParam>& DiskParams,
    const std::vector<SphrParam>& SphrParams);

} // namespace potential
