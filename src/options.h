
// Copyright 2020 Arnaud Becheler    <arnaud.becheler@gmail.com>

/***********************************************************************                                                                         *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
************************************************************************/

#ifndef __SPOCK_OPTIONS_H_INCLUDED__
#define __SPOCK_OPTIONS_H_INCLUDED__

#include <boost/program_options.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

namespace bpo = boost::program_options;

namespace app
{
  ///
  /// @brief Returns a map with the program options
  ///
  auto handle_options(int argc, char* argv[])
  {
    ///
    /// @brief Declare a group of options that will be allowed only on command line
    ///
    bpo::options_description generic_options{"Command-line-only options"};
    generic_options.add_options()
    ("help,h", "help screen")
    ("verbose,v", "verbose mode")
    ("version", "software version")
    ("config", bpo::value<std::string>()->required(), "configuration file")
    ;

    ///
    /// @brief Declare a simpler way to call on command line
    ///
    bpo::positional_options_description positional_options;
    positional_options.add("config", 1);

    ///
    /// @brief File paths options for file dependencies
    ///
    /// @todo if we get user to provide all variables in one config file do we need these files? Obviously we keep SPICE and TLE but what about others?
    /// @note these options can be passed as a boost program option file - or a command line - or both ;)
    bpo::options_description dependencies_options("Dependencies Options");
    dependencies_options.add_options()
    	//
    	("filepath.SPICE", bpo::value<std::string>(), "Path to user SPICE installation")
    	//
    	("filepath.thrust", bpo::value<std::string>(), "path to user's thrust file")
    	//
    	("filepath.solar_power", bpo::value<std::string>(), "name of OpenGL solar power file")
    	//
    	("filepath.surface_geometry", bpo::value<std::string>(), "name of surface geometry file")
    	//
    	("filepath.TLE_constellation", bpo::value<std::string>(), "name of TLE constellation GPS file")
    	//
    	("filepath.Kalman", bpo::value<std::string>(), "name of Kalman filtering file");

    ///
    /// @brief Density MOD options
    ///
    bpo::options_description density_options("Density MOD Options");
    density_options.add_options()
    	//
    	("density.mod", bpo::value<double>(), "desired density mod")
    	//
    	("density.mod.amplitude", bpo::value<double>(), "density mod amplitude")
    	//
    	("density.mod.phase", bpo::value<double>(), "density mod phase");

    ///
    /// @brief Time options
    ///
    bpo::options_description time_options("Time Options");
    time_options.add_options()
    	//
    	("time.initial_epoch", bpo::value<std::string>(), "initial epoch in UTC format DD-MM-YYYY HH:MM:SS")
    	//
    	("time.final_epoch", bpo::value<std::string>(), "final epoch in UTC format DD-MM-YYYY HH:MM:SS")
    	//
    	("time.step", bpo::value<float>(), "time step in UTC format DD-MM-YYYY HH:MM:SS");

    ///
    /// @brief Spacecraft options
    ///
    /// \todo unfinished
    ///
    /// \todo why isn't right ascention included in original code as an option?
    ///
    /// \todo ask if we want pos and veloc provided in ECEF or ECI by user (it shouldn't matter which we just need a default)
    ///
    bpo::options_description spacecraft_options("Spacecraft Options");
    spacecraft_options.add_options()
    	//
    	("spacecraft.number", bpo::value<int>()->default_value(1), "number of spacecraft being modelled")
    	//
    	("spacecraft.name", bpo::value<std::vector<std::string> >(), "names of each spacecraft in vector")
    	//
    	("spacecraft.GPS.number", bpo::value<int>()->default_value(0), "number of spacecraft using GPS")
    	//
    	("spacecraft.surfaces.number", bpo::value<int>(), "number of surfaces on each spacecraft (must be same # for all SC)")
    	//
    	("spacecraft.solar_cell.efficiency", bpo::value<std::vector<double> >(), "efficiency of solar cells on spacecraft")
    	//
    	("spacecraft.inclination", bpo::value<std::vector<double> >(), "inclinations of spacecraft in vector of len=num spacecraft")
    	//
    	("spacecraft.eccentricity", bpo::value<std::vector<double> >(), "eccentricity of all spacecraft in vector of len=num spacecraft")
    	//
    	("spacecraft.apogee_altitude", bpo::value<std::vector<double> >(), "apogee altitude of all spacecraft in vector")
    	//
    	("spacecraft.true_anomaly", bpo::value<std::vector<double> >(), "true anomalies of all spacecraft in vector")
    	//
    	("spacecraft.arg_of_periapsis", bpo::value<std::vector<double> >(), "argument of periapsis of all spacecraft in vector")
    	//
    	("spacecraft.x_pos", bpo::value<std::vector<double> >(), "x positon of all spacecraft in vector")
    	//
    	("spacecraft.y_pos", bpo::value<std::vector<double> >(), "y position of all spacecraft in vector")
    	//
    	("spacecraft.z_pos", bpo::value<std::vector<double> >(), "z position of all spacecraft in vector")
    	//
    	("spacecraft.x_veloc", bpo::value<std::vector<double> >(), "x direction velocities of all spacecraft in vector")
    	//
    	("spacecraft.y_veloc", bpo::value<std::vector<double> >(), "y direction velocities of all spacecraft in vector")
    	//
    	("spacecraft.z_veloc", bpo::value<std::vector<double> >(), "z direction velocities of all spacecraft in vector");

    ///
    /// @brief Forces options
    ///
    bpo::options_description forces_options("Forces Options");
    forces_options.add_options()
    	//
    	("include_Earth_pressure", bpo::value<std::string>()->default_value("no"), "yes or no")
    	//
    	("include_solar_pressure", bpo::value<std::string>()->default_value("no"), "yes or no")
    	//
    	("include_drag", bpo::value<std::string>()->default_value("no"), "yes or no")
    	//
    	("include_Sun_gravity", bpo::value<std::string>()->default_value("no"), "yes or no")
    	//
    	("include_Moon_gravity", bpo::value<std::string>()->default_value("no"), "yes or no");

    ///
    /// @brief Attitude options
    ///
    /// \todo should the description list what the components are?
    ///
    bpo::options_description attitude_options("Attitude Options");
    attitude_options.add_options()
    	//
    	("attitude_profile", bpo::value<std::string>()->default_value("nadir"), "desired attitude profile")
    	//
    	("use angular_velocity_vector", bpo::value<std::string>(), "yes or no")
    	//
    	("angular_velocity", bpo::value<std::vector<double> >(), "array of angular velocity components");

    ///
    /// @brief KALMAN options
    ///
    bpo::options_description kalman_options("Kalman Options");
    kalman_options.add_options()
    	//
    	("use_kalman", bpo::value<std::string>()->default_value("no"), "yes or no");

    ///
    /// @brief Orbit options
    ///
    /// \todo Section is incomplete
    ///
    bpo::options_description orbit_options("Orbit Options");
    orbit_options.add_options()
    	// TODO this is dumb, this should be in the spacecraft section? I think???
    	("orbit_type", bpo::value<std::string>()->default_value("state_ecef"), "type of orbit");

    ///
    /// @brief Ground stations options
    ///
    bpo::options_description ground_stations_options("Ground Stations Options");
    ground_stations_options.add_options()
    	//
    	("num_ground_stations", bpo::value<int>(), "number of ground stations")
    	//
    	("name", bpo::value<std::vector<std::string> >(), "names of all ground stations in vector")
    	//
    	("lat", bpo::value<std::vector<double> >(), "latitude of all ground stations in vector")
    	//
    	("long", bpo::value<std::vector<double> >(), "longitude of all ground stations in vector")
    	//
    	("alt", bpo::value<std::vector<double> >(), "altitude of all ground stations in vector")
    	//
    	("min_elev_angle", bpo::value<std::vector<double> >(), "minimum elevation angle of all ground stations");

    bpo::options_description command_line_options;
    command_line_options.add(generic_options);

    bpo::options_description file_options{"General options (command line values will overwrite congif file values)"};

    file_options.add(dependencies_options).add(density_options).add(time_options);
    file_options.add(spacecraft_options).add(forces_options).add(attitude_options);
    file_options.add(kalman_options).add(orbit_options);

    bpo::variables_map vm;

    try
    {

      bpo::store(
        bpo::command_line_parser(argc, argv).
        options(command_line_options).
        positional(positional_options).
        run(), vm); // can throw

      // --help option
      if (vm.count("help"))
      {
        std::cout << "--------------------------------------------------------------------------------------" << std::endl;
        std::cout << "| This is SpOCK propagator.                                                           |" << std::endl;
        std::cout << "|   - Purpose: propagates satellites orbits.                                          |" << std::endl;
        std::cout << "|   - Author: Aaron Ridley, Aidan Kingwell, Arnaud Becheler 2022.                     |" << std::endl;
        std::cout << "|   - Usage: " << argv[0] << " [options] <config> ...                                       |" << std::endl;
        std::cout << "--------------------------------------------------------------------------------------|" << std::endl;
        std::cout << "\n" << generic_options << std::endl;
        std::cout << "\n" << file_options << std::endl;
        return vm;
        // SUCCESS
      }
      // --version option
      if (vm.count("version"))
      {
        std::cout << "SpOCK version 0.1" << std::endl;
        return vm;
        // SUCCESS
      }
      bpo::notify(vm); // throws on error, so do after help in case there are any problems
    }
    catch(boost::program_options::required_option& e)
    {
      throw;
    }
    catch(boost::program_options::error& e)
    {
      throw;
    }
    if (vm.count("config"))
    {
      std::ifstream ifs{vm["config"].as<std::string>().c_str()};
      if (ifs){
        store(parse_config_file(ifs, file_options), vm);
      }
    }
    notify(vm);
    return vm;
  } // end of handle_options
}

#endif
