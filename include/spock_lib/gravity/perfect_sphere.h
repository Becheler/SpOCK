//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__
#define __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__

#include "../physic_constant.hpp"

namespace upc = spock::physic_constant;

namespace spock::gravity::model
{
  ///
  /// @brief Gravity field computed under the assumtion of a single sphere planetoid.
  ///
  class perfect_sphere
  {
  public:
    ///
    /// @brief Return the acceleration due to gravity
    ///
    static auto get_field(double altitude)
    {
      return upc::G * (upc::MASS_OF_EARTH ) / (altitude * altitude) ;
    }

  };

}

#endif
