#ifndef D4EST_QUAD_H
#define D4EST_QUAD_H 

#include <d4est_geometry.h>

/**
 * @file   d4est_quad.h
 * @author Trevor Vincent <tvincent@cita.utoronto.ca>
 * @date   Sun May 21 18:44:59 2017
 * 
 * @brief  This module is for Gaussian quadrature. 
 * d4est have built-in quadrature
 * types for Gauss-Legendre, Gauss-Legendre-Lobatto, and
 * a custom set for compactified intervals: "Gauss-Legendre-Compactified"
 * for special use with compactified cubed spheres
 * 
 */

typedef struct d4est_quadrature d4est_quadrature_t;

typedef enum {QUAD_GAUSS_LEGENDRE,
              QUAD_GAUSS_LEGENDRE_LOBATTO,
              QUAD_GAUSS_LEGENDRE_COMPACTIFIED,
              QUAD_NOT_SET} d4est_quadrature_type_t; 

typedef enum {QUAD_MORTAR,
              QUAD_VOLUME} d4est_quadrature_object_type_t;

typedef enum
  {
    QUAD_JAC_TIMES_POLY_INTEGRAND,
    QUAD_DRDX_JAC_DRDX_TIMES_POLY_INTEGRAND,
    QUAD_UNKNOWN_INTEGRAND
  } d4est_quadrature_integrand_type_t;

typedef struct {

  p4est_qcoord_t q [(P4EST_DIM)];
  p4est_qcoord_t dq;
  p4est_topidx_t tree;
  p4est_topidx_t face; 
  
  /* mortar attributes */
  int mortar_side_id;
  int mortar_subface_id;
  
} d4est_quadrature_mortar_t;

typedef struct {

  p4est_qcoord_t q [(P4EST_DIM)];
  p4est_qcoord_t dq;
  p4est_topidx_t tree;
  int element_id;
  
} d4est_quadrature_volume_t;


typedef void
(*d4est_quadrature_reinit_fcn_t)
(
 p4est_t* p4est,
 p4est_ghost_t* ghost,
 void* ghost_data,
 d4est_operators_t* d4est_ops,
 d4est_geometry_t* d4est_geom,
 d4est_quadrature_t* d4est_quad
);


typedef void
(*d4est_quadrature_destroy_fcn_t)
(
 p4est_t*,
 d4est_operators_t*,
 d4est_geometry_t*,
 d4est_quadrature_t*
);


typedef double*
(*d4est_quadrature_getter_fcn_t)
(
 d4est_operators_t*,
 d4est_geometry_t*,
 d4est_quadrature_t*,
 void*,
 d4est_quadrature_object_type_t,
 d4est_quadrature_integrand_type_t,
 int,
 int
);

typedef  double*
(*d4est_quadrature_getter_fcn_ext_t)
(
 d4est_operators_t*,
 d4est_geometry_t*,
 d4est_quadrature_t*,
 void*,
 d4est_quadrature_object_type_t,
 d4est_quadrature_integrand_type_t,
 int,
 int,
 int
);

struct d4est_quadrature {

  d4est_quadrature_type_t quad_type;
  
  d4est_quadrature_getter_fcn_t get_weights;
  d4est_quadrature_getter_fcn_t get_rst;
  d4est_quadrature_getter_fcn_ext_t get_interp;
  d4est_quadrature_getter_fcn_ext_t get_interp_trans;

  void* user;
  d4est_quadrature_destroy_fcn_t user_destroy;
  d4est_quadrature_reinit_fcn_t user_reinit;
};

/* This file was automatically generated.  Do not edit! */
double d4est_quadrature_innerproduct(d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geom,d4est_quadrature_t *d4est_quad,void *object,d4est_quadrature_object_type_t object_type,d4est_quadrature_integrand_type_t integrand_type,double *u,double *v,double *jac_quad,int deg_quad);
void d4est_quadrature_interpolate(d4est_operators_t *d4est_ops,d4est_quadrature_t *d4est_quadrature,d4est_geometry_t *d4est_geometry,void *object,d4est_quadrature_object_type_t object_type,d4est_quadrature_integrand_type_t integrand_type,double *u_lobatto_in,int deg_lobatto,double *u_quad_out,int deg_quad);
d4est_rst_t d4est_quadrature_get_rst_points(d4est_operators_t *d4est_ops,d4est_quadrature_t *d4est_quadrature,d4est_geometry_t *d4est_geometry,void *object,d4est_quadrature_object_type_t object_type,d4est_quadrature_integrand_type_t integrand_type,int degree);
void d4est_quadrature_apply_mass_matrix(d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geometry,d4est_quadrature_t *d4est_quadrature,void *object,d4est_quadrature_object_type_t object_type,d4est_quadrature_integrand_type_t integrand_type,double *in,int deg_lobatto,double *jac_quad,int deg_quad,double *out);
void d4est_quadrature_apply_stiffness_matrix(d4est_operators_t *d4est_ops,d4est_quadrature_t *d4est_quadrature,d4est_geometry_t *d4est_geometry,void *object,d4est_quadrature_object_type_t object_type,d4est_quadrature_integrand_type_t integrand_type,double *in,int deg_lobatto,double *jac_quad,double *rst_xyz[(P4EST_DIM)][(P4EST_DIM)],int deg_quad,double *out);
void d4est_quadrature_apply_galerkin_integral(d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geometry,d4est_quadrature_t *d4est_quadrature,void *object,d4est_quadrature_object_type_t object_type,d4est_quadrature_integrand_type_t integrand_type,double *in_quad,int deg_lobatto,double *jac_quad,int deg_quad,double *out);
void d4est_quadrature_destroy(p4est_t *p4est,d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geom,d4est_quadrature_t *d4est_quad);
void d4est_quadrature_reinit(p4est_t *p4est,d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geom,d4est_quadrature_t *d4est_quad);
d4est_quadrature_t *d4est_quadrature_new(p4est_t *p4est,d4est_operators_t *d4est_ops,d4est_geometry_t *d4est_geom,const char *input_file,const char *input_section,const char *printf_prefix);


#endif
