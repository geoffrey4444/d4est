#ifndef D4EST_ESTIMATOR_BI_H
#define D4EST_ESTIMATOR_BI_H 

#include "../EllipticSystem/problem_data.h"
#include "../EllipticSystem/problem_weakeqn_ptrs.h"

typedef struct {
  
  penalty_calc_t u_penalty_fcn;
  penalty_calc_t u_dirichlet_penalty_fcn;
  penalty_calc_t gradu_penalty_fcn;
  h_calc_method_t ip_flux_h_calc;
  double penalty_prefactor;
  
} d4est_estimator_bi_penalty_data_t;

inline static double
bi_gradu_prefactor_maxp_minh
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double max_p = util_max_int(deg_m, deg_p);
  double min_h = (h_m < h_p) ? h_m : h_p;
  return sqrt(min_h/max_p);
}

inline static double
bi_u_prefactor_conforming_maxp_minh
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double max_p = (double)util_max_int(deg_m, deg_p);
  double min_h = (h_m < h_p) ? h_m : h_p;
  return sqrt(penalty_prefactor*max_p*max_p/min_h);
}

inline static double
bi_gradu_prefactor_max_h_over_p
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double minus_h_over_p = h_m/(double)deg_m;
  double plus_h_over_p = h_p/(double)deg_p;

  double max_h_over_p = (minus_h_over_p > plus_h_over_p) ?
                        minus_h_over_p :
                        plus_h_over_p;
  
  return sqrt(max_h_over_p);
}


inline static double
bi_u_prefactor_conforming_max_p2_over_h
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double minus_p2_over_h = ((double)deg_m)*((double)deg_m)/h_m;
  double plus_p2_over_h = ((double)deg_p)*((double)deg_p)/h_p;
  double max_p2_over_h = (minus_p2_over_h > plus_p2_over_h) ?
                         minus_p2_over_h :
                         plus_p2_over_h;
  return sqrt(penalty_prefactor*max_p2_over_h);
}


inline static double
houston_gradu_prefactor_max_h_over_p
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double minus_h_over_p = h_m/(double)deg_m;
  double plus_h_over_p = h_p/(double)deg_p;

  double max_h_over_p = (minus_h_over_p > plus_h_over_p) ?
                        minus_h_over_p :
                        plus_h_over_p;
  
  return sqrt(.5*max_h_over_p);
}


inline static double
houston_u_prefactor_max_p2_over_h
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double minus_p2_over_h = ((double)deg_m)*((double)deg_m)/h_m;
  double plus_p2_over_h = ((double)deg_p)*((double)deg_p)/h_p;
  double max_p2_over_h = (minus_p2_over_h > plus_p2_over_h) ?
                         minus_p2_over_h :
                         plus_p2_over_h;
  return sqrt(.5*penalty_prefactor*max_p2_over_h);
}

inline static double
houston_u_dirichlet_prefactor_max_p2_over_h
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double minus_p2_over_h = ((double)deg_m)*((double)deg_m)/h_m;
  double plus_p2_over_h = ((double)deg_p)*((double)deg_p)/h_p;
  double max_p2_over_h = (minus_p2_over_h > plus_p2_over_h) ?
                         minus_p2_over_h :
                         plus_p2_over_h;
  return sqrt(penalty_prefactor*max_p2_over_h);
}



inline static double
houston_gradu_prefactor_maxp_minh
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double max_p = util_max_int(deg_m, deg_p);
  double min_h = (h_m < h_p) ? h_m : h_p;
  return sqrt(.5*min_h/max_p);
}

inline static double
houston_u_prefactor_maxp_minh
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double max_p = (double)util_max_int(deg_m, deg_p);
  double min_h = (h_m < h_p) ? h_m : h_p;
  return sqrt(.5*penalty_prefactor*max_p*max_p/min_h);
}

inline static double
houston_u_dirichlet_prefactor_maxp_minh
(
 int deg_m,
 double h_m,
 int deg_p,
 double h_p,
 double penalty_prefactor
)
{
  double max_p = (double)util_max_int(deg_m, deg_p);
  double min_h = (h_m < h_p) ? h_m : h_p;
  return sqrt(penalty_prefactor*max_p*max_p/min_h);
}

/* This file was automatically generated.  Do not edit! */
curved_flux_fcn_ptrs_t d4est_estimator_bi_dirichlet_fetch_fcns(grid_fcn_t bndry_fcn,d4est_estimator_bi_penalty_data_t *penalty_data);
void d4est_estimator_bi_compute(p4est_t *p4est,problem_data_t *vecs,weakeqn_ptrs_t *fcns,d4est_estimator_bi_penalty_data_t penalty_data,grid_fcn_t u_bndry_fcn,p4est_ghost_t *ghost,d4est_element_data_t *ghost_data,d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geom,d4est_quadrature_t *d4est_quad,double(*get_diam)(d4est_element_data_t *));


#endif