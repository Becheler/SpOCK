//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__
#define __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__

#include "../physic_constant.hpp"
#include <units/quantity_point_kind.h> // defining altitude

namespace upc = spock::physic_constant;

using namespace units::isq;
using namespace si::mass_references;
using namespace si::volume_references;

namespace spock::gravity::model
{
  ///
  /// @brief Gravity field computed under the assumtion of a single sphere planetoid.
  ///
  class perfect_sphere
  {
  public:
    ///
    /// @brief quantity_point_kind to mark “absolute” kinds of quantities like altitude (as opposed to height)
    ///
    using altitude = units::quantity_point_kind<vertical_point_kind, units::isq::si::metre>;
    ///
    /// @brief Return the acceleration due to gravity
    ///
    static constexpr auto field_at(altitude alt)
    {
      return upc::g * (upc::earth_mass ) / (altitude * altitude) ;
    }

  };

}

#endif
