#ifndef _TIME_INTEGRATOR_H_
#define _TIME_INTEGRATOR_H_

#include <SpaceOperator.h>
#include <LevelSetOperator.h>
#include <MultiPhaseOperator.h>
#include <LaserAbsorptionSolver.h>
#include <RiemannSolutions.h>
#include <EmbeddedBoundaryOperator.h>
#include <HyperelasticityOperator.h>
using std::vector;

/********************************************************************
 * Numerical time-integrator: The base class
 *******************************************************************/
class TimeIntegratorBase
{
protected:
  MPI_Comm&       comm;
  IoData&         iod;
  SpaceOperator&  spo;

  vector<LevelSetOperator*>& lso;
  MultiPhaseOperator& mpo;  

  SpaceVariable3D& volume;

  int i0, j0, k0, imax, jmax, kmax;
  int ii0, jj0, kk0, iimax, jjmax, kkmax;

  //! Laser absorption solver (NULL if laser is not activated)
  LaserAbsorptionSolver* laser;

  //! Embedded boundary method (NULL if not activated)
  EmbeddedBoundaryOperator* embed;

  //! Hyperelaticity operator (NULL if not activated)
  HyperelasticityOperator* heo;

  //!< Internal variable to temporarily store old ID
  SpaceVariable3D IDn;

  //!< Internal variable to temporarily store Phi (e.g., for material ID updates)
  vector<SpaceVariable3D*> Phi_tmp;

  //!< Internal variable to store the mat. ID tracked by each level set
  vector<int> ls_mat_id;

  //!< Solutions of exact Riemann problems
  RiemannSolutions riemann_solutions;

public:
  TimeIntegratorBase(MPI_Comm &comm_, IoData& iod_, DataManagers3D& dms_, SpaceOperator& spo_, 
                     vector<LevelSetOperator*>& lso_, MultiPhaseOperator& mpo_,
                     LaserAbsorptionSolver* laser_, EmbeddedBoundaryOperator* embed_,
                     HyperelasticityOperator* heo_);

  virtual ~TimeIntegratorBase();

  // Integrate the ODE system for one time-step. Implemented in derived classes
  virtual void AdvanceOneTimeStep(SpaceVariable3D &V, SpaceVariable3D &ID, 
                                  vector<SpaceVariable3D*> &Phi,
                                  SpaceVariable3D *L, SpaceVariable3D *Xi, double time,
                                  double dt, int time_step, int subcycle, double dts) {
    print_error("*** Error: AdvanceOneTimeStep function not defined.\n");
    exit_mpi();}

  virtual void Destroy();

  // all the tasks that are done at the end of a time-step, independent of 
  // time integrator
  void UpdateSolutionAfterTimeStepping(SpaceVariable3D &V, SpaceVariable3D &ID,
                                       vector<SpaceVariable3D*> &Phi,
                                       vector<std::unique_ptr<EmbeddedBoundaryDataSet> > *EBDS,
                                       SpaceVariable3D *L,
                                       double time, int time_step, int subcycle, double dts);
                                        
};

/********************************************************************
 * Numerical time-integrator: Forward Euler (TVD)
 *******************************************************************/
class TimeIntegratorFE : public TimeIntegratorBase
{
  //! conservative state variable at time n
  SpaceVariable3D Un;

  //! "residual", i.e. the right-hand-side of the ODE
  SpaceVariable3D Rn;  
  vector<SpaceVariable3D*> Rn_ls;
  SpaceVariable3D *Rn_xi;

public:
  TimeIntegratorFE(MPI_Comm &comm_, IoData& iod_, DataManagers3D& dms_, SpaceOperator& spo_,
                   vector<LevelSetOperator*>& lso_, MultiPhaseOperator &mpo_,
                   LaserAbsorptionSolver* laser_, EmbeddedBoundaryOperator* embed_,
                   HyperelasticityOperator* heo_);
  ~TimeIntegratorFE();

  void AdvanceOneTimeStep(SpaceVariable3D &V, SpaceVariable3D &ID, 
                          vector<SpaceVariable3D*>& Phi,
                          SpaceVariable3D *L, SpaceVariable3D *Xi, double time,
                          double dt, int time_step, int subcycle, double dts);

  void Destroy();

};

/********************************************************************
 * Numerical time-integrator: 2nd-order Runge-Kutta (Heun's method, TVD)
 *******************************************************************/
class TimeIntegratorRK2 : public TimeIntegratorBase
{
  //! conservative state variable at time n
  SpaceVariable3D Un;
  //! intermediate state
  SpaceVariable3D U1;
  SpaceVariable3D V1;
  //! "residual", i.e. the right-hand-side of the ODE
  SpaceVariable3D R;  

  //! level set variables
  vector<SpaceVariable3D*> Phi1; 
  vector<SpaceVariable3D*> Rls; 

  //! reference map equation
  SpaceVariable3D* Xi1;
  SpaceVariable3D* Rxi;

public:
  TimeIntegratorRK2(MPI_Comm &comm_, IoData& iod_, DataManagers3D& dms_, SpaceOperator& spo_,
                    vector<LevelSetOperator*>& lso_, MultiPhaseOperator &mpo_,
                    LaserAbsorptionSolver* laser_, EmbeddedBoundaryOperator* embed_,
                    HyperelasticityOperator* heo_);
  ~TimeIntegratorRK2();

  void AdvanceOneTimeStep(SpaceVariable3D &V, SpaceVariable3D &ID,
                          vector<SpaceVariable3D*>& Phi,
                          SpaceVariable3D *L, SpaceVariable3D *Xi, double time,
                          double dt, int time_step, int subcycle, double dts);

  void Destroy(); 

};

/********************************************************************
 * Numerical time-integrator: 3rd-order Runge-Kutta (Gottlieb & Shu, TVD)
 *******************************************************************/
class TimeIntegratorRK3 : public TimeIntegratorBase
{
  //! conservative state variable at time n
  SpaceVariable3D Un;
  //! intermediate state
  SpaceVariable3D U1;
  SpaceVariable3D V1;
  SpaceVariable3D V2;
  //! "residual", i.e. the right-hand-side of the ODE
  SpaceVariable3D R;  

  //!level set variables
  vector<SpaceVariable3D*> Phi1;
  vector<SpaceVariable3D*> Rls;

  //! reference map equation
  SpaceVariable3D* Xi1;
  SpaceVariable3D* Rxi;

public:
  TimeIntegratorRK3(MPI_Comm &comm_, IoData& iod_, DataManagers3D& dms_, SpaceOperator& spo_,
                    vector<LevelSetOperator*>& lso_, MultiPhaseOperator &mpo_,
                    LaserAbsorptionSolver* laser_, EmbeddedBoundaryOperator* embed_,
                    HyperelasticityOperator* heo_);
  ~TimeIntegratorRK3();

  void AdvanceOneTimeStep(SpaceVariable3D &V, SpaceVariable3D &ID,
                          vector<SpaceVariable3D*>& Phi,
                          SpaceVariable3D *L, SpaceVariable3D *Xi, double time,
                          double dt, int time_step, int subcycle, double dts);

  void Destroy(); 

};

//----------------------------------------------------------------------

#endif
