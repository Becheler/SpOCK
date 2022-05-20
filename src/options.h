
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

namespace bpo = boost::program_options;

/// @brief Returns a map with the program options
auto handle_options(int argc, char* argv[])
{
  // Declare a group of options that will be allowed only on command line
  bpo::options_description generic_options{"Command-line-only options"};
  generic_options.add_options()
  ("help,h", "help screen")
  ("verbose,v", "verbose mode")
  ("version", "software version")
  ("config", bpo::value<std::string>()->required(), "configuration file")
  ;

  // Declare a simpler way to call on command line
  bpo::positional_options_description positional_options;
  positional_options.add("config", 1);

  // Allowed both on command line and in config file
  bpo::options_description model_options{"Propagation model parameters"};
  model_options.add_options()
  ("lon_0", bpo::value<double>(), "Origin point longitude")
  ("lat_0", bpo::value<double>(), "Origin point latitude")
  ("alt_0", bpo::value<double>(), "Number of gene copies at introduction point")
  ("duration", bpo::value<int>(), "Number of generations to simulate")
  ;

  bpo::options_description command_line_options;
  command_line_options.add(generic_options).add(model_options);

  bpo::options_description file_options{"General options (command line values will overwrite congif file values)"};
  file_options.add(model_options);

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
      std::cout << "|   - Usage: " << argv[0] << " [options] <config> ...                                 |" << std::endl;
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

#endif
