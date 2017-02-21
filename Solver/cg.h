#ifndef CG_H
#define CG_H 

#include <problem_data.h>
#include <problem_weakeqn_ptrs.h>

typedef struct {

  int max_iter;
  int mpi_rank;
  int monitor;
  double rtol;
  double atol;

  int final_iter;
  double final_fnrm;
  
} cg_solver_params_t;

void
cg_nr_solve
(
 p4est_t* p4est,
 problem_data_t* vecs,
 weakeqn_ptrs_t* fcns,
 dgmath_jit_dbase_t* dgmath_jit_dbase,
 p4est_ghost_t* ghost,
 element_data_t* ghost_data,
 double eta_max,
 double fnrm,
 int max_iter,
 int mpi_rank,
 int* final_iter,
 double* final_fnrm
);

void
cg_solve
(
 p4est_t* p4est,
 problem_data_t* vecs,
 weakeqn_ptrs_t* fcns,
 dgmath_jit_dbase_t* dgbase,
 p4est_ghost_t* ghost,
 element_data_t* ghost_data,
 cg_solver_params_t* params
);

void curved_cg_solve
(
 p4est_t* p4est,
 problem_data_t* vecs,
 curved_weakeqn_ptrs_t* fcns,
 dgmath_jit_dbase_t* dgbase,
 d4est_geometry_t* geom,
 p4est_ghost_t* ghost,
 curved_element_data_t* ghost_data,
 cg_solver_params_t* params
);

#endif