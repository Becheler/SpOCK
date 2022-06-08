#ifndef PHYSIC_CONSTANTS_HPP
#define PHYSIC_CONSTANTS_HPP

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
    inline constexpr float G = 6.674*std::pow(10,-11); // m3 / kg.s2
    ///
    /// @brief Earth Mass
    ///
    inline constexpr float MASS_OF_EARTH = 5.972 * std::pow(10,24); // kg
  }
}

#endif
