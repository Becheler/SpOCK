//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__
#define __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__

#include <units/isq/si/length.h>       // units::isq::si::metre
#include <units/quantity_point_kind.h> // to define altitude

#include <ostream>

namespace spock::gravity::model
{
  ///
  /// @brief Gravity field computed under the assumtion of a single sphere planetoid.
  ///
  template<class P>
  class perfect_sphere
  {
    // We need a new kind of unit to represent the more specific usage of a length quantity
    struct vertical_kind : units::kind<vertical_kind, units::isq::si::dim_length> {};
    // A quantity point is an absolute quantity with respect to an origin, with limited arithmetic
    struct vertical_point_kind : units::point_kind<vertical_point_kind, vertical_kind> {};

  public:
    ///
    /// \typedef Planet type being modeled, eg spock::physic_constants::earth
    ///
    using planet_type = P;

    /// @brief quantity_point_kind to mark “absolute” kinds of quantities like altitude (as opposed to height)
    ///
    using altitude = units::quantity_point_kind<vertical_point_kind, units::isq::si::metre>;
    ///
    /// @brief Acceleration due to gravity at altitude h above the surface of a sphere.
    ///
    static constexpr units::isq::Acceleration auto acceleration_at(const altitude& alt)
    {
      using units::isq::si::metre;
      constexpr auto g_0 = &planet_type::g_0;
      constexpr auto radius = &planet_type::r;
      constexpr units::isq::si::length<metre, double> h = units::quantity_point_kind_cast<metre>(alt);
      constexpr units::isq::si::length<metre, double> r = units::quantity_kind_cast<metre>(r);
      return g_0 * (r/(r + h)) * (r/(r + h));
    }
  }; // end class perfect_sphere
} // end namespace spock::gravity::model

///
/// @Brief text output
///
template<class CharT, class Traits, class Planet>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os,
           const typename spock::gravity::model::perfect_sphere<Planet>::altitude& a)
{
  return os << a.relative().common() << " AMSL";
}

#endif
