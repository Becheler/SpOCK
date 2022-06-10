//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__
#define __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__

#include "../physic_constant.hpp"

#include <units/isq/si/length.h>       // units::isq::si::metre
#include <units/quantity_point_kind.h> // defining altitude

#include <ostream>
namespace upc = spock::physic_constant;

namespace spock::gravity::model
{
  ///
  /// @brief Gravity field computed under the assumtion of a single sphere planetoid.
  ///
  class perfect_sphere
  {
    // We need a kind to represent the more specific usage of a length quantity
    struct vertical_kind : units::kind<vertical_kind, units::isq::si::dim_length> {};
    // A quantity point is an absolute quantity with respect to an origin
    struct vertical_point_kind : units::point_kind<vertical_point_kind, vertical_kind> {};

  public:
    ///
    /// @brief quantity_point_kind to mark “absolute” kinds of quantities like altitude (as opposed to height)
    ///
    using altitude = units::quantity_point_kind<vertical_point_kind, units::isq::si::metre>;
    ///
    /// @brief Return the acceleration due to gravity
    ///
    static constexpr units::isq::Acceleration auto acceleration_at(const altitude& alt)
    {
      return upc::g * (upc::earth_mass ) / (alt.relative() * alt.relative()) ;
    }

  };

  ///
  /// @Brief text output
  ///
  template<class CharT, class Traits>
  std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const spock::gravity::model::perfect_sphere::altitude& a)
  {
    return os << a.relative().common() << " AMSL";
  }

}

#endif
