// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2015 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban, Michael Taylor, Rainer Gericke
// =============================================================================
//
// Template for a Magic Formula tire model
//
// ChMFTire is based on the Pacejka 2002 formulae as written in
// Hans B. Pacejka's "Tire and Vehicle Dynamics" Third Edition, Elsevier 2012
// ISBN: 978-0-08-097016-5
//
// Actually implemented:
// =============================================================================

#ifndef CH_MFTIRE_H
#define CH_MFTIRE_H

#include <vector>

#include "chrono/physics/ChBody.h"
#include "chrono/assets/ChCylinderShape.h"

#include "chrono_vehicle/wheeled_vehicle/tire/ChForceElementTire.h"
#include "chrono_vehicle/ChTerrain.h"

namespace chrono {
namespace vehicle {

/// @addtogroup vehicle_wheeled_tire
/// @{

/// Pacjeka 02 tire model.
class CH_VEHICLE_API ChMFTire : public ChForceElementTire {
   public:
    ChMFTire(const std::string& name);

    ChMFTire(const std::string& name, std::string& tirFileName);

    virtual ~ChMFTire() {}

    /// Get the name of the vehicle subsystem template.
    virtual std::string GetTemplateName() const override { return "MFTire"; }

    /// Add visualization assets for the rigid tire subsystem.
    virtual void AddVisualizationAssets(VisualizationType vis) override;

    /// Remove visualization assets for the rigid tire subsystem.
    virtual void RemoveVisualizationAssets() override;

    /// Get the tire radius.
    virtual double GetRadius() const override { return m_states.R_eff; }

    /// Set the limit for camber angle (in degrees).  Default: 3 degrees.
    void SetGammaLimit(double gamma_limit) { m_gamma_limit = gamma_limit; }

    /// Get the width of the tire.
    virtual double GetWidth() const override { return m_par.WIDTH; }

    /// Get the tire deflection
    virtual double GetDeflection() const override { return m_data.depth; }

    /// Get visualization width.
    virtual double GetVisualizationWidth() const { return m_par.WIDTH; }

    /// Get the slip angle used in Pac89 (expressed in radians).
    /// The reported value will have opposite sign to that reported by ChTire::GetSlipAngle because ChPac89 uses
    /// internally a different frame convention.
    double GetSlipAngle_internal() const { return m_states.alpha; }

    /// Get the longitudinal slip used in Pac89.
    /// The reported value will be similar to that reported by ChTire::GetLongitudinalSlip.
    double GetLongitudinalSlip_internal() const { return m_states.kappa; }

    /// Get the camber angle used in Pac89 (expressed in radians).
    /// The reported value will be similar to that reported by ChTire::GetCamberAngle.
    double GetCamberAngle_internal() { return m_states.gamma * CH_C_DEG_TO_RAD; }

   protected:
    void SetMFParamsByFile(std::string& tirFileName);
    
    /// Set the parameters in the Pac89 model.
    virtual void SetMFParams() = 0;

    double m_gamma_limit;  ///< limit camber angle

    bool m_use_friction_ellipsis;

    /// Road friction
    double m_mu_road;

    double m_Shf;
    double m_Cy;
    double m_By;

    VehicleSide m_measured_side;
    bool m_allow_mirroring;

    unsigned int m_use_mode;
  
    struct MFCoeff {
         // [UNITS]
         double time_factor = 1;
         double length_factor = 1;
         double angle_factor = 1;
         double mass_factor = 1;
         double force_factor = 1;
         
         // [MODEL]
         int FITTYP = 6;     // MFTire 5.2; 61 = 6.1; 62 = 6.2
         int USE_MODE = 1;  // Tyre use switch (IUSED)
         double VXLOW = 1;
         double LONGVL = 16.6;  // Measurement speed

         // [DIMENSION]
         double UNLOADED_RADIUS = 0;  // Free tyre radius
         double WIDTH = 0;            // Nominal section width of the tyre
         double ASPECT_RATIO = 0;     // Nominal aspect ratio
         double RIM_RADIUS = 0;       // Nominal rim radius
         double RIM_WIDTH = 0;        // Rim width

         // [VERTICAL]
         double VERTICAL_STIFFNESS = 0;  // Tyre vertical stiffness
         double VERTICAL_DAMPING = 0;    // Tyre vertical damping
         double BREFF = 0;               // Low load stiffness e.r.r.
         double DREFF = 0;               // Peak value of e.r.r.
         double FREFF = 0;               // High load stiffness e.r.r.
         double FNOMIN = 0;              // Nominal wheel load

         // [SCALING_COEFFICIENTS]
         double LFZO = 1;   // Scale factor of nominal (rated) load
         double LCX = 1;    // Scale factor of Fx shape factor
         double LMUX = 1;   // Scale factor of Fx peak friction coefficient
         double LEX = 1;    // Scale factor of Fx curvature factor
         double LKX = 1;    // Scale factor of Fx slip stiffness
         double LHX = 1;    // Scale factor of Fx horizontal shift
         double LVX = 1;    // Scale factor of Fx vertical shift
         double LGAX = 1;   // Scale factor of camber for Fx
         double LCY = 1;    // Scale factor of Fy shape factor
         double LMUY = 1;   // Scale factor of Fy peak friction coefficient
         double LEY = 1;    // Scale factor of Fy curvature factor
         double LKY = 1;    // Scale factor of Fy cornering stiffness
         double LHY = 1;    // Scale factor of Fy horizontal shift
         double LVY = 1;    // Scale factor of Fy vertical shift
         double LGAY = 1;   // Scale factor of camber for Fy
         double LTR = 1;    // Scale factor of Peak of pneumatic trail
         double LRES = 1;   // Scale factor for offset of residual torque
         double LGAZ = 1;   // Scale factor of camber for Mz
         double LXAL = 1;   // Scale factor of alpha influence on Fx
         double LYKA = 1;   // Scale factor of alpha influence on Fx
         double LVYKA = 1;  // Scale factor of kappa induced Fy
         double LS = 1;     // Scale factor of Moment arm of Fx
         double LSGKP = 1;  // Scale factor of Relaxation length of Fx
         double LSGAL = 1;  // Scale factor of Relaxation length of Fy
         double LGYR = 1;   // Scale factor of gyroscopic torque
         double LMX = 1;    // Scale factor of overturning couple
         double LVMX = 1;   // Scale factor of Mx vertical shift
         double LMY = 1;    // Scale factor of rolling resistance torque

         // [LONGITUDINAL_COEFFICIENTS]
         double PCX1 = 0;  // Shape factor Cfx for longitudinal force
         double PDX1 = 0;  // Longitudinal friction Mux at Fznom
         double PDX2 = 0;  // Variation of friction Mux with load
         double PDX3 = 0;  // Variation of friction Mux with camber
         double PEX1 = 0;  // Longitudinal curvature Efx at Fznom
         double PEX2 = 0;  // Variation of curvature Efx with load
         double PEX3 = 0;  // Variation of curvature Efx with load squared
         double PEX4 = 0;  // Factor in curvature Efx while driving
         double PKX1 = 0;  // Longitudinal slip stiffness Kfx/Fz at Fznom
         double PKX2 = 0;  // Variation of slip stiffness Kfx/Fz with load
         double PKX3 = 0;  // Exponent in slip stiffness Kfx/Fz with load
         double PHX1 = 0;  // Horizontal shift Shx at Fznom
         double PHX2 = 0;  // Variation of shift Shx with load
         double PVX1 = 0;  // Vertical shift Svx/Fz at Fznom
         double PVX2 = 0;  // Variation of shift Svx/Fz with load
         double RBX1 = 0;  // Slope factor for combined slip Fx reduction
         double RBX2 = 0;  // Variation of slope Fx reduction with kappa
         double RCX1 = 0;  // Shape factor for combined slip Fx reduction
         double REX1 = 0;  // Curvature factor of combined Fx
         double REX2 = 0;  // Curvature factor of combined Fx with load
         double RHX1 = 0;  // Shift factor for combined slip Fx reduction
         double PTX1 = 0;  // Relaxation length SigKap0/Fz at Fznom
         double PTX2 = 0;  // Variation of SigKap0/Fz with load
         double PTX3 = 0;  // Variation of SigKap0/Fz with exponent of load

         // [OVERTURNING_COEFFICIENTS]
         double QSX1 = 0;  // Lateral force induced overturning moment
         double QSX2 = 0;  // Camber induced overturning couple
         double QSX3 = 0;  // Fy induced overturning couple

         // [LATERAL_COEFFICIENTS]
         double PCY1 = 0;  // Shape factor Cfy for lateral forces
         double PDY1 = 0;  // Lateral friction Muy
         double PDY2 = 0;  // Variation of friction Muy with load
         double PDY3 = 0;  // Variation of friction Muy with squared camber
         double PEY1 = 0;  // Lateral curvature Efy at Fznom
         double PEY2 = 0;  // Variation of curvature Efy with load
         double PEY3 = 0;  // Zero order camber dependency of curvature Efy
         double PEY4 = 0;  // Variation of curvature Efy with camber
         double PKY1 = 0;  // Maximum value of stiffness Kfy/Fznom
         double PKY2 = 0;  // Load at which Kfy reaches maximum value
         double PKY3 = 0;  // Variation of Kfy/Fznom with camber
         double PHY1 = 0;  // Horizontal shift Shy at Fznom
         double PHY2 = 0;  // Variation of shift Shy with load
         double PHY3 = 0;  // Variation of shift Shy with camber
         double PVY1 = 0;  // Vertical shift in Svy/Fz at Fznom
         double PVY2 = 0;  // Variation of shift Svy/Fz with load
         double PVY3 = 0;  // Variation of shift Svy/Fz with camber
         double PVY4 = 0;  // Variation of shift Svy/Fz with camber and load
         double RBY1 = 0;  // Slope factor for combined Fy reduction
         double RBY2 = 0;  // Variation of slope Fy reduction with alpha
         double RBY3 = 0;  // Shift term for alpha in slope Fy reduction
         double RCY1 = 0;  // Shape factor for combined Fy reduction
         double REY1 = 0;  // Curvature factor of combined Fy
         double REY2 = 0;  // Curvature factor of combined Fy with load
         double RHY1 = 0;  // Shift factor for combined Fy reduction
         double RHY2 = 0;  // Shift factor for combined Fy reduction with load
         double RVY1 = 0;  // Kappa induced side force Svyk/Muy*Fz at Fznom
         double RVY2 = 0;  // Variation of Svyk/Muy*Fz with load
         double RVY3 = 0;  // Variation of Svyk/Muy*Fz with camber
         double RVY4 = 0;  // Variation of Svyk/Muy*Fz with alpha
         double RVY5 = 0;  // Variation of Svyk/Muy*Fz with kappa
         double RVY6 = 0;  // Variation of Svyk/Muy*Fz with atan(kappa)
         double PTY1 = 0;  // Peak value of relaxation length SigAlp0/R0
         double PTY2 = 0;  // Value of Fz/Fznom where SigAlp0 is extreme

         // [ROLLING_COEFFICIENTS]
         double QSY1 = 0;  // Rolling resistance torque coefficient
         double QSY2 = 0;  // Rolling resistance torque depending on Fx
         double QSY3 = 0;  // Rolling resistance torque depending on speed
         double QSY4 = 0;  // Rolling resistance torque depending on speed ^4

         // [INERTIA]
         double MASS = 0;
         double IXX  = 0;
         double IYY  = 0;
         
         // [ALIGNING_COEFFICIENTS]
         double QBZ1 = 0;   // Trail slope factor for trail Bpt at Fznom
         double QBZ2 = 0;   // Variation of slope Bpt with load
         double QBZ3 = 0;   // Variation of slope Bpt with load squared
         double QBZ4 = 0;   // Variation of slope Bpt with camber
         double QBZ5 = 0;   // Variation of slope Bpt with absolute camber
         double QBZ9 = 0;   // Slope factor Br of residual torque Mzr
         double QBZ10 = 0;  // Slope factor Br of residual torque Mzr
         double QCZ1 = 0;   // Shape factor Cpt for pneumatic trail
         double QDZ1 = 0;   // Peak trail Dpt" = Dpt*(Fz/Fznom*R0)
         double QDZ2 = 0;   // Variation of peak Dpt" with load
         double QDZ3 = 0;   // Variation of peak Dpt" with camber
         double QDZ4 = 0;   // Variation of peak Dpt" with camber squared
         double QDZ6 = 0;   // Peak residual torque Dmr" = Dmr/(Fz*R0)
         double QDZ7 = 0;   // Variation of peak factor Dmr" with load
         double QDZ8 = 0;   // Variation of peak factor Dmr" with camber
         double QDZ9 = 0;   // Variation of peak factor Dmr" with camber and load
         double QEZ1 = 0;   // Trail curvature Ept at Fznom
         double QEZ2 = 0;   // Variation of curvature Ept with load
         double QEZ3 = 0;   // Variation of curvature Ept with load squared
         double QEZ4 = 0;   // Variation of curvature Ept with sign of Alpha-t
         double QEZ5 = 0;   // Variation of Ept with camber and sign Alpha-t
         double QHZ1 = 0;   // Trail horizontal shift Sht at Fznom
         double QHZ2 = 0;   // Variation of shift Sht with load
         double QHZ3 = 0;   // Variation of shift Sht with camber
         double QHZ4 = 0;   // Variation of shift Sht with camber and load
         double SSZ1 = 0;   // Nominal value of s/R0: effect of Fx on Mz
         double SSZ2 = 0;   // Variation of distance s/R0 with Fy/Fznom
         double SSZ3 = 0;   // Variation of distance s/R0 with camber
         double SSZ4 = 0;   // Variation of distance s/R0 with load and camber
         double QTZ1 = 0;   // Gyration torque constant
         double MBELT = 0;  // Belt mass of the wheel
     };

     MFCoeff m_par;


    /// Initialize this tire by associating it to the specified wheel.
    virtual void Initialize(std::shared_ptr<ChWheel> wheel) override;

    /// Update the state of this tire system at the current time.
    virtual void Synchronize(double time,              ///< [in] current time
                             const ChTerrain& terrain  ///< [in] reference to the terrain system
                             ) override;

    /// Advance the state of this tire by the specified time step.
    virtual void Advance(double step) override;

    struct ContactData {
        bool in_contact;      // true if disc in contact with terrain
        ChCoordsys<> frame;   // contact frame (x: long, y: lat, z: normal)
        ChVector<> vel;       // relative velocity expressed in contact frame
        double normal_force;  // magnitude of normal contact force
        double depth;         // penetration depth
    };

    struct TireStates {
        double kappa;            // slip ratio [-1:+1]
        double alpha;            // slip angle [-PI/2:+PI/2]
        double gamma;            // inclination angle
        double vx;               // Longitudinal speed
        double vsx;              // Longitudinal slip velocity
        double vsy;              // Lateral slip velocity = Lateral velocity
        double omega;            // Wheel angular velocity about its spin axis
        double R_eff;            // Effective Radius
        ChVector<> disc_normal;  //(temporary for debug)
    };

    TireStates m_states;
    std::shared_ptr<ChVisualShape> m_cyl_shape;  ///< visualization cylinder asset
};

/// @} vehicle_wheeled_tire

}  // end namespace vehicle
}  // end namespace chrono

#endif
