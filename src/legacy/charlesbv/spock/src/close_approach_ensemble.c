#include "options.h"
#include "moat_prototype.h"
#include "gsl/gsl_poly.h"

///
/// @brief Determines if there is a close approach
///
/// @details Determines if there is a close approach (defined as a minimum of
///          the distance) between the primary ensemble sc eee_prim and the
///          secondary ensemble sc eee in the interval of time spanning TCA (unperturbed).
///          If there is, it returns the position, velocity, and acceleration of
///          sc eee_prim and sc eee at both the beginning and end of interval of
///          time 3 * OPTIONS->dt in which the close approach between sc eee_prim
///          and sc eee is expected to be. It also returns the coefficients gamma
///          that are then used to calculate the min distance using the function
///          ancas (if there is a min distance).
///
/// @warning for now works only if two reference satellies only
///
/// @warning the 2 reference sc (unperturbed orbits) are assumed to have the same epoch start.
///
/// @note in arguments of close_approach_ensemble, start means the oldest time of
///       4 four latest points, and end means the most recent time of 4 four latest points.
///
/// @see  Algorithm as defined in Alfvano 2009 end of section II (Satellite Conjunction Monte Carlo Analysis) (same notations)
int close_approach_ensemble(
  double *eci_x_primary_sc_in_span,
  double *eci_y_primary_sc_in_span,
  double *eci_z_primary_sc_in_span,
  double *eci_vx_primary_sc_in_span,
  double *eci_vy_primary_sc_in_span,
  double *eci_vz_primary_sc_in_span,
  double *eci_ax_primary_sc_in_span,
  double *eci_ay_primary_sc_in_span,
  double *eci_az_primary_sc_in_span,
  double *eci_x_secondary_sc_in_span,
  double *eci_y_secondary_sc_in_span,
  double *eci_z_secondary_sc_in_span,
  double *eci_vx_secondary_sc_in_span,
  double *eci_vy_secondary_sc_in_span,
  double *eci_vz_secondary_sc_in_span,
  double *eci_ax_secondary_sc_in_span,
  double *eci_ay_secondary_sc_in_span,
  double *eci_az_secondary_sc_in_span,
  int time_step_of_tca,
  double *gamma0,
  double *gamma1,
  double *gamma2,
  double *gamma3,
  int *min_exists,
  double r1_start[3],
  double v1_start[3],
  double a1_start[3],
  double r1_end[3],
  double v1_end[3],
  double a1_end[3],
  double r2_start[3],
  double v2_start[3],
  double a2_start[3],
  double r2_end[3],
  double v2_end[3],
  double a2_end[3],
  int *time_step_start_interval,
  double *min_distance_in_time_spanning_tca,
  double *direction_distance,
  int initial_epoch_time_step_in_span,
  int final_epoch_time_step_in_span,
  int iProc,
  double et_time_step_of_save_tca,
  OPTIONS_T *OPTIONS )
{
  /* Declarations */
  int nb_time_steps_in_tca_time_span = time_step_of_tca * 2 + 1; // recall that nb_time_steps_in_tca_time_span is odd
  double et_start_of_span = et_time_step_of_save_tca - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );

  char time_et_time_step_of_save_tca[256], time_end_epoch[256], time_start_epoch[256];
  double eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

  double p1, p2, p3, p4;
  double D;
  double tau1, tau1_square, tau1_cube;
  double tau2, tau2_square, tau2_cube;
  double dist_at_step_plus_n_minus_one_dt, dist_at_step_plus_n_dt;
  double dist_at_step, dist_at_step_minus_one_dt, dist_at_step_minus_two_dt;
  double eci_r_secondary_minus_eci_r_primary[3], eci_v_secondary_minus_eci_v_primary[3];


  // the initial epoch needs to start at least three 3 time steps (because of the algorithm)
  // before TCA. So check that it's the case and if it's not the return an error message

  if (initial_epoch_time_step_in_span > time_step_of_tca - 2)
  {
    et2utc_c(et_time_step_of_save_tca, "ISOC", 3, 255, time_et_time_step_of_save_tca);
    et2utc_c(et_start_of_span + initial_epoch_time_step_in_span * OPTIONS->dt , "ISOC", 3, 255, time_start_epoch);
    printf("***! The propagation needs to start at least two time steps before TCA. Here the propagation starts at %s, but the first TCA is %s. The program will stop. (iProc: %d) !***\n", time_start_epoch , time_et_time_step_of_save_tca, iProc); MPI_Finalize(); exit(0);
  }

  if ( final_epoch_time_step_in_span < time_step_of_tca )
  {
    et2utc_c(et_time_step_of_save_tca, "ISOC", 3, 255, time_et_time_step_of_save_tca);
    et2utc_c(et_start_of_span + final_epoch_time_step_in_span * OPTIONS->dt , "ISOC", 3, 255, time_end_epoch);
    printf("***! The propation can not end before TCA (it can end at TCA). Here the propagation ends at %s, but the last TCA is %s. The program will stop. (iProc: %d) !***\n", time_end_epoch , time_et_time_step_of_save_tca, iProc); MPI_Finalize(); exit(0);
  }

  // D(TCA)
  eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca] - eci_x_primary_sc_in_span[time_step_of_tca];
  eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca] - eci_y_primary_sc_in_span[time_step_of_tca];
  eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca] - eci_z_primary_sc_in_span[time_step_of_tca];
  v_mag(&dist_at_step, eci_r_secondary_minus_eci_r_primary );

  // D(TCA-dt)
  eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-1] - eci_x_primary_sc_in_span[time_step_of_tca-1];
  eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-1] - eci_y_primary_sc_in_span[time_step_of_tca-1];
  eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-1] - eci_z_primary_sc_in_span[time_step_of_tca-1];
  v_mag(&dist_at_step_minus_one_dt, eci_r_secondary_minus_eci_r_primary );

  // D(TCA-2*dt)
  eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-2] - eci_x_primary_sc_in_span[time_step_of_tca-2];
  eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-2] - eci_y_primary_sc_in_span[time_step_of_tca-2];
  eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-2] - eci_z_primary_sc_in_span[time_step_of_tca-2];
  v_mag(&dist_at_step_minus_two_dt, eci_r_secondary_minus_eci_r_primary );

  // Determine direction
  *direction_distance = dist_at_step_minus_two_dt - dist_at_step_minus_one_dt;

  // Move backward if *direction_distance < 0, move forward if *direction_distance > 0
  int istep;
  int found_greater_distance = 0;
  double dist_at_step_minus_n_dt, dist_at_step_minus_n_minus_one_dt;
  dist_at_step_minus_n_minus_one_dt = dist_at_step_minus_two_dt;
  dist_at_step_minus_n_dt = dist_at_step_minus_two_dt;

  // this will be overwritten in the loop below. But in the case where
  // time_step_of_tca-3 <  initial_epoch_time_step_in_span
  // (so WHERE TIME_STEP_OF_TCA = INITIAL_EPOCH_TIME_STEP_IN_SPAN + 2 because
  //  time_step_of_tca can't be < initial_epoch_time_step_in_span + 2 because of
  // if condiction at beginning of this function), we don't get in the loop so
  // dist_at_step_minus_n_dt has to be equal to the distance at time_step_of_tca - 2,
  // which is the distance at initial epoch. This is a border limit case.

  if ( *direction_distance <= 0 )
  {
    istep = 3;
    while ( ( istep < time_step_of_tca ) && ( found_greater_distance == 0 ) && ( time_step_of_tca-istep >=  initial_epoch_time_step_in_span ) )
    {
      // go bakward in time until either finding a greater distance or reaching
      // the beginning of the time span (ie istep = time_step_of_tca)
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep] - eci_x_primary_sc_in_span[time_step_of_tca-istep];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep] - eci_y_primary_sc_in_span[time_step_of_tca-istep];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep] - eci_z_primary_sc_in_span[time_step_of_tca-istep];
      v_mag(&dist_at_step_minus_n_dt, eci_r_secondary_minus_eci_r_primary );

      if ( dist_at_step_minus_n_dt >= dist_at_step_minus_n_minus_one_dt )
      {
        found_greater_distance = 1;
      }

      dist_at_step_minus_n_minus_one_dt = dist_at_step_minus_n_dt;
      istep = istep + 1;
    }
    istep = istep - 1;
  }
  else
  {
    istep = 1;
    dist_at_step_plus_n_minus_one_dt = dist_at_step;
    dist_at_step_plus_n_dt = dist_at_step;
    // this will be overwritten in the loop below. But in the case where
    // time_step_of_tca+1 >  final_epoch_time_step_in_span
    // (so WHERE TIME_STEP_OF_TCA + 1 = FINAL_EPOCH_TIME_STEP_IN_SPAN + 1
    // (so TIME_STEP_OF_TCA = FINAL_EPOCH_TIME_STEP_IN_SPAN) because
    // time_step_of_tca + 1 can't be > final_epoch_time_step_in_span + 1
    // (because time_step_of_tca can't be > final_epoch_time_step_in_span)
    // because of if condiction at beginning of this function), we don't get
    // in the loop so dist_at_step_plus_n_dt has to be equal to the distance at
    // time_step_of_tca, which is the distance at final epoch. This is a border limit case.
    while ( ( istep < time_step_of_tca ) && ( found_greater_distance == 0 ) && (time_step_of_tca+istep <= final_epoch_time_step_in_span) )
    {
      // go foward in time until either finding a greater distance or
      // reaching the end of the time span (ie istep = time_step_of_tca)
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep] - eci_x_primary_sc_in_span[time_step_of_tca+istep];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep] - eci_y_primary_sc_in_span[time_step_of_tca+istep];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep] - eci_z_primary_sc_in_span[time_step_of_tca+istep];

      v_mag(&dist_at_step_plus_n_dt, eci_r_secondary_minus_eci_r_primary );
      if ( dist_at_step_plus_n_dt >= dist_at_step_plus_n_minus_one_dt )
      {
        found_greater_distance = 1;
      }

      dist_at_step_plus_n_minus_one_dt = dist_at_step_plus_n_dt;
      istep = istep + 1;
    }
    istep = istep - 1;
  }

  // INITIALIZE ANCAS

  // By initialioze ANCS, we mean calculate the gamma coefficients (see Alfano 1994)
  // that are then used in the function ancas (we call the cunfction ancas only
  // if we know in this function here that there is a minimum (see test at the
  // end of this function))

  // If found_greater_distance = 1, then use the last four consecutive distances
  // in ANCAS algorithm to determine minimum distance

  if ( found_greater_distance == 1 )
  {
    // if a greater distance have been found, then compute minimum distance with ANCAS
    // The reason we don't use the function ancas is because we use 4 values
    // of the function to determine the min, while the function ancas uses 2
    // values of the function and 2 values of the derivative of the function
    // (we actually could do that too but we want to use a similar algorithm
    // to Alfano 2009 to then compare the results)

    // Four last points
    if ( *direction_distance <= 0 )
    {
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep] - eci_x_primary_sc_in_span[time_step_of_tca-istep];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep] - eci_y_primary_sc_in_span[time_step_of_tca-istep];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep] - eci_z_primary_sc_in_span[time_step_of_tca-istep];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca-istep] - eci_vx_primary_sc_in_span[time_step_of_tca-istep];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca-istep] - eci_vy_primary_sc_in_span[time_step_of_tca-istep];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca-istep] - eci_vz_primary_sc_in_span[time_step_of_tca-istep];

      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p1 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

      // p2
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep + 1] - eci_x_primary_sc_in_span[time_step_of_tca-istep + 1];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep + 1] - eci_y_primary_sc_in_span[time_step_of_tca-istep + 1];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep + 1] - eci_z_primary_sc_in_span[time_step_of_tca-istep + 1];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca-istep + 1] - eci_vx_primary_sc_in_span[time_step_of_tca-istep + 1];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca-istep + 1] - eci_vy_primary_sc_in_span[time_step_of_tca-istep + 1];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca-istep + 1] - eci_vz_primary_sc_in_span[time_step_of_tca-istep + 1];

      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p2 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

      // p3
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep + 2] - eci_x_primary_sc_in_span[time_step_of_tca-istep + 2];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep + 2] - eci_y_primary_sc_in_span[time_step_of_tca-istep + 2];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep + 2] - eci_z_primary_sc_in_span[time_step_of_tca-istep + 2];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca-istep + 2] - eci_vx_primary_sc_in_span[time_step_of_tca-istep + 2];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca-istep + 2] - eci_vy_primary_sc_in_span[time_step_of_tca-istep + 2];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca-istep + 2] - eci_vz_primary_sc_in_span[time_step_of_tca-istep + 2];
      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p3 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

      // p4
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep + 3] - eci_x_primary_sc_in_span[time_step_of_tca-istep + 3];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep + 3] - eci_y_primary_sc_in_span[time_step_of_tca-istep + 3];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep + 3] - eci_z_primary_sc_in_span[time_step_of_tca-istep + 3];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca-istep + 3] - eci_vx_primary_sc_in_span[time_step_of_tca-istep + 3];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca-istep + 3] - eci_vy_primary_sc_in_span[time_step_of_tca-istep + 3];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca-istep + 3] - eci_vz_primary_sc_in_span[time_step_of_tca-istep + 3];
      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p4 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

    }
    else
    {

      // p4
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep] - eci_x_primary_sc_in_span[time_step_of_tca+istep];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep] - eci_y_primary_sc_in_span[time_step_of_tca+istep];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep] - eci_z_primary_sc_in_span[time_step_of_tca+istep];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca+istep] - eci_vx_primary_sc_in_span[time_step_of_tca+istep];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca+istep] - eci_vy_primary_sc_in_span[time_step_of_tca+istep];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca+istep] - eci_vz_primary_sc_in_span[time_step_of_tca+istep];
      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p4 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

      // p3
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep - 1] - eci_x_primary_sc_in_span[time_step_of_tca+istep - 1];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep - 1] - eci_y_primary_sc_in_span[time_step_of_tca+istep - 1];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep - 1] - eci_z_primary_sc_in_span[time_step_of_tca+istep - 1];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca+istep - 1] - eci_vx_primary_sc_in_span[time_step_of_tca+istep - 1];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca+istep - 1] - eci_vy_primary_sc_in_span[time_step_of_tca+istep - 1];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca+istep - 1] - eci_vz_primary_sc_in_span[time_step_of_tca+istep - 1];
      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p3 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

      // p2
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep - 2] - eci_x_primary_sc_in_span[time_step_of_tca+istep - 2];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep - 2] - eci_y_primary_sc_in_span[time_step_of_tca+istep - 2];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep - 2] - eci_z_primary_sc_in_span[time_step_of_tca+istep - 2];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca+istep - 2] - eci_vx_primary_sc_in_span[time_step_of_tca+istep - 2];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca+istep - 2] - eci_vy_primary_sc_in_span[time_step_of_tca+istep - 2];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca+istep - 2] - eci_vz_primary_sc_in_span[time_step_of_tca+istep - 2];

      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p2 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;

      // p1
      eci_r_secondary_minus_eci_r_primary[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep - 3] - eci_x_primary_sc_in_span[time_step_of_tca+istep - 3];
      eci_r_secondary_minus_eci_r_primary[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep - 3] - eci_y_primary_sc_in_span[time_step_of_tca+istep - 3];
      eci_r_secondary_minus_eci_r_primary[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep - 3] - eci_z_primary_sc_in_span[time_step_of_tca+istep - 3];

      eci_v_secondary_minus_eci_v_primary[0] = eci_vx_secondary_sc_in_span[time_step_of_tca+istep - 3] - eci_vx_primary_sc_in_span[time_step_of_tca+istep - 3];
      eci_v_secondary_minus_eci_v_primary[1] = eci_vy_secondary_sc_in_span[time_step_of_tca+istep - 3] - eci_vy_primary_sc_in_span[time_step_of_tca+istep - 3];
      eci_v_secondary_minus_eci_v_primary[2] = eci_vz_secondary_sc_in_span[time_step_of_tca+istep - 3] - eci_vz_primary_sc_in_span[time_step_of_tca+istep - 3];
      v_dot( &eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary, eci_r_secondary_minus_eci_r_primary, eci_v_secondary_minus_eci_v_primary );
      p1 = 2 * eci_r_secondary_minus_eci_r_primary_dot_eci_v_secondary_minus_eci_v_primary;
    }

    // times
    tau1 = 1./3;
    tau2 = 2./3;
    tau1_cube = tau1 * tau1 * tau1;
    tau2_cube = tau2 * tau2 * tau2;
    tau1_square = tau1 * tau1;
    tau2_square = tau2 * tau2;

    // D
    D = tau1_cube * tau2_square + tau1_square * tau2 + tau1 * tau2_cube - tau1 * tau2_square - tau1_cube * tau2 - tau1_square * tau2_cube;

    // ANCAS's equations
    *gamma0 = p1;
    *gamma1 = ( ( tau2_cube - tau2_square ) * ( p2 - p1 ) + ( tau1_square - tau1_cube ) * ( p3 - p1 ) + ( tau1_cube * tau2_square - tau1_square * tau2_cube) * ( p4 - p1 ) ) /  D;
    *gamma2 = ( ( tau2 - tau2_cube ) * ( p2 - p1 ) + ( tau1_cube - tau1 ) * ( p3 - p1 ) + ( tau1 * tau2_cube - tau1_cube * tau2 ) * ( p4 - p1 ) ) /  D;
    *gamma3 = ( ( tau2_square - tau2 ) * ( p2 - p1 ) + ( tau1 - tau1_square ) * ( p3 - p1 ) + ( tau1_square * tau2 - tau1 * tau2_square ) * ( p4 - p1 ) ) /  D;

    // Check if there is a root, because if there's not then it means there is
    // min distance between the 2 sc so no need to compute gsl_poly_solve_cubic in ancas function
    /// @todo: can surely be replaced by using STL or BOOST math library
    int real_root_exists = 1;
    double min1;
    double max1;
    if ( *gamma0 > 0 )
    {
      min1 = *gamma1;
      if (*gamma1 + *gamma2 < min1)
      {
        min1 = *gamma1 + *gamma2 ;
      }
      if (*gamma1 + *gamma2 + *gamma3 < min1)
      {
        min1 = *gamma1 + *gamma2 + *gamma3 ;
      }
      if (min1 > -*gamma0)
      {
        real_root_exists = 0;
      }
    }
    else
    {
      max1 = *gamma1;
      if (*gamma1 + *gamma2 > max1)
      {
        max1 = *gamma1 + *gamma2 ;
      }
      if(*gamma1 + *gamma2 + *gamma3 > max1)
      {
        max1 = *gamma1 + *gamma2 + *gamma3;
      }
      if (max1 < -*gamma0)
      {
        real_root_exists = 0;
      }
    }

    if ( real_root_exists == 1 ){
      *min_exists = 1;

      // Record position, velocity, and acceration at P1 and P4 of each of the 2 sc. These will be used in ancas to calculate the min distance
      if ( *direction_distance <= 0 )
      {
        *time_step_start_interval =  time_step_of_tca-istep;

        r1_start[0] = eci_x_primary_sc_in_span[time_step_of_tca-istep];	r1_start[1] = eci_y_primary_sc_in_span[time_step_of_tca-istep];	r1_start[2] = eci_z_primary_sc_in_span[time_step_of_tca-istep];
        v1_start[0] = eci_vx_primary_sc_in_span[time_step_of_tca-istep];	v1_start[1] = eci_vy_primary_sc_in_span[time_step_of_tca-istep];	v1_start[2] = eci_vz_primary_sc_in_span[time_step_of_tca-istep];
        a1_start[0] = eci_ax_primary_sc_in_span[time_step_of_tca-istep];	a1_start[1] = eci_ay_primary_sc_in_span[time_step_of_tca-istep];	a1_start[2] = eci_az_primary_sc_in_span[time_step_of_tca-istep];

        r2_start[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep];	r2_start[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep];	r2_start[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep];
        v2_start[0] = eci_vx_secondary_sc_in_span[time_step_of_tca-istep];	v2_start[1] = eci_vy_secondary_sc_in_span[time_step_of_tca-istep];	v2_start[2] = eci_vz_secondary_sc_in_span[time_step_of_tca-istep];
        a2_start[0] = eci_ax_secondary_sc_in_span[time_step_of_tca-istep];	a2_start[1] = eci_ay_secondary_sc_in_span[time_step_of_tca-istep];	a2_start[2] = eci_az_secondary_sc_in_span[time_step_of_tca-istep];


        r1_end[0] = eci_x_primary_sc_in_span[time_step_of_tca-istep+3];	r1_end[1] = eci_y_primary_sc_in_span[time_step_of_tca-istep+3];	r1_end[2] = eci_z_primary_sc_in_span[time_step_of_tca-istep+3];
        v1_end[0] = eci_vx_primary_sc_in_span[time_step_of_tca-istep+3];	v1_end[1] = eci_vy_primary_sc_in_span[time_step_of_tca-istep+3];	v1_end[2] = eci_vz_primary_sc_in_span[time_step_of_tca-istep+3];
        a1_end[0] = eci_ax_primary_sc_in_span[time_step_of_tca-istep+3];	a1_end[1] = eci_ay_primary_sc_in_span[time_step_of_tca-istep+3];	a1_end[2] = eci_az_primary_sc_in_span[time_step_of_tca-istep+3];

        r2_end[0] = eci_x_secondary_sc_in_span[time_step_of_tca-istep+3];	r2_end[1] = eci_y_secondary_sc_in_span[time_step_of_tca-istep+3];	r2_end[2] = eci_z_secondary_sc_in_span[time_step_of_tca-istep+3];
        v2_end[0] = eci_vx_secondary_sc_in_span[time_step_of_tca-istep+3];	v2_end[1] = eci_vy_secondary_sc_in_span[time_step_of_tca-istep+3];	v2_end[2] = eci_vz_secondary_sc_in_span[time_step_of_tca-istep+3];
        a2_end[0] = eci_ax_secondary_sc_in_span[time_step_of_tca-istep+3];	a2_end[1] = eci_ay_secondary_sc_in_span[time_step_of_tca-istep+3];	a2_end[2] = eci_az_secondary_sc_in_span[time_step_of_tca-istep+3 ];
      }
      else
      {
        *time_step_start_interval =  time_step_of_tca+istep-3;

        r1_start[0] = eci_x_primary_sc_in_span[time_step_of_tca+istep-3];	r1_start[1] = eci_y_primary_sc_in_span[time_step_of_tca+istep-3];	r1_start[2] = eci_z_primary_sc_in_span[time_step_of_tca+istep-3];
        v1_start[0] = eci_vx_primary_sc_in_span[time_step_of_tca+istep-3];	v1_start[1] = eci_vy_primary_sc_in_span[time_step_of_tca+istep-3];	v1_start[2] = eci_vz_primary_sc_in_span[time_step_of_tca+istep-3];
        a1_start[0] = eci_ax_primary_sc_in_span[time_step_of_tca+istep-3];	a1_start[1] = eci_ay_primary_sc_in_span[time_step_of_tca+istep-3];	a1_start[2] = eci_az_primary_sc_in_span[time_step_of_tca+istep-3];

        r2_start[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep-3];	r2_start[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep-3];	r2_start[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep-3];
        v2_start[0] = eci_vx_secondary_sc_in_span[time_step_of_tca+istep-3];	v2_start[1] = eci_vy_secondary_sc_in_span[time_step_of_tca+istep-3];	v2_start[2] = eci_vz_secondary_sc_in_span[time_step_of_tca+istep-3];
        a2_start[0] = eci_ax_secondary_sc_in_span[time_step_of_tca+istep-3];	a2_start[1] = eci_ay_secondary_sc_in_span[time_step_of_tca+istep-3];	a2_start[2] = eci_az_secondary_sc_in_span[time_step_of_tca+istep-3];

        r1_end[0] = eci_x_primary_sc_in_span[time_step_of_tca+istep];	r1_end[1] = eci_y_primary_sc_in_span[time_step_of_tca+istep];	r1_end[2] = eci_z_primary_sc_in_span[time_step_of_tca+istep];
        v1_end[0] = eci_vx_primary_sc_in_span[time_step_of_tca+istep];	v1_end[1] = eci_vy_primary_sc_in_span[time_step_of_tca+istep];	v1_end[2] = eci_vz_primary_sc_in_span[time_step_of_tca+istep];
        a1_end[0] = eci_ax_primary_sc_in_span[time_step_of_tca+istep];	a1_end[1] = eci_ay_primary_sc_in_span[time_step_of_tca+istep];	a1_end[2] = eci_az_primary_sc_in_span[time_step_of_tca+istep];

        r2_end[0] = eci_x_secondary_sc_in_span[time_step_of_tca+istep];	r2_end[1] = eci_y_secondary_sc_in_span[time_step_of_tca+istep];	r2_end[2] = eci_z_secondary_sc_in_span[time_step_of_tca+istep];
        v2_end[0] = eci_vx_secondary_sc_in_span[time_step_of_tca+istep];	v2_end[1] = eci_vy_secondary_sc_in_span[time_step_of_tca+istep];	v2_end[2] = eci_vz_secondary_sc_in_span[time_step_of_tca+istep];
        a2_end[0] = eci_ax_secondary_sc_in_span[time_step_of_tca+istep];	a2_end[1] = eci_ay_secondary_sc_in_span[time_step_of_tca+istep];	a2_end[2] = eci_az_secondary_sc_in_span[time_step_of_tca-istep+3 ];
      }
    }
    else
    {
      *min_exists = 0;
    }
  } // end of if a greater distance have been found, then compute minimum distance with ANCAS

  else{
    *min_exists = 0;

    if ( *direction_distance <= 0 )
    {
      // if no min in distance has been found (in other words if the derivative of
      // the distance with respect to time does not cancel), then save the last
      // distance dist_at_step_minus_n_dt. This last distance dist_at_step_minus_n_dt
      // corresponds to the distance at the beginning (if *direction_distance < 0)
      // or end (if *direction_distance > 0) of the time spanning TCA, which is
      // the minimum distance between over this entire time spanning TCA
      *min_distance_in_time_spanning_tca = dist_at_step_minus_n_dt;
    }
    else
    {
      *min_distance_in_time_spanning_tca = dist_at_step_plus_n_dt;
    }
  }

  // END OF INITIALIZE ANCAS

  return 0;
}
