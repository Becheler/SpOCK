//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__
#define __PERFECT_SPHERE_GRAVITY_CLASS_H_INCLUDED__

#include <units/isq/si/length.h>       // units::isq::si::metre
#include <units/quantity_point_kind.h> // to define altitude
#include <units/isq/dimensions/acceleration.h> // Acceleration concept
#include <ostream>
#include <concepts>

#include "spock_lib/physic_constants.hpp"    // access to radius_q quantity

namespace spock::gravity::model
{
  //
  // ///
  // /// @brief Requirements for a planetoid struct to be modeled as a perfect_sphere
  // ///
  // /// @see e.g. <https://www.sandordargo.com/blog/2021/02/17/cpp-concepts-4-ways-to-use-them>
  // ///
  // template <typename T>
  // concept SphericBody = requires(T)
  // {
  //   // struct T must have a Acceleration static member gravity
  //   { T::gravity } -> units::isq::Acceleration;
  //   // struct T must have a static member radius of type radius_q
  //   { T::radius }  -> std::convertible_to<spock::physic_constants::radius_q&>;
  // };

  ///
  /// @brief Gravity field computed under the assumtion of a single sphere planetoid.
  ///
  /// @note The template class must fulfill the requirements of the SphericBody concept.
  template<class T>
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
    using planet_type = T;

    /// @brief quantity_point_kind to mark “absolute” kinds of quantities like altitude (as opposed to height)
    ///
    using altitude = units::quantity_point_kind<vertical_point_kind, units::isq::si::metre>;
    ///
    /// @brief Acceleration due to gravity at altitude h above the surface of a sphere.
    ///
    static constexpr units::isq::Acceleration auto acceleration_at(const altitude& alt)
    {
      using units::isq::si::metre;
      using units::isq::si::dim_length;
      constexpr auto g_0 = planet_type::gravity;
      constexpr auto r = planet_type::radius.common();
      auto h = alt.relative().common();
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
