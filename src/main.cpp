//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#include <boost/program_options.hpp>
#include "options.h"
#include "utils.h"

namespace bpo = boost::program_options;

// This class will later go in its own header ...
struct Propagator
{
  // Program options
  bpo::variables_map _vm;

  //! Verbosity level
  bool _verbose;

  //! Constructor
  Propagator(bpo::variables_map const& vm, bool verbose):
  _vm(vm),
  _verbose(verbose)
  {}

  void run()
  {
    if(_verbose)
    {
      std::cout << " ... orbit propagating ... " << std::endl;
    }
  }
}; // end class Propagator

namespace
{
  const size_t ERROR_IN_COMMAND_LINE = 1;
  const size_t SUCCESS = 0;
  const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

int main(int argc, char* argv[])
{
  // Initialize the MPI execution environment
  MPI_Init(&argc, &argv);
  // Determines the size of the group associated with a communicator
  int nb_process;
  MPI_Comm_size(MPI_COMM_WORLD, &nb_process);
  // Determines the rank of the calling process in the communicator
  int current_process;
  MPI_Comm_rank(MPI_COMM_WORLD, &current_process);

  // Load options
  bpo::variables_map vm;
  bool verbose = false;
  try{
    vm = handle_options(argc, argv);
    // --help option
    if (vm.count("help") || vm.count("version") )
    {
      return SUCCESS;
    }
  }
  catch(boost::program_options::required_option& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    return ERROR_IN_COMMAND_LINE;
  }
  catch(boost::program_options::error& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    return ERROR_IN_COMMAND_LINE;
  }
  catch ( const std::exception& e )
  {
    std::cerr << e.what() << std::endl;
    return ERROR_IN_COMMAND_LINE;
  }
  // the following can run without fear because everything is required to be present

  // --verbose option
  if (vm.count("verbose"))
  {
    verbose = true;
    spock::utils::PrintVariableMap(vm);
  }

  if(verbose){ std::cout << "Initialization" << std::endl; }

  spock::spice::set_up(vm.earth_orientation_parameters);
  spock::spice::set_up(vm.planet_ephemerides_files);
  spock::spice::set_up(vm.earth_binary_pck);

  Propagator propagator(vm, verbose);

  if(verbose){ std::cout << "Running ..." << std::endl; }

  try
  {
    propagator.run();
  }
  catch(const std::domain_error &)
  {
    return 1;
  }

  return 0;
}
