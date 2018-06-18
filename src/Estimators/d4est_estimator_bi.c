#include <pXest.h>
#include <d4est_util.h>
#include <d4est_elliptic_data.h>
#include <d4est_elliptic_eqns.h>
#include <d4est_element_data.h>
#include <d4est_linalg.h>
#include <d4est_mesh.h>
#include <d4est_estimator_bi.h>
#include <d4est_poisson_flux.h>
#include <d4est_mortars.h>
#include <d4est_poisson.h>

static double
d4est_estimator_get_diam
(
 d4est_mesh_data_t* d4est_factors,
 d4est_element_data_t* ed,
 diam_compute_option_t diam_opt
)
{
  d4est_mesh_data_on_element_t md_on_e = d4est_mesh_data_on_element
                                         (
                                          d4est_factors,
                                          ed
                                         );
  
  return d4est_geometry_compute_diam(md_on_e.xyz,ed->deg,diam_opt); 
}

static void
d4est_estimator_bi_dirichlet
(
 p4est_t* p4est,
 d4est_element_data_t* e_m,
 int f_m,
 int mortar_side_id_m,
 d4est_operators_t* d4est_ops,
 d4est_geometry_t* d4est_geom,
 d4est_quadrature_t* d4est_quad,
 d4est_mesh_data_t* d4est_factors,
 d4est_poisson_flux_boundary_data_t* boundary_data,
 void* boundary_condition_fcn_data,
 void* flux_parameter_data
)
{

  
  d4est_poisson_dirichlet_bc_t* bc_data = boundary_condition_fcn_data;
  d4est_estimator_bi_penalty_data_t* penalty_data = flux_parameter_data;
  double* estimator = penalty_data->estimator;
  d4est_quadrature_mortar_t* face_object = boundary_data->face_object;
  int face_nodes_m_lobatto = boundary_data->face_nodes_m_lobatto;
  int face_nodes_m_quad = boundary_data->face_nodes_m_quad;
  int deg_mortar_quad = boundary_data->deg_mortar_quad;

  double* u_m_on_f_m_quad = boundary_data->u_m_on_f_m_quad;
  double* sj_on_f_m_quad = boundary_data->sj_on_f_m_quad;
  double* j_div_sj_quad = boundary_data->j_div_sj_quad;
  double* xyz_on_f_m_lobatto [(P4EST_DIM)]; 
  double* drst_dxyz_quad [(P4EST_DIM)][(P4EST_DIM)];
  double* dudx_m_on_f_m_quad [(P4EST_DIM)];  
  double* n_on_f_m_quad [(P4EST_DIM)];
  D4EST_COPY_DBYD_MAT(boundary_data->drst_dxyz_quad, drst_dxyz_quad);
  D4EST_COPY_DIM_VEC(boundary_data->dudx_m_on_f_m_quad, dudx_m_on_f_m_quad);
  D4EST_COPY_DIM_VEC(boundary_data->n_on_f_m_quad, n_on_f_m_quad);
  D4EST_COPY_DIM_VEC(boundary_data->xyz_on_f_m_lobatto, xyz_on_f_m_lobatto); 
  double* Je2_prefactor = P4EST_ALLOC(double, face_nodes_m_quad);
  double* Je2 = P4EST_ALLOC(double, face_nodes_m_quad);

  double* h_quad = P4EST_ALLOC(double, face_nodes_m_quad);
  d4est_poisson_flux_sipg_calculate_h
    (
     p4est,
     &e_m,
     f_m,
     d4est_ops,
     d4est_geom,
     d4est_quad,
     penalty_data->sipg_flux_h,
     j_div_sj_quad,
     h_quad,
     1,
     1,
     &face_nodes_m_quad,
     (penalty_data->size_params == NULL) ? d4est_mesh_get_size_parameters(d4est_factors) : *penalty_data->size_params
    );
  
  for (int i = 0; i < face_nodes_m_quad; i++){
    Je2_prefactor[i] = penalty_data->u_dirichlet_penalty_fcn
                       (
                        e_m->deg,
                        h_quad[i],
                        e_m->deg,
                        h_quad[i],
                        penalty_data->penalty_prefactor
                       );
  }

  P4EST_FREE(h_quad);

  double* u_at_bndry_lobatto = P4EST_ALLOC(double, face_nodes_m_lobatto);
  double* u_at_bndry_lobatto_to_quad = P4EST_ALLOC(double, face_nodes_m_quad);
  
  for (int i = 0; i < face_nodes_m_lobatto; i++){
    u_at_bndry_lobatto[i] = bc_data->dirichlet_fcn
                            (
                             boundary_data->xyz_on_f_m_lobatto[0][i],
                             boundary_data->xyz_on_f_m_lobatto[1][i],
#if (P4EST_DIM)==3
                             boundary_data->xyz_on_f_m_lobatto[2][i],
#endif
                             penalty_data->user
                            );
  }


  d4est_quadrature_interpolate
    (
     d4est_ops,
     d4est_quad,
     d4est_geom,
     &face_object,
     QUAD_OBJECT_MORTAR,
     QUAD_INTEGRAND_UNKNOWN,
     u_at_bndry_lobatto,
     e_m->deg,
     u_at_bndry_lobatto_to_quad,
     deg_mortar_quad
    );

  
  for (int dim = 0; dim < (P4EST_DIM); dim++){

    for(int i = 0; i < face_nodes_m_quad; i++){
      double u_m_on_f_m_min_u_at_bndry_quad = u_m_on_f_m_quad[i] - u_at_bndry_lobatto_to_quad[i];    
      Je2[i] = n_on_f_m_quad[dim][i]*Je2_prefactor[i]*(u_m_on_f_m_min_u_at_bndry_quad);
    }
    
    double Je2MJe2 = d4est_quadrature_innerproduct
                     (
                      d4est_ops,
                      d4est_geom,
                      d4est_quad,
                      &face_object,
                      QUAD_OBJECT_MORTAR,
                      QUAD_INTEGRAND_UNKNOWN,
                      Je2,
                      Je2,
                      sj_on_f_m_quad,
                      deg_mortar_quad
                     );


    /* e_m->local_estimator += Je2MJe2; */
    estimator[e_m->id] += Je2MJe2;

  }


  P4EST_FREE(u_at_bndry_lobatto);
  P4EST_FREE(u_at_bndry_lobatto_to_quad);
  P4EST_FREE(Je2_prefactor);
  P4EST_FREE(Je2);
}

static void
d4est_estimator_bi_interface
(
 p4est_t* p4est,
 d4est_element_data_t** e_m,
 int faces_m,
 int f_m,
 int mortar_side_id_m,
 d4est_element_data_t** e_p,
 int faces_p,
 int f_p,
 int mortar_side_id_p,
 int* e_m_is_ghost,
 int orientation,
 d4est_operators_t* d4est_ops,
 d4est_geometry_t* d4est_geom,
 d4est_quadrature_t* d4est_quad,
 d4est_mesh_data_t* d4est_factors,
 d4est_poisson_flux_interface_data_t* interface_data,
 void* params
)
{
  d4est_quadrature_mortar_t* mortar_face_object = interface_data->mortar_face_object;
  
  int faces_mortar = interface_data->faces_mortar;
  int total_side_nodes_m_lobatto = interface_data->total_side_nodes_m_lobatto;
  int total_nodes_mortar_lobatto = interface_data->total_nodes_mortar_lobatto;
  int total_nodes_mortar_quad = interface_data->total_nodes_mortar_quad;
  
  double* u_m_on_f_m_mortar_quad = interface_data->u_m_on_f_m_mortar_quad;
  double* sj_on_f_m_mortar_quad = interface_data->sj_on_f_m_mortar_quad;
  double* j_div_sj_on_f_m_mortar_quad = interface_data->j_div_sj_on_f_m_mortar_quad;
  double* u_p_on_f_p_mortar_quad = interface_data->u_p_on_f_p_mortar_quad;
  double* j_div_sj_on_f_p_mortar_quad = interface_data->j_div_sj_on_f_p_mortar_quad;
  
  double* drst_dxyz_m_on_mortar_quad [(P4EST_DIM)][(P4EST_DIM)];
  double* dudx_m_on_f_m_mortar_quad [(P4EST_DIM)];
  double* dudx_p_on_f_p_mortar_quad [(P4EST_DIM)];
  double* n_on_f_m_mortar_quad [(P4EST_DIM)];

  D4EST_COPY_DBYD_MAT(interface_data->drst_dxyz_m_on_mortar_quad, drst_dxyz_m_on_mortar_quad);
  D4EST_COPY_DIM_VEC(interface_data->dudx_m_on_f_m_mortar_quad, dudx_m_on_f_m_mortar_quad);
  D4EST_COPY_DIM_VEC(interface_data->dudx_p_on_f_p_mortar_quad, dudx_p_on_f_p_mortar_quad);
  D4EST_COPY_DIM_VEC(interface_data->n_on_f_m_mortar_quad, n_on_f_m_mortar_quad);
  
  int* deg_mortar_quad = interface_data->deg_mortar_quad;
  int* nodes_mortar_quad = interface_data->nodes_mortar_quad;
  int* nodes_mortar_lobatto = interface_data->nodes_mortar_lobatto;
  int* deg_mortar_lobatto = interface_data->deg_mortar_lobatto;
  int* face_nodes_m_lobatto = interface_data->deg_mortar_lobatto;
  int* deg_m_lobatto = interface_data->deg_m_lobatto;
  int* deg_p_lobatto = interface_data->deg_p_lobatto;
  
  d4est_estimator_bi_penalty_data_t* penalty_data = params;
  double* estimator = penalty_data->estimator;
   double* hm_mortar_quad = P4EST_ALLOC(double, total_nodes_mortar_quad);
  double* hp_mortar_quad = P4EST_ALLOC(double, total_nodes_mortar_quad);

  d4est_poisson_flux_sipg_calculate_h
    (
     p4est,
     e_m,
     f_m,
     d4est_ops,
     d4est_geom,
     d4est_quad,
     penalty_data->sipg_flux_h,
     j_div_sj_on_f_m_mortar_quad,
     hm_mortar_quad,
     interface_data->faces_mortar,
     faces_m,
     nodes_mortar_quad,
     (penalty_data->size_params == NULL) ? d4est_mesh_get_size_parameters(d4est_factors) : *penalty_data->size_params
    );


  d4est_element_data_t* e_p_oriented [(P4EST_HALF)];
 d4est_element_data_reorient_f_p_elements_to_f_m_order(e_p, (P4EST_DIM)-1, f_m, f_p, orientation, faces_p, e_p_oriented);
  
    d4est_poisson_flux_sipg_calculate_h
    (
     p4est,
     &e_p_oriented[0],
     f_p,
     d4est_ops,
     d4est_geom,
     d4est_quad,
     penalty_data->sipg_flux_h,
     j_div_sj_on_f_p_mortar_quad,
     hp_mortar_quad,
     interface_data->faces_mortar,
     faces_p,
     nodes_mortar_quad,
     (penalty_data->size_params == NULL) ? d4est_mesh_get_size_parameters(d4est_factors) : *penalty_data->size_params
    );

  double* Je1_prefactor = P4EST_ALLOC(double, total_nodes_mortar_quad);
  double* Je1 = P4EST_ALLOC(double, total_nodes_mortar_quad);
  double* Je2_prefactor = P4EST_ALLOC(double, total_nodes_mortar_quad);
  double* Je2 [(P4EST_DIM)]; D4EST_ALLOC_DIM_VEC(Je2, total_nodes_mortar_quad);
  
  int stride = 0;
  for (int f = 0; f < faces_mortar; f++){
    for (int k = 0; k < nodes_mortar_quad[f]; k++){
      int ks = k + stride;
      
      Je1_prefactor[ks] = penalty_data->gradu_penalty_fcn
                          (
                           (faces_m == faces_mortar) ? deg_m_lobatto[f] : deg_m_lobatto[0],
                           hm_mortar_quad[ks],
                           (faces_p == faces_mortar) ? deg_p_lobatto[f] : deg_p_lobatto[0],
                           hp_mortar_quad[ks],
                           penalty_data->penalty_prefactor
                          ); 

      Je2_prefactor[ks] = penalty_data->u_penalty_fcn
                          (
                           (faces_m == faces_mortar) ? deg_m_lobatto[f] : deg_m_lobatto[0],
                           hm_mortar_quad[ks],
                           (faces_p == faces_mortar) ? deg_p_lobatto[f] : deg_p_lobatto[0],
                           hp_mortar_quad[ks],
                           penalty_data->penalty_prefactor
                          );    
      
    }
    stride += nodes_mortar_quad[f];
  }

  P4EST_FREE(hm_mortar_quad);
  P4EST_FREE(hp_mortar_quad);  

  /* calculate symmetric interior penalty flux */
  int k;
  int f;
  int ks;
  double n_ks;
  double sj_ks;
  stride = 0;
  for (f = 0; f < faces_mortar; f++){
    for (k = 0; k < nodes_mortar_quad[f]; k++){
      ks = k + stride;
      sj_ks = sj_on_f_m_mortar_quad[ks];
      Je1[ks] = 0.;
      for (int d = 0; d < (P4EST_DIM); d++){
        n_ks = n_on_f_m_mortar_quad[d][ks];        
        Je1[ks] += Je1_prefactor[ks]*n_ks*
                   (dudx_m_on_f_m_mortar_quad[d][ks] - dudx_p_on_f_p_mortar_quad[d][ks]);
        Je2[d][ks] = n_ks*u_m_on_f_m_mortar_quad[ks];
        Je2[d][ks] -= n_ks*u_p_on_f_p_mortar_quad[ks];
        Je2[d][ks] *= Je2_prefactor[ks];
      }
    }
    stride += nodes_mortar_quad[f];
  }


    
  /* the contribution in every direction must be added up due to it being a vector norm */
  stride = 0;
  for (f = 0; f < faces_mortar; f++){
    for (int d = 0; d < (P4EST_DIM); d++){

      double Je2MJe2 = d4est_quadrature_innerproduct
                       (
                        d4est_ops,
                        d4est_geom,
                        d4est_quad,
                        mortar_face_object,
                        QUAD_OBJECT_MORTAR,
                        QUAD_INTEGRAND_UNKNOWN,
                        &Je2[d][stride],
                        &Je2[d][stride],
                        &sj_on_f_m_mortar_quad[stride],
                        deg_mortar_quad[f]
                       );

      
      /* even if it's a ghost we can still add it to the ghosts estimator because we will not send it back! */

      if(faces_m == (P4EST_HALF)){
        /* e_m[f]->local_estimator += Je2MJe2; */
        estimator[e_m[f]->id] += Je2MJe2;
      }
      else{
        /* e_m[0]->local_estimator += Je2MJe2; */
        estimator[e_m[0]->id] += Je2MJe2;
      }
    }
    stride += nodes_mortar_quad[f];
  }


    
  stride = 0;
  for (f = 0; f < faces_mortar; f++){  

    double Je1MJe1 = d4est_quadrature_innerproduct
                     (
                      d4est_ops,
                      d4est_geom,
                      d4est_quad,
                      mortar_face_object,
                      QUAD_OBJECT_MORTAR,
                      QUAD_INTEGRAND_UNKNOWN,
                      &Je1[stride],
                      &Je1[stride],
                      &sj_on_f_m_mortar_quad[stride],
                      deg_mortar_quad[f]
                     );
    
   
    /* even if it's a ghost we can still add it to the ghosts estimator because we will not send it back! */
    if(faces_m == (P4EST_HALF)){
      /* e_m[f]->local_estimator += Je1MJe1; */
      estimator[e_m[f]->id] += Je1MJe1;
    }
    else{
      /* e_m[0]->local_estimator += Je1MJe1; */
      estimator[e_m[0]->id] += Je1MJe1;
    }
    stride += nodes_mortar_quad[f];
  }

  P4EST_FREE(Je1);
  P4EST_FREE(Je1_prefactor);
  D4EST_FREE_DIM_VEC(Je2);
  P4EST_FREE(Je2_prefactor);
}



double*
d4est_estimator_bi_compute
(
 p4est_t* p4est,
 d4est_elliptic_data_t* d4est_elliptic_data,
 d4est_elliptic_eqns_t* fcns,
 d4est_estimator_bi_penalty_data_t bi_penalty_data,
 d4est_xyz_fcn_t u_bndry_fcn,
 d4est_ghost_t* d4est_ghost,
 d4est_ghost_data_t* d4est_ghost_data,
 d4est_operators_t* d4est_ops,
 d4est_geometry_t* d4est_geom_for_residual,
 d4est_mesh_data_t* d4est_factors_for_residual,
 d4est_geometry_t* d4est_geom_for_integrals,
 d4est_mesh_data_t* d4est_factors_for_integrals,
 d4est_quadrature_t* d4est_quad,
 diam_compute_option_t diam_opt,
 int which_field
)
{
  double* estimator = P4EST_ALLOC(double, p4est->local_num_quadrants);
  d4est_elliptic_eqns_build_residual
    (
     p4est,
     d4est_ghost,
     d4est_ghost_data,
     fcns,
     d4est_elliptic_data,
     d4est_ops,
     d4est_geom_for_residual,
     d4est_quad,
     d4est_factors_for_residual
    );
  
  d4est_mesh_compute_l2_norm_sqr
    (
     p4est,
     d4est_ops,
     d4est_geom_for_integrals,
     d4est_quad,
     d4est_factors_for_integrals,
     d4est_elliptic_data->Au,
     d4est_elliptic_data->local_nodes,
     /* STORE_LOCALLY, */
     NULL,
     estimator
    );



  for (p4est_topidx_t tt = p4est->first_local_tree;
       tt <= p4est->last_local_tree;
       ++tt)
    {
      p4est_tree_t* tree = p4est_tree_array_index (p4est->trees, tt);
      sc_array_t* tquadrants = &tree->quadrants;
      int Q = (p4est_locidx_t) tquadrants->elem_count;
      for (int q = 0; q < Q; ++q) {
        p4est_quadrant_t* quad = p4est_quadrant_array_index (tquadrants, q);
        d4est_element_data_t* ed = quad->p.user_data; 
        int deg = ed->deg;
        int volume_nodes_lobatto = d4est_lgl_get_nodes((P4EST_DIM),deg);
        double h = d4est_estimator_get_diam(d4est_factors_for_integrals, ed, diam_opt);
        estimator[ed->id] *= h*h/(deg*deg);
        
        /* d4est_util_copy_1st_to_2nd */
        /*   ( */
        /*    &(vecs->u[ed->nodal_stride]), */
        /*    &(ed->u_elem)[0], */
        /*    volume_nodes_lobatto */
        /*   ); */
       
      }
    }
  
  
  d4est_ghost_data_exchange(p4est,d4est_ghost,d4est_ghost_data,d4est_elliptic_data->u);
  
  /* p4est_ghost_exchange_data(p4est,ghost,ghost_data); */

  double* dudr_local [(P4EST_DIM)];
  D4EST_ALLOC_DIM_VEC(dudr_local,d4est_elliptic_data->local_nodes);

  int ghost_nodes = d4est_mesh_get_ghost_nodes(d4est_ghost);
  double* dudr_ghost [(P4EST_DIM)];
  D4EST_ALLOC_DIM_VEC(dudr_ghost, ghost_nodes);
  
  d4est_poisson_compute_dudr
    (
     p4est,
     d4est_ghost,
     d4est_ghost_data,
     d4est_ops,
     d4est_geom_for_integrals,
     d4est_quad,
     d4est_factors_for_integrals,
     dudr_local,
     dudr_ghost,
     d4est_elliptic_data->u,
     d4est_elliptic_data->local_nodes,
     which_field
    );
  
  d4est_poisson_flux_data_t flux_data;
  flux_data.interface_fcn = d4est_estimator_bi_interface;
  flux_data.boundary_fcn = d4est_estimator_bi_dirichlet;
  
  flux_data.d4est_ghost_data = d4est_ghost_data;
  flux_data.d4est_ghost = d4est_ghost;
  flux_data.p4est = p4est;
  flux_data.which_field = which_field;
  flux_data.local_nodes = d4est_elliptic_data->local_nodes;
  flux_data.u = d4est_elliptic_data->u;
  flux_data.Au = d4est_elliptic_data->Au;
  for (int i = 0; i < (P4EST_DIM); i++){
    flux_data.dudr_local[i] = dudr_local[i];
    flux_data.dudr_ghost[i] = dudr_ghost[i];
  }
  
  d4est_poisson_dirichlet_bc_t bc_data;
  bc_data.dirichlet_fcn = u_bndry_fcn;
  bc_data.user = NULL;

  flux_data.bc_type = BC_DIRICHLET;
  flux_data.bc_data = &bc_data;

  bi_penalty_data.estimator = estimator;
  flux_data.flux_data = &bi_penalty_data;
  
  d4est_mortars_fcn_ptrs_t flux_fcns = d4est_poisson_flux_fetch_fcns(&flux_data);
  
  d4est_mortars_compute_flux_on_local_elements
    (
     p4est,
     d4est_ghost,
     /* ghost_data, */
     d4est_ops,
     d4est_geom_for_integrals,
     d4est_quad,
     d4est_factors_for_integrals,
     &flux_fcns
     /* DO_NOT_EXCHANGE_GHOST_DATA */
    );

  /* DEBUG_PRINT_ARR_DBL(estimator, p4est->local_num_quadrants); */
  

  D4EST_FREE_DIM_VEC(dudr_local);  
  D4EST_FREE_DIM_VEC(dudr_ghost);  
  return estimator;
}
