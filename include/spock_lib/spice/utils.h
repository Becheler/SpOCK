//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//
#ifndef __SPICE_CPP_UTILS_H_INCLUDED__
#define __SPICE_CPP_UTILS_H_INCLUDED__

#include <SpiceUsr.h>

///
/// @brief SPICE wrapper
///
namespace spock::spice
{
  ///
  /// @brief loads various SPICE kernels for use by the application.
  ///
  void load_kernel(const std::string & file, bool verbose)
  {
    // call to spice
    furnsh_c(file.c_str());
    if(verbose)
    {
      std::cout << "SPICE kernel " << file << " loaded." << std::endl;
    }
  }
}
#endif
