// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2024 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Author: Radu Serban
// =============================================================================
//
// Miscellaneous enumerations for the SPH-based Chrono::FSI fluid solver
//
// =============================================================================

#ifndef CH_FSI_DEFINITIONS_SPH_H
#define CH_FSI_DEFINITIONS_SPH_H

namespace chrono {
namespace fsi {

/// @addtogroup fsi_physics
/// @{

/// Physics problem type.
enum class PhysicsProblem {
    CFD,  ///< incompressible fluid problem
    CRM   ///< continuous granular problem
};

/// SPH method.
enum class SPHMethod {
    WCSPH,  ///< Weakly Compressible SPH (explicit)
    I2SPH   ///< Implicit SPH
};

/// Equation of State type.
/// see https://pysph.readthedocs.io/en/latest/reference/equations.html#basic-wcsph-equations
enum class EosType { TAIT, ISOTHERMAL };

/// SPH kernel type.
enum class KernelType { QUADRATIC, CUBIC_SPLINE, QUINTIC_SPLINE, WENDLAND };

/// Visosity method type.
enum class ViscosityType { LAMINAR, ARTIFICIAL_UNILATERAL, ARTIFICIAL_BILATERAL };

/// Boundary type.
enum class BoundaryType { ADAMI, HOLMES };

/// Rheology type.
enum class Rheology { INERTIA_RHEOLOGY, NONLOCAL_FLUIDITY };

/// Friction law in ISPH.
enum class FrictionLaw { CONSTANT, LINEAR, NONLINEAR };

/// Linear solver type.
enum class SolverType { JACOBI, BICGSTAB, GMRES, CR, CG, SAP };

/// BCE pattern in cross section of 1-D flexible elements.
/// The available patterns are illustrated below (assuming 3 BCE layers):
/// <pre>
/// FULL:
///      X--X--X
///      X--X--X
///      X--X--X
/// STAR:
///      ---X---
///      X--X--X
///      ---X---
/// </pre>
enum class BcePatternMesh1D { FULL, STAR };

/// BCE pattern along normal of 2-D surface of flexible elements.
/// The choices are illustrated below (assuming 3 BCE layers):
/// <pre>
/// OUTWARD:
///    ^ n
///    |    ...--X--X--X--...
///    |    ...--X--X--X--...
/// ---|---------X--X--X-------- surface
///
/// CENTERED:
///    ^ n
///    |    ...--X--X--X--...
/// ---|---------X--X--X-------- surface
///    |    ...--X--X--X--...
///
/// INWARD:
///    ^ n
/// ---|---------X--X--X-------- surface
///    |    ...--X--X--X--...
///    |    ...--X--X--X--...
/// </pre>
enum class BcePatternMesh2D { CENTERED, OUTWARD, INWARD };

/// Output level.
enum class OutputLevel {
    STATE,           ///< marker state, velocity, and acceleration
    STATE_PRESSURE,  ///< STATE plus density and pressure
    CFD_FULL,        ///< STATE_PRESSURE plus various CFD parameters
    CRM_FULL         ///< STATE_PRESSURE plus normal and shear stress
};

/// Output mode.
enum class OutputMode {
    CSV,  ///< comma-separated value
    CHPF  ///< binary
};

/// @} fsi_physics

}  // namespace fsi
}  // namespace chrono

#endif
