#include <d4est_geometry_brick.h>
#include <ini.h>
#include <zlog.h>



static void
d4est_geometry_brick_D2X(d4est_geometry_t* d4est_geom,
                                p4est_topidx_t which_tree,
                                p4est_qcoord_t q0 [(P4EST_DIM)],
                                p4est_qcoord_t dq,
                                const double rst[(P4EST_DIM)], /* [-1,1]^3 */
                                double d2xyz_drstdrst[(P4EST_DIM)][(P4EST_DIM)][(P4EST_DIM)]
                               )
{

    for (int d1 = 0; d1 < (P4EST_DIM); d1++){
      for (int d2 = 0; d2 < (P4EST_DIM); d2++){
        for (int d3 = 0; d3 < (P4EST_DIM); d3++){
          d2xyz_drstdrst[d1][d2][d3] = 0.;
        }
      }
    }
}


static
int d4est_geometry_brick_get_number_of_regions
(
 d4est_geometry_t* d4est_geom
){
  return 1;
}

static
int d4est_geometry_brick_get_region
(
 d4est_geometry_t* d4est_geom,
 p4est_qcoord_t q [(P4EST_DIM)],
 p4est_qcoord_t dq,
 int tree
){
  return 0;
}


static
int d4est_geometry_brick_input_handler
(
 void* user,
 const char* section,
 const char* name,
 const char* value
)
{
  d4est_geometry_brick_attr_t* pconfig = user;
  if (d4est_util_match_couple(section,pconfig->input_section,name,"X0")) {
    D4EST_ASSERT(pconfig->X0 == -1);
    pconfig->X0 = atof(value);
  }
  else if (d4est_util_match_couple(section,pconfig->input_section,name,"X1")) {
    D4EST_ASSERT(pconfig->X1 == -1);
    pconfig->X1 = atof(value);
  }
  else if (d4est_util_match_couple(section,pconfig->input_section,name,"Y0")) {
    D4EST_ASSERT(pconfig->Y0 == -1);
    pconfig->Y0 = atof(value);
  }
  else if (d4est_util_match_couple(section,pconfig->input_section,name,"Y1")) {
    D4EST_ASSERT(pconfig->Y1 == -1);
    pconfig->Y1 = atof(value);
  }
  else if (d4est_util_match_couple(section,pconfig->input_section,name,"Z0")) {
    D4EST_ASSERT(pconfig->Z0 == -1);
    pconfig->Z0 = atof(value);
  }
  else if (d4est_util_match_couple(section,pconfig->input_section,name,"Z1")) {
    D4EST_ASSERT(pconfig->Z1 == -1);
    pconfig->Z1 = atof(value);
  }
  else {
    return 0;  /* unknown section/name, error */
  }
  return 1;
}

static
d4est_geometry_brick_attr_t*
d4est_geometry_brick_input
(
 const char* input_file,
 const char* input_section
)
{
  d4est_geometry_brick_attr_t* brick_attrs = P4EST_ALLOC(d4est_geometry_brick_attr_t, 1);
  snprintf (brick_attrs->input_section, sizeof(brick_attrs->input_section), "%s", input_section);
  brick_attrs->X0 = -1;
  brick_attrs->X1 = -1;
  brick_attrs->Y0 = -1;
  brick_attrs->Y1 = -1;
  brick_attrs->Z0 = -1;
  brick_attrs->Z1 = -1;
  
  if (ini_parse(input_file, d4est_geometry_brick_input_handler, brick_attrs) < 0) {
    D4EST_ABORT("Can't load input file");
  }

  /* If any of these asserts fail, the parameters were not set correctly */
  D4EST_ASSERT(brick_attrs->X0 < brick_attrs->X1);
  D4EST_ASSERT(brick_attrs->Y0 < brick_attrs->Y1);
#if (P4EST_DIM)==3
  D4EST_ASSERT(brick_attrs->Z0 < brick_attrs->Z1);
#endif
  
  return brick_attrs;
}



static void
d4est_geometry_brick_X(
                       d4est_geometry_t * geom,
                       p4est_topidx_t which_tree,
                       p4est_qcoord_t q0 [(P4EST_DIM)],
                       p4est_qcoord_t dq,
                       const double coords[(P4EST_DIM)],
                       coords_type_t coords_type,
                       double xyz[(P4EST_DIM)]
)
{
  d4est_geometry_brick_attr_t* brick_attrs = geom->user;

  double tcoords [(P4EST_DIM)];
  d4est_geometry_get_tree_coords_in_range_0_to_1(q0, dq, coords, coords_type, tcoords);
  
  double x0 = brick_attrs->X0;
  double x1 = brick_attrs->X1;
  double y0 = brick_attrs->Y0;
  double y1 = brick_attrs->Y1;
  double z0 = brick_attrs->Z0;
  double z1 = brick_attrs->Z1;
  
  xyz[0] = (x1-x0)*tcoords[0] + x0;
  xyz[1] = (y1-y0)*tcoords[1] + y0;
#if (P4EST_DIM)==3
  xyz[2] = (z1-z0)*tcoords[2] + z0;
  
#endif
}

static void
d4est_geometry_brick_DX(d4est_geometry_t* d4est_geom,
                        p4est_topidx_t which_tree,
                        p4est_qcoord_t q0 [(P4EST_DIM)],
                        p4est_qcoord_t dq,
                        const double rst[(P4EST_DIM)], /* [-1,1]^3 */
                        double dxyz_drst[(P4EST_DIM)][(P4EST_DIM)]
                       )
{
  d4est_geometry_brick_attr_t* brick_attrs = d4est_geom->user;

  /* topological coordinates of element corners */
  double amin = q0[0];
  double amax = q0[0] + dq;
  double bmin = q0[1];
  double bmax = q0[1] + dq;

  /* transform element corners to [0,1]^3 topological space */
  amin /= (double)P4EST_ROOT_LEN;
  amax /= (double)P4EST_ROOT_LEN;
  bmin /= (double)P4EST_ROOT_LEN;
  bmax /= (double)P4EST_ROOT_LEN;
  
  double x0 = brick_attrs->X0;
  double x1 = brick_attrs->X1;
  double y0 = brick_attrs->Y0;
  double y1 = brick_attrs->Y1;

  /* transform element corners to [X0,X1] X [YO,Y1] X [Z0,Z1] topological space */
  amin = (x1-x0)*amin + x0;
  amax = (x1-x0)*amax + x0;
  bmin = (y1-y0)*bmin + y0;
  bmax = (y1-y0)*bmax + y0;

  dxyz_drst[0][0] = (amax - amin)/2.;
  dxyz_drst[0][1] = 0;

  dxyz_drst[1][0] = 0;
  dxyz_drst[1][1] = (bmax - bmin)/2.;

#if (P4EST_DIM)==3
  double cmin = q0[2];
  double cmax = q0[2] + dq;
  cmin /= (double)P4EST_ROOT_LEN;
  cmax /= (double)P4EST_ROOT_LEN;
  double z0 = brick_attrs->Z0;
  double z1 = brick_attrs->Z1;
  cmin = (z1-z0)*cmin + z0;
  cmax = (z1-z0)*cmax + z0;
  dxyz_drst[0][2] = 0;
  dxyz_drst[1][2] = 0;
  dxyz_drst[2][0] = 0;
  dxyz_drst[2][1] = 0;
  dxyz_drst[2][2] = (cmax - cmin)/2.;
#endif
  
}

static void
d4est_geometry_brick_destroy
(
 d4est_geometry_t* geom
)
{
  P4EST_FREE(geom->user);
  P4EST_FREE(geom);
}
 
void
d4est_geometry_brick_new_aux
(
 d4est_geometry_t* d4est_geom,
 d4est_geometry_brick_attr_t* brick_attrs
)
{
#if (P4EST_DIM)==2
  p4est_connectivity_t* conn = p4est_connectivity_new_unitsquare();
#elif (P4EST_DIM)==3
  p4est_connectivity_t* conn = p8est_connectivity_new_unitcube();
#else
  D4EST_ABORT("DIM must be 2 or 3\n");
#endif
    
  d4est_geom->p4est_conn = conn;
  d4est_geom->user = brick_attrs;
  d4est_geom->X = d4est_geometry_brick_X;
  d4est_geom->DX = d4est_geometry_brick_DX;
  d4est_geom->JAC = NULL;
  d4est_geom->destroy = d4est_geometry_brick_destroy;
}


void
d4est_geometry_brick_new
(
 int mpirank,
 const char* input_file,
 const char* input_section,
 zlog_category_t *c_default,
 d4est_geometry_t* d4est_geom
)
{
  d4est_geometry_brick_attr_t* brick_attrs = d4est_geometry_brick_input(input_file, input_section);

  d4est_geometry_brick_new_aux(d4est_geom, brick_attrs);

  d4est_geom->get_region = d4est_geometry_brick_get_region;
  d4est_geom->D2X = d4est_geometry_brick_D2X;
  d4est_geom->get_number_of_regions = d4est_geometry_brick_get_number_of_regions;
  
  if (mpirank == 0){
    zlog_debug(c_default, "NAME = brick in %d-D", (P4EST_DIM));
    zlog_debug(c_default, "X0 = %.25f", brick_attrs->X0);
    zlog_debug(c_default, "X1 = %.25f", brick_attrs->X1);
    zlog_debug(c_default, "YO = %.25f", brick_attrs->Y0);
    zlog_debug(c_default, "Y1 = %.25f", brick_attrs->Y1);
#if (P4EST_DIM)==3
    zlog_debug(c_default, "Z0 = %.25f", brick_attrs->Z0);
    zlog_debug(c_default, "Z1 = %.25f", brick_attrs->Z1);
#endif
  }
}


int
d4est_geometry_brick_which_child_of_root
(
 p4est_qcoord_t q [(P4EST_DIM)],
 p4est_qcoord_t dq
)
{
  p4est_qcoord_t child_origin [(P4EST_DIM)];
  p4est_qcoord_t root_half = (P4EST_ROOT_LEN)/2;
  
  for (int i = 0; i < (P4EST_CHILDREN); i++){
    for (int j = 0; j < (P4EST_DIM); j++){
      child_origin[j] = d4est_reference_is_child_left_or_right(i,j)*root_half;
    }
    
    int check = 0;
    for (int j = 0; j < (P4EST_DIM); j++){
      if (q[j] >= child_origin[j] && q[j] <= child_origin[j] + root_half
          && q[j] + dq >= child_origin[j] && q[j] + dq <= child_origin[j] + root_half
         )
        {
          check += 1;
        }
    }
    if(check == (P4EST_DIM)){
      return i;
    }
  }
  D4EST_ABORT("quadrant should exist in some child of the root node");
}
