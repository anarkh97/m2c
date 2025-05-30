/************************************************************************
 * Copyright © 2020 The Multiphysics Modeling and Computation (M2C) Lab
 * <kevin.wgy@gmail.com> <kevinw3@vt.edu>
 ************************************************************************/

#ifndef _LINEAR_SYSTEM_SOLVER_H_
#define _LINEAR_SYSTEM_SOLVER_H_
#include<petscksp.h>
#include<LinearOperator.h>

/*********************************************************************
 * class LinearSystemSolver is responsible for solving large-scale linear
 * systems Ax = b, where x and b are SpaceVariable3D. The actual work is
 * done by PETSc. In some sense, LinearSystemSolver is just
 * a wrapper over PETSc/KSP. 
 *********************************************************************
*/

enum LinearSolverConvergenceReason 
         {NONE = 0, CONVERGED_REL_TOL = 1, CONVERGED_ABS_TOL = 2, CONVERGED_OTHER = 3,
          DIVERGED_ITS = 4, DIVERGED_DTOL = 5, DIVERGED_OTHER = 6};

class LinearSystemSolver : public LinearOperator {

  KSP ksp;

  std::vector<double> rnorm_history; //!< stores the history of residual norm for the "Solve"

  bool write_log_to_screen;
  string log_filename;
  string equation_name; //!< optional (used in log file only)

  SpaceVariable3D Xtmp, Rtmp; //!< for temporary use

public:

  LinearSystemSolver(MPI_Comm &comm_, DM &dm_, LinearSolverData &lin_input, const char *equation_name_ = "");
  ~LinearSystemSolver(); 
  void Destroy();

  void SetLinearOperator(std::vector<RowEntries>& row_entries);

  /** By default, every time "Solve" is called, the solver rebuilds the preconditioner. But if one solves \n
   *  multiple linear systems with the same operator (A) but different RHS (b), the same preconditioner can \n
   *  be reused. In this case, one may call this function after the first "Solve". */
  void UsePreviousPreconditioner(bool reuse_or_not = true);

  bool Solve(SpaceVariable3D &b, SpaceVariable3D &x, //!< x: both input (initial guess) & output (solution)
             LinearSolverConvergenceReason *reason = NULL, int *numIts = NULL,
             std::vector<double> *rnorm = NULL, std::vector<int> *rnorm_its = NULL);

  void ConvergedDefaultSetUMIRNorm(); //!< modifies the initial residual norm

  void GetTolerances(double *rtol, double *abstol, double *dtol, int *maxits); //!< set NULL to params not needed

  void SetTolerances(double relative_error, double absolute_error, double divergence_tol, int max_iterations);

  void GetSolverType(string *ksp_type, string *pc_type = NULL); //!< if only one is needed, use NULL for the other

  void ComputeResidual(SpaceVariable3D &b, SpaceVariable3D &x, SpaceVariable3D &res); //!< res = b-Ax

private:

  void SetTolerancesInput(LinearSolverData &lin_input);

};

#endif
