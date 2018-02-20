#ifndef D4EST_SOLVER_FCG_H
#define D4EST_SOLVER_FCG_H 

#include <pXest.h>
#include <d4est_elliptic_data.h>
#include <d4est_elliptic_eqns.h>
#include <krylov_pc.h>

typedef struct {
  char input_section [50]; /* useful when using cg in multiple contexts */
  
  int monitor;
  int iter;
  int vi;
  int precond_flag;
  double rtol;
  double atol;

  
} d4est_solver_fcg_params_t;



/* This file was automatically generated.  Do not edit! */
void d4est_solver_fcg_solve(p4est_t *p4est,d4est_elliptic_data_t *vecs,d4est_elliptic_eqns_t *fcns,p4est_ghost_t **ghost,d4est_element_data_t **ghost_data,d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geom,d4est_quadrature_t *d4est_quad,d4est_mesh_data_t *d4est_factors,void *fcg_params,krylov_pc_t *krylov_pc);
void d4est_solver_fcg_input(p4est_t *p4est,const char *input_file,const char *input_section,const char *printf_prefix,d4est_solver_fcg_params_t *input);

#endif
