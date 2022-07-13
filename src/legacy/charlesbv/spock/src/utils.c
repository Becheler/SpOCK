#ifndef __SPOCK_LEGACY_UTILS_H_INCLUDED__
#define __SPOCK_OPTIONS_H_INCLUDED__

///
/// @brief Position, velocity and acceleration of ensembles
///
/// @details This functions shares the position, velocity, and acceleration of all
///          ensembles eee (associated with reference satellite ii) between
///          all nodes for the entire time spanning TCA
int receive_r_v_a_in_tca_span(
  double ****save_r_i2cg_INRTL,
  double ****save_v_i2cg_INRTL,
  double ****save_a_i2cg_INRTL,
  int iProc,
  int nProcs,
  int ii,
  int eee,
  int nb_ensemble_min_per_proc,
  CONSTELLATION_T *CONSTELLATION,
  int nProcs_that_are_gonna_run_ensembles )
{

  int ccc;
  int eee_all_other_iproc;

  MPI_Barrier(MPI_COMM_WORLD);
  for (ccc = 0; ccc < nProcs; ccc++)
  {
    // receive the positions by iProc sent by all other iProc
    if ( ccc != iProc )
    {
      eee_all_other_iproc = ccc * nb_ensemble_min_per_proc +  eee - iProc * nb_ensemble_min_per_proc;
      if (nProcs > 1)
      {
        if (iProc < nProcs_that_are_gonna_run_ensembles)
        {
          MPI_Recv(&save_r_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][0], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_r_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][1], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_r_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][2], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

          MPI_Recv(&save_v_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][0], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_v_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][1], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_v_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][2], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

          MPI_Recv(&save_a_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][0], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_a_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][1], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_a_i2cg_INRTL[ii][eee_all_other_iproc][CONSTELLATION->spacecraft[ii][eee_all_other_iproc].ispan][2], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
      }
    }
  }
  return 0;
}

///
/// @brief Distance between to spacecrafts
///
double distance_between_two_sc( double eci_r_sc1[3], double eci_r_sc2[3] )
{
  double diffce[3];
  v_sub(diffce, eci_r_sc1, eci_r_sc2);
  double distance;
  v_mag( &distance, diffce );
  return distance;
}

///
/// @brief Allocate memory
///
int allocate_memory_r_a_v_in_span(
  double ****save_x_i2cg_INRTL,
  double ****save_y_i2cg_INRTL,
  double ****save_z_i2cg_INRTL,
  double ****save_vx_i2cg_INRTL,
  double ****save_vy_i2cg_INRTL,
  double ****save_vz_i2cg_INRTL,
  double ****save_ax_i2cg_INRTL,
  double ****save_ay_i2cg_INRTL,
  double ****save_az_i2cg_INRTL,
  int nb_tca,
  int nb_satellites_not_including_gps,
  int total_ensemble_final_with_ref,
  int nb_time_steps_in_tca_time_span,
  int iProc,
  int iDebugLevel)
{
  //  int iiitca;
  int ii;
  int eee;

  if ( iDebugLevel >= 3 )
  {
    printf("---- (generate_ephemerides)(allocate_memory_r_a_v_in_span) iProc %d just got in allocate_memory_r_a_v_in_span.\n", iProc);
  }

  // allocate memory for (*save_x_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_x_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_x_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_x_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_x_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_x_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_x_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_x_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;

      if ( (*save_x_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_x_i2cg_INRTL)");
      }
    }
  }

  // allocate memory for (*save_y_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_y_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_y_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_y_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_y_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_y_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_y_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_y_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;
      if ( (*save_y_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_y_i2cg_INRTL)");
      }
    }
  }

  // allocate memory for (*save_z_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_z_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_z_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_z_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_z_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_z_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_z_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_z_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;

      if ( (*save_z_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_z_i2cg_INRTL)");
      }

    }
  }

  // allocate memory for (*save_vx_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_vx_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_vx_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_vx_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_vx_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_vx_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_vx_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_vx_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;

      if ( (*save_vx_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_vx_i2cg_INRTL)");
      }
    }
  }

  // allocate memory for (*save_vy_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_vy_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_vy_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_vy_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_vy_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_vy_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_vy_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_vy_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;

      if ( (*save_vy_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_vy_i2cg_INRTL)");
      }
    }
  }

  // allocate memory for (*save_vz_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_vz_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_vz_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_vz_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_vz_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_vz_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_vz_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_vz_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;

      if ( (*save_vz_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_vz_i2cg_INRTL)");
      }
    }
  }

  // allocate memory for (*save_ax_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  (*save_ax_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_ax_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_ax_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_ax_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * )));

    if ( (*save_ax_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_ax_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_ax_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;

      if ( (*save_ax_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_ax_i2cg_INRTL)");
      }

    } // all ensembles
  }// all ref sc


  // allocate memory for (*save_ay_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  // // (*save_ay_i2cg_INRTL)[OPTIONS->nb_satellites_not_including_gps][total_ensemble_final_with_ref][nb_time_steps_in_tca_time_span]
  (*save_ay_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_ay_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_ay_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_ay_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_ay_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_ay_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_ay_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double )) ) ;

      if ( (*save_ay_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_ay_i2cg_INRTL)");
      }
    }
  }

  // allocate memory for (*save_az_i2cg_INRTL): x eci for all sc at each time step of the time spanning TCA
  // // (*save_az_i2cg_INRTL)[OPTIONS->nb_satellites_not_including_gps][total_ensemble_final_with_ref][nb_time_steps_in_tca_time_span]
  (*save_az_i2cg_INRTL) = static_cast<double ***>(malloc( nb_tca * sizeof( double ** ) ));

  if ( (*save_az_i2cg_INRTL) == NULL )
  {
    print_error_any_iproc(iProc, "Not enough memory for (*save_az_i2cg_INRTL)");
  }

  for (ii = 0; ii < nb_satellites_not_including_gps; ii++)
  {
    // all ref sc
    (*save_az_i2cg_INRTL)[ii] = static_cast<double **>(malloc( total_ensemble_final_with_ref * sizeof( double * ) ));

    if ( (*save_az_i2cg_INRTL)[ii] == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for (*save_az_i2cg_INRTL)");
    }

    for (eee = 0; eee < total_ensemble_final_with_ref; eee++)
    {
      // all ensembles
      (*save_az_i2cg_INRTL)[ii][eee] = static_cast<double *>(malloc( nb_time_steps_in_tca_time_span * sizeof( double ) )) ;
      if ( (*save_az_i2cg_INRTL)[ii][eee] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for (*save_az_i2cg_INRTL)");
      }
    }

  }

  if ( iDebugLevel >= 3 ){
    printf("---- (generate_ephemerides)(allocate_memory_r_a_v_in_span) iProc %d just got out from allocate_memory_r_a_v_in_span.\n", iProc);
  }

  return 0;
}

// for a given TCA, compute_collision_between_one_secondary_and_all_primary computes the collision between a secondary sc and all primary sc during the time spanning the TCA
int compute_collision_between_one_secondary_and_all_primary(
  double *save_x_i2cg_INRTL_sec,
  double *save_y_i2cg_INRTL_sec,
  double *save_z_i2cg_INRTL_sec,
  double *save_vx_i2cg_INRTL_sec,
  double *save_vy_i2cg_INRTL_sec,
  double *save_vz_i2cg_INRTL_sec,
  double *save_ax_i2cg_INRTL_sec,
  double *save_ay_i2cg_INRTL_sec,
  double *save_az_i2cg_INRTL_sec,
  double *save_x_i2cg_INRTL_prim,
  double *save_y_i2cg_INRTL_prim,
  double *save_z_i2cg_INRTL_prim,
  double *save_vx_i2cg_INRTL_prim,
  double *save_vy_i2cg_INRTL_prim,
  double *save_vz_i2cg_INRTL_prim,
  double *save_ax_i2cg_INRTL_prim,
  double *save_ay_i2cg_INRTL_prim,
  double *save_az_i2cg_INRTL_prim,
  OPTIONS_T *OPTIONS,
  int iProc,
  int *nb_coll_per_step_per_iproc_in_tca,
  double *et_time_step_of_save_tca,
  int nb_time_steps_in_tca_time_span,
  int iiitca,
  int eee_prim,
  int eee_sec,
  FILE *tca_file,
  FILE *dca_file,
  FILE *sample_file,
  int write_collision_files,
  int *eee_prim_that_collide)
{
  eee_prim = eee_prim;  eee_sec = eee_sec;
  //  char time_tca_ensemble[256];
  double direction_distance = 0;
  double min_distance_in_time_spanning_tca = 1e6;
  double tca_ensemble_1, dca_ensemble_1, tca_ensemble_2, dca_ensemble_2, tca_ensemble_3, dca_ensemble_3;
  int time_step_start_interval;
  double r_primary_start[3], v_primary_start[3], a_primary_start[3], r_primary_end[3], v_primary_end[3],  a_primary_end[3],r_secondary_start[3], v_secondary_start[3], a_secondary_start[3], r_secondary_end[3], v_secondary_end[3], a_secondary_end[3];
  int close_approach_exists;
  double gamma0,  gamma1, gamma2, gamma3;
  double et_start_of_span;

  et_start_of_span = et_time_step_of_save_tca[iiitca] - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
  double et_initial_epoch, et_final_epoch;

  str2et_c(OPTIONS->initial_epoch, &et_initial_epoch);
  str2et_c(OPTIONS->final_epoch, &et_final_epoch);

  // technically, the cast here is not necessary. Indeed, by definition et_start_of_span is at a step
  int initial_epoch_time_step_in_span = (int)(nearbyint((et_initial_epoch - et_start_of_span) / OPTIONS->dt));
  // technically, the cast here is not necessary. Indeed, by definition et_start_of_span is at a step and
  // so is et_final_epoch (even if the user chose a dt such that the final epoch does not fall in a multiple
  // of dt + inital epoch (-> see in load_option))
  int final_epoch_time_step_in_span = (int)(nearbyint((et_final_epoch - et_start_of_span) / OPTIONS->dt));
  int min_time_step_final_epoch_and_time_step_end_span = nb_time_steps_in_tca_time_span-1;

  if ( final_epoch_time_step_in_span < min_time_step_final_epoch_and_time_step_end_span )
  {
    min_time_step_final_epoch_and_time_step_end_span = final_epoch_time_step_in_span;
  }

  int max_time_step_initial_epoch_and_time_step_start_span = 0;
  if ( initial_epoch_time_step_in_span > max_time_step_initial_epoch_and_time_step_start_span )
  {
    max_time_step_initial_epoch_and_time_step_start_span = initial_epoch_time_step_in_span;
  }

  int time_step_of_tca =  (int)(nb_time_steps_in_tca_time_span / 2);
  int step_coll;

  tca_ensemble_1 = -1; dca_ensemble_1 = 1e6; tca_ensemble_2 = -1; dca_ensemble_2 = 1e6; tca_ensemble_3 = -1;  dca_ensemble_3 = 1e6;
  time_step_start_interval = -1;

  close_approach_ensemble(
    save_x_i2cg_INRTL_prim,
    save_y_i2cg_INRTL_prim,
    save_z_i2cg_INRTL_prim,
    save_vx_i2cg_INRTL_prim,
    save_vy_i2cg_INRTL_prim,
    save_vz_i2cg_INRTL_prim,
    save_ax_i2cg_INRTL_prim,
    save_ay_i2cg_INRTL_prim,
    save_az_i2cg_INRTL_prim,
    save_x_i2cg_INRTL_sec,
    save_y_i2cg_INRTL_sec,
    save_z_i2cg_INRTL_sec,
    save_vx_i2cg_INRTL_sec,
    save_vy_i2cg_INRTL_sec,
    save_vz_i2cg_INRTL_sec,
    save_ax_i2cg_INRTL_sec,
    save_ay_i2cg_INRTL_sec,
    save_az_i2cg_INRTL_sec,
    time_step_of_tca,
    &gamma0,
    &gamma1,
    &gamma2,
    &gamma3,
    &close_approach_exists,
    r_primary_start,
    v_primary_start,
    a_primary_start,
    r_primary_end,
    v_primary_end,
    a_primary_end,
    r_secondary_start,
    v_secondary_start,
    a_secondary_start,
    r_secondary_end,
    v_secondary_end,
    a_secondary_end,
    &time_step_start_interval,
    &min_distance_in_time_spanning_tca,
    &direction_distance,
    initial_epoch_time_step_in_span,
    final_epoch_time_step_in_span,
    iProc,
    et_time_step_of_save_tca[iiitca],
    OPTIONS
  );

  // here start means the oldest time of 4 four latest points, and end means
  // the most recent time of 4 four latest points (see Alfano 2009)
  // time_step_start_interval represents the time step in the time spanning TCA (unperturbed)
  // for which the interval of four points starts (interval for which a min distance
  // between the primary ensemble sc (represented by eee_prim) and the secondary
  // ensemble sc (represented by eee) is found (in other words, this interal is
  // the one represnted by P1, P2, P3, and P4 in Alfano 2009)).
  //
  // For instance, if time_step_start_interval = 3, then it is at the third time
  // step of the interval of time spanning TCA (unperturbed) (recall: the
  // interval starts at TCA - span/2) that the interval of four points
  // (P1, P2, P3, and P4) starts. So the close approach between the primary
  // and the secondary sc is expected to be between the third and the seventh
  // time step of the time spanning TCA (unpertubed).
  //
  //In this discussion, there is 2 intervals. The first one is the intervl spanning
  // TCA (unpertubed): varies from TCA - span/2 to TCA + span/2. The second
  // interval we are talking about is the interval in which the min distance
  // between the primary ensemble sc and the secondary ensemble sc is expected
  // to be (it is represented by the four points P1, P2, P3, and P4 in Alfano 2009

  if ( close_approach_exists == 1 )
  {
    // if a close approach in the time spanning TCA (of the unpertubed orbits)
    // has been found betwee eee_prim and eee (which represents a secondary sc)
    // then compute ANCAS to find TCA and DCA of this close approach
    tca_ensemble_1 = -1; dca_ensemble_1 = 1e6; tca_ensemble_2 = -1; dca_ensemble_2 = 1e6; tca_ensemble_3 = -1; dca_ensemble_3 = 1e6;

    ancas(
      OPTIONS->min_dist_close_approach,
      et_start_of_span + time_step_start_interval * OPTIONS->dt,
      3 * OPTIONS->dt,
      r_primary_start,
      v_primary_start,
      a_primary_start,
      r_primary_end,
      v_primary_end,
      a_primary_end,
      r_secondary_start,
      v_secondary_start,
      a_secondary_start,
      r_secondary_end,
      v_secondary_end,
      a_secondary_end,
      gamma0,
      gamma1,
      gamma2,
      gamma3,
      &tca_ensemble_1,
      &dca_ensemble_1,
      &tca_ensemble_2,
      &dca_ensemble_2,
      &tca_ensemble_3,
      &dca_ensemble_3
    );

    // 3 * OPTIONS->dt necause the interval over which the min distance is
    // calculated of time covers 4 points (it is represented by the four points
    // P1, P2, P3, and P4 in Alfano 2009)

    if (  dca_ensemble_1 <= OPTIONS->min_dist_collision )
    {
      step_coll = (int)( (tca_ensemble_1 - et_start_of_span) / OPTIONS->dt );
      // time step in time spanning TCA where the collision between secondary
      // sc ewee and primary sc eee_prim occurs
      nb_coll_per_step_per_iproc_in_tca[step_coll] = nb_coll_per_step_per_iproc_in_tca[step_coll] + 1;

      if (write_collision_files == 1)
      {
        fprintf(tca_file, "%f ", tca_ensemble_1 - et_initial_epoch );
        fprintf(dca_file, "%f ", dca_ensemble_1 );
      }
    }

    if (  dca_ensemble_2 <= OPTIONS->min_dist_collision )
    {
      step_coll = (int)( (tca_ensemble_2 - et_start_of_span) / OPTIONS->dt );
      // time step in time spanning TCA where the collision between secondary
      // sc ewee and primary sc eee_prim occurs

      nb_coll_per_step_per_iproc_in_tca[step_coll] = nb_coll_per_step_per_iproc_in_tca[step_coll] + 1;
      if (write_collision_files == 1)
      {
        fprintf(tca_file, "%f ", tca_ensemble_2 - et_initial_epoch );
        fprintf(dca_file, "%f ", dca_ensemble_2  );
      }
    }

    if (  dca_ensemble_3 <= OPTIONS->min_dist_collision )
    {
      step_coll = (int)( (tca_ensemble_3 - et_start_of_span) / OPTIONS->dt );
      // time step in time spanning TCA where the collision between secondary
      // sc ewee and primary sc eee_prim occurs

      nb_coll_per_step_per_iproc_in_tca[step_coll] = nb_coll_per_step_per_iproc_in_tca[step_coll] + 1;
      if (write_collision_files == 1)
      {
        fprintf(tca_file, "%f ", tca_ensemble_3 - et_initial_epoch );
        fprintf(dca_file, "%f ", dca_ensemble_3);
      }
    }
  } // end of if a close approach in the time spanning TCA (of the unpertubed orbits) has been found betwee eee_prim and eee (which represents a secondary sc) then compute ANCAS to find TCA and DCA of this close approach
  return 0;
}

///
/// @brief Heading of a spacecraft based on its ECEF velocity.
///
/// @note 0 is North, 90 is East, 180 is South, 270 is West
///
int compute_heading(double *heading, double v_ecef[3], double lon, double lat, double earth_flattening)
{
  // lon, lat, heading in radians
  double local_north_in_enu[3];
  double local_east_in_enu[3];
  double T_enu_to_ecef[3][3];
  double T_ecef_to_enu[3][3];
  double v_enu[3];

  local_north_in_enu[0] = 0; local_north_in_enu[1] = 1; local_north_in_enu[2] = 0;
  local_east_in_enu[0] = 1; local_east_in_enu[1] = 0; local_east_in_enu[2] = 0;

  // convert velocity from ECEF to ENU coordinates of the sub satellite location
  compute_T_enu_to_ecef( T_enu_to_ecef, lat, lon, earth_flattening);
  m_trans(T_ecef_to_enu, T_enu_to_ecef);
  m_x_v(v_enu, T_ecef_to_enu, v_ecef);

  // Dot product between the local north and the vector ground station to sc in ENU reference system
  double unit_v_enu[3];
  v_norm(unit_v_enu, v_enu);

  double unit_v_enu_dot_local_north_in_enu;
  v_dot(&unit_v_enu_dot_local_north_in_enu, unit_v_enu, local_north_in_enu);

  double unit_v_enu_dot_local_east_in_enu;
  v_dot(&unit_v_enu_dot_local_east_in_enu, unit_v_enu, local_east_in_enu);

  if ( unit_v_enu_dot_local_east_in_enu >= 0 )
  {
    *heading = acos( unit_v_enu_dot_local_north_in_enu );
  }
  else
  {
    *heading = 2*M_PI - acos( unit_v_enu_dot_local_north_in_enu );
  }

  return 0;
}

#endif
