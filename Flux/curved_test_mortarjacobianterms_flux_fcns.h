#ifndef CURVED_TEST_MORTARJACOBIANTERMS_FLUX_FCNS_H
#define CURVED_TEST_MORTARJACOBIANTERMS_FLUX_FCNS_H 

#include <curved_compute_flux.h>
#include <d4est_operators.h>

typedef struct {
  
  double* u;
  double global_err;
  double local_eps;
  d4est_operators_t* d4est_ops;
  
} test_mortarjacobianterms_data_t;

void curved_test_mortarjacobianterms_init_vecs
(
 p4est_iter_volume_info_t * info,
 void *user_data
);

curved_flux_fcn_ptrs_t
curved_test_mortarjacobianterms_fetch_fcns
(
 test_mortarjacobianterms_data_t* data
);

#endif
