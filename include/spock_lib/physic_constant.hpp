#ifndef PHYSIC_CONSTANTS_HPP
#define PHYSIC_CONSTANTS_HPP

#include <units/isq/si/constants.h> // standard_gravity
#include <units/isq/si/mass.h>      // kg

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

    // unnamed namespace to prevent alias from being visible from other files
    namespace
    {
      using units::isq::Acceleration;
      using units::isq::si::mass_references::kg;
      using units::isq::si::length_references::km;
      ///
      /// @brief quantity_point_kind to mark “absolute” kinds of quantities like altitude (as opposed to height)
      ///
      using radius = quantity_kind<radius, units::isq::si::metre, double>;
      // Quantities of kind radius are now clearly distinct from more generic usages of length quantities.
      struct radius : kind<radius, si::dim_length> {};
    }

    struct earth
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the Earth. It is defined by standard as 9.80665 m/s2.
      ///
      inline constexpr  Acceleration auto g_0 = units::isq::si::si2019::standard_gravity<>;
      ///
      /// @brief Earth Mass
      ///
      inline constexpr auto mass = 5.9722 * std::pow(10,24) * kg;
      ///
      /// @brief Earth Radius
      ///
      inline constexpr auto radius = 6371 * km;
    }
  }
}

#endif
