#include "options.h"
#include "moat_prototype.h"
#include "gsl/gsl_poly.h"

///
/// @brief Interpolate the positions of a satellite
///
int interpolate_position(char filenamella[300], char filenameecef[300], double dt_before, double dt_after, int sat_number, OPTIONS_T *OPTIONS, PARAMS_T *PARAMS)
{
  double heading; // heading
  FILE *fplla = NULL, *fpecef = NULL;
  double earth_flattening    = 1/298.257223560;
  double earth_radius        = 6378.137;
  char name_sat[300];
  char filename_position_interpolated[300];
  FILE *fp_position_interpolated;
  char text_name_interpolated[300];
  double ecef_x_previous_step = 0, ecef_y_previous_step = 0, ecef_z_previous_step = 0;
  double ecef_vx_previous_step = 0, ecef_vy_previous_step = 0, ecef_vz_previous_step = 0; // heading
  double et_to_interpolate;
  char *line_ecef = NULL;
  size_t len_ecef = 0;
  char *line_lla = NULL;
  size_t len_lla = 0;
  ssize_t read;
  double lla_to_interpolate[3];
  double ecef_to_interpolate[3];
  double ecef_v_to_interpolate[3]; // heading
  char time_to_interpolate[300];
  double lla_interpolated[3];
  double ecef_interpolated[3];
  double ecef_v_interpolated[3];// heading
  double time_for_interpolation=0;
  double save_previous_et=0;
  int line_number = 0;
  char times[300];
  char *next;
  char text_output[300], text[300];
  int find_file_name;
  int found_eoh;

  strcpy(name_sat,OPTIONS->filename_output[sat_number]);
  next = &name_sat[0];
  strcpy(text_output," ");
  find_file_name =  (int)(strchr(next, '.') - next);
  strncat(text_output, next, find_file_name);

  /* Algorithm */

  // Read satellite position file
  fplla = fopen(filenamella, "r");
  if (fplla == NULL)
  {
    printf("***! Could not find the file %s. The program will stop. !***\n", filenamella); MPI_Finalize();exit(0);
  }

  fpecef= fopen(filenameecef, "r");

  if (fpecef == NULL)
  {
    printf("***! Could not find the file %s. The program will stop. !***\n", filenameecef); MPI_Finalize();exit(0);
  }

  // Skip the header ECEF
  found_eoh = 0;
  while ( found_eoh == 0 )
  {
    getline(&line_ecef, &len_ecef, fpecef);
    sscanf(line_ecef, "%s", text);
    if (  strcmp( "#START", text  ) == 0 )
    {
      found_eoh = 1;
    }
  }

  // Skip the header LLA
  found_eoh = 0;
  while ( found_eoh == 0 )
  {
    getline(&line_lla, &len_lla, fplla);
    sscanf(line_lla, "%s", text);
    if (  strcmp( "#START", text  ) == 0 )
    {
      found_eoh = 1;
    }
  }

  strcpy(text_name_interpolated, OPTIONS->dir_output_run_name_sat_name[sat_number]);
  strcat(text_name_interpolated, "/");
  strcat(text_name_interpolated,"interpolated_position_LLA_ECEF_");
  strcat(text_name_interpolated, name_sat);
  strcpy(filename_position_interpolated, text_name_interpolated);
  fp_position_interpolated = fopen(filename_position_interpolated, "w+");

  while ( (read = getline(&line_lla, &len_lla,  fplla)) != -1 )
  {
    // ecef and lla files have the same number of time steps
    getline(&line_ecef, &len_ecef, fpecef);
    sscanf(line_lla, "%19[^\n] %lf %lf %lf", time_to_interpolate, &lla_to_interpolate[0], &lla_to_interpolate[1], &lla_to_interpolate[2]);
    sscanf(line_ecef, "%19[^\n] %lf %lf %lf %lf %lf %lf", time_to_interpolate, &ecef_to_interpolate[0], &ecef_to_interpolate[1], &ecef_to_interpolate[2], &ecef_v_to_interpolate[0], &ecef_v_to_interpolate[1], &ecef_v_to_interpolate[2]); // heading
    str2et_c(time_to_interpolate, &et_to_interpolate);

    if ( line_number > 0 )
    {
      time_for_interpolation = save_previous_et + dt_after;
      while (time_for_interpolation - 0.00001 <= et_to_interpolate)
      {
        ecef_interpolated[0] = ecef_x_previous_step + ( ( time_for_interpolation - save_previous_et ) / ( et_to_interpolate - save_previous_et )  ) * ( ecef_to_interpolate[0] - ecef_x_previous_step ) ;
        ecef_interpolated[1] = ecef_y_previous_step + ( ( time_for_interpolation - save_previous_et ) / ( et_to_interpolate - save_previous_et ) ) * ( ecef_to_interpolate[1] - ecef_y_previous_step ) ;
        ecef_interpolated[2] = ecef_z_previous_step + ( ( time_for_interpolation - save_previous_et ) / ( et_to_interpolate - save_previous_et ) ) * ( ecef_to_interpolate[2] - ecef_z_previous_step ) ;
        // heading
        ecef_v_interpolated[0] = ecef_vx_previous_step + ( ( time_for_interpolation - save_previous_et ) / ( et_to_interpolate - save_previous_et )  ) * ( ecef_v_to_interpolate[0] - ecef_vx_previous_step ) ;
        ecef_v_interpolated[1] = ecef_vy_previous_step + ( ( time_for_interpolation - save_previous_et ) / ( et_to_interpolate - save_previous_et ) ) * ( ecef_v_to_interpolate[1] - ecef_vy_previous_step ) ;
        ecef_v_interpolated[2] = ecef_vz_previous_step + ( ( time_for_interpolation - save_previous_et ) / ( et_to_interpolate - save_previous_et ) ) * ( ecef_v_to_interpolate[2] - ecef_vz_previous_step ) ;
        // end of heading

        geocentric_to_geodetic(
          ecef_interpolated,
          &earth_radius,
          &earth_flattening,
          &lla_interpolated[2],
          &lla_interpolated[1],
          &lla_interpolated[0]
        );

        // calculate heading of satellite
        compute_heading(
          &heading,
          ecef_v_interpolated,
          lla_interpolated[0],
          lla_interpolated[1],
          PARAMS->EARTH.flattening
        );

        // heading
        lla_interpolated[2] = lla_interpolated[2];
        lla_interpolated[1] = lla_interpolated[1] *RAD2DEG;
        lla_interpolated[0] = lla_interpolated[0] * RAD2DEG;

        // write in the extrapolated file the interpolated positions
        et2utc_c(time_for_interpolation, "ISOC" ,0 ,255 , times);
        fprintf(fp_position_interpolated, "%s %f %f %f %f %f %f %f \n", times, lla_interpolated[0], lla_interpolated[1], lla_interpolated[2], ecef_interpolated[0], ecef_interpolated[1], ecef_interpolated[2], heading*RAD2DEG); // heading
        time_for_interpolation = time_for_interpolation + dt_after;
      }

      save_previous_et = et_to_interpolate;
      ecef_x_previous_step = ecef_to_interpolate[0];
      ecef_y_previous_step = ecef_to_interpolate[1];
      ecef_z_previous_step = ecef_to_interpolate[2];
      ecef_vx_previous_step = ecef_v_to_interpolate[0];// heading
      ecef_vy_previous_step = ecef_v_to_interpolate[1];// heading
      ecef_vz_previous_step = ecef_v_to_interpolate[2];// heading

    }
    else
    {
      save_previous_et = et_to_interpolate;
      ecef_x_previous_step = ecef_to_interpolate[0];
      ecef_y_previous_step = ecef_to_interpolate[1];
      ecef_z_previous_step = ecef_to_interpolate[2];
      ecef_vx_previous_step = ecef_v_to_interpolate[0];// heading
      ecef_vy_previous_step = ecef_v_to_interpolate[1];// heading
      ecef_vz_previous_step = ecef_v_to_interpolate[2];// heading

      geocentric_to_geodetic(
        ecef_to_interpolate,
        &earth_radius,
        &earth_flattening,
        &lla_to_interpolate[2],
        &lla_to_interpolate[1],
        &lla_to_interpolate[0]
      );

      // calculate heading of satellite
      compute_heading(
        &heading,
        ecef_v_to_interpolate,
        lla_to_interpolate[0],
        lla_to_interpolate[1],
        PARAMS->EARTH.flattening
      ); // heading

      lla_to_interpolate[2] = lla_to_interpolate[2] ;
      lla_to_interpolate[1] = lla_to_interpolate[1] *RAD2DEG;
      lla_to_interpolate[0] = lla_to_interpolate[0] * RAD2DEG;

      // just to get the same time format as the rest of the file
      et2utc_c(et_to_interpolate, "ISOC" ,0 ,255 , times);
      fprintf(
        fp_position_interpolated,
        "%s %f %f %f %f %f %f %f \n",
        times,
        lla_to_interpolate[0],
        lla_to_interpolate[1],
        lla_to_interpolate[2],
        ecef_to_interpolate[0],
        ecef_to_interpolate[1],
        ecef_to_interpolate[2],
        heading*RAD2DEG
      ); // heading
    }
    line_number = line_number + 1;
  }
  fclose(fplla);
  fclose(fpecef);
  fclose(fp_position_interpolated);
  return 0;
}
