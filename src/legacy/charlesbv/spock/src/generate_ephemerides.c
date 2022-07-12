#include "options.h"
#include "moat_prototype.h"
#include "gsl/gsl_poly.h"

#include "write_output.c"

int generate_ephemerides
(
  CONSTELLATION_T  *CONSTELLATION,
  OPTIONS_T        *OPTIONS,
  PARAMS_T         *PARAMS,
  GROUND_STATION_T *GROUND_STATION,
  int iProc,
  int nProcs,
  int iDebugLevel
){

  if ( ( iDebugLevel >= 1 ) )
  {
    printf("-- (generate_ephemerides) Just got in generate_ephemerides. (iProcc %d)\n", iProc);
  }

  char temp_iproc_file[256];
  char temp_nb_proc[256];

  int eee_prim_that_collide;
  // locality: line 1400, 1608
  double et_current_tca;
  int start_itca=0;
  char time_itca2[256];
  double **save_first_distance_unpertubed_orbit = NULL;
  double **save_last_distance_unpertubed_orbit = NULL;
  int nb_tca_without_collisions;
  int nb_tca_with_collisions;

  int compute_collisions_was_on_but_no_tca_found = 0;
  double *total_collision_each_dt;
  int already_propagated_ref_sc = 0;
  FILE *file_collision = NULL;
  double et_step_collision;

  //  int eee_sec;
  int eee_prim;

  int ispan_constellation;
  int eee_all_other_iproc;
  int total_ensemble_final = OPTIONS->nb_ensemble_min_per_proc*nProcs;
  int total_ensemble_final_with_ref = total_ensemble_final + 1;
  double et_start_of_span, et_end_of_span;
  int **total_nb_collisions = NULL;
  char time_itca[256];
  int ppp;
  int ***nb_coll_per_step_in_TCA = NULL;
  int iiitca;
  int ****nb_coll_per_step_per_iproc_in_tca = NULL;

  //  int istepspan;
  float min_altitude_constellation = 1.0e32;
  double **previous_eci_r = NULL;
  double **previous_eci_v = NULL;
  double **previous_eci_a = NULL;
  int isc_ref;
  int close_approach_exists;
  double gamma0,  gamma1, gamma2, gamma3;
  int time_step_of_tca;
  int *done_with_tca = NULL;
  int itca;
  int iground;

  FILE *tca_file;
  FILE *dca_file;
  FILE *sample_file;

  double density;
  char times_att[300];
  double previous_lat = 0;
  double tca1 = -1, dca1 = 1e6, tca2 = -1, dca2 = 1e6, tca3 = -1, dca3 = 1e6;

  int nb_tca = 0;
  /// @warning for now works only if two reference satellies only (add a dimension if more than 2 sc)
  double *save_tca = NULL;
  /// @warning for now works only if two reference satellies only (add a dimension if more than 2 sc)
  double *et_time_step_of_save_tca = NULL;
  /// @warning for now works only if two reference satellies only (add a dimension if more than 2 sc)
  double *save_dca;
  double  max_tca = -1;
  double dca_at_max_tca = 1.e6;
  double ***save_x_i2cg_INRTL = NULL, ***save_y_i2cg_INRTL = NULL, ***save_z_i2cg_INRTL = NULL;
  double ***save_vx_i2cg_INRTL = NULL, ***save_vy_i2cg_INRTL = NULL, ***save_vz_i2cg_INRTL = NULL;
  double ***save_ax_i2cg_INRTL = NULL, ***save_ay_i2cg_INRTL = NULL, ***save_az_i2cg_INRTL = NULL;
  int nb_time_steps_in_tca_time_span = (int) (nearbyint( CONSTELLATION->collision_time_span / OPTIONS->dt )) + 1 ; // the cast here is not necessary as we made sure in initialize_constellation that CONSTELLATION->collision_time_span is an even multiple of OPTIONS->dt

  double min_end_time;

  // other variables
  int ii;
  int eee;
  int fff;
  double starttime;
  double endtime;
  double twrite = 0.0;
  double new_dt_for_gps;
  int save_include_drag;
  int save_include_solar_pressure;
  int save_include_earth_pressure;
  double save_solar_cell_efficiency;
  int choose_tle_to_initialise_orbit = 0;
  int ccc;
  int compute_collisions = 0;

  // Collisions
  if ( ( OPTIONS->nb_satellites_not_including_gps > 1 ) && ( OPTIONS->nb_ensembles > 0 ) && ( (strcmp(OPTIONS->type_orbit_initialisation, "collision" ) == 0 ) || (strcmp(OPTIONS->type_orbit_initialisation, "collision_vcm" ) == 0 )))
  {
    compute_collisions = 1;
  }


  // SETTING THINGS UP FOR PARALLEL PROGRAMMING


  // Which iProc runs which main sc / which sc is run by which iProc
  int nProcs_that_are_gonna_run_ensembles;
  nProcs_that_are_gonna_run_ensembles = nProcs;
  if ( nProcs > OPTIONS->nb_ensembles_min )
  {
    nProcs_that_are_gonna_run_ensembles = OPTIONS->nb_ensembles_min;
  }

  // For each iProc, set up the first main sc (iStart_save[iProcf])
  // and the last main sc (iEnd_save[iProcf]) that it's going to run.
  // iStart_save and iEnd_save are two arrays that have the same values for all
  // procs (so if you're iProc 0 or iProc 1, you have value recorded for iStart_save[0]
  // and iEnd_save[0] and the same value recorded for iStart_save[1] and iEnd_save[1]) ->
  // they are not "iProc-dependent"
  int *iProc_run_main_sc;
  iProc_run_main_sc = static_cast<int *>(malloc(nProcs * sizeof(int)));

  int nscEachPe, nscLeft;
  nscEachPe = (OPTIONS->n_satellites)/nProcs;
  nscLeft = (OPTIONS->n_satellites) - (nscEachPe * nProcs);

  int *iStart_save, *iEnd_save;
  iStart_save = static_cast<int *>(malloc( nProcs * sizeof(int)));
  iEnd_save = static_cast<int *>(malloc( nProcs  * sizeof(int)));

  int iProcf;
  int i;
  for (iProcf = 0; iProcf < nProcs; iProcf++)
  {
    iStart_save[iProcf] = 0;
    iEnd_save[iProcf] = 0;
  }

  for (iProcf = 0; iProcf < nProcs; iProcf++)
  {
    for (i=0; i<iProcf; i++)
    {
      iStart_save[iProcf] += nscEachPe;
      if (i < nscLeft && iProcf > 0) iStart_save[iProcf]++;
    }

    iEnd_save[iProcf] = iStart_save[iProcf]+nscEachPe;
    if (iProcf  < nscLeft) iEnd_save[iProcf]++;
    iStart_save[iProcf] = iStart_save[iProcf];
    iEnd_save[iProcf] = iEnd_save[iProcf];
    if ( iStart_save[iProcf] >= iEnd_save[iProcf] )
    {
      // this happens if the iProc iProcf does not run any main sc
      iProc_run_main_sc[iProcf] = 0;
    }
    else{
      iProc_run_main_sc[iProcf] = 1;
    }
  }

  // For each main sc, start_ensemble is 0 if the iProc runs this main sc.
  // start_ensemble is 1 is the iProc does not run this main sc. (so each iProc has
  // a different array start_ensemble -> start_ensemble is "iProc-dependent")
  int *start_ensemble;
  start_ensemble = static_cast<int *>(malloc(OPTIONS->n_satellites * sizeof(int)));

  for (ii = 0; ii < OPTIONS->n_satellites; ii++)
  {
    if ( (ii >= iStart_save[iProc]) & ( ii < iEnd_save[iProc]) )
    {
      start_ensemble[ii] = 0;
    }
    else{
      start_ensemble[ii] = 1;
    }
  }

  // array_sc is the array of sc (main and ensemble sc) run by this iProc.
  // So array_sc is "iProc-dependent": each iProc has a different array array_sc.
  // What array_sc has: the first elt is 0, whatever iProc it is (because it represents main sc.
  // However, it does not mean that all iProc will run a main sc, this is decided later in the code
  // (using start_ensemble)). The next elts (1, 2, ..., OPTIONS->nb_ensemble_min_per_proc) represent
  // the ensemble sc run by this iProc. So for example if there are 20 ensembles and 2 iProc,
  // array_sc is:
  // for iProc 0: [0, 1, 2, 3, ..., 9, 10]
  // for iProc 1: [0, 11, 12, 13, ..., 19, 20]
  int ielt;
  int *array_sc;
  // + 2 (and not + 1) because if there is no ensemble, we still want array_sc[1]
  // to exist (because later in the code we call array_sc[start_ensemble[ii]],
  // and start_ensemble[ii] = 1 if the iProc does not run main sc ii).
  array_sc = static_cast<int *>(malloc((OPTIONS->nb_ensemble_min_per_proc + 2) * sizeof(int)));

  // if there is no ensemble, we still want array_sc[1] to exist
  // (because later in the code we call array_sc[start_ensemble[ii]],
  // and start_ensemble[ii] = 1 if the iProc does not run main sc ii).
  // We set to -1 because we want to make sure that if there is no ensemble,
  // eee will never be equal to array_sc[1]. If there are ensembles, array_sc[1]
  // is overwritten right below anyway
  array_sc[0] = 0;
  array_sc[1] = -1;
  for ( ielt = 1; ielt < OPTIONS->nb_ensemble_min_per_proc + 1; ielt ++ )
  {
    if (iProc < nProcs_that_are_gonna_run_ensembles)
    {
      array_sc[ielt] = iProc * OPTIONS->nb_ensemble_min_per_proc + ielt;
    }
  }

  // for each main, which_iproc_is_running_main_sc is the iProc that runs it. For example, which_iproc_is_running_main_sc[3] is equal to the iProc that runs the main sc 3. So which_iproc_is_running_main_sc is the same array for all iProc -> which_iproc_is_running_main_sc is not "iProc-dependent"
  int *which_iproc_is_running_main_sc;
  which_iproc_is_running_main_sc = static_cast<int *>(malloc( OPTIONS->n_satellites * sizeof(int)));
  for (ii = 0; ii < OPTIONS->n_satellites; ii++)
  {
    for (ccc = 0; ccc < nProcs; ccc++)
    {
      if ( ( ii >= iStart_save[ccc] ) && ( ii < iEnd_save[ccc]))
      {
        which_iproc_is_running_main_sc[ii] = ccc;
      }
    }
  }

  // end of SET THINGS UP FOR PARALLEL PROGRAMMING

  // Compute the starttime and endtime
  str2et_c(OPTIONS->initial_epoch, &starttime);
  str2et_c(OPTIONS->final_epoch, &endtime);
  min_end_time = endtime;
  twrite = CONSTELLATION->et;

  // Check the TLE epochs of the satellites are older than the simulation epoch
  // (if the user chose to initilaize the orbits with TLEs)
  // // If running TLE (GPS or other satellites), the epochs of the last TLEs
  // have be older than the epoch of the start time of the constellation (the current version does not allow for propagating satellite going back in time)
  if ( (strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0 ) || (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ) )
  {
    for (ii = 0; ii < OPTIONS->n_satellites - OPTIONS->nb_gps; ii++)
    {
      // if main sc ii is run by this iProc
      if ( start_ensemble[ii] == 0 )
      {
        if (CONSTELLATION->spacecraft[ii][0].et > CONSTELLATION->et)
        {
          printf("The epochs of the TLEs of the satellites have to be older than the epoch of the start time of the constellation. The program will stop. \n");
          printf("\nYou can choose 'now n' as the first line of the #TIME section in the input file input.d to run the constellation as from the current time. This will guarantee that the TLEs of the satellites are older than the epoch of the start time of the constellation ('n' is the number of hours to propagate the constellation for (n can be a decimal value)). \n");
          exit(0);
        }
      }
    }
    // this means that the user chose to initialize the orbits with TLEs
    choose_tle_to_initialise_orbit = 1;
  }

  // Check the TLE epochs of the GPS are older than the simulation epoch
  // (if the user chose to run GPS)
  if (OPTIONS->nb_gps > 0)
  {
    for (ii = OPTIONS->n_satellites - OPTIONS->nb_gps; ii < OPTIONS->n_satellites; ii++)
    {
      if ( start_ensemble[ii] == 0 )
      {
        // if main sc ii is run by this iProc
        if (CONSTELLATION->spacecraft[ii][0].et > CONSTELLATION->et)
        {
          printf("The epochs of the last TLEs of the GPS satellites have to be older than the epoch of the start time of the constellation. The program will stop. \n");
          printf("\nYou can choose 'now n' as the first line of the #TIME section in the input file input.d to run the constellation as from the current time. This will guarantee that the last TLEs of the GPS satellites are older than the epoch of the start time of the constellation ('n' is the number of hours to propagate the constellation for (n can be a decimal value)). \n");
          exit(0);
        }
      }
    }
  }

  if ( ( iDebugLevel >= 2 ) )
  {
    printf("-- (generate_ephemerides) Opening output files... (iProc %d)\n", iProc);
  }

  // Open SpOCK's output files (other than ensemble output files)
  for (ii = 0; ii < OPTIONS->n_satellites; ii++)
  {
    // go through all main sc
    strcpy(CONSTELLATION->spacecraft[ii][0].filenameecef, OPTIONS->dir_output_run_name_sat_name[ii]);
    strcat(CONSTELLATION->spacecraft[ii][0].filenameecef, "/");
    strcat(CONSTELLATION->spacecraft[ii][0].filenameecef, "ECEF_");
    strcat(CONSTELLATION->spacecraft[ii][0].filenameecef, OPTIONS->filename_output[ii]);

    if ( start_ensemble[ii] == 0 )
    {
      // if main sc ii is run by this iProc
      if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
      {
        // (longer format) (CBV)
        CONSTELLATION->spacecraft[ii][0].fp     = fopen( CONSTELLATION->spacecraft[ii][0].filename, "w+");
      }

      // (ECEF format) (CBV)
      CONSTELLATION->spacecraft[ii][0].fpecef = fopen( CONSTELLATION->spacecraft[ii][0].filenameecef, "w+");

      if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
      {
        // (lla format) (CBV)
        CONSTELLATION->spacecraft[ii][0].fpout  = fopen( CONSTELLATION->spacecraft[ii][0].filenameout, "w+");
      }

      if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
      {
        // (lla format) (CBV)
        CONSTELLATION->spacecraft[ii][0].fprho  = fopen( CONSTELLATION->spacecraft[ii][0].filenamerho, "w+");
      }

      if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
      {
        // (lla format) (CBV)
        CONSTELLATION->spacecraft[ii][0].fpatt  = fopen( CONSTELLATION->spacecraft[ii][0].filenameatt, "w+");
      }

      if (GROUND_STATION->nb_ground_stations > 0)
      {
        if ( ii <  OPTIONS->n_satellites - OPTIONS->nb_gps)
        {
          // do not compute the ground station coverage for gps
          for (iground = 0; iground < GROUND_STATION->nb_ground_stations; iground++)
          {
            CONSTELLATION->spacecraft[ii][0].fp_coverage_ground_station[iground] =fopen( CONSTELLATION->spacecraft[ii][0].filename_coverage_ground_station[iground], "w+");
          }
        }
      }

      if (( strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0) || (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ))
      {
        if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
        {
          // (longer format) (CBV)
          CONSTELLATION->spacecraft[ii][0].fptle = fopen( CONSTELLATION->spacecraft[ii][0].filenametle, "w+");
        }
      }

      if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
      {
        if (CONSTELLATION->spacecraft[ii][0].INTEGRATOR.solar_cell_efficiency != -1)
        {
          // power file
          CONSTELLATION->spacecraft[ii][0].fpower =fopen( CONSTELLATION->spacecraft[ii][0].filenamepower, "w+");
          // power file
          CONSTELLATION->spacecraft[ii][0].fpeclipse =fopen( CONSTELLATION->spacecraft[ii][0].filenameeclipse, "w+");
        }
      }
    } // end of if main sc ii is run by this iProc
  } // end of go through all main sc
  // end of Open SpOCK's output files (other than ensemble output files)

  if ( ( iDebugLevel >= 2 ) )
  {
    printf("-- (generate_ephemerides) Done opening output files... (iProc %d)\n", iProc);
  }

  if ( ( iDebugLevel >= 5 ) )
  {
    printf("-- (generate_ephemerides) Sending choose_tle_to_initialise_orbit to all proc... (iProc %d)\n", iProc);
  }

  // Open SpOCK's ensemble output files
  for (ii = 0; ii < OPTIONS->n_satellites; ii++)
  {
    // go over all sc
    if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
    {
      // if sc is not a GPS
      if ( OPTIONS->nb_ensembles_min > 0 )
      {
        // if running ensembles
        if ( array_sc[1] > 0 )
        {
          // if this iProc runs ensembles (otherwise array_sc[1] = - 1)
          // names of files per proc
          sprintf(temp_iproc_file, "%d", iProc+1);
          sprintf(temp_nb_proc, "%d", nProcs_that_are_gonna_run_ensembles);
          for (fff = 0; fff < OPTIONS->nb_ensembles_output_files ; fff ++)
          {

            if ( (strcmp(OPTIONS->filename_output_ensemble[fff], "tca" )!= 0 ) &&
            (strcmp(OPTIONS->filename_output_ensemble[fff], "dca" )!= 0 ) &&
            (strcmp(OPTIONS->filename_output_ensemble[fff], "sample" )!= 0 ))
            {
              strcpy(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], OPTIONS->dir_output_run_name_sat_name[ii]);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], "/");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], "/ensemble/iproc_");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],temp_iproc_file);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"-");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],temp_nb_proc);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"_");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],OPTIONS->filename_output_ensemble[fff]);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"_");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],OPTIONS->filename_output[ii]);
              CONSTELLATION->spacecraft[ii][0].fpiproc[fff] = fopen(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"w+");
            }

            else if (ii == 0)
            {

              if (strcmp(OPTIONS->filename_output_ensemble[fff], "tca" ) == 0)
              {
                strcpy(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], OPTIONS->dir_output_run_name_collision_tca);
              }

              if (strcmp(OPTIONS->filename_output_ensemble[fff], "dca" ) == 0)
              {
                strcpy(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], OPTIONS->dir_output_run_name_collision_dca);
              }

              if (strcmp(OPTIONS->filename_output_ensemble[fff], "sample" ) == 0)
              {
                strcpy(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], OPTIONS->dir_output_run_name_collision_sample);
              }

              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], "/");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff], "iproc_");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],temp_iproc_file);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"-");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],temp_nb_proc);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"_");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],OPTIONS->filename_output_ensemble[fff]);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"_");
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],OPTIONS->dir_output_run_name_temp);
              strcat(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],".txt");

              if (strcmp(OPTIONS->filename_output_ensemble[fff], "tca" ) == 0)
              {
                tca_file = fopen(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"w+");
              }

              if (strcmp(OPTIONS->filename_output_ensemble[fff], "dca" ) == 0)
              {
                dca_file = fopen(CONSTELLATION->spacecraft[ii][0].filenameiproc[fff],"w+");
              }
            }
          }
        } // end of if this iProc runs ensembles (otherwise array_sc[1] = - 1)
      } // end of if running ensembles
    } // end of sc is not a GPS
  } // end of go trhough all sc
  // end of Open SpOCK's ensemble output files

  // Write header in SpOCK's output files
  for (ii = 0; ii < OPTIONS->n_satellites; ii++)
  {
    write_output( CONSTELLATION->spacecraft[ii], 1, choose_tle_to_initialise_orbit, ii, OPTIONS->n_satellites,OPTIONS->nb_gps,  OPTIONS->nb_ensembles_min, OPTIONS->nb_ensemble_min_per_proc,  iProc, OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS, PARAMS->EARTH.earth_fixed_frame,1,1, ( CONSTELLATION->et ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
  }

  // If the orbit initialization of the satellites was done with TLEs, then propagate each satellite until their epoch reaches the same as the constellation's epoch start time
  if ( (strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0) ||
  (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ) ||
  (strcmp( OPTIONS->type_orbit_initialisation, "collision_vcm" ) == 0))
  {

    if (iProc == 0)
    {
      printf("Propagating the satellites until the epoch start time of the constellation...\n");
    }

    // go through all main sc except gps
    for (ii = 0; ii < OPTIONS->n_satellites - OPTIONS->nb_gps; ii++)
    {
      // write the initial position (at the TLE epoch start)

      if ((strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0) || (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ))
      {
        write_output( CONSTELLATION->spacecraft[ii] , 2, choose_tle_to_initialise_orbit, ii, OPTIONS->n_satellites,OPTIONS->nb_gps, OPTIONS->nb_ensembles_min,  OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble,previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,1,( CONSTELLATION->et ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
      }

      // if this iProc runs main sc ii
      if ( start_ensemble[ii] == 0)
      {
        save_include_drag = CONSTELLATION->spacecraft[ii][0].INTEGRATOR.include_drag;
        save_include_solar_pressure = CONSTELLATION->spacecraft[ii][0].INTEGRATOR.include_solar_pressure;
        save_include_earth_pressure = CONSTELLATION->spacecraft[ii][0].INTEGRATOR.include_earth_pressure;
        save_solar_cell_efficiency = CONSTELLATION->spacecraft[ii][0].INTEGRATOR.solar_cell_efficiency;
      }

      // if main sc is not a GPS
      if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
      {
        // if running ensembles
        if ( OPTIONS->nb_ensembles_min > 0 )
        {
          for (eee = 1L + iProc * OPTIONS->nb_ensemble_min_per_proc; eee< 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + OPTIONS->nb_ensemble_min_per_proc ; eee++){ // go through all ensembles
            save_include_drag = CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.include_drag;
            save_include_solar_pressure = CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.include_solar_pressure;
            save_include_earth_pressure = CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.include_earth_pressure;
            save_solar_cell_efficiency = CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.solar_cell_efficiency;

            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.dt = OPTIONS->dt;
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.dt_pos_neg = OPTIONS->dt_pos_neg;

          } // end of go through all ensembles
        } // end of if running ensembles
      } // end of if main sc is not a GPS

      // if tle_sgp4 then we directly jump from the tle epoch to the constellation
      // start time (ie we don't calculate the r/v at every time step between those two times)
      if (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) != 0 )
      {
        // if this iProc runs main sc ii
        if ( start_ensemble[ii] == 0)
        {
          while ( ( ( CONSTELLATION->spacecraft[ii][0].et - twrite ) < 0 ) && ( ( twrite - CONSTELLATION->spacecraft[ii][0].et ) > CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt_pos_neg ) )
          {
            // don't care about starttime here because this is used for linear interpolation
            // with the density drivers (F10.7, Ap, ...) and the attitude, which
            // we do not care for the satellites from their TLE epoch time to
            // the constellation epoch start time (for now!) (this is for interpolation
            // of the thermospheric drivers reasons)
            propagate_spacecraft( &CONSTELLATION->spacecraft[ii][0], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel,  start_ensemble, array_sc );

            if ((strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0) || (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ) )
            {
              write_output( CONSTELLATION->spacecraft[ii] , 2, choose_tle_to_initialise_orbit, ii, OPTIONS->n_satellites,OPTIONS->nb_gps, OPTIONS->nb_ensembles_min,  OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble,previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
            }
          } // end while propagation
        } // end of if this iProc runs main sc ii

        // if running ensembles
        if ( OPTIONS->nb_ensembles_min > 0 )
        {
          while ( ( ( CONSTELLATION->spacecraft[ii][1L + iProc * OPTIONS->nb_ensemble_min_per_proc].et - twrite ) < 0 ) && ( ( twrite - CONSTELLATION->spacecraft[ii][1L + iProc * OPTIONS->nb_ensemble_min_per_proc].et ) > CONSTELLATION->spacecraft[ii][1L + iProc * OPTIONS->nb_ensemble_min_per_proc].INTEGRATOR.dt_pos_neg ) )
          {
            // ENSEMBLES SPACECRAFT (BUT NOT FOR GPS SATELLITES)
            if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
            {
              if ( OPTIONS->nb_ensembles_min > 0 )
              {
                for (eee = 1L + iProc * OPTIONS->nb_ensemble_min_per_proc; eee< 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + OPTIONS->nb_ensemble_min_per_proc ; eee++)
                {
                  propagate_spacecraft( &CONSTELLATION->spacecraft[ii][eee], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel, start_ensemble, array_sc );
                } // end of loop over all ensembles for each iProc<
              } // enf if ensembles are run
            } // end if sc is not gps
          } // end while of propagatoin
        } // end of if running ensembles
      } // end of if not using tle_sgp4

      // propagate ONE LAST TIME UNTIL THE SATELLITE EPOCH IS THE SAME AS THE CONSTELLATION EPOCH
      if (fabs(starttime-CONSTELLATION->spacecraft[ii][0].et) > OPTIONS->dt/1.e6)
      {
        // if sc.et not equal to constellation initial epoch
        if (start_ensemble[ii] == 0)
        {
          // if this iProc runs main sc ii
          CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt = fabs(starttime - CONSTELLATION->spacecraft[ii][0].et);
          CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt_pos_neg = starttime - CONSTELLATION->spacecraft[ii][0].et;
          propagate_spacecraft( &CONSTELLATION->spacecraft[ii][0], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel, start_ensemble, array_sc );
        }
        // // ENSEMBLES SPACECRAFT (BUT NOT FOR GPS SATELLITES)
        if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
        {
          // if main sc is not a gps
          if ( OPTIONS->nb_ensembles_min > 0 )
          {
            // if running ensembles
            for (eee = 1L + iProc * OPTIONS->nb_ensemble_min_per_proc; eee< 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + OPTIONS->nb_ensemble_min_per_proc ; eee++)
            {
              // go over all ensemble sc
              CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.dt = fabs(starttime - CONSTELLATION->spacecraft[ii][eee].et);
              CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.dt_pos_neg = starttime - CONSTELLATION->spacecraft[ii][eee].et;
              propagate_spacecraft( &CONSTELLATION->spacecraft[ii][eee], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel,  start_ensemble, array_sc );
            } // end of go over all ensemble sc
          } // end of if running ensembles
        } // end of if main sc is not a gps

        if ((strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0) || (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ))
        {
          write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit, ii, OPTIONS->n_satellites,OPTIONS->nb_gps, OPTIONS->nb_ensembles_min,  OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble,previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
        }

        // END OF PROPAGATE ONE LAST TIME UNTIL THE SATELLITE EPOCH IS THE SAME AS THE CONSTELLATION EPOCH
      }

      // GO BACK TO DT AND FORCES CHOSEN IN THE INPUT FILE
      if (start_ensemble[ii] == 0)
      {
        // if this iProc runs main sc ii
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt = OPTIONS->dt;
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt_pos_neg = OPTIONS->dt_pos_neg;
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.include_drag  = save_include_drag; // the drag is now computed (if the user decided so) because the epoch reached the epoch start time of the constellation
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.include_solar_pressure  = save_include_solar_pressure; // the solar_pressure is now computed (if the user decided so) because the epoch reached the epoch start time of the constellation
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.include_earth_pressure  = save_include_earth_pressure; // the earth_pressure is now computed (if the user decided so) because the epoch reached the epoch start time of the constellation
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.solar_cell_efficiency  = save_solar_cell_efficiency; // the solar_power is now computed (if the user decided so) because the epoch reached the epoch start time of the constellation
      }

      // // ENSEMBLES SPACECRAFT (BUT NOT FOR GPS SATELLITES)
      if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
      {
        if ( OPTIONS->nb_ensembles_min > 0 )
        {
          for (eee = 1L + iProc * OPTIONS->nb_ensemble_min_per_proc; eee< 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + OPTIONS->nb_ensemble_min_per_proc ; eee++)
          {
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.dt = OPTIONS->dt;
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.dt_pos_neg = OPTIONS->dt_pos_neg;
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.include_drag  = save_include_drag;
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.include_solar_pressure  = save_include_solar_pressure;
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.include_earth_pressure  = save_include_earth_pressure;
            CONSTELLATION->spacecraft[ii][eee].INTEGRATOR.solar_cell_efficiency  = save_solar_cell_efficiency;
          }
        }
      }

      if (start_ensemble[ii] == 0)
      {
        // if this iProc runs main sc ii
        if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
        {
          if ((strcmp( OPTIONS->type_orbit_initialisation, "tle" ) == 0) || (strcmp(OPTIONS->type_orbit_initialisation, "tle_sgp4" ) == 0 ) )
          {
            fclose(CONSTELLATION->spacecraft[ii][0].fptle);
          }
        }
      }
    } // end of go through all main sc except gps
  } // end of if the orbit initialization of the satellites was done with TLEs, then propagate each satellite until their epoch reaches the same as the constellation's epoch start time

  // Propagate each GPS satellite until their epoch reaches the same as the constellation's epoch start time
  if (OPTIONS->nb_gps > 0)
  {

    if (iProc == 0)
    {
      printf("Propagating the GPS satellites until the epoch start time of the constellation...\n");
    }

    for (ii = OPTIONS->n_satellites - OPTIONS->nb_gps; ii < OPTIONS->n_satellites; ii++)
    {
      // go through all GPS sc
      if (start_ensemble[ii] == 0)
      {
        // if this iProc runs main sc ii

        double time_between_last_gps_epoch_and_constellation_epoch_starttime = -1.0;
        new_dt_for_gps = -1.0;

        while ( ( ( CONSTELLATION->spacecraft[ii][0].et - twrite ) < 0 ) && ( ( twrite - CONSTELLATION->spacecraft[ii][0].et ) > CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt_pos_neg ) )
        {
          propagate_spacecraft( &CONSTELLATION->spacecraft[ii][0], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc , iDebugLevel, start_ensemble, array_sc); // don't care about starttime here because this is used for linear interpolation with the density drivers (F10.7, Ap, ...) and the attitude, which we do not care for the GPS satellites (for now!)
        }

        time_between_last_gps_epoch_and_constellation_epoch_starttime =fabs( starttime - CONSTELLATION->spacecraft[ii][0].et);
        new_dt_for_gps = time_between_last_gps_epoch_and_constellation_epoch_starttime;
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt = new_dt_for_gps;
        propagate_spacecraft( &CONSTELLATION->spacecraft[ii][0], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel, start_ensemble, array_sc );

        write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit, ii, OPTIONS->n_satellites,OPTIONS->nb_gps,OPTIONS->nb_ensembles_min,  OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);

        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt = OPTIONS->dt;
        CONSTELLATION->spacecraft[ii][0].INTEGRATOR.dt_pos_neg = OPTIONS->dt_pos_neg;
      } // end of if this iProc runs main sc ii
    } // end of going through all GPS sc
  } // end of if (OPTIONS->nb_gps > 0)

  if (compute_collisions == 1)
  {
    // if computing the probability of collision
    if (nProcs > OPTIONS->nb_ensembles_min)
    {
      print_error_any_iproc(iProc, "If computing the probability of collision, the number of processors used in MPI has to be smaller than (or equal to) the number of ensembles (this bug will be fixed in the future)");
    }

    if ( ( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Allocating memory for some of the collision assessment variables. (iProc %d) \n", iProc);
    }

    save_tca = static_cast<double *>(malloc( OPTIONS->nb_time_steps * sizeof( double ) ));

    if ( save_tca == NULL )
    {
      print_error_any_iproc(iProc, "No memory space for save_tca");
    }

    et_time_step_of_save_tca = static_cast<double *>(malloc( OPTIONS->nb_time_steps * sizeof( double ) ));

    if ( et_time_step_of_save_tca == NULL )
    {
      print_error_any_iproc(iProc, "No memory space for et_time_step_of_save_tca");
    }

    save_dca = static_cast<double *>(malloc( OPTIONS->nb_time_steps * sizeof( double ) ));

    if ( save_dca == NULL )
    {
      print_error_any_iproc(iProc, "No memory space for save_dca");
    }

    if (( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Done allocating memory for some of the collision assessment variables. (iProc %d)\n", iProc);
    }

  } // end of if computing the probability of collision

  if (compute_collisions == 1)
  {
    // if computing the probability of collision 2020-11-13 i don't want to worry about dt_pos_neg (backward propagation) for collision avoidance
    if ( ( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Allocating memory for other collision assessment variables (only by main node). (iProc %d)\n", iProc);
    }

    // allocate memory for previous_eci_r[nsatref][3]:
    previous_eci_r = static_cast<double **>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof( double * ) ));

    if (  previous_eci_r == NULL )
    {
      print_error(iProc, "Not enough memory for previous_eci_r");
    }

    for ( isc_ref = 0; isc_ref < OPTIONS->nb_satellites_not_including_gps ; isc_ref++)
    {
      previous_eci_r[isc_ref] = static_cast<double *>(malloc( 3 * sizeof( double * ) ));
      if (  previous_eci_r[isc_ref] == NULL ){
        print_error(iProc, "Not enough memory for previous_eci_r");
      }
    }

    // allocate memory for previous_eci_v[nsatref][3]:
    previous_eci_v = static_cast<double **>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof( double * ) ));

    if (  previous_eci_v == NULL )
    {
      print_error(iProc, "Not enough memory for previous_eci_v");
    }

    for ( isc_ref = 0; isc_ref < OPTIONS->nb_satellites_not_including_gps ; isc_ref++)
    {
      previous_eci_v[isc_ref] = static_cast<double *>(malloc( 3 * sizeof( double * ) ));
      if (  previous_eci_v[isc_ref] == NULL )
      {
        print_error(iProc, "Not enough memory for previous_eci_v");
      }
    }

    // allocate memory for previous_eci_a[nsatref][3]:
    previous_eci_a = static_cast<double **>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof( double * ) ));

    if (  previous_eci_a == NULL )
    {
      print_error(iProc, "Not enough memory for previous_eci_a");
    }

    for ( isc_ref = 0; isc_ref < OPTIONS->nb_satellites_not_including_gps ; isc_ref++)
    {

      previous_eci_a[isc_ref] = static_cast<double *>(malloc( 3 * sizeof( double * ) ));

      if (  previous_eci_a[isc_ref] == NULL )
      {
        print_error(iProc, "Not enough memory for previous_eci_a");
      }
    }

    total_collision_each_dt = static_cast<double *>(malloc( OPTIONS->nb_time_steps * sizeof(double)));
    for (ccc = 0; ccc < OPTIONS->nb_time_steps ; ccc++)
    {
      total_collision_each_dt[ccc] = 0;
    }

    /// @warning If collision assessment is on, first propagate only the unperturbed orbits
    /// If collision assessment is on, then first propagate all reference sc (unpertubed orbits)
    /// to compute times of close approach (and allocate memory for variables)
    save_first_distance_unpertubed_orbit = static_cast<double **>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof( double *) ));

    if (save_first_distance_unpertubed_orbit == NULL)
    {
      print_error_any_iproc(iProc, "No memory for save_first_distance_unpertubed_orbit");
    }

    for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
    {
      save_first_distance_unpertubed_orbit[ii] =static_cast<double *>( malloc(3 * sizeof(double)));
      if (save_first_distance_unpertubed_orbit[ii] == NULL){
        print_error_any_iproc(iProc, "No memory for save_first_distance_unpertubed_orbit");
      }
    }

    save_last_distance_unpertubed_orbit = static_cast<double **>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof( double *) ));

    if (save_last_distance_unpertubed_orbit == NULL)
    {
      print_error_any_iproc(iProc, "No memory for save_last_distance_unpertubed_orbit");
    }

    for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
    {
      save_last_distance_unpertubed_orbit[ii] = static_cast<double *>(malloc(3 * sizeof(double)));
      if (save_last_distance_unpertubed_orbit[ii] == NULL){
        print_error_any_iproc(iProc, "No memory for save_last_distance_unpertubed_orbit");
      }
    }

    for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
    {
      if ( start_ensemble[ii] == 0)
      {
        // if this iProc runs main sc ii
        v_copy( save_first_distance_unpertubed_orbit[ii],  CONSTELLATION->spacecraft[ii][0].r_i2cg_INRTL);
      }
    }

    if (( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Done allocating memory for other collision assessment variables (only by main node). (iProc %d)\n", iProc);
    }

    if (( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Propagating all unpertubed orbits to determine times of close approach (only by main node). (iProc %d)\n", iProc);
    }

    OPTIONS->first_run_to_find_tca_before_collision_assessment = 1;

    while ( ( fabs(CONSTELLATION->et - endtime) < 0.01 ) && ( min_altitude_constellation > 100.0) )
    {
      // propagate all unrpertubed orbits to determine times of close approach
      already_propagated_ref_sc = 1;

      // PRINT PROGRESS TO SCREEN
      if (iProc == 0)
      {
        printf("\033[A\33[2K\rPropagating the unpertubed orbits to compute times of close approach... %.0f%%\n", ( CONSTELLATION->et - starttime ) / fabs( endtime - starttime ) *100.0);
      }

      for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
      {
        if ( start_ensemble[ii] == 0)
        {
          // if this iProc runs main sc ii
          previous_lat = CONSTELLATION->spacecraft[ii][0].GEODETIC.latitude*RAD2DEG;
          v_copy( previous_eci_r[ii], CONSTELLATION->spacecraft[ii][0].r_i2cg_INRTL);
          v_copy( previous_eci_v[ii], CONSTELLATION->spacecraft[ii][0].v_i2cg_INRTL);
          v_copy( previous_eci_a[ii], CONSTELLATION->spacecraft[ii][0].a_i2cg_INRTL);

          propagate_spacecraft( &CONSTELLATION->spacecraft[ii][0], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel, start_ensemble, array_sc );

          // iProc 0 needs to send previous_eci_r/v/a and r/v/a_i2cg_INRT to iProc 1 so that iProc 1 can find if there is a close approach. But this is only if iProc 1 runs main sc 1. If it's iProc 0 that runs main sc 1, then no need to send anything. // !!!!!!!!!! FOR NOW WORKS ONLY IF TWO REFERENCE SATELLIES ONLY
          if ( (iProc == 0) && ( which_iproc_is_running_main_sc[1] == 1 ) )
          {
            for (ccc = 0; ccc < 3; ccc++)
            {
              MPI_Send(&previous_eci_r[0][ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
              MPI_Send(&previous_eci_v[0][ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
              MPI_Send(&previous_eci_a[0][ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
              MPI_Send(&CONSTELLATION->spacecraft[0][0].r_i2cg_INRTL[ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
              MPI_Send(&CONSTELLATION->spacecraft[0][0].v_i2cg_INRTL[ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
              MPI_Send(&CONSTELLATION->spacecraft[0][0].a_i2cg_INRTL[ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            }
          }

          /// @warrnings For now works only if two reference satellies only
          if (ii == 1)
          {
            // iProc 1 needs to receive previous_eci_r/v/a and r/v/a_i2cg_INRT from iProc 0 so that iProc 1 can find if there is a close approach. But this is only if iProc 1 runs main sc 1. If it's iProc 0 that runs main sc 1, then no need to receive anything
            if ( (iProc == 1) && ( which_iproc_is_running_main_sc[1] == 1 ) )
            {
              for (ccc = 0; ccc < 3; ccc++)
              {
                MPI_Recv(&previous_eci_r[0][ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&previous_eci_v[0][ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&previous_eci_a[0][ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&CONSTELLATION->spacecraft[0][0].r_i2cg_INRTL[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&CONSTELLATION->spacecraft[0][0].v_i2cg_INRTL[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&CONSTELLATION->spacecraft[0][0].a_i2cg_INRTL[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              }
            }
            tca1 = -1; dca1 = 1e6; tca2 = -1; dca2 = 1e6; tca3 = -1; dca3 = 1e6;
            close_approach_exists = 0;

            if ( ( CONSTELLATION->et + OPTIONS->dt > starttime + 3 * OPTIONS->dt )  && (CONSTELLATION->et + OPTIONS->dt <  endtime - OPTIONS->dt)  )
            {
              // To use the collision assessment algorithm, the propagation needs to start at least three 3 time steps before TCA and needs to end at least one time step after TCA. Therefore, do not try to find TCA earlier than 3 times steps after initial epoch or later than 1 time step before final epoch. 2020-11-13 i don't want to worry about dt_pos_neg (backward propagation) for collision avoidance
              ancas_existence_of_min_using_two_values_of_function_and_two_values_of_its_derivative( CONSTELLATION->et, OPTIONS->dt, previous_eci_r[ii-1], previous_eci_v[ii-1], previous_eci_a[ii-1], CONSTELLATION->spacecraft[ii-1][0].r_i2cg_INRTL, CONSTELLATION->spacecraft[ii-1][0].v_i2cg_INRTL, CONSTELLATION->spacecraft[ii-1][0].a_i2cg_INRTL, previous_eci_r[ii], previous_eci_v[ii], previous_eci_a[ii], CONSTELLATION->spacecraft[ii][0].r_i2cg_INRTL, CONSTELLATION->spacecraft[ii][0].v_i2cg_INRTL, CONSTELLATION->spacecraft[ii][0].a_i2cg_INRTL, &close_approach_exists, &gamma0, &gamma1, &gamma2, &gamma3);
            }


            if ( close_approach_exists == 1 )
            {
              ancas(OPTIONS->min_dist_close_approach, CONSTELLATION->et, OPTIONS->dt, previous_eci_r[ii-1], previous_eci_v[ii-1], previous_eci_a[ii-1], CONSTELLATION->spacecraft[ii-1][0].r_i2cg_INRTL, CONSTELLATION->spacecraft[ii-1][0].v_i2cg_INRTL, CONSTELLATION->spacecraft[ii-1][0].a_i2cg_INRTL, previous_eci_r[ii], previous_eci_v[ii], previous_eci_a[ii], CONSTELLATION->spacecraft[ii][0].r_i2cg_INRTL, CONSTELLATION->spacecraft[ii][0].v_i2cg_INRTL, CONSTELLATION->spacecraft[ii][0].a_i2cg_INRTL, gamma0, gamma1, gamma2, gamma3, &tca1, &dca1, &tca2, &dca2, &tca3, &dca3 );
            }

            if ( tca1 > -1 )
            {
              save_tca[nb_tca] = tca1;
              et_time_step_of_save_tca[nb_tca] = CONSTELLATION->et;
              save_dca[nb_tca] = dca1;
              nb_tca = nb_tca + 1;
            }

            if ( tca2 > -1 )
            {
              save_tca[nb_tca] = tca2;
              et_time_step_of_save_tca[nb_tca] = CONSTELLATION->et;
              save_dca[nb_tca] = dca2;
              nb_tca = nb_tca + 1;
            }

            if ( tca3 > -1 )
            {
              save_tca[nb_tca] = tca3;
              et_time_step_of_save_tca[nb_tca] = CONSTELLATION->et;
              save_dca[nb_tca] = dca3;
              nb_tca = nb_tca + 1;
            }

          } // end of if ii == 1

          // write output
          if ( (CONSTELLATION->spacecraft[ii][0].et - twrite) >= OPTIONS->dt - 0.01)
          {
            if ( ( fmod( CONSTELLATION->spacecraft[ii][0].et - starttime, OPTIONS->dt_output ) < OPTIONS->dt / 2. ) || ( fabs( fmod( CONSTELLATION->spacecraft[ii][0].et - starttime, OPTIONS->dt_output ) - OPTIONS->dt_output ) < OPTIONS->dt / 2. ) || ( CONSTELLATION->spacecraft[ii][0].et > endtime - 0.01) )
            {
              write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit,ii, OPTIONS->n_satellites,OPTIONS->nb_gps,  OPTIONS->nb_ensembles_min, OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,0, ( CONSTELLATION->et + OPTIONS->dt_pos_neg) , nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
              et2utc_c(CONSTELLATION->spacecraft[ii][0].et, "ISOC" ,6 ,255 , times_att);
            }
          } // end of write output
        } // end of if this iProc runs main sc ii
      } //  end of loop over all main satellites

      // update the constellation time
      CONSTELLATION->et = CONSTELLATION->et + OPTIONS->dt;
      twrite = CONSTELLATION->et ;

    } // end of  propagate all unrpertubed orbits to determine times of close approach a

    if ( ( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Done propagating all unpertubed orbits to determine times of close approach (only by main node). (iProc %d)\n", iProc);
    }

    for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
    {
      if ( start_ensemble[ii] == 0)
      {
        // if this iProc runs main sc ii
        v_copy( save_last_distance_unpertubed_orbit[ii],  CONSTELLATION->spacecraft[ii][0].r_i2cg_INRTL);
      }
    }

    // if nb_tca, save_dca, et_time_step_of_save_tca, save_tca were calculated by
    // iProc 1 then send them to iProc 0 (and iProc 0 needs to receive them).
    // They were calculated by iProc if main sc 1 is run by iProc 1. Otherwise,
    // they were calculated by iProc 0. Also, need to send
    // save_last_distance_unpertubed_orbit[1] and save_first_distance_unpertubed_orbit[1]
    // to iProc 0 (if save_last_distance_unpertubed_orbit[1]
    // and save_first_distance_unpertubed_orbit[1] was calculated by iProc 1,
    // so if main sc 1 was run by iProc 1). Note: iProc > 1 will never get here
    // because main sc 1 (or 0) can never be run by iProc > 1. However, when looking
    // at the close approach between the ensembles, iProc > 1 will be doing stuff.
    if ( which_iproc_is_running_main_sc[1] == 1 )
    {

      if (iProc == 1)
      {

        MPI_Send(&nb_tca, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        for (ccc = 0; ccc < nb_tca; ccc++)
        {
          MPI_Send(&save_dca[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
          MPI_Send(&save_tca[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
          MPI_Send(&et_time_step_of_save_tca[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }

        for (ccc = 0; ccc < 3; ccc++)
        {
          MPI_Send(&save_last_distance_unpertubed_orbit[1][ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
          MPI_Send(&save_first_distance_unpertubed_orbit[1][ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }

      }

      if (iProc == 0)
      {

        MPI_Recv(&nb_tca, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (ccc = 0; ccc < nb_tca; ccc++)
        {
          MPI_Recv(&save_dca[ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_tca[ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&et_time_step_of_save_tca[ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (ccc = 0; ccc < 3; ccc++)
        {
          MPI_Recv(&save_last_distance_unpertubed_orbit[1][ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&save_first_distance_unpertubed_orbit[1][ccc], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

      }
    }

    // Reinitialize time to propagate ensembles satellites from initial epoch
    // to furthest TCA ( + CONSTELLATION->collision_time_span )
    CONSTELLATION->et = starttime;
    twrite = CONSTELLATION->et;

    if (iProc == 0)
    {
      // only need one proc to do the job here
      // We look at the distance at initial epoch and the distance at final epoch
      // (unless a time span of a tca includes them, in which case we don't look
      // at the distance at initial epoch and the distance at final epoch). If these
      // distances are smaller than OPTIONS->min_dist_close_approach, then we consider
      // them as a TCA. Indeed, it is not because the 2 unperturbed have not reached
      // a closest approach in the propagation that they should be ignored in the
      // probability of collision.
      if ( nb_tca <= 0)
      {
        // if no close approach between initial and final epoch
        if ( distance_between_two_sc( save_first_distance_unpertubed_orbit[0], save_first_distance_unpertubed_orbit[1] ) < OPTIONS->min_dist_close_approach)
        {
          // if the distance at initial epoch is smaller than OPTIONS->min_dist_close_approach
          // then consider this at a tca (that starts at inital epoch + period / 2)
          save_tca[nb_tca] = starttime + CONSTELLATION->collision_time_span / 2.;
          et_time_step_of_save_tca[nb_tca] = starttime + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
          save_dca[nb_tca] = distance_between_two_sc( save_first_distance_unpertubed_orbit[0], save_first_distance_unpertubed_orbit[1] );
          // !!!!!!!!! this is incorrect. What would be correct would be to take
          // the distance at inital epoch + period / 2 but we did not save this distance.
          // Since save_dca is never used after, we don't really care, except in the
          // collision output file where dca is reported
          nb_tca = nb_tca + 1;
        }

        if (distance_between_two_sc( save_last_distance_unpertubed_orbit[0], save_last_distance_unpertubed_orbit[1] ) < OPTIONS->min_dist_close_approach)
        {
          // if the distance at final epoch is smaller than OPTIONS->min_dist_close_approach
          // then consider this at a tca (that starts at final epoch - period / 2)
          save_tca[nb_tca] = endtime - CONSTELLATION->collision_time_span / 2.;
          et_time_step_of_save_tca[nb_tca] = endtime - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
          save_dca[nb_tca] = distance_between_two_sc( save_last_distance_unpertubed_orbit[0], save_last_distance_unpertubed_orbit[1] );
          // !!!!!!!!! this is incorrect. What would be correct would be to take
          // the distance at final epoch - period / 2 but we did not save this
          // distance. Since save_dca is never used after, we don't really care
          nb_tca = nb_tca + 1;
        }

      } else {
        // if there is at a least one close approach between initial and final epoch
        if ( et_time_step_of_save_tca[0] - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt ) > starttime )
        { // if the span of the first tca starts after the initial epoch
          if ( distance_between_two_sc( save_first_distance_unpertubed_orbit[0], save_first_distance_unpertubed_orbit[1] ) < OPTIONS->min_dist_close_approach)
          {
            // if the distance at initial epoch is smaller than OPTIONS->min_dist_close_approach
            // then consider this at a tca (that starts at inital epoch + period / 2)
            // need to move all previously calculated tca by one index

            for ( ccc = nb_tca; ccc > 0; ccc-- )
            {
              save_tca[ccc] = save_tca[ccc-1];
              et_time_step_of_save_tca[ccc] = et_time_step_of_save_tca[ccc-1];
              save_dca[ccc] = save_dca[ccc-1];
            }

            save_tca[0] = starttime + CONSTELLATION->collision_time_span / 2.;
            et_time_step_of_save_tca[0] = starttime + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
            save_dca[0] = distance_between_two_sc( save_first_distance_unpertubed_orbit[0], save_first_distance_unpertubed_orbit[1] ) < OPTIONS->min_dist_close_approach;
            // !!!!!!!!! this is incorrect. What would be correct would be to
            // take the distance at inital epoch + period / 2 but we did not save
            // this distance. Since save_dca is never used after, we don't really care,
            // except in the collision output file where dca is reported
            nb_tca = nb_tca + 1;
          }

        } // end of if the span of the first tca starts after the initial epoch

        if ( et_time_step_of_save_tca[nb_tca - 1] + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt ) < endtime )
        {
          // if the span of the last tca ends before the final epoch
          if (distance_between_two_sc( save_last_distance_unpertubed_orbit[0], save_last_distance_unpertubed_orbit[1] ) < OPTIONS->min_dist_close_approach)
          {
            // if the distance at final epoch is smaller than OPTIONS->min_dist_close_approach
            // then consider this at a tca (that starts at final epoch - period / 2)
            save_tca[nb_tca] = endtime - CONSTELLATION->collision_time_span / 2.;
            et_time_step_of_save_tca[nb_tca] = endtime - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
            save_dca[nb_tca] = distance_between_two_sc( save_last_distance_unpertubed_orbit[0], save_last_distance_unpertubed_orbit[1] ) < OPTIONS->min_dist_close_approach;
            // !!!!!!!!! this is incorrect. What would be correct would be to take
            // the distance at final epoch - period / 2 but we did not save this distance.
            // Since save_dca is never used after, we don't really care
            nb_tca = nb_tca + 1;
          }
        } // end of if the span of the last tca ends before the final epoch
      } // end of if there is at a least one close approach between initial and final epoch

      // Update compute_collisions: if there is no close approach between the unpertubed orbits, then don't compute collisions
      if ( nb_tca  <= 0 )
      {
        compute_collisions = 0;
        compute_collisions_was_on_but_no_tca_found = 1;
      }

      if (nProcs > 1)
      {
        for (ccc = 1; ccc < nProcs_that_are_gonna_run_ensembles; ccc++)
        {
          MPI_Send(&nb_tca, 1, MPI_INT, ccc, 0, MPI_COMM_WORLD);
          MPI_Send(&compute_collisions, 1, MPI_INT, ccc, 0, MPI_COMM_WORLD);
          MPI_Send(&already_propagated_ref_sc, 1, MPI_INT, ccc, 0, MPI_COMM_WORLD);
        }
      }
    } else {

      if (nProcs > 1)
      {
        if (iProc < nProcs_that_are_gonna_run_ensembles)
        {
          MPI_Recv(&nb_tca, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&compute_collisions, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&already_propagated_ref_sc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
      }
    }
  } // end of if compute_collision

  if (compute_collisions == 1)
  {

    done_with_tca =  static_cast<int *>(malloc( nb_tca * sizeof(int) ));

    if ( done_with_tca == NULL )
    {
      print_error(iProc, "Not enough memory for done_with_tca");
    }

    // allocate meory for r/v/a eci for all sc at each time step of the time spanning each TCA
    if ( ( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) Allocating big variables for collision assessment. (iProc %d)\n", iProc);
    }

    if (( iDebugLevel >= 3 ) )
    {
      printf("--- (generate_ephemerides) Allocating save_x_i2cg_INRTL, save_y_i2cg_INRTL, save_z_i2cg_INRTL, save_vx_i2cg_INRTL, save_vy_i2cg_INRTL, save_vz_i2cg_INRTL, save_ax_i2cg_INRTL, save_ay_i2cg_INRTL, save_az_i2cg_INRTL for collision assessment. (iProc %d)\n", iProc);
    }

    allocate_memory_r_a_v_in_span(&save_x_i2cg_INRTL, &save_y_i2cg_INRTL, &save_z_i2cg_INRTL, &save_vx_i2cg_INRTL, &save_vy_i2cg_INRTL, &save_vz_i2cg_INRTL, &save_ax_i2cg_INRTL, &save_ay_i2cg_INRTL, &save_az_i2cg_INRTL,  nb_tca, OPTIONS->nb_satellites_not_including_gps,  total_ensemble_final_with_ref, nb_time_steps_in_tca_time_span, iProc, iDebugLevel);

    if ( ( iDebugLevel >= 3 ) )
    {
      printf("--- (generate_ephemerides) Done allocating save_x_i2cg_INRTL, save_y_i2cg_INRTL, save_z_i2cg_INRTL, save_vx_i2cg_INRTL, save_vy_i2cg_INRTL, save_vz_i2cg_INRTL, save_ax_i2cg_INRTL, save_ay_i2cg_INRTL, save_az_i2cg_INRTL for collision assessment.(iProc %d)\n", iProc);
    }

    // allocate memory for nb_coll_per_step_per_iproc_in_tca nref * nb_tca * nProcs * ndtspan
    nb_coll_per_step_per_iproc_in_tca = static_cast<int ****>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof(int ***) ));
    if ( nb_coll_per_step_per_iproc_in_tca == NULL )
    {
      print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_per_iproc_in_tca");
    }

    for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
    {
      // all reference sc
      nb_coll_per_step_per_iproc_in_tca[ii] = static_cast<int ***>(malloc( nb_tca * sizeof( int ** ) ));
      for (iiitca = 0; iiitca < nb_tca; iiitca++)
      {
        // all TCA
        nb_coll_per_step_per_iproc_in_tca[ii][iiitca] = static_cast<int **>(malloc( nProcs * sizeof( int *) ));
        for (ccc = 0; ccc < nProcs; ccc++)
        {
          // all procs
          nb_coll_per_step_per_iproc_in_tca[ii][iiitca][ccc] = static_cast<int *>(malloc( nb_time_steps_in_tca_time_span * sizeof(int) ));

          if ( nb_coll_per_step_per_iproc_in_tca[ii][iiitca][ccc] == NULL )
          {
            print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_per_iproc_in_tca");
          }

          for (ppp = 0; ppp < nb_time_steps_in_tca_time_span; ppp++)
          {
            nb_coll_per_step_per_iproc_in_tca[ii][iiitca][ccc][ppp] = 0;
          }

        } // end of all procs

        if ( nb_coll_per_step_per_iproc_in_tca[ii][iiitca] == NULL )
        {
          print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_per_iproc_in_tca");
        }

      } // end of all TCA

      if ( nb_coll_per_step_per_iproc_in_tca[ii] == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_per_iproc_in_tca");
      }

    }// end of all reference sc
    // end of allocate memory for nb_coll_per_step_per_iproc_in_tca

    if (( iDebugLevel >= 2 ) )
    {
      printf("--- (generate_ephemerides) End of allocating big variables for collision assessment.(iProc %d)\n", iProc);
    }

    if (iProc == 0)
    {
      // allocate memory for nb_coll_per_step_in_TCA[ii][iiitca][ppp]
      nb_coll_per_step_in_TCA =  static_cast<int ***>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof(int **) ));

      if ( nb_coll_per_step_in_TCA == NULL )
      {
        print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_in_TCA");
      }

      for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++)
      {
        // all reference sc
        nb_coll_per_step_in_TCA[ii] =  static_cast<int **>(malloc( nb_tca * sizeof( int * ) ));

        for (iiitca = 0; iiitca < nb_tca; iiitca++)
        {
          // all TCA
          nb_coll_per_step_in_TCA[ii][iiitca] =  static_cast<int *>(malloc( nb_time_steps_in_tca_time_span * sizeof( int ) ));

          if ( nb_coll_per_step_in_TCA[ii][iiitca] == NULL )
          {
            print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_in_TCA");
          }

          for (ppp = 0; ppp < nb_time_steps_in_tca_time_span; ppp++)
          {
            nb_coll_per_step_in_TCA[ii][iiitca][ppp] = 0;
          }

        } // end of all TCA

        if ( nb_coll_per_step_in_TCA[ii] == NULL )
        {
          print_error_any_iproc(iProc, "Not enough memory for nb_coll_per_step_in_TCA");
        }

      }// end of all reference sc
      // end of allocate memory for nb_coll_per_step_in_TCA[ii][iiitca][ppp]

      // allocate memory for total_nb_collisions[ii][iiitca]
      total_nb_collisions =  static_cast<int **>(malloc( OPTIONS->nb_satellites_not_including_gps * sizeof(int *) ));
      if ( total_nb_collisions == NULL ){
        print_error_any_iproc(iProc, "Not enough memory for total_nb_collisions");
      }
      for (ii = 0; ii < OPTIONS->nb_satellites_not_including_gps; ii++){ // all reference sc
        total_nb_collisions[ii] =  static_cast<int *>(malloc( nb_tca * sizeof( int ) ));
        if ( total_nb_collisions[ii] == NULL ){
          print_error_any_iproc(iProc, "Not enough memory for total_nb_collisions");
        }
        for (iiitca = 0; iiitca < nb_tca; iiitca++){
          total_nb_collisions[ii][iiitca] = 0;
        }
      } // end of all reference sc
      // end of allocate memory for total_nb_collisions[ii][iiitca]

      // Compute max tca (furthest time of close approach). This will be the end epoch to propagate ensembles until
      max_tca = et_time_step_of_save_tca[0];

      dca_at_max_tca = save_dca[0];

      for (ccc = 0; ccc < nb_tca; ccc++)
      {
        for (ppp = 1; ppp < nProcs_that_are_gonna_run_ensembles; ppp++)
        {
          if (nProcs > 1)
          {
            MPI_Send(&save_tca[ccc], 1, MPI_DOUBLE, ppp, 0, MPI_COMM_WORLD);
            MPI_Send(&et_time_step_of_save_tca[ccc], 1, MPI_DOUBLE, ppp, 0, MPI_COMM_WORLD);
          }
        }

        printf("\n");
        etprint(save_tca[ccc], "tca");
        ptd(save_dca[ccc], "dca");
        printf("tca %d out of %d\n", ccc+1, nb_tca);

        if ( et_time_step_of_save_tca[ccc] > max_tca )
        {
          max_tca =  et_time_step_of_save_tca[ccc];
          dca_at_max_tca = save_dca[ccc];
        }

      }

      if (nProcs > 1)
      {
        for (ppp = 1; ppp < nProcs_that_are_gonna_run_ensembles; ppp++)
        {
          MPI_Send(&max_tca, 1, MPI_DOUBLE, ppp, 0, MPI_COMM_WORLD);
        }
      }
    } else {

      //  pti(iProc, "iProc");
      for (ccc = 0; ccc < nb_tca; ccc++)
      {
        if (nProcs > 1)
        {
          if (iProc < nProcs_that_are_gonna_run_ensembles)
          {
            MPI_Recv(&save_tca[ccc], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&et_time_step_of_save_tca[ccc], 1, MPI_DOUBLE, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          }
        }
      }

      if (nProcs > 1)
      {
        if (iProc < nProcs_that_are_gonna_run_ensembles)
        {
          MPI_Recv(&max_tca, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
      }

    }
  } // end of if collision assessment is on, then first propagate all reference sc (unpertubed orbits) to compute times of close approach (and allocate memory for variables)



  OPTIONS->first_run_to_find_tca_before_collision_assessment = 0;

  if (iProc == 0)
  {
    printf("\n");
  }

  if ( ( iDebugLevel >= 1 ) )
  {
    printf("--- (generate_ephemerides) Propagating all spacecraft (reference and ensembles spacecraft). (iProc %d)\n", iProc);
  }

  if (compute_collisions == 1)
  {
    // some reinitialization of paramters
    if ((strcmp(OPTIONS->test_omniweb_or_external_file, "swpc_mod") == 0) && (OPTIONS->swpc_need_predictions))
    {
      for (ii = 0; ii < OPTIONS->n_satellites; ii++)
      {
        // go through all reference satellites (and their associated ensembles deeper in the loop)
        CONSTELLATION->aaa_mod[ii] =  OPTIONS->aaa_mod[ii];

        for (eee = 1L + iProc * OPTIONS->nb_ensemble_min_per_proc; eee< 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + OPTIONS->nb_ensemble_min_per_proc ; eee++)
        {
          // go through all ensembles of reference sc ii
          CONSTELLATION->sum_sigma_for_f107_average[ii][eee] = 0;
        }

      }
    }
  }

  double sma_ini = CONSTELLATION->spacecraft[0][0].OE.sma;

  // propagate the constellation by dt
  while ( ( fabs( CONSTELLATION->et - endtime ) > OPTIONS->dt / 2. ) && ( min_altitude_constellation > 100.0) )
  {
    // Time to stop the propagation
    min_end_time = endtime;

    if ( ( compute_collisions == 1 ) )
    {
      //2020-11-13 i don't want to worry about dt_pos_neg (backward propagation) for collision avoidance
      if ( max_tca + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt ) < endtime )
      {
        min_end_time =  max_tca + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
      }
    }

    // Print progress to screen
    if (iProc == 0)
    {
      print_progress( min_end_time, CONSTELLATION->et , starttime, iProc, OPTIONS->nb_gps )  ;
    }

    /// start collision assessment when the secondary sc time enters the span of time around TCA.
    if ( ( compute_collisions == 1 )  )
    {
      /// @warning CONSTELLATION->et  is temporary and assumes the reference sc have the same epoch start. If it's not the case then this needs to be changed (below and at other locations in the code) //2020-11-13 i don't want to worry about dt_pos_neg (backward propagation) for collision avoidance
      itca = -1; // if itca is different from -1 then it means that the time is in the time spanning TCA (unpertubed)
      time_step_of_tca = -1;
      for ( ccc = start_itca; ccc < nb_tca; ccc++ )
      {
        if ( ( CONSTELLATION->et + OPTIONS->dt >= et_time_step_of_save_tca[ccc] - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt ) )  )
        {
          itca = ccc;
          et_current_tca = et_time_step_of_save_tca[ccc];
          et_start_of_span = et_time_step_of_save_tca[ccc] - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
          et_end_of_span = et_time_step_of_save_tca[ccc] + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt );
          time_step_of_tca = (int)(nb_time_steps_in_tca_time_span / 2);
          done_with_tca[itca] = 0;
        }
      }
    }  // end of  start collision assessment when the secondary sc time enters the span of time around TCA

    // go through all reference satellites (and their associated ensembles deeper in the loop)
    for (ii = 0; ii < OPTIONS->n_satellites; ii++)
    {
      if ( start_ensemble[ii] == 0)
      {
        // if this iProc runs main sc ii
        if ( already_propagated_ref_sc == 0 )
        {
          // i unpertubed orbits (eee = 0) would have already been propagated so don't propagate them again here
          previous_lat = CONSTELLATION->spacecraft[ii][0].GEODETIC.latitude*RAD2DEG;

          propagate_spacecraft( &CONSTELLATION->spacecraft[ii][0], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc, iDebugLevel, start_ensemble, array_sc );

          if (CONSTELLATION->spacecraft[ii][0].GEODETIC.altitude <  min_altitude_constellation)
          {
            min_altitude_constellation = CONSTELLATION->spacecraft[ii][0].GEODETIC.altitude;
          }
        }
      } // end of if this iProc runs main sc ii


        if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
        {
          // if the reference satellite is not a GPS, we propagate the associated ensembles
          if ( OPTIONS->nb_ensembles_min > 0 )
          {
            // if ensembles are computed
            if ( array_sc[1] > 0 )
            {
              // if this iProc runs ensembles (otherwise array_sc[1] = - 1)
              for (eee = 1L + iProc * OPTIONS->nb_ensemble_min_per_proc; eee< 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + OPTIONS->nb_ensemble_min_per_proc ; eee++)
              {
                // go through all ensembles of reference sc ii
                propagate_spacecraft( &CONSTELLATION->spacecraft[ii][eee], PARAMS, starttime, OPTIONS->et_oldest_tle_epoch, &density, GROUND_STATION, OPTIONS, CONSTELLATION, iProc  , iDebugLevel, start_ensemble, array_sc );

                ispan_constellation = (int)( fabs( CONSTELLATION->et + OPTIONS->dt_pos_neg - et_start_of_span ) / OPTIONS->dt );

                if ( ( compute_collisions == 1 )  && ( itca != -1 ) && ( ispan_constellation < nb_time_steps_in_tca_time_span ) )
                {

                  // // Save positions of all ensembles
                  save_x_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].r_i2cg_INRTL[0];
                  save_y_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].r_i2cg_INRTL[1];
                  save_z_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].r_i2cg_INRTL[2];

                  save_vx_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].v_i2cg_INRTL[0];
                  save_vy_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].v_i2cg_INRTL[1];
                  save_vz_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].v_i2cg_INRTL[2];

                  save_ax_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].a_i2cg_INRTL[0];
                  save_ay_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].a_i2cg_INRTL[1];
                  save_az_i2cg_INRTL[ii][eee][ispan_constellation] = CONSTELLATION->spacecraft[ii][eee].a_i2cg_INRTL[2];

                  if ( ii == 0 )
                  {
                    //  send the position/velocity/acceleration of a primary sc computed by a iProc to all other iProc
                    /// @warning  for now works only if two reference satellies only  ("TWO" because see next if condition). Note that main sc 0 is always run by iProc 0 and never by iProc > 0.
                    if (nProcs > 1)
                    {
                      for (ccc = 0; ccc < nProcs_that_are_gonna_run_ensembles; ccc++)
                      {
                        // send the position/velocity/acceleration computed by a iProc to all other iProc
                        if ( ccc != iProc )
                        {
                          MPI_Send(&save_x_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                          MPI_Send(&save_y_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                          MPI_Send(&save_z_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);

                          MPI_Send(&save_vx_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                          MPI_Send(&save_vy_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                          MPI_Send(&save_vz_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);

                          MPI_Send(&save_ax_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                          MPI_Send(&save_ay_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                          MPI_Send(&save_az_i2cg_INRTL[ii][eee][ispan_constellation], 1, MPI_DOUBLE, ccc, iProc, MPI_COMM_WORLD);
                        }
                      } // end of send the position/velocity/acceleration of a primary computed by a iProc to all other iProc
                    } // end of if nProcs > 1

                    for (ccc = 0; ccc < nProcs; ccc++)
                    {
                      // receive the positions by iProc sent by all other iProc
                      if ( ccc != iProc )
                      {
                        eee_all_other_iproc = ccc * OPTIONS->nb_ensemble_min_per_proc +  eee - iProc * OPTIONS->nb_ensemble_min_per_proc;
                        if (nProcs > 1)
                        {
                          if (iProc < nProcs_that_are_gonna_run_ensembles)
                          {
                            MPI_Recv(&save_x_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&save_y_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&save_z_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                            MPI_Recv(&save_vx_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&save_vy_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&save_vz_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                            MPI_Recv(&save_ax_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&save_ay_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&save_az_i2cg_INRTL[ii][eee_all_other_iproc][ispan_constellation], 1, MPI_DOUBLE, ccc, ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                          }
                        }
                      }
                    } // end of receive the positions by iProc sent by all other iProc
                  }  // end of if ii == 0
                }	// end of if the time is in the time span around TCA

                if (ii == 1)
                {
                  // only get in this loop if secondary sc
                  if ( ( compute_collisions == 1 )  && ( itca != -1 ) && ( ispan_constellation == nb_time_steps_in_tca_time_span - 1 ) )
                  {
                    if ( ( iDebugLevel >= 5 ) )
                    {
                      printf("----- (generate_ephemerides) Just got out of time spanning TCA %d and now computing collisions for this time span (iProc %d)\n", itca+1, iProc);
                    }

                    eee_prim_that_collide = -1;

                    for (eee_prim = 1; eee_prim< 1 + nProcs * OPTIONS->nb_ensemble_min_per_proc ; eee_prim++)
                    {
                      compute_collision_between_one_secondary_and_all_primary(
                        save_x_i2cg_INRTL[1][eee],
                        save_y_i2cg_INRTL[1][eee],
                        save_z_i2cg_INRTL[1][eee],
                        save_vx_i2cg_INRTL[1][eee],
                        save_vy_i2cg_INRTL[1][eee],
                        save_vz_i2cg_INRTL[1][eee],
                        save_ax_i2cg_INRTL[1][eee],
                        save_ay_i2cg_INRTL[1][eee],
                        save_az_i2cg_INRTL[1][eee],
                        save_x_i2cg_INRTL[0][eee_prim],
                        save_y_i2cg_INRTL[0][eee_prim],
                        save_z_i2cg_INRTL[0][eee_prim],
                        save_vx_i2cg_INRTL[0][eee_prim],
                        save_vy_i2cg_INRTL[0][eee_prim],
                        save_vz_i2cg_INRTL[0][eee_prim],
                        save_ax_i2cg_INRTL[0][eee_prim],
                        save_ay_i2cg_INRTL[0][eee_prim],
                        save_az_i2cg_INRTL[0][eee_prim],
                        OPTIONS,
                        iProc,
                        nb_coll_per_step_per_iproc_in_tca[1][itca][iProc],
                        et_time_step_of_save_tca,
                        nb_time_steps_in_tca_time_span,
                        itca, eee_prim, eee, tca_file,
                        dca_file,
                        sample_file,
                        OPTIONS->write_collision_files,
                        &eee_prim_that_collide );

                      } // end of go over all primary ensemble sc (eee_prim) to look for a collision with secondary ensemble sc eee

                      if ( ( iDebugLevel >= 5 ) )
                      {
                        printf("----- (generate_ephemerides) DONE just got out of time spanning TCA %d and now computing collisions for this time span (iProc %d)\n", itca+1, iProc);
                      }
                    }
                  } // end of if ii == 1
                } // end of go through all ensembles of reference sc ii
              } // end of if this iProc runs ensembles (otherwise array_sc[1] = - 1)
            } // end of if ensembles are computed
          } // end of if reference satellite is not GPS, we propagate the associated ensembles

          // WRITE OUTPUT

          if ( ( already_propagated_ref_sc == 1 )   )
          {
            // set output_only_at_tca to 1 to output only at tca  (if computing collisions)
            int output_only_at_tca = 1;

            //2020-11-13 i don't want to worry about dt_pos_neg (backward propagation) for ensemble propagation
            if ( output_only_at_tca != 1 )
            {
              if ( (array_sc[start_ensemble[ii]] >= 0) && ((CONSTELLATION->spacecraft[ii][1 + iProc * OPTIONS->nb_ensemble_min_per_proc].et - twrite) >= OPTIONS->dt - 0.01) )
              {
                if ( ( fmod( CONSTELLATION->spacecraft[ii][1 + iProc * OPTIONS->nb_ensemble_min_per_proc].et - starttime, OPTIONS->dt_output ) < OPTIONS->dt / 2.) ||
                ( fabs( fmod( CONSTELLATION->spacecraft[ii][1 + iProc * OPTIONS->nb_ensemble_min_per_proc].et - starttime, OPTIONS->dt_output ) - OPTIONS->dt_output ) < OPTIONS->dt / 2. ) ||
                ( CONSTELLATION->spacecraft[ii][1 + iProc * OPTIONS->nb_ensemble_min_per_proc].et > min_end_time - 0.01))
                {
                  write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit,ii, OPTIONS->n_satellites,OPTIONS->nb_gps,  OPTIONS->nb_ensembles_min, OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,0,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
                }
              }

              if ( ( ii == (OPTIONS->n_satellites - 1) ) && (compute_collisions == 0) )
              {
                twrite = CONSTELLATION->et + OPTIONS->dt;
              }

              if ( ( ii == (OPTIONS->n_satellites - 1) ) && (compute_collisions == 1) )
              {
                twrite = CONSTELLATION->et + OPTIONS->dt;
              }
            }
            else
            {
              // to output only at tca of unperturbed orbit
              if (itca != -1)
              {
                if ( ( CONSTELLATION->spacecraft[ii][1 + iProc * OPTIONS->nb_ensemble_min_per_proc].et >= et_current_tca - OPTIONS->dt ) &&
                ( CONSTELLATION->spacecraft[ii][1 + iProc * OPTIONS->nb_ensemble_min_per_proc].et <= et_current_tca + OPTIONS->dt  ) )
                {
                  write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit,ii, OPTIONS->n_satellites,OPTIONS->nb_gps,  OPTIONS->nb_ensembles_min, OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,0,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
                }
              }
            }
          }else{

            if ( (array_sc[start_ensemble[ii]] >= 0) && (fabs(CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et - twrite) >= OPTIONS->dt - 0.01) )
            {
              if (OPTIONS->dt_pos_neg >= 0)
              {
                if ( ( fmod( CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et - starttime, OPTIONS->dt_output ) < OPTIONS->dt / 2.) ||
                ( fabs( fmod( CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et - starttime, OPTIONS->dt_output ) - OPTIONS->dt_output ) < OPTIONS->dt / 2.) ||
                ( CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et > min_end_time - 0.01) )
                {
                  write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit,ii, OPTIONS->n_satellites,OPTIONS->nb_gps,  OPTIONS->nb_ensembles_min, OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
                  et2utc_c(CONSTELLATION->spacecraft[ii][0].et, "ISOC" ,6 ,255 , times_att);
                }
              }
              else
              {
                if ( ( fmod(fabs( CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et - starttime), OPTIONS->dt_output ) < OPTIONS->dt / 2.) || ( fabs( fmod( CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et - starttime, OPTIONS->dt_output ) - OPTIONS->dt_output ) < OPTIONS->dt / 2.) || ( CONSTELLATION->spacecraft[ii][array_sc[start_ensemble[ii]]].et < min_end_time + 0.01) )
                {
                  write_output( CONSTELLATION->spacecraft[ii] , 0, choose_tle_to_initialise_orbit,ii, OPTIONS->n_satellites,OPTIONS->nb_gps,  OPTIONS->nb_ensembles_min, OPTIONS->nb_ensemble_min_per_proc,  iProc,  OPTIONS->nb_ensembles_output_files, OPTIONS->filename_output_ensemble, previous_lat,OPTIONS,PARAMS->EARTH.earth_fixed_frame,1,1, ( CONSTELLATION->et + OPTIONS->dt_pos_neg ), nProcs, iDebugLevel, compute_collisions, start_ensemble, array_sc, CONSTELLATION, PARAMS);
                  et2utc_c(CONSTELLATION->spacecraft[ii][0].et, "ISOC" ,6 ,255 , times_att);
                }
              }
            }

            if ( ( ii == (OPTIONS->n_satellites - 1) ) && (compute_collisions == 0) )
            {
              twrite = CONSTELLATION->et + OPTIONS->dt_pos_neg;
            }

            if ( ( ii == (OPTIONS->n_satellites - 1) ) && (compute_collisions == 1) )
            {
              twrite = CONSTELLATION->et + OPTIONS->dt_pos_neg;
            }
          }
        } // go through all reference satellites (and their associated ensembles deeper in the loop)  (END OF LOOP OVER ALL MAIN SATELLITES)

        if ( ( compute_collisions == 1 ))
        {
          if ( ( CONSTELLATION->et + OPTIONS->dt ) > max_tca + ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt ) )
          {
            // if collision assessment is on, then the ensembles are propagated until
            // the time of closest approach
            break;
          }
        }
        CONSTELLATION->et = CONSTELLATION->et + OPTIONS->dt_pos_neg;
      } // end of propagate the constellation by dt  (END OF WHILE PROPAGATION OF ALL SATELLITES AND ENSEMBLES)

      // END OF PROPAGATION

      if (( iDebugLevel >= 1 ) )
      {
        printf("--- (generate_ephemerides) Done propagating all spacecraft (reference and ensembles spacecraft). (iProc %d)\n", iProc);
      }

      if (iProc == 0)
      {
        printf("\n");
      }

      // COMPUTE COLLISION FOR EACH TCA
      if ( ( compute_collisions == 1 ) )
      {

        // if collision assessment is on
        if ( ( iDebugLevel >= 4 ) )
        {
          printf("----- (generate_ephemerides) Sending nb_coll_per_step_per_iproc_in_tca to main node... (iProc %d)\n", iProc);
        }

        /// @warning for now works only if two reference satellies only
        ii = 1;
        for ( iiitca = start_itca; iiitca < nb_tca; iiitca++ )
        {
          for ( ppp = 0; ppp < nb_time_steps_in_tca_time_span; ppp++)
          {
            if (iProc != 0)
            {
              // send step_coll to main node
              if (nProcs > 1)
              {
                MPI_Send(&nb_coll_per_step_per_iproc_in_tca[ii][iiitca][iProc][ppp], 1, MPI_INT, 0, 2 * ii *   + iiitca * ( nb_time_steps_in_tca_time_span ) *  ( nProcs * OPTIONS->nb_ensemble_min_per_proc + 1 + 1 ) + ppp * ( 1 + nProcs * OPTIONS->nb_ensemble_min_per_proc ) + iProc , MPI_COMM_WORLD);
              }
            } // end of send step_coll to main node
          }
        }

        if ( ( iDebugLevel >= 4 ) )
        {
          printf("----- (generate_ephemerides) Done sending nb_coll_per_step_per_iproc_in_tca to main node. (iProc %d)\n", iProc);
        }

        if ( iProc == 0 )
        {
          if ( ( iDebugLevel >= 4 ) )
          {
            printf("----- (generate_ephemerides) Main node receiving nb_coll_per_step_per_iproc_in_tca... (iProc %d)\n", iProc);
          }

          nb_tca_without_collisions = 0;
          nb_tca_with_collisions = nb_tca - start_itca;

          for ( iiitca = start_itca; iiitca < nb_tca; iiitca++ )
          {
            for ( ppp = 1; ppp < nb_time_steps_in_tca_time_span-1-1; ppp++)
            {
              for (ccc = 0; ccc < nProcs; ccc++)
              {
                if (ccc > 0)
                {
                  if (nProcs > 1)
                  {
                    MPI_Recv(&nb_coll_per_step_per_iproc_in_tca[ii][iiitca][ccc][ppp], 1, MPI_DOUBLE, ccc,2 * ii *   + iiitca * ( nb_time_steps_in_tca_time_span ) *  ( nProcs * OPTIONS->nb_ensemble_min_per_proc + 1 + 1 ) + ppp * ( 1 + nProcs * OPTIONS->nb_ensemble_min_per_proc ) + ccc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                  }
                }
                nb_coll_per_step_in_TCA[ii][iiitca][ppp] = nb_coll_per_step_in_TCA[ii][iiitca][ppp] + nb_coll_per_step_per_iproc_in_tca[ii][iiitca][ccc][ppp];
                total_nb_collisions[ii][iiitca]  = total_nb_collisions[ii][iiitca]  + nb_coll_per_step_per_iproc_in_tca[ii][iiitca][ccc][ppp];
              }
            }
            if (total_nb_collisions[ii][iiitca] == 0)
            {
              nb_tca_without_collisions = nb_tca_without_collisions + 1;
            }
          }

          nb_tca_with_collisions = nb_tca - start_itca - nb_tca_without_collisions;

          if ( ( iDebugLevel >= 4 ) )
          {
            printf("----- (generate_ephemerides) Main node done receiving nb_coll_per_step_per_iproc_in_tca. (iProc %d)\n", iProc);
          }

        } // end of if iproc == 0
      } // end of if collision assessment is on

      // END OF  COLLISION

      for (ii = 0; ii < OPTIONS->n_satellites; ii++)
      {
        if ( start_ensemble[ii] == 0)
        {
          // if this iProc runs main sc ii
          fclose( CONSTELLATION->spacecraft[ii][0].fpecef );
          // this was open by all iProc
        }
      }

      MPI_Barrier(MPI_COMM_WORLD);

      int find_file_name;
      char *next;

      if (iProc == 0)
      {
        if (OPTIONS->nb_gps > 0)
        {
          char ch;
          CONSTELLATION->file_CYGNSS_constellation_for_specular = fopen(CONSTELLATION->filename_CYGNSS_constellation_for_specular, "w+");
          for (ii = 0;ii< OPTIONS->n_satellites - OPTIONS->nb_gps;ii++)
          {
            // (ECEF format) (CBV)
            CONSTELLATION->spacecraft[ii][0].fpecef = fopen( CONSTELLATION->spacecraft[ii][0].filenameecef, "r");

            while( ( ch = fgetc(CONSTELLATION->spacecraft[ii][0].fpecef) ) != EOF )
            {
              fputc(ch,CONSTELLATION->file_CYGNSS_constellation_for_specular);
            }
            fprintf(CONSTELLATION->file_CYGNSS_constellation_for_specular,"\n");
            fprintf(CONSTELLATION->file_CYGNSS_constellation_for_specular,"\n");
            // this was open by all iProc
            fclose( CONSTELLATION->spacecraft[ii][0].fpecef );
          }

          CONSTELLATION->file_GPS_constellation_for_specular = fopen(CONSTELLATION->filename_GPS_constellation_for_specular, "w+");

          for (ii = OPTIONS->n_satellites - OPTIONS->nb_gps;ii< OPTIONS->n_satellites;ii++)
          {
            // (ECEF format) (CBV)
            CONSTELLATION->spacecraft[ii][0].fpecef = fopen( CONSTELLATION->spacecraft[ii][0].filenameecef, "r");

            while( ( ch = fgetc(CONSTELLATION->spacecraft[ii][0].fpecef) ) != EOF )
            {
              fputc(ch,CONSTELLATION->file_GPS_constellation_for_specular);
            }

            fprintf(CONSTELLATION->file_GPS_constellation_for_specular,"\n");
            fprintf(CONSTELLATION->file_GPS_constellation_for_specular,"\n");
            // this was open by all iProc
            fclose( CONSTELLATION->spacecraft[ii][0].fpecef );
          }

          fclose(CONSTELLATION->file_CYGNSS_constellation_for_specular);
          fclose(CONSTELLATION->file_GPS_constellation_for_specular);
        }
      } // end of if iProc == 0

      // COLLISION

      MPI_Barrier(MPI_COMM_WORLD);

      if ( iProc == 0 )
      {
        if ( (compute_collisions == 1) )
        {
          // if collision assessment is on
          ii = 1; // !!!!!!!!! FOR NOW WORKS ONLY IF TWO REFERENCE SATELLIES ONLY

          file_collision = fopen(CONSTELLATION->filename_collision, "w+");

          fprintf(file_collision, "Total number of ensembles: %d.\nTime step of integration DT: %.2f seconds.\nNumber of close approaches: %d (+ %d without any collision).\n", nProcs * OPTIONS->nb_ensemble_min_per_proc, OPTIONS->dt, nb_tca_with_collisions, nb_tca_without_collisions);

          for ( iiitca = start_itca; iiitca < nb_tca; iiitca++ )
          {
            fprintf(file_collision, "Cumulative probability of collision during the time spanning TCA %d: %f.\n",iiitca + 1, (double)total_nb_collisions[ii][iiitca] / ( nProcs * OPTIONS->nb_ensemble_min_per_proc * nProcs * OPTIONS->nb_ensemble_min_per_proc ) );
          }

          for ( iiitca = start_itca; iiitca < nb_tca; iiitca++ )
          {
            //    pti(total_nb_collisions[ii][iiitca], "Total number of collisions");
            printf("CA %d - Total cumulative probability of collision: %.5f\n", iiitca + 1,(double)total_nb_collisions[ii][iiitca] / ( nProcs * OPTIONS->nb_ensemble_min_per_proc * nProcs * OPTIONS->nb_ensemble_min_per_proc )) ;
            fprintf(file_collision, "\n#Detailed results for close approach %d\n", iiitca+1);
            et2utc_c(save_tca[iiitca], "ISOC", 3, 255, time_itca);
            fprintf(file_collision, "TCA: %s.\n", time_itca);
            fprintf(file_collision, "DCA: %.1f m.\n", save_dca[iiitca] * 1000.); // use dca with caution (see comment "this is incorrect. What would be correct would be to take the distance at inital epoch + period / 2 but we did not save this distance. Since save_dca is never used after, we don't really care, except in the collision output file where dca is reported")
            fprintf(file_collision, "Primary object sample near conjunction point: %d\n", eee_prim_that_collide);// this is one of the samples that is near the conjnuction point (see function compute_collision_between_one_secondary_and_all_primary for the definition of "near")

            if ( total_nb_collisions[ii][iiitca] > 0 )
            {
              fprintf(file_collision, "Cumulative Probability of collision during the time spanning TCA: %f.\n", (double)total_nb_collisions[ii][iiitca] / ( nProcs * OPTIONS->nb_ensemble_min_per_proc * nProcs * OPTIONS->nb_ensemble_min_per_proc ) );
              fprintf(file_collision, "Total number of collisions: %d.\n", total_nb_collisions[ii][iiitca] );
              et2utc_c(et_time_step_of_save_tca[iiitca] - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt) + OPTIONS->dt , "ISOC", 3, 255, time_itca); // "+ OPTIONS->dt": see explanation below (comments starting with "ABOUT THE TIME SPAN")
              fprintf(file_collision, "Time spanning TCA %d: %s - ", iiitca + 1, time_itca);
              et2utc_c(et_time_step_of_save_tca[iiitca] + ((int)(nb_time_steps_in_tca_time_span / 2)* OPTIONS->dt )- OPTIONS->dt, "ISOC", 3, 255, time_itca); // "- OPTIONS->dt": see explanation below (comments starting with "ABOUT THE TIME SPAN")
              fprintf(file_collision, "%s (duration of span: %.3f seconds (%d time steps)).\n", time_itca, CONSTELLATION->collision_time_span - 2 * OPTIONS->dt , nb_time_steps_in_tca_time_span - 2 ); // "- 2 * OPTIONS->dt" and "- 2": see explanation below (comments starting with "ABOUT THE TIME SPAN").
              fprintf(file_collision, "##REAL_TIME NB_COLLISIONS_PER_DT\n" );

              for ( ppp = 1; ppp < nb_time_steps_in_tca_time_span-1-1; ppp++)
              {
                et_step_collision = et_time_step_of_save_tca[iiitca] - ((int)(nb_time_steps_in_tca_time_span / 2) * OPTIONS->dt ) + ppp * OPTIONS->dt;
                et2utc_c(et_step_collision, "ISOC", 3, 255, time_itca);
                et2utc_c(et_step_collision+ OPTIONS->dt, "ISOC", 3, 255, time_itca2);
                fprintf(file_collision, "%s -> %s %d\n", time_itca, time_itca2, nb_coll_per_step_in_TCA[ii][iiitca][ppp]);
              }
              fprintf(file_collision, "#End of results for close approach %d\n", iiitca+1);
            }
          }
          fclose(file_collision);
        } // end of if collision assessment is on
        else if (compute_collisions_was_on_but_no_tca_found == 1)
        {
          // the user chose to compute collision but to TCA was found between the initial epoch and the final epoch
          file_collision = fopen(CONSTELLATION->filename_collision, "w+");
          fprintf(file_collision, "No close approach (< %.2f meters) for the unperturbed orbits was found between the initial epoch (%s) and the final epoch (%s).\n", OPTIONS->min_dist_close_approach*1000, OPTIONS->initial_epoch, OPTIONS->final_epoch);
          fclose(file_collision);
        }
      } // end of if iProc == 0

      // end of COLLISION

      // Close files needed

      for (ii = 0; ii < OPTIONS->n_satellites; ii++)
      {
        if ( start_ensemble[ii] == 0)
        {
          // if this iProc runs main sc ii
          if ( CONSTELLATION->spacecraft[ii][0].INTEGRATOR.isGPS == 0 )
          {
            fclose( CONSTELLATION->spacecraft[ii][0].fp );
            fclose( CONSTELLATION->spacecraft[ii][0].fpout );
            fclose( CONSTELLATION->spacecraft[ii][0].fprho );
            fclose( CONSTELLATION->spacecraft[ii][0].fpatt );
          }

          /// @warning comment line below if you did not open this file at the beginning of this function
          if (ii < OPTIONS->n_satellites - OPTIONS->nb_gps)
          {
            fclose(CONSTELLATION->spacecraft[ii][0].INTEGRATOR.file_given_output);
          }

          if (GROUND_STATION->nb_ground_stations > 0)
          {
            if (ii < OPTIONS->n_satellites - OPTIONS->nb_gps)
            {
              //do not compute the ground station coverage for gps
              for (iground = 0; iground < GROUND_STATION->nb_ground_stations; iground++)
              {
                fclose(CONSTELLATION->spacecraft[ii][0].fp_coverage_ground_station[iground]);
              }
            }
          }

          if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
          {
            if (CONSTELLATION->spacecraft[ii][0].INTEGRATOR.solar_cell_efficiency != -1)
            {
              fclose( CONSTELLATION->spacecraft[ii][0].fpower );
              fclose( CONSTELLATION->spacecraft[ii][0].fpeclipse );
            }
          }
        } // end of if this iProc runs main sc ii

        if (ii < OPTIONS->n_satellites - OPTIONS-> nb_gps)
        {
          if ( OPTIONS->nb_ensembles_min > 0 )
          {
            if ( array_sc[1] > 0 )
            {
              // if this iProc runs ensembles (otherwise array_sc[1] = - 1)
              for (fff = 0; fff < OPTIONS->nb_ensembles_output_files ; fff ++)
              {
                if ( (strcmp(OPTIONS->filename_output_ensemble[fff], "tca" )!= 0 ) && (strcmp(OPTIONS->filename_output_ensemble[fff], "dca" )!= 0 ) && (strcmp(OPTIONS->filename_output_ensemble[fff], "sample" )!= 0 ))
                {
                  fclose(CONSTELLATION->spacecraft[ii][0].fpiproc[fff]);
                }
                else if (ii == 0)
                {
                  if (strcmp(OPTIONS->filename_output_ensemble[fff], "tca" ) == 0)
                  {
                    fclose( tca_file );
                  }
                  if (strcmp(OPTIONS->filename_output_ensemble[fff], "dca" ) == 0)
                  {
                    fclose( dca_file );
                  }
                }
              }
            } // end of if this iProc runs ensembles (otherwise array_sc[1] = - 1)
          }
        }
        free( CONSTELLATION->spacecraft[ii]);
      }

      if (( iDebugLevel >= 1 ) )
      {
        printf("-- (generate_ephemerides) Just got out from generate_ephemerides.(iProc %d)\n", iProc);
      }

      return 0;
    }
