// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban
//
// Translational spring-damper-actuator (TSDA) with force optionally specified
// through a user-supplied functor object (default, linear TSDA).
//
// Optionally, a ChLinkTSDA can have internal dynamics, described by a system of
// ODEs. The internal states are integrated simultaneous with the containing
// system and they can be accessed and used in the force calculation.
// Such objects can be used in active suspension models.
// =============================================================================

#ifndef CH_LINK_TSDA_H
#define CH_LINK_TSDA_H

#include "chrono/physics/ChLink.h"
#include "chrono/physics/ChBody.h"
#include "chrono/solver/ChVariablesGenericDiagonalMass.h"
#include "chrono/solver/ChKblockGeneric.h"

namespace chrono {

/// Class for translational spring-damper-actuator (TSDA) with the force optionally specified through a functor object
/// (default, linear TSDA). Optionally, a ChLinkTSDA can have internal dynamics, described by a system of ODEs. The
/// internal states are integrated simultaneous with the containing system and they can be accessed and used in the
/// force calculation. ChLinkTSDA provides optional support for computing Jacobians of the generalized forces.
class ChApi ChLinkTSDA : public ChLink {
  public:
    ChLinkTSDA();
    ChLinkTSDA(const ChLinkTSDA& other);
    ~ChLinkTSDA();

    /// "Virtual" copy constructor (covariant return type).
    virtual ChLinkTSDA* Clone() const override;

    /// Set spring rest (free) length.
    /// Optionally, the free length can be calculated from the initial configuration (see #Initialize).
    void SetRestLength(double len) { m_rest_length = len; }

    /// Set spring coefficient (default: 0).
    /// Used only if no force functor is provided.
    void SetSpringCoefficient(double k) { m_k = k; }

    /// Set damping coefficient (default: 0).
    /// Used only if no force functor is provided.
    void SetDampingCoefficient(double r) { m_r = r; }

    /// Set constant actuation force (default: 0).
    /// Used only if no force functor is provided.
    void SetActuatorForce(double f) { m_f = f; }

    /// Declare the forces generated by this spring as stiff (default: false).
    /// If stiff, Jacobian information will be generated.
    void IsStiff(bool val) { m_stiff = val; }

    /// Get current states.
    const ChVectorDynamic<>& GetStates() const { return m_states; }

    /// Get the spring rest (free) length.
    double GetRestLength() const { return m_rest_length; }

    /// Get current length.
    double GetLength() const { return m_length; }

    /// Get current deformation.
    double GetDeformation() const { return m_length - m_rest_length; }

    /// Get current length rate of change.
    double GetVelocity() const { return m_length_dt; }

    /// Get current force (in the direction of the force element).
    double GetForce() const { return m_force; }

    /// Get the endpoint location on 1st body (expressed in body coordinate system)
    const ChVector<>& GetPoint1Rel() const { return m_loc1; }

    /// Get the endpoint location on 1st body (expressed in absolute coordinate system)
    const ChVector<>& GetPoint1Abs() const { return m_aloc1; }

    /// Get the endpoint location on 2nd body (expressed in body coordinate system)
    const ChVector<>& GetPoint2Rel() const { return m_loc2; }

    /// Get the endpoint location on 1st body (expressed in absolute coordinate system)
    const ChVector<>& GetPoint2Abs() const { return m_aloc2; }

    /// Class to be used as a callback interface for calculating the general spring-damper force.
    class ChApi ForceFunctor {
      public:
        virtual ~ForceFunctor() {}

        /// Calculate and return the general spring-damper force at the specified configuration.
        /// If the link has internal ODE states, the current states can be accessed with link->GetStates().
        virtual double operator()(double time,         ///< current time
                                  double rest_length,  ///< undeformed length
                                  double length,       ///< current length
                                  double vel,          ///< current velocity (positive when extending)
                                  ChLinkTSDA* link     ///< back-pointer to associated link
                                  ) = 0;
    };

    /// Specify the functor object for calculating the force.
    void RegisterForceFunctor(ForceFunctor* functor) { m_force_fun = functor; }

    /// Class to be used as a callback interface for specifying the ODE, y' = f(t,y); y(0) = y0.
    class ChApi ODE {
      public:
        virtual ~ODE() {}

        /// Specify number of states (dimension of y).
        virtual int GetNumStates() const = 0;

        /// Set initial conditions.
        /// Must load y0 = y(0).
        virtual void SetInitialConditions(ChVectorDynamic<>& states,  ///< output initial conditions vector
                                          ChLinkTSDA* link            ///< back-pointer to associated link
                                          ) = 0;

        /// Calculate and return the ODE right-hand side at the provided time and states.
        /// Must load f(t,y).
        virtual void CalculateRHS(double time,                      ///< current time
                                  const ChVectorDynamic<>& states,  ///< current ODE states
                                  ChVectorDynamic<>& rhs,           ///< output ODE right-hand side vector
                                  ChLinkTSDA* link                  ///< back-pointer to associated link
                                  ) = 0;

        /// Calculate the Jacobian of the ODE right-hand side with rerspect to the ODE states.
        /// Only used if the link force is declared as stiff.  If provided, load df/dy into the provided matrix 'jac'
        /// (already set to zero before the call) and return 'true'. In that case, the user-provided Jacobian will
        /// overwrite the default finite-difference approximation.
        virtual bool CalculateJac(double time,                      ///< current time
                                  const ChVectorDynamic<>& states,  ///< current ODE states
                                  const ChVectorDynamic<>& rhs,     ///< current ODE right-hand side vector
                                  ChMatrixDynamic<>& jac,           ///< output Jacobian matrix
                                  ChLinkTSDA* link                  ///< back-pointer to associated link
        ) {
            return false;
        }
    };

    /// Specify the functor object for calculating the ODE right-hand side.
    void RegisterODE(ODE* functor);

    /// Initialize the spring, specifying the two bodies to be connected, the location of the two anchor points of each
    /// body (each expressed in body or absolute coordinates), and the imposed rest length of the spring.
    void Initialize(
        std::shared_ptr<ChBody> body1,  ///< first body to link
        std::shared_ptr<ChBody> body2,  ///< second body to link
        bool pos_are_relative,          ///< if true, point locations are relative to bodies
        ChVector<> loc1,                ///< point on 1st body (rel. or abs., see flag above)
        ChVector<> loc2,                ///< point on 2nd body (rel. or abs., see flag above)
        bool auto_rest_length = true,   ///< if true, initializes the rest length as the distance between loc1 and loc2
        double rest_length = 0          ///< rest length (no need to define if auto_rest_length=true.)
    );

    /// Method to allow serialization of transient data to archives.
    virtual void ArchiveOUT(ChArchiveOut& marchive) override;

    /// Method to allow deserialization of transient data from archives.
    virtual void ArchiveIN(ChArchiveIn& marchive) override;

  private:
    virtual void Update(double mytime, bool update_assets = true) override;

    virtual int GetDOF() override { return m_nstates; }

    // Interface to solver
    ChVariables& Variables() { return *m_variables; }
    virtual void InjectVariables(ChSystemDescriptor& descriptor) override;
    virtual void InjectKRMmatrices(ChSystemDescriptor& descriptor) override;

    virtual void IntStateGather(const unsigned int off_x,
                                ChState& x,
                                const unsigned int off_v,
                                ChStateDelta& v,
                                double& T) override;
    virtual void IntStateScatter(const unsigned int off_x,
                                 const ChState& x,
                                 const unsigned int off_v,
                                 const ChStateDelta& v,
                                 const double T) override;
    virtual void IntStateGatherAcceleration(const unsigned int off_a, ChStateDelta& a) override;
    virtual void IntStateScatterAcceleration(const unsigned int off_a, const ChStateDelta& a) override;
    virtual void IntLoadResidual_F(const unsigned int off, ChVectorDynamic<>& R, const double c) override;
    virtual void IntLoadResidual_Mv(const unsigned int off,
                                    ChVectorDynamic<>& R,
                                    const ChVectorDynamic<>& v,
                                    const double c) override;
    virtual void IntToDescriptor(const unsigned int off_v,
                                 const ChStateDelta& v,
                                 const ChVectorDynamic<>& R,
                                 const unsigned int off_L,
                                 const ChVectorDynamic<>& L,
                                 const ChVectorDynamic<>& Qc) override;
    virtual void IntFromDescriptor(const unsigned int off_v,
                                   ChStateDelta& v,
                                   const unsigned int off_L,
                                   ChVectorDynamic<>& L) override;

    virtual void KRMmatricesLoad(double Kfactor, double Rfactor, double Mfactor) override;

    // Interface to the solver (old style)
    virtual void VariablesFbReset() override;
    virtual void VariablesFbLoadForces(double factor = 1) override;
    virtual void VariablesQbLoadSpeed() override;
    virtual void VariablesQbSetSpeed(double step = 0) override;
    virtual void VariablesFbIncrementMq() override;
    virtual void VariablesQbIncrementPosition(double step) override;
    virtual void ConstraintsFbLoadForces(double factor = 1) override;

    /// Manager class for Jacobians matrices. \n
    /// - Variables associated with this link (order important): body1, body2, ODE states. \n
    /// - Generalized forces for link (order important): applied force on body1, applied force on body2, ODE RHS. \n
    /// The K and R blocks in m_KRM have the form: [A B], \n
    /// with A of size (12+nstates) x 12 and B of size (12+nstates) x nstates. \n
    /// These blocks are computed using finite-differences. However, if the user-provided ODE class implements
    /// CalculateJac, that will be used to override the bottom-right (nstates x nstates) block of R.
    class SpringJacobians {
      public:
        ChKblockGeneric m_KRM;  ///< linear combination of K, R, M for the variables associated with this link
        ChMatrixDynamic<> m_J;  ///< Jacobian of ODE right-hand side with respect to ODE states (contributes to R term)
        ChMatrixDynamic<> m_K;  ///< K contribution from this link
        ChMatrixDynamic<> m_R;  ///< R contribution from this link
    };

    /// Compute generalized forces Q given the packed states and state derivatives.
    void ComputeQ(double time,                  ///< current time
                  const ChState& state_x,       ///< state position to evaluate Q
                  const ChStateDelta& state_w,  ///< state speed to evaluate Q
                  ChVectorDynamic<>& Qforce     ///< output forcing vector
    );

    /// Create the Jacobian matrices, allocate space, associate variables.
    void CreateJacobianMatrices();

    /// Compute the Jacobian of the generalized forcing with respect to states of the two connected bodies and internal
    /// states (as needed).  Most of this information is computed using forward finite-differences.
    void ComputeJacobians(double time,                 ///< current time
                          const ChState& state_x,      ///< state position to evaluate jacobians
                          const ChStateDelta& state_w  ///< state speed to evaluate jacobians
    );

    ChVector<> m_loc1;     ///< location of end point on body1 (relative to body1)
    ChVector<> m_loc2;     ///< location of end point on body2 (relative to body1)
    ChVector<> m_aloc1;    ///< location of end point on body1 (absolute)
    ChVector<> m_aloc2;    ///< location of end point on body2 (absolute)
    double m_rest_length;  ///< undeformed length
    double m_length;       ///< current length
    double m_length_dt;    ///< current length rate of change

    bool m_stiff;  ///< true if loads are stiff (triggers Jacobian calculation)

    double m_k;  ///< spring coefficient (if no force functor provided)
    double m_r;  ///< damping coefficient (if no force functor provided)
    double m_f;  ///< constant actuation (if no force functor provided)

    ForceFunctor* m_force_fun;  ///< functor for force calculation
    double m_force;             ///< force in distance coordinates

    ODE* m_ode_fun;                               ///< functor for ODE specification
    int m_nstates;                                ///< number of internal ODE states
    ChVectorDynamic<> m_states;                   ///< vector of internal ODE states
    ChVariablesGenericDiagonalMass* m_variables;  ///< carrier for internal dynamics states

    ChVectorDynamic<> m_Qforce;    ///< generalized forcing terms
    SpringJacobians* m_jacobians;  ///< Jacobian information

    static const double m_FD_delta;  ///< perturbation for finite-difference Jacobian approximation

    friend class ChSystemParallel;
};

CH_CLASS_VERSION(ChLinkTSDA, 0)

}  // end namespace chrono

#endif
