#ifndef D4EST_SOLVER_SCHWARZ_HELPERS_H
#define D4EST_SOLVER_SCHWARZ_HELPERS_H 


/* This file was automatically generated.  Do not edit! */
void d4est_solver_schwarz_convert_subdomain_field_to_global_nodal_field(p4est_t *p4est,d4est_mesh_data_t *d4est_factors,d4est_solver_schwarz_data_t *schwarz_data,double *field_over_subdomains,double *field,int mpirank,int sub_id,int local_nodes);
void d4est_solver_schwarz_apply_weights_over_all_subdomains(d4est_solver_schwarz_data_t *schwarz_data,d4est_solver_schwarz_operators_t *schwarz_ops,double *du_restricted_field_over_subdomains,double *weighted_du_restricted_field_over_subdomains);
void d4est_solver_schwarz_compute_correction(d4est_solver_schwarz_data_t *schwarz_data,d4est_solver_schwarz_operators_t *schwarz_ops,double *du_restricted_field_over_subdomains,double *correction_field_over_subdomains);
void d4est_solver_schwarz_restrict_field_over_subdomains(d4est_solver_schwarz_data_t *schwarz_data,d4est_solver_schwarz_operators_t *schwarz_ops,double *field_over_subdomains,double *restricted_field_over_subdomains);
void d4est_solver_schwarz_restrict_transpose_restricted_field_over_subdomains(d4est_solver_schwarz_data_t *schwarz_data,d4est_solver_schwarz_operators_t *schwarz_ops,double *restricted_field_over_subdomains,double *field_over_subdomains);
void d4est_solver_schwarz_restrict_nodal_field_to_subdomains(p4est_t *p4est,d4est_operators_t *d4est_ops,d4est_mesh_data_t *d4est_factors,d4est_ghost_data_t *ghost_data,d4est_solver_schwarz_data_t *schwarz_data,d4est_solver_schwarz_operators_t *schwarz_ops,double *field,int ghost_data_num_of_field,double *restricted_field_over_subdomains);


#endif
