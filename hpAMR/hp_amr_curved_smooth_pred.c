#include "../pXest/pXest.h"
#include "../ElementData/curved_element_data.h"
#include "../EllipticSystem/problem_data.h"
#include "../EllipticSystem/problem_weakeqn_ptrs.h"
#include "../LinearAlgebra/linalg.h"
#include "../Utilities/util.h"
#include "../hpAMR/hp_amr.h"
#include "../hpAMR/hp_amr_curved_smooth_pred.h"

#if (P4EST_DIM)==3
#define ONE_OVER_CHILDREN 0.125
#else
#define ONE_OVER_CHILDREN 0.25
#endif


void
hp_amr_curved_smooth_pred_print
(
 p4est_t* p4est
)
{
  for (p4est_topidx_t tt = p4est->first_local_tree;
       tt <= p4est->last_local_tree;
       ++tt)
    {
      p4est_tree_t* tree = p4est_tree_array_index (p4est->trees, tt);
      sc_array_t* tquadrants = &tree->quadrants;
      int Q = (p4est_locidx_t) tquadrants->elem_count;
      for (int q = 0; q < Q; ++q) {
        p4est_quadrant_t* quad = p4est_quadrant_array_index (tquadrants, q);
        curved_element_data_t* ed = quad->p.user_data;
        printf("Element %d predictor = %.25f\n", ed->id, ed->local_predictor);
      }
    }
}


hp_amr_scheme_t*
hp_amr_curved_smooth_pred_init
(
 p4est_t* p4est,
 int max_degree,
 curved_smooth_pred_marker_t marker
)
{
  mpi_assert((max_degree < (MAX_DEGREE)-1));
  
  hp_amr_scheme_t* scheme = P4EST_ALLOC(hp_amr_scheme_t, 1);
  hp_amr_curved_smooth_pred_data_t* smooth_pred_data;
  smooth_pred_data = P4EST_ALLOC(hp_amr_curved_smooth_pred_data_t, 1);
  smooth_pred_data->max_degree = max_degree;
  smooth_pred_data->marker = marker;
  smooth_pred_data->predictors = NULL; 

  scheme->pre_refine_callback
    = hp_amr_curved_smooth_pred_pre_refine_callback;
  
  scheme->balance_replace_callback_fcn_ptr
    = hp_amr_curved_smooth_pred_balance_replace_callback;

  scheme->refine_replace_callback_fcn_ptr
    = hp_amr_curved_smooth_pred_refine_replace_callback;

  scheme->iter_volume
    = hp_amr_curved_smooth_pred_set_refinement;

  scheme->hp_amr_scheme_data
    = smooth_pred_data;

  scheme->post_balance_callback
    = hp_amr_curved_smooth_pred_post_balance_callback;
  
  return scheme;
}

void
hp_amr_curved_smooth_pred_pre_refine_callback
(
 p4est_t* p4est,
 void* user
)
{
  hp_amr_curved_smooth_pred_data_t* smooth_pred_data =
    (hp_amr_curved_smooth_pred_data_t*)user;

  if (smooth_pred_data->predictors == NULL){
    smooth_pred_data->predictors = P4EST_REALLOC
                                   (
                                    smooth_pred_data->predictors,
                                    double,
                                    p4est->local_num_quadrants
                                   );
    linalg_fill_vec(smooth_pred_data->predictors, 0., p4est->local_num_quadrants);
  }
  
  int pred_stride = 0;
  for (p4est_topidx_t tt = p4est->first_local_tree;
       tt <= p4est->last_local_tree;
       ++tt)
    {
      p4est_tree_t* tree = p4est_tree_array_index (p4est->trees, tt);
      sc_array_t* tquadrants = &tree->quadrants;
      int Q = (p4est_locidx_t) tquadrants->elem_count;
      for (int q = 0; q < Q; ++q) {
        p4est_quadrant_t* quad = p4est_quadrant_array_index (tquadrants, q);
        curved_element_data_t* ed = quad->p.user_data;
        ed->local_predictor = smooth_pred_data->predictors[pred_stride];
        pred_stride++;
      }
    }
}

void
hp_amr_curved_smooth_pred_post_balance_callback
(
 p4est_t* p4est,
 void* user
)
{
  hp_amr_curved_smooth_pred_data_t* smooth_pred_data =
    (hp_amr_curved_smooth_pred_data_t*)user;
  smooth_pred_data->predictors = P4EST_REALLOC
                                 (
                                  smooth_pred_data->predictors,
                                  double,
                                  p4est->local_num_quadrants
                                 );  
  int pred_stride = 0;
  for (p4est_topidx_t tt = p4est->first_local_tree;
       tt <= p4est->last_local_tree;
       ++tt)
    {
      p4est_tree_t* tree = p4est_tree_array_index (p4est->trees, tt);
      sc_array_t* tquadrants = &tree->quadrants;
      int Q = (p4est_locidx_t) tquadrants->elem_count;
      for (int q = 0; q < Q; ++q) {
        p4est_quadrant_t* quad = p4est_quadrant_array_index (tquadrants, q);
        curved_element_data_t* ed = quad->p.user_data;
        smooth_pred_data->predictors[pred_stride] = ed->local_predictor;
        pred_stride++;
      }
    }
  
  /* printf("POST-BALANCE PREDICTOR PRINTOUT\n"); */
  /* hp_amr_curved_smooth_pred_print */
  /*   ( */
  /*    p4est */
  /*   ); */
  
}


void
hp_amr_curved_smooth_pred_destroy(hp_amr_scheme_t* scheme){

  hp_amr_curved_smooth_pred_data_t* smooth_pred_data =
    (hp_amr_curved_smooth_pred_data_t*)scheme->hp_amr_scheme_data;  
  P4EST_FREE(smooth_pred_data->predictors);
  P4EST_FREE(smooth_pred_data);
  P4EST_FREE(scheme);
}

/* void */
/* hp_amr_curved_smooth_pred_set_marker */
/* ( */
/*  hp_amr_scheme_t* scheme, */
/*  smooth_pred_marker_t marker */
/* ) */
/* { */
/*   hp_amr_curved_smooth_pred_data_t* smooth_pred_data = */
/*     (hp_amr_curved_smooth_pred_data_t*)scheme->hp_amr_scheme_data; */
/*   smooth_pred_data->marker = marker; */

/*   if (strcmp(marker.type,"percentile") == 0){ */
/*     printf("[HP_AMR_CURVED_SMOOTH_PRED]: Using marker percentile marker with percentile = %d\n", *(int*)marker.user); */
/*   } */
/*   else if (strcmp(marker.type,"sig_average") == 0){ */
/*     printf("[HP_AMR_CURVED_SMOOTH_PRED]: Using marker sigaverage marker with sigma = %f\n", *(double*)marker.user); */
/*   } */
/*   else{ */
/*     mpi_abort("[D4EST_ERROR]: Marker type not supported"); */
/*   } */
/* } */

/* static int */
/* hp_amr_curved_smooth_pred_percentile_mark_element */
/* ( */
/*  double eta2, */
/*  estimator_stats_t* stats, */
/*  void* marker_user */
/* ) */
/* { */
/*   double eta2_percentile */
/*     = estimator_stats_get_percentile(stats, *(int*)marker_user); */
/*   return (eta2 >= eta2_percentile); */
/* } */

/* static int */
/* hp_amr_curved_smooth_pred_sigaverage_mark_element */
/* ( */
/*  double eta2, */
/*  estimator_stats_t* stats, */
/*  void* marker_user */
/* ) */
/* { */
/*   double eta2_avg = stats->mean; */
/*   double sigma = *(double*)marker_user; */
/*   return (eta2 >= sigma * eta2_avg); */
/* } */

/* smooth_pred_marker_t */
/* hp_amr_curved_smooth_pred_get_percentile_marker */
/* ( */
/*  int* percentile */
/* ){ */
/*   mpi_assert(*percentile == 5 || */
/*              *percentile == 10 || */
/*              *percentile == 15 || */
/*              *percentile == 20 || */
/*              *percentile == 25 */
/*             ); */
/*   smooth_pred_marker_t marker; */
/*   marker.user = (void*)percentile; */
/*   marker.mark_element_fcn = hp_amr_curved_smooth_pred_percentile_mark_element; */
/*   marker.type = "percentile"; */
/*   return marker; */
/* } */


/* smooth_pred_marker_t */
/* hp_amr_curved_smooth_pred_get_NULL_marker */
/* ( */
/* ){ */
/*   smooth_pred_marker_t marker; */
/*   marker.user = NULL; */
/*   marker.mark_element_fcn = NULL; */
/*   marker.type = "NULL"; */
/*   return marker; */
/* } */


/* smooth_pred_marker_t */
/* hp_amr_curved_smooth_pred_get_sigaverage_marker */
/* ( */
/*  double* sigma */
/* ){ */
/*   mpi_assert(!(*sigma < 0.)); */
/*   smooth_pred_marker_t marker; */
/*   marker.user = (void*)sigma; */
/*   marker.mark_element_fcn = hp_amr_curved_smooth_pred_sigaverage_mark_element; */
/*   marker.type = "sig_average";   */
/*   return marker; */
/* } */


void
hp_amr_curved_smooth_pred_set_refinement
(
 p4est_iter_volume_info_t* info,
 void* user_data
)
{
  hp_amr_data_t* hp_amr_data = (hp_amr_data_t*) info->p4est->user_pointer;
  hp_amr_curved_smooth_pred_data_t* smooth_pred_data = (hp_amr_curved_smooth_pred_data_t*) (hp_amr_data->hp_amr_scheme_data);
  curved_element_data_t* elem_data = (curved_element_data_t*) info->quad->p.user_data;
  estimator_stats_t** stats = hp_amr_data->estimator_stats;
  
  double eta2 = elem_data->local_estimator;
  double eta2_pred = elem_data->local_predictor;
  /* double eta2_avg = stats->mean; */

  gamma_params_t gamma_hpn =
    smooth_pred_data->marker.set_element_gamma_fcn
    (
     info->p4est,
     eta2,
     stats,
     elem_data,
     smooth_pred_data->marker.user
    );
  
  int is_marked = 
    smooth_pred_data->marker.mark_element_fcn
    (
     info->p4est,
     eta2,
     stats,
     elem_data,
     smooth_pred_data->marker.user
    );

  
  if (is_marked){
    /* it's smooth! -> p-refine*/
    if (eta2 <= elem_data->local_predictor && elem_data->deg < smooth_pred_data->max_degree){
      hp_amr_data->refinement_log[elem_data->id] = util_min_int(elem_data->deg + 1, smooth_pred_data->max_degree);
      eta2_pred = gamma_hpn.gamma_p*eta2;
    }
    else {
      hp_amr_data->refinement_log[elem_data->id] = -elem_data->deg;
      /* if (smooth_pred_data->norm_type == DG_NORM) */
        eta2_pred = gamma_hpn.gamma_h*eta2*util_dbl_pow_int(.5, 2*(elem_data->deg))*(ONE_OVER_CHILDREN);
      /* else if (smooth_pred_data->norm_type == L2_NORM) */
        /* eta2_pred = smooth_pred_data->gamma_h*eta2*util_dbl_pow_int(.5, 2*(elem_data->deg + 1))*(ONE_OVER_CHILDREN); */
      /* else */
        /* mpi_abort("[D4EST_ERROR]: Not a supported norm type"); */
    }
  }
  /* do not refine */
  else {
    eta2_pred = gamma_hpn.gamma_n*eta2_pred;
    hp_amr_data->refinement_log[elem_data->id] = elem_data->deg;
  }
  
  elem_data->local_predictor = eta2_pred;
}

void
hp_amr_curved_smooth_pred_balance_replace_callback (
			     p4est_t * p4est,
			     p4est_topidx_t which_tree,
			     int num_outgoing,
			     p4est_quadrant_t * outgoing[],
			     int num_incoming,
			     p4est_quadrant_t * incoming[]
			     )
{
#ifdef SAFETY  
  mpi_assert(num_outgoing == 1);
#endif
  hp_amr_data_t* hp_amr_data = (hp_amr_data_t*) p4est->user_pointer;
  d4est_operators_t* d4est_ops = hp_amr_data->d4est_ops;
  hp_amr_curved_smooth_pred_data_t* smooth_pred_data = (hp_amr_curved_smooth_pred_data_t*) (hp_amr_data->hp_amr_scheme_data);
  estimator_stats_t** stats = hp_amr_data->estimator_stats;
  curved_element_data_t* parent_data = (curved_element_data_t*) outgoing[0]->p.user_data;
  curved_element_data_t* child_data;
  int i;

  int degh [(P4EST_CHILDREN)];
  int degH = parent_data->deg;

  gamma_params_t gamma_hpn
    = smooth_pred_data->marker.set_element_gamma_fcn(p4est,parent_data->local_estimator, stats, parent_data, smooth_pred_data->marker.user);
  
  for (i = 0; i < (P4EST_CHILDREN); i++)
    degh[i] = degH;

  int volume_nodes = d4est_operators_get_nodes((P4EST_DIM), degH);
    
  int h_pow = -1;
  /* if (smooth_pred_data->norm_type == l2_norm_type) */
    /* h_pow = parent_data->deg+1; */
  /* else if (smooth_pred_data->norm_type == dg_norm_type) */
    h_pow = parent_data->deg;
  /* else */
    /* mpi_abort("[ERROR]: hp_amr_curved_smooth_pred norm_type not supported"); */

  double* temp_data = P4EST_ALLOC(double, volume_nodes*(P4EST_CHILDREN));
  d4est_operators_apply_hp_prolong
    (
     d4est_ops,
     &(parent_data->u_storage[0]),
     degH,
     (P4EST_DIM),
     &degh[0],
     temp_data
    );
  
  for (i = 0; i < (P4EST_CHILDREN); i++){
    child_data = (curved_element_data_t*) incoming[i]->p.user_data;   
    child_data->deg = parent_data->deg;
    child_data->local_predictor = (ONE_OVER_CHILDREN)*gamma_hpn.gamma_h*util_dbl_pow_int(.5, 2*(h_pow))*parent_data->local_predictor;
    linalg_copy_1st_to_2nd(&temp_data[volume_nodes*i], &child_data->u_storage[0], volume_nodes);
  }

  P4EST_FREE(temp_data);
}


void
hp_amr_curved_smooth_pred_refine_replace_callback (
			     p4est_t * p4est,
			     p4est_topidx_t which_tree,
			     int num_outgoing,
			     p4est_quadrant_t * outgoing[],
			     int num_incoming,
			     p4est_quadrant_t * incoming[]
			     )
{
#ifdef SAFETY  
  mpi_assert(num_outgoing == 1);
#endif
  hp_amr_data_t* hp_amr_data = (hp_amr_data_t*) p4est->user_pointer;
  d4est_operators_t* d4est_ops = hp_amr_data->d4est_ops;
  /* hp_amr_curved_smooth_pred_data_t* smooth_pred_data = (hp_amr_curved_smooth_pred_data_t*) (hp_amr_data->hp_amr_scheme_data); */
  
  curved_element_data_t* parent_data = (curved_element_data_t*) outgoing[0]->p.user_data;
  curved_element_data_t* child_data;
  int i;

  int degh [(P4EST_CHILDREN)];
  int degH = parent_data->deg;
    
  for (i = 0; i < (P4EST_CHILDREN); i++)
    degh[i] = degH;

  int volume_nodes = d4est_operators_get_nodes((P4EST_DIM), degH);
    
  double* temp_data = P4EST_ALLOC(double, volume_nodes*(P4EST_CHILDREN));
  d4est_operators_apply_hp_prolong
    (
     d4est_ops,
     &(parent_data->u_storage[0]),
     degH,
     (P4EST_DIM),
     &degh[0],
     temp_data
    );
  
  for (i = 0; i < (P4EST_CHILDREN); i++){
    child_data = (curved_element_data_t*) incoming[i]->p.user_data;
    child_data->deg = parent_data->deg;
    child_data->local_predictor = parent_data->local_predictor;
    linalg_copy_1st_to_2nd(&temp_data[volume_nodes*i], &child_data->u_storage[0], volume_nodes);
  }

  P4EST_FREE(temp_data);
}
