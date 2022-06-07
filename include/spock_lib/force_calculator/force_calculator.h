//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __FORCE_CALCULATOR_CLASS_H_INCLUDED__
#define __FORCE_CALCULATOR_CLASS_H_INCLUDED__

#include <iostream>

namespace spock
{
  ///
  /// @brief Force calculator class
  ///
  template<class GravityType>
  class force_calculator
  {
  public:
    template<class CoordinateType>
    auto compute_at(CoordinateType const& x)
    {
      std::cout << "compute gravity "<< std::endl;
      return "gravity_field is not yet implemented";
    }
  };
}

#endif
