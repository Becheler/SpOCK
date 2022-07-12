#include "options.h"
#include "moat_prototype.h"
#include "gsl/gsl_poly.h"

///
/// @brief Interpolate the positions of a satellite
///
/// @note ANCAS stands for Alfano/Negron Close Approach Software
/// @see Collision Avoidance Methods
///
int ancas(double min_dist_close_approach, double et_start, double dt_interval, double r1_start[3], double v1_start[3], double a1_start[3], double r1_end[3],double  v1_end[3], double  a1_end[3],double r2_start[3], double v2_start[3], double a2_start[3], double r2_end[3],double  v2_end[3], double  a2_end[3], double gamma0, double gamma1, double gamma2, double gamma3, double *tca1, double *dca1, double *tca2, double *dca2, double *tca3, double *dca3){ // source: Alfvano 1994

  double gamma0_normalized,  gamma1_normalized, gamma2_normalized;
  double root1 = -1e6, root2 = -1e6, root3 = -1e6 ;
  double tca2_before = *tca2, dca2_before = *dca2;
  double tca3_before = *tca3, dca3_before = *dca3;

  // Normalize the coefficients so that the coefficient of order 3 is 1
  gamma0_normalized = gamma0 / gamma3;
  gamma1_normalized = gamma1 / gamma3;
  gamma2_normalized = gamma2 / gamma3;

  gsl_poly_solve_cubic( gamma2_normalized, gamma1_normalized, gamma0_normalized, &root1, &root2, &root3 );

  // test if it's a minimum and not a maximum
  double C_dot_of_f_dot_at_root;
  double found_closest_approach_root1 = 0, found_closest_approach_root2 = 0, found_closest_approach_root3 = 0;

  if (( root1 >=0 ) && ( root1 <= 1 ) )
  {
    C_dot_of_f_dot_at_root = 3 * gamma3 * root1*root1 + 2*gamma2 * root1 + gamma1 ;
    if ( C_dot_of_f_dot_at_root > 0 )
    {
      // Find the associated range of closest approach (noted rmin_root1)
      double rmin_root1, q_rdi_root1, q_rdj_root1, q_rdk_root1;
      double alpha0, alpha1, alpha2, alpha3, alpha4, alpha5;
      double dt_interval_square = dt_interval*dt_interval;
      double root1_to_the_square = root1 * root1;
      double root1_to_the_third = root1 * root1 * root1;
      double root1_to_the_fourth = root1 * root1 * root1 * root1 ;
      double root1_to_the_fifth = root1 * root1 * root1 * root1 * root1;

      // I component
      double frdi_start, frdi_dot_start, frdi_dot_dot_start;
      double frdi_end, frdi_dot_end, frdi_dot_dot_end;
      frdi_start = r2_start[0] - r1_start[0];
      frdi_dot_start = v2_start[0] - v1_start[0];
      frdi_dot_dot_start = a2_start[0] - a1_start[0];
      frdi_end = r2_end[0] - r1_end[0];
      frdi_dot_end = v2_end[0] - v1_end[0];
      frdi_dot_dot_end = a2_end[0] - a1_end[0];

      alpha0 = frdi_start;
      alpha1 = frdi_dot_start * dt_interval;
      alpha2 = 0.5 * frdi_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdi_start - 6 * frdi_dot_start * dt_interval -1.5 * frdi_dot_dot_start * dt_interval_square
      + 10 * frdi_end - 4 * frdi_dot_end * dt_interval + 0.5 * frdi_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdi_start + 8 * frdi_dot_start * dt_interval + 1.5 * frdi_dot_dot_start * dt_interval_square
      -15 * frdi_end + 7 * frdi_dot_end * dt_interval - frdi_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdi_start - 3 * frdi_dot_start * dt_interval - 0.5 * frdi_dot_dot_start * dt_interval_square
      +6 * frdi_end - 3 * frdi_dot_end * dt_interval + 0.5 * frdi_dot_dot_end * dt_interval_square;

      q_rdi_root1 = alpha5 * root1_to_the_fifth + alpha4 * root1_to_the_fourth + alpha3 * root1_to_the_third + alpha2 * root1_to_the_square + alpha1 * root1 + alpha0;

      // // J component
      double frdj_start, frdj_dot_start, frdj_dot_dot_start;
      double frdj_end, frdj_dot_end, frdj_dot_dot_end;
      frdj_start = r2_start[1] - r1_start[1];
      frdj_dot_start = v2_start[1] - v1_start[1];
      frdj_dot_dot_start = a2_start[1] - a1_start[1];
      frdj_end = r2_end[1] - r1_end[1];
      frdj_dot_end = v2_end[1] - v1_end[1];
      frdj_dot_dot_end = a2_end[1] - a1_end[1];

      alpha0 = frdj_start;
      alpha1 = frdj_dot_start * dt_interval;
      alpha2 = 0.5 * frdj_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdj_start - 6 * frdj_dot_start * dt_interval -1.5 * frdj_dot_dot_start * dt_interval_square
      + 10 * frdj_end - 4 * frdj_dot_end * dt_interval + 0.5 * frdj_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdj_start + 8 * frdj_dot_start * dt_interval + 1.5 * frdj_dot_dot_start * dt_interval_square
      -15 * frdj_end + 7 * frdj_dot_end * dt_interval - frdj_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdj_start - 3 * frdj_dot_start * dt_interval - 0.5 * frdj_dot_dot_start * dt_interval_square
      +6 * frdj_end - 3 * frdj_dot_end * dt_interval + 0.5 * frdj_dot_dot_end * dt_interval_square;

      q_rdj_root1 = alpha5 * root1_to_the_fifth + alpha4 * root1_to_the_fourth + alpha3 * root1_to_the_third + alpha2 * root1_to_the_square  + alpha1 * root1 + alpha0;

      // // K component
      double frdk_start, frdk_dot_start, frdk_dot_dot_start;
      double frdk_end, frdk_dot_end, frdk_dot_dot_end;
      frdk_start = r2_start[2] - r1_start[2];
      frdk_dot_start = v2_start[2] - v1_start[2];
      frdk_dot_dot_start = a2_start[2] - a1_start[2];
      frdk_end = r2_end[2] - r1_end[2];
      frdk_dot_end = v2_end[2] - v1_end[2];
      frdk_dot_dot_end = a2_end[2] - a1_end[2];

      alpha0 = frdk_start;
      alpha1 = frdk_dot_start * dt_interval;
      alpha2 = 0.5 * frdk_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdk_start - 6 * frdk_dot_start * dt_interval -1.5 * frdk_dot_dot_start * dt_interval_square
      + 10 * frdk_end - 4 * frdk_dot_end * dt_interval + 0.5 * frdk_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdk_start + 8 * frdk_dot_start * dt_interval + 1.5 * frdk_dot_dot_start * dt_interval_square
      -15 * frdk_end + 7 * frdk_dot_end * dt_interval - frdk_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdk_start - 3 * frdk_dot_start * dt_interval - 0.5 * frdk_dot_dot_start * dt_interval_square
      +6 * frdk_end - 3 * frdk_dot_end * dt_interval + 0.5 * frdk_dot_dot_end * dt_interval_square;

      q_rdk_root1 = alpha5 * root1_to_the_fifth + alpha4 * root1_to_the_fourth + alpha3 * root1_to_the_third + alpha2 * root1_to_the_square + alpha1 * root1 + alpha0;

      // // Compute rmin
      rmin_root1 = sqrt( q_rdi_root1 * q_rdi_root1 + q_rdj_root1 * q_rdj_root1 + q_rdk_root1 * q_rdk_root1 );

      if ( ( rmin_root1 <= min_dist_close_approach ) && ( rmin_root1 >= 0 ))
      {
        *dca1 = rmin_root1;

        // Find the associated time of closest approach (noted t_root1)
        *tca1 = et_start+ root1 * dt_interval;
        found_closest_approach_root1 = 1;
      }
    }
  }

  if ( ( root2 >=0 ) && ( root2 <= 1 ) )
  {
    C_dot_of_f_dot_at_root = 3 * gamma3 * root2*root2 + 2*gamma2 * root2 + gamma1 ;
    if ( C_dot_of_f_dot_at_root > 0 )
    {
      // Find the associated range of closest approach (noted rmin_root2)
      double rmin_root2, q_rdi_root2, q_rdj_root2, q_rdk_root2;
      double alpha0, alpha1, alpha2, alpha3, alpha4, alpha5;
      double dt_interval_square = dt_interval*dt_interval;
      double root2_to_the_square = root2 * root2;
      double root2_to_the_third = root2 * root2 * root2;
      double root2_to_the_fourth = root2 * root2 * root2 * root2 ;
      double root2_to_the_fifth = root2 * root2 * root2 * root2 * root2;

      // // I component
      double frdi_start, frdi_dot_start, frdi_dot_dot_start;
      double frdi_end, frdi_dot_end, frdi_dot_dot_end;
      frdi_start = r2_start[0] - r1_start[0];
      frdi_dot_start = v2_start[0] - v1_start[0];
      frdi_dot_dot_start = a2_start[0] - a1_start[0];
      frdi_end = r2_end[0] - r1_end[0];
      frdi_dot_end = v2_end[0] - v1_end[0];
      frdi_dot_dot_end = a2_end[0] - a1_end[0];

      alpha0 = frdi_start;
      alpha1 = frdi_dot_start * dt_interval;
      alpha2 = 0.5 * frdi_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdi_start - 6 * frdi_dot_start * dt_interval -1.5 * frdi_dot_dot_start * dt_interval_square
      + 10 * frdi_end - 4 * frdi_dot_end * dt_interval + 0.5 * frdi_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdi_start + 8 * frdi_dot_start * dt_interval + 1.5 * frdi_dot_dot_start * dt_interval_square
      -15 * frdi_end + 7 * frdi_dot_end * dt_interval - frdi_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdi_start - 3 * frdi_dot_start * dt_interval - 0.5 * frdi_dot_dot_start * dt_interval_square
      +6 * frdi_end - 3 * frdi_dot_end * dt_interval + 0.5 * frdi_dot_dot_end * dt_interval_square;

      q_rdi_root2 = alpha5 * root2_to_the_fifth + alpha4 * root2_to_the_fourth + alpha3 * root2_to_the_third + alpha2 * root2_to_the_square + alpha1 * root2 + alpha0;

      // // J component
      double frdj_start, frdj_dot_start, frdj_dot_dot_start;
      double frdj_end, frdj_dot_end, frdj_dot_dot_end;
      frdj_start = r2_start[1] - r1_start[1];
      frdj_dot_start = v2_start[1] - v1_start[1];
      frdj_dot_dot_start = a2_start[1] - a1_start[1];
      frdj_end = r2_end[1] - r1_end[1];
      frdj_dot_end = v2_end[1] - v1_end[1];
      frdj_dot_dot_end = a2_end[1] - a1_end[1];

      alpha0 = frdj_start;
      alpha1 = frdj_dot_start * dt_interval;
      alpha2 = 0.5 * frdj_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdj_start - 6 * frdj_dot_start * dt_interval -1.5 * frdj_dot_dot_start * dt_interval_square
      + 10 * frdj_end - 4 * frdj_dot_end * dt_interval + 0.5 * frdj_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdj_start + 8 * frdj_dot_start * dt_interval + 1.5 * frdj_dot_dot_start * dt_interval_square
      -15 * frdj_end + 7 * frdj_dot_end * dt_interval - frdj_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdj_start - 3 * frdj_dot_start * dt_interval - 0.5 * frdj_dot_dot_start * dt_interval_square
      +6 * frdj_end - 3 * frdj_dot_end * dt_interval + 0.5 * frdj_dot_dot_end * dt_interval_square;

      q_rdj_root2 = alpha5 * root2_to_the_fifth + alpha4 * root2_to_the_fourth + alpha3 * root2_to_the_third + alpha2 * root2_to_the_square + alpha1 * root2 + alpha0;

      // // K component
      double frdk_start, frdk_dot_start, frdk_dot_dot_start;
      double frdk_end, frdk_dot_end, frdk_dot_dot_end;
      frdk_start = r2_start[2] - r1_start[2];
      frdk_dot_start = v2_start[2] - v1_start[2];
      frdk_dot_dot_start = a2_start[2] - a1_start[2];
      frdk_end = r2_end[2] - r1_end[2];
      frdk_dot_end = v2_end[2] - v1_end[2];
      frdk_dot_dot_end = a2_end[2] - a1_end[2];

      alpha0 = frdk_start;
      alpha1 = frdk_dot_start * dt_interval;
      alpha2 = 0.5 * frdk_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdk_start - 6 * frdk_dot_start * dt_interval -1.5 * frdk_dot_dot_start * dt_interval_square
      + 10 * frdk_end - 4 * frdk_dot_end * dt_interval + 0.5 * frdk_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdk_start + 8 * frdk_dot_start * dt_interval + 1.5 * frdk_dot_dot_start * dt_interval_square
      -15 * frdk_end + 7 * frdk_dot_end * dt_interval - frdk_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdk_start - 3 * frdk_dot_start * dt_interval - 0.5 * frdk_dot_dot_start * dt_interval_square
      +6 * frdk_end - 3 * frdk_dot_end * dt_interval + 0.5 * frdk_dot_dot_end * dt_interval_square;

      q_rdk_root2 = alpha5 * root2_to_the_fifth + alpha4 * root2_to_the_fourth + alpha3 * root2_to_the_third + alpha2 * root2_to_the_square + alpha1 * root2 + alpha0;

      // // Compute rmin
      rmin_root2 = sqrt( q_rdi_root2 * q_rdi_root2 + q_rdj_root2 * q_rdj_root2 + q_rdk_root2 * q_rdk_root2 );

      if ( ( rmin_root2 <= min_dist_close_approach ) && ( rmin_root2 >= 0 ) )
      {
        *dca2 = rmin_root2;
        // Find the associated time of closest approach (noted t_root2)
        *tca2 = et_start+ root2 * dt_interval;
        found_closest_approach_root2 = 1;
      }
    }
  }
  if ( ( root3 >=0 ) && ( root3 <= 1 ) )
  {
    C_dot_of_f_dot_at_root = 3 * gamma3 * root3*root3 + 2*gamma2 * root3 + gamma1 ;
    if ( C_dot_of_f_dot_at_root > 0 )
    {
      // Find the associated range of closest approach (noted rmin_root3)
      double rmin_root3, q_rdi_root3, q_rdj_root3, q_rdk_root3;
      double alpha0, alpha1, alpha2, alpha3, alpha4, alpha5;
      double dt_interval_square = dt_interval*dt_interval;
      double root3_to_the_square = root3 * root3;
      double root3_to_the_third = root3 * root3 * root3;
      double root3_to_the_fourth = root3 * root3 * root3 * root3 ;
      double root3_to_the_fifth = root3 * root3 * root3 * root3 * root3;

      // // I component
      double frdi_start, frdi_dot_start, frdi_dot_dot_start;
      double frdi_end, frdi_dot_end, frdi_dot_dot_end;
      frdi_start = r2_start[0] - r1_start[0];
      frdi_dot_start = v2_start[0] - v1_start[0];
      frdi_dot_dot_start = a2_start[0] - a1_start[0];
      frdi_end = r2_end[0] - r1_end[0];
      frdi_dot_end = v2_end[0] - v1_end[0];
      frdi_dot_dot_end = a2_end[0] - a1_end[0];

      alpha0 = frdi_start;
      alpha1 = frdi_dot_start * dt_interval;
      alpha2 = 0.5 * frdi_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdi_start - 6 * frdi_dot_start * dt_interval -1.5 * frdi_dot_dot_start * dt_interval_square
      + 10 * frdi_end - 4 * frdi_dot_end * dt_interval + 0.5 * frdi_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdi_start + 8 * frdi_dot_start * dt_interval + 1.5 * frdi_dot_dot_start * dt_interval_square
      -15 * frdi_end + 7 * frdi_dot_end * dt_interval - frdi_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdi_start - 3 * frdi_dot_start * dt_interval - 0.5 * frdi_dot_dot_start * dt_interval_square
      +6 * frdi_end - 3 * frdi_dot_end * dt_interval + 0.5 * frdi_dot_dot_end * dt_interval_square;

      q_rdi_root3 = alpha5 * root3_to_the_fifth + alpha4 * root3_to_the_fourth + alpha3 * root3_to_the_third + alpha2 * root3_to_the_square + alpha1 * root3 + alpha0;

      // // J component
      double frdj_start, frdj_dot_start, frdj_dot_dot_start;
      double frdj_end, frdj_dot_end, frdj_dot_dot_end;
      frdj_start = r2_start[1] - r1_start[1];
      frdj_dot_start = v2_start[1] - v1_start[1];
      frdj_dot_dot_start = a2_start[1] - a1_start[1];
      frdj_end = r2_end[1] - r1_end[1];
      frdj_dot_end = v2_end[1] - v1_end[1];
      frdj_dot_dot_end = a2_end[1] - a1_end[1];

      alpha0 = frdj_start;
      alpha1 = frdj_dot_start * dt_interval;
      alpha2 = 0.5 * frdj_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdj_start - 6 * frdj_dot_start * dt_interval -1.5 * frdj_dot_dot_start * dt_interval_square
      + 10 * frdj_end - 4 * frdj_dot_end * dt_interval + 0.5 * frdj_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdj_start + 8 * frdj_dot_start * dt_interval + 1.5 * frdj_dot_dot_start * dt_interval_square
      -15 * frdj_end + 7 * frdj_dot_end * dt_interval - frdj_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdj_start - 3 * frdj_dot_start * dt_interval - 0.5 * frdj_dot_dot_start * dt_interval_square
      +6 * frdj_end - 3 * frdj_dot_end * dt_interval + 0.5 * frdj_dot_dot_end * dt_interval_square;

      q_rdj_root3 = alpha5 * root3_to_the_fifth + alpha4 * root3_to_the_fourth + alpha3 * root3_to_the_third + alpha2 * root3_to_the_square + alpha1 * root3 + alpha0;

      // // K component
      double frdk_start, frdk_dot_start, frdk_dot_dot_start;
      double frdk_end, frdk_dot_end, frdk_dot_dot_end;

      frdk_start = r2_start[2] - r1_start[2];
      frdk_dot_start = v2_start[2] - v1_start[2];
      frdk_dot_dot_start = a2_start[2] - a1_start[2];
      frdk_end = r2_end[2] - r1_end[2];
      frdk_dot_end = v2_end[2] - v1_end[2];
      frdk_dot_dot_end = a2_end[2] - a1_end[2];

      alpha0 = frdk_start;
      alpha1 = frdk_dot_start * dt_interval;
      alpha2 = 0.5 * frdk_dot_dot_start * dt_interval_square;

      alpha3 = -10 * frdk_start - 6 * frdk_dot_start * dt_interval -1.5 * frdk_dot_dot_start * dt_interval_square
      + 10 * frdk_end - 4 * frdk_dot_end * dt_interval + 0.5 * frdk_dot_dot_end * dt_interval_square;

      alpha4 = 15 * frdk_start + 8 * frdk_dot_start * dt_interval + 1.5 * frdk_dot_dot_start * dt_interval_square
      -15 * frdk_end + 7 * frdk_dot_end * dt_interval - frdk_dot_dot_end * dt_interval_square;

      alpha5 = -6 * frdk_start - 3 * frdk_dot_start * dt_interval - 0.5 * frdk_dot_dot_start * dt_interval_square
      +6 * frdk_end - 3 * frdk_dot_end * dt_interval + 0.5 * frdk_dot_dot_end * dt_interval_square;

      q_rdk_root3 = alpha5 * root3_to_the_fifth + alpha4 * root3_to_the_fourth + alpha3 * root3_to_the_third + alpha2 * root3_to_the_square + alpha1 * root3 + alpha0;

      // // Compute rmin
      rmin_root3 = sqrt( q_rdi_root3 * q_rdi_root3 + q_rdj_root3 * q_rdj_root3 + q_rdk_root3 * q_rdk_root3 );
      if ( ( rmin_root3 <= min_dist_close_approach ) && ( rmin_root3 >= 0 ) )
      {
        // Find the associated time of closest approach (noted t_root3)
        *dca3 = rmin_root3;
        *tca3 = et_start + root3 * dt_interval;

        found_closest_approach_root3 = 1;
      }
    }
  }

  if ( ( found_closest_approach_root1 == 1 ) || ( found_closest_approach_root2 == 1 ) || ( found_closest_approach_root3 == 1 ) )
  {
    // accuracy if 0.00001 second
    if ( fabs( root1 - root2 ) < 0.00001 )
    {
      *tca2 = tca2_before;
      *dca2 = dca2_before;
    }

    // accuracy if 0.00001 second
    if ( fabs( root3 - root1 ) < 0.00001 )
    {
      *tca3 = tca3_before;
      *dca3 = dca3_before;
    }

    // accuracy if 0.00001 second
    if ( ( *tca2 != tca2_before  ) && (fabs( root2 - root3 ) < 0.00001 ) )
    {
      *tca3 = tca3_before;
      *dca3 = dca3_before      ;
    }
  }
  return 0;
}

///
/// @brief Determines the existence of a minimum distance between two spacecrafts
///
/// @see Alfvano 1994
///
int ancas_existence_of_min_using_two_values_of_function_and_two_values_of_its_derivative(
  double et_start,
  double dt_interval,
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
  int *min_exists,
  double *gamma0,
  double *gamma1,
  double *gamma2,
  double *gamma3){

  double fd_start, fd_dot_start, fd_dot_dot_start;
  double fd_dot_start_temp, fd_dot_dot_start_temp, fd_dot_dot_start_temp2;
  double fd_dot_end_temp, fd_dot_dot_end_temp, fd_dot_dot_end_temp2;
  double fd_end, fd_dot_end, fd_dot_dot_end;
  double rd_start[3], rd_end[3];
  double rd_dot_start[3], rd_dot_end[3];
  double rd_dot_dot_start[3], rd_dot_dot_end[3];

  char et_start_str[256];
  et2utc_c(et_start, "ISOC", 3, 255, et_start_str);

  char et_end_str[256];
  et2utc_c(et_start + dt_interval, "ISOC", 3, 255, et_end_str);

  // Set up variables

  // rd
  v_sub( rd_start, r2_start, r1_start);
  v_sub( rd_end, r2_end, r1_end);

  // rd_dot
  v_sub( rd_dot_start, v2_start, v1_start);
  v_sub( rd_dot_end, v2_end, v1_end);

  // rd_dot_dot
  v_sub( rd_dot_dot_start, a2_start, a1_start);
  v_sub( rd_dot_dot_end, a2_end, a1_end);

  // f
  v_dot(&fd_start, rd_start, rd_start);
  v_dot(&fd_end, rd_end, rd_end);

  // fd_dot
  v_dot(&fd_dot_start_temp, rd_dot_start, rd_start);
  fd_dot_start = fd_dot_start_temp * 2;

  v_dot(&fd_dot_end_temp, rd_dot_end, rd_end);
  fd_dot_end = fd_dot_end_temp * 2;

  // fd_dot_dot
  v_dot( &fd_dot_dot_start_temp, rd_dot_dot_start, rd_start );
  v_dot( &fd_dot_dot_start_temp2, rd_dot_start, rd_dot_start );

  fd_dot_dot_start = 2 * ( fd_dot_dot_start_temp + fd_dot_dot_start_temp2 );

  v_dot( &fd_dot_dot_end_temp, rd_dot_dot_end, rd_end );
  v_dot( &fd_dot_dot_end_temp2, rd_dot_end, rd_dot_end );

  fd_dot_dot_end = 2 * ( fd_dot_dot_end_temp + fd_dot_dot_end_temp2 );

  // Calculate coefficients of order 3 polynomial
  *gamma0 = fd_dot_start;
  *gamma1 = fd_dot_dot_start * dt_interval;
  *gamma2 = -3 * fd_dot_start - 2 * fd_dot_dot_start * dt_interval + 3 * fd_dot_end - fd_dot_dot_end * dt_interval;
  *gamma3 = 2 * fd_dot_start + fd_dot_dot_start * dt_interval - 2 * fd_dot_end + fd_dot_dot_end * dt_interval;

  // Check if there is a root, because if there's not then it means there is min distance between the 2 sc so no need to compute gsl_poly_solve_cubic in ancas function
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

  if ( real_root_exists == 1 )
  {
    *min_exists = 1;
  }
  else
  {
    *min_exists = 0;
  }

  return 0;
}
