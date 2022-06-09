#ifndef PHYSIC_CONSTANTS_HPP
#define PHYSIC_CONSTANTS_HPP

#include <units/isq/si/constants.h> // standard_gravity

#include <cmath> // std::pow
///
/// @brief Components for orbital propagation
///
namespace spock
{
  ///
  /// @brief Compile-time physic constants
  ///
  namespace physic_constant
  {
    ///
    /// @brief Universal Gravitational Constant
    ///
    inline constexpr  units::isq::Acceleration auto g = units::isq::si::si2019::standard_gravity<>;
    ///
    /// @brief Earth Mass
    ///
    inline constexpr auto earth_mass = 5.972 * std::pow(10,24) * kg;
  }
}

#endif
