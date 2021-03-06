#!/bin/bash

function write_submit_graham {
    cat <<EOF1 > submit.sh
#!/bin/bash
#SBATCH --ntasks=${4}               # number of MPI processes
#SBATCH --mem-per-cpu=1024M      # memory; default unit is megabytes
#SBATCH --time=$5-00:00           # time (DD-HH:MM)

source /home/tvincent/d4est.env
cd ${1}
time mpirun -np $4 ./${2}  2>&1 | tee disco4est.out

EOF1
}

function write_options {

    cat <<EOF > options.input
[initial_mesh]
min_quadrants = -1
min_level = 0
fill_uniform = 1
region0_deg = 1
region0_deg_quad_inc = $1
region1_deg = 1
region1_deg_quad_inc = 0

[problem]
rho0_div_rhoc = $2
R = $3
cx = 0
cy = 0
cz = 0
do_not_solve = 0
use_dirichlet = $4

[amr]
scheme = smooth_pred
num_of_amr_steps = 100
max_degree = 7 
gamma_h = $5
initial_pred = $4
gamma_p = 0.1
gamma_n = 1.
percentile = 5
amr_level_for_uniform_p = 100

[flux]
name = sipg
sipg_penalty_prefactor = $6
sipg_flux_h = $7
sipg_penalty_fcn = maxp_sqr_over_minh

[geometry]
name = cubed_sphere
R0 = $8
R1 = $9
R2 = $11
compactify_outer_shell = 0
compactify_inner_shell = $10
DX_compute_method = analytic
JAC_compute_method = numerical

[compactified_geometry]
name = cubed_sphere
R0 = 1
R1 = 2
R2 = 1000
compactify_outer_shell = 0
compactify_inner_shell = $10
DX_compute_method = analytic
JAC_compute_method = numerical

[d4est_vtk_geometry]
name = cubed_sphere
R0 = 1
R1 = 2
R2 = 3
compactify_outer_shell = 0
compactify_inner_shell = $10
DX_compute_method = analytic
JAC_compute_method = numerical

[d4est_solver_newton]
atol = 1e-15
rtol = 1e-15
imax = 1000000
imin = 1
monitor = 1

[d4est_solver_fcg]
atol = 1e-15
rtol = 1e-15
iter = 1000000
monitor = 1
vi = 5
precond_flag = 0


[d4est_solver_cg]
atol = 1e-15
rtol = 1e-15
iter = 1000000000
monitor = 1

[d4est_vtk]
filename = two_punctures
geometry_section = d4est_vtk_geometry
output_type = ascii
grid_type = dg
write_tree = 1
write_level = 1
write_rank = 1
wrap_rank = 0
write_deg = 1


[quadrature]
name = legendre

[d4est_solver_newton_petsc]
snes_atol = 1e-15
snes_rtol = 1e-50
snes_stol = 1e-1
snes_max_funcs = 1000000000
snes_type = newtonls
snes_max_it = 5
snes_monitor = 1
snes_linesearch_order = 3
snes_linesearch_monitor = 1
snes_converged_reason = 1
snes_view = 1

[d4est_solver_krylov_petsc]
ksp_type = fcg
ksp_atol = 1e-15
ksp_rtol = $7
ksp_max_it = 10000
ksp_view = 1
ksp_monitor = 1
ksp_converged_reason = 1
ksp_initial_guess_nonzero = 0
ksp_monitor_singular_value = 1


[d4est_solver_krylov_petsc_no_mg]
ksp_type = fcg
ksp_atol = 1e-15
ksp_rtol = 1e-15
ksp_max_it = 10000
ksp_view = 1
ksp_monitor = 1
ksp_converged_reason = 1
ksp_initial_guess_nonzero = 0
ksp_monitor_singular_value = 1

[multigrid]
vcycle_imax = 1;
vcycle_rtol = 1e-9;
vcycle_atol = 0.;
smoother_name = mg_smoother_cheby
bottom_solver_name = mg_bottom_solver_cheby

[mg_bottom_solver_cg]
bottom_iter = 100;
bottom_rtol = 1e-10;
bottom_atol = 0.;
bottom_print_residual_norm = 0;

[mg_smoother_cheby]
cheby_imax = 15;
cheby_eigs_cg_imax = $8
cheby_eigs_lmax_lmin_ratio = 30.;
cheby_eigs_max_multiplier = 1.;
cheby_eigs_reuse_fromdownvcycle = 0;
cheby_eigs_reuse_fromlastvcycle = 0;
cheby_print_residual_norm = 0;
cheby_print_eigs = 0;

[mg_bottom_solver_cheby]
cheby_imax = 15;
cheby_eigs_cg_imax = $8;
cheby_eigs_lmax_lmin_ratio = 30.;
cheby_eigs_max_multiplier = 1.;
cheby_eigs_reuse_fromdownvcycle = 0;
cheby_eigs_reuse_fromlastvcycle = 0;
cheby_print_residual_norm = 0;
cheby_print_eig = 0;


EOF
}

arr1=( 0 1 ) #min_level
arr2=( .001 ) #deg
arr3=( 8 ) #deg_quad_inc
arr4=( 10000 ) #hrefine til inview
arr5=( .25 1 ) #penalty
arr6=( 2. ) #domain size
arr7=( "H_EQ_VOLUME_DIV_AREA" "H_EQ_J_DIV_SJ_MIN_LOBATTO" ) #penalty
arr8=( 5 10 )
arr9=( 8 15 )
arr10=( 0 1 )
arr11=( 100 1000 )

for a in "${arr1[@]}"
do
    for b in "${arr2[@]}"
    do
	for c in "${arr3[@]}"
	do
	    for d in "${arr4[@]}"
	    do
		for e in "${arr5[@]}"
		do
		    for f in "${arr6[@]}"
		    do
			for g in "${arr7[@]}"
			do
			    for h in "${arr8[@]}"
			    do
				for i in "${arr9[@]}"
				do
				    for i in "${arr10[@]}"
				    do
					for i in "${arr11[@]}"
					do 					NEWDIR="2pun_${a}_${b}_${c}_${d}_${e}_${f}_${g}_${h}_${i}_${j}_${k}"
					mkdir $NEWDIR
					cd $NEWDIR
					SHORTNAME="2pun${a}${b}${c}${d}${e}${f}${g}${h}${i}"
					rundir=$PWD
					executable_path=$1
					executable=$2
					cores=$3
					hours=$4
					nodes=$((${cores} / 8))
					write_options $a $b $c $d $e $f $g $h $i $j $k
					write_submit_graham $rundir $executable $SHORTNAME $cores $hours $nodes
					cp "${executable_path}/${executable}" "${PWD}/${executable}"
					cp "${executable_path}/*.conf" "${PWD}/"
					cd ..
					done
				    done
				done
			    done
			done
		    done  
		done
	    done
	done
    done  
done

