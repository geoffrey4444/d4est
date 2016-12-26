#include "../Operators/grad.h"
#include "../pXest/pXest.h"
#include "../dGMath/dgmath.h"
#include "../LinearAlgebra/linalg.h"


void
grad(double*u, double* gradu [(P4EST_DIM)], double h, int deg, dgmath_jit_dbase_t* dgmath_jit_dbase)
{
  int vol_nodes = dgmath_get_nodes((P4EST_DIM),deg);
  int i;
  for (i = 0; i < (P4EST_DIM); i++){
    dgmath_apply_Dij(
                     dgmath_jit_dbase,
                     u,
                     (P4EST_DIM),
                     deg,
                     i,
                     gradu[i]
                    );
    linalg_vec_scale(2./h, gradu[i], vol_nodes);
  }
}

void
grad_euc_norm(double* u, double* gradu_norm,  double h, int deg, dgmath_jit_dbase_t* dgmath_jit_dbase)
{
  int vol_nodes = dgmath_get_nodes((P4EST_DIM),deg);

  double* gradu [(P4EST_DIM)];
  int i,j;
  for (i = 0; i < (P4EST_DIM); i++)
    gradu[i] = P4EST_ALLOC(double, vol_nodes);

  grad(u, gradu, h, deg, dgmath_jit_dbase);
  
  for (i = 0; i < vol_nodes; i++){
    gradu_norm[i] = 0.;
    for (j = 0; j < (P4EST_DIM); j++){
      gradu_norm[i] += gradu[j][i]*gradu[j][i];
    }
    gradu_norm[i] = sqrt(gradu_norm[i]);
  }

  for (i = 0; i < (P4EST_DIM); i++)
    P4EST_FREE(gradu[i]);
}