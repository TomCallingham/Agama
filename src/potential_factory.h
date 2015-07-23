/** \file    potential_factory.h
    \brief   Creation and input/output of Potential instances
    \author  EV
    \date    2010-2015

    This file provides several utility function to manage instances of CDensity and CPotential: 
    creating a density or potential model from parameters provided in CConfigPotential, 
    creating a potential from a set of point masses or from an N-body snapshot file,
    loading potential coefficients from a text file, 
    writing expansion coefficients to a text file,
    and creating a spherical mass model that approximates the given density model.
    Note that potential here is elementary (non-composite, no central black hole).
*/

#pragma once
#include "potential_base.h"
#include "particles_base.h"
#include "units.h"
#include <string>

namespace potential {

/// \name Definitions of all known potential types and parameters
///@{

/// List of all known potential and density types
enum PotentialType {
    PT_UNKNOWN,     ///< undefined
    PT_COEFS,       ///< not an actual density model, but a way to load pre-computed coefficients of potential expansion
    PT_DIRECT,      ///< direct evaluation of potential from Poisson equaiton:  CPotentialDirect
    PT_COMPOSITE,   ///< a superposition of multiple potential instances:  CPotentialComposite
    PT_NB,          ///< a set of frozen particles:  CPotentialNB
    PT_BSE,         ///< basis-set expansion for infinite systems:  CPotentialBSE
    PT_BSECOMPACT,  ///< basis-set expansion for systems with non-singular density and finite extent:  CPotentialBSECompact
    PT_SPLINE,      ///< spline spherical-harmonic expansion:  CPotentialSpline
    PT_CYLSPLINE,   ///< expansion in azimuthal angle with two-dimensional meridional-plane interpolating splines:  CPotentialCylSpline
    PT_LOG,         ///< logaritmic potential:  CPotentialLog
    PT_HARMONIC,    ///< simple harmonic oscillator:  CPotentialHarmonic
    PT_SCALEFREE,   ///< single power-law density profile:  CPotentialScaleFree
    PT_SCALEFREESH, ///< spherical-harmonic approximation to a power-law density:  CPotentialScaleFreeSH
    PT_SPHERICAL,   ///< arbitrary spherical mass model:  CPotentialSpherical
    PT_DEHNEN,      ///< Dehnen(1993) density model:  CPotentialDehnen
    PT_MIYAMOTONAGAI,///< Miyamoto-Nagai(1975) flattened model:  CPotentialMiyamotoNagai
    PT_FERRERS,     ///< Ferrers finite-extent profile:  CPotentialFerrers
    PT_PLUMMER,     ///< Plummer model:  CDensityPlummer
    PT_ISOCHRONE,   ///< isochrone model:  CDensityIsochrone
    PT_PERFECTELLIPSOID,  ///< Kuzmin/de Zeeuw integrable potential:  CDensityPerfectEllipsoid
    PT_NFW,         ///< Navarro-Frenk-White profile:  CDensityNFW
    PT_SERSIC,      ///< Sersic density profile:  CDensitySersic
    PT_EXPDISK,     ///< exponential (in R) disk with a choice of vertical density profile:  CDensityExpDisk
    PT_ELLIPSOIDAL, ///< a generalization of spherical mass profile with arbitrary axis ratios:  CDensityEllipsoidal
    PT_MGE,         ///< Multi-Gaussian expansion:  CDensityMGE
    PT_GALPOT,      ///< Walter Dehnen's GalPot (exponential discs and spheroids)
};

/// structure that contains parameters for all possible potentials
struct ConfigPotential
{
    double mass;                             ///< total mass of the model (not applicable to all potential types)
    double scalerad;                         ///< scale radius of the model (if applicable)
    double scalerad2;                        ///< second scale radius of the model (if applicable)
    double q, p;                             ///< axis ratio of the model (if applicable)
    double gamma;                            ///< central cusp slope (for Dehnen and scale-free models)
    double sersicIndex;                      ///< Sersic index (for Sersic density model)
    size_t numCoefsRadial, numCoefsAngular;  ///< number of radial and angular coefficients in spherical-harmonic expansion
    size_t numCoefsVertical;                 ///< number of coefficients in z-direction for Cylindrical potential
    double alpha;                            ///< shape parameter for BSE potential
#if 0
    double rmax;                             ///< radius of finite density model for BSECompact potential
    double treecodeEps;                      ///< treecode smooothing length (negative means adaptive smoothing based on local density, absolute value is the proportionality coefficient between eps and mean interparticle distance)
    double treecodeTheta;                    ///< tree cell opening angle
#endif
    PotentialType potentialType;             ///< currently selected potential type
    PotentialType densityType;               ///< if pot.type == BSE or Spline, this gives the underlying density profile approximated by these expansions or flags that an Nbody file should be used
    SymmetryType symmetryType;               ///< if using Nbody file with the above two potential expansions, may assume certain symmetry on the coefficients (do not compute them but just assign to zero)
    double splineSmoothFactor;               ///< for smoothing Spline potential coefs initialized from discrete point mass set
    double splineRMin, splineRMax;           ///< if nonzero, specifies the inner- and outermost grid node radii
    double splineZMin, splineZMax;           ///< if nonzero, gives the grid extent in z direction for Cylindrical spline potential
    std::string fileName;                    ///< name of file with coordinates of points, or coefficients of expansion, or any other external data array
#if 0
    double mbh;                              ///< mass of central black hole (in the composite potential)
    double binary_q;                         ///< binary BH mass ratio (0<=q<=1)
    double binary_sma;                       ///< binary BH semimajor axis
    double binary_ecc;                       ///< binary BH eccentricity (0<=ecc<1)
    double binary_phase;                     ///< binary BH orbital phase (0<=phase<2*pi)
#endif
};

///@}
/// \name Factory routines that create an instance of specific potential from a set of parameters
///@{

/** create a density model according to the parameters. 
    This only deals witj finite-mass models, including some of the CPotential descendants.
    \param[in] configPotential contains the parameters (density type, mass, shape, etc.)
    \return    the instance of CDensity, or NULL in case of incorrect parameters
*/
const BaseDensity* createDensity(const ConfigPotential& config);

/** create an instance of CPotential according to the parameters passed. 
    \param[in,out] config specifies the potential parameters, which could be modified, 
                   e.g. if the potential coefficients are loaded from a file.
                   Massive black hole (config->Mbh) is not included in the potential 
                   (the returned potential is always non-composite)
    \return        the instance of potential, or NULL in case of failure
*/
const BasePotential* createPotential(ConfigPotential& config);

/** create a potential of a generic expansion kind from a set of point masses.
    \param[in] configPotential contains the parameters (potential type, number of terms in expansion, etc.)
    \param[in] points is the array of particles that are used in computing the coefficients; 
               if potential type is PT_NB, then an instance of tree-code potential is created.
    \return    a new instance of potential on success, or NULL on failure (e.g. if potential type is inappropriate).
*/
template<typename CoordT>
const BasePotential* createPotentialFromPoints(const ConfigPotential& configPotential, const particles::PointMassSet<CoordT>& points);

/** load a potential from a text or snapshot file.

    The input file may contain one of the following kinds of data:
    - a Nbody snapshot in a text or binary format, handled by classes derived from CBasicIOSnapshot;
    - a potential coefficients file for CPotentialBSE, CPotentialBSECompact, CPotentialSpline, or CPotentialCylSpline;
    - a density model described by CDensityEllipsoidal or CDensityMGE.
    The data format is determined from the first line of the file, and 
    if it is allowed by the parameters passed in configPotential, then 
    the file is read and the instance of a corresponding potential is created. 
    If the input data was not the potential coefficients and the new potential 
    is of BSE or Spline type, then a new file with potential coefficients is 
    created and written via writePotential(), so that later one may load this 
    coef file instead of the original one, which speeds up initialization.
    \param[in,out] configPotential contains the potential parameters and may be updated 
                   upon reading the file (e.g. the number of expansion coefficients 
                   may change). If the file doesn't contain appropriate kind of potential
                   (i.e. if configPotential->PotentialType is PT_NB but the file contains 
                   BSE coefficients or a description of MGE model), an error is returned.
                   configPotential->NbodyFile contains the file name from which the data is loaded.
    \return        a new instance of BasePotential* on success
    \throws        std::runtime_error or other potential-specific exception on failure
*/
const BasePotential* readPotential(ConfigPotential& configPotential);
    
/** Utility function providing a legacy interface compatible with the original GalPot.
    It reads the parameters from a text file and converts them into the internal unit system, 
    then constructs the potential using `createGalaxyPotential` routine.
    \param[in]  filename is the name of parameter file;
    \param[in]  units is the specification of internal unit system;
    \returns    the new CompositeCyl potential
    \throws     a std::runtime_error exception if file is not readable or does not contain valid parameters.
*/
const potential::BasePotential* readGalaxyPotential(const char* filename, const units::Units& units);
    
/** write potential expansion coefficients to a text file.

    The potential must be one of the following expansion classes: 
    CPotentialBSE, CPotentialBSECompact, CPotentialSpline, CPotentialCylSpline, CPotentialScaleFreeSH. 
    The coefficients stored in a file may be later loaded by readPotential() function.
    \param[in] fileName is the output file
    \param[in] potential is the pointer to potential
    \throws std::runtime_error on failure (file not writeable or potential is of inappropriate type).
*/
void writePotential(const std::string &fileName, const BasePotential& potential);

#if 0
/** create an equivalent spherical mass model for the given density profile. 
    \param[in] density is the non-spherical density model to be approximated
    \param[in] poten (optional) another spherical mass model that provides the potential 
               (if it is not given self-consistently by this density profile).
    \param[in] numNodes specifies the number of radial grid points in the M(r) profile
    \param[in] Rmin is the radius of the innermost non-zero grid node (0 means auto-select)
    \param[in] Rmax is the radius outermost grid node (0 means auto-select)
    \return    new instance of CMassModel on success, or NULL on fail (e.g. if the density model was infinite) 
*/
CMassModel* createMassModel(const CDensity* density, int numNodes=NUM_RADIAL_POINTS_SPHERICAL_MODEL, 
    double Rmin=0, double Rmax=0, const CMassModel* poten=NULL);
#endif

/// \name Correspondence between potential/density names and corresponding classes
///@{

/// return the name of the potential of a given type, or empty string if unavailable
const char* getPotentialNameByType(PotentialType type);

/// return the name of the density of a given type, or empty string if unavailable
const char* getDensityNameByType(PotentialType type);

/// return the name of the symmetry of a given type, or empty string if unavailable
const char* getSymmetryNameByType(SymmetryType type);

/// return the type of density or potential object
PotentialType getPotentialType(const BaseDensity& d);

/// return the type of the potential model by its name, or PT_UNKNOWN if unavailable
PotentialType getPotentialTypeByName(const std::string& PotentialName);

/// return the type of the density model by its name, or PT_UNKNOWN if unavailable
PotentialType getDensityTypeByName(const std::string& DensityName);

/// return the type of symmetry by its name, or ST_DEFAULT if unavailable
SymmetryType getSymmetryTypeByName(const std::string& SymmetryName);

/// return file extension for writing the coefficients of potential of the given type
const char* getCoefFileExtension(PotentialType potType);

/// find potential type by file extension
PotentialType getCoefFileType(const std::string& fileName);

///@}

}; // namespace
