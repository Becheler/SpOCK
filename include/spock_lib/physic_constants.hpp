#ifndef PHYSIC_CONSTANTS_HPP
#define PHYSIC_CONSTANTS_HPP

#include <units/isq/si/constants.h> // standard_gravity
#include <units/isq/si/mass.h>      // enables the use of units::isq::si::kg
#include <units/isq/si/length.h>    // enables the use of units::isq::si::metre
#include <units/quantity_kind.h>    // to define new quantity kinds like radius

#include <cmath> // std::pow
///
/// @brief Components for orbital propagation
///
namespace spock
{
  ///
  /// @brief Compile-time physic constants
  ///
  namespace physic_constants
  {

    // unnamed namespace to prevent alias from being visible from other files
    namespace
    {
      using units::isq::Acceleration;
      using units::isq::si::mass_references::kg;
      using units::isq::si::length_references::km;
      ///
      /// @brief We need a kind to represent the more specific usage of a length quantity
      ///
      /// @note Quantities of kind radius are now clearly distinct from more generic usages of length quantities.
      ///
      struct radius : units::kind<radius, units::isq::si::dim_length> {};
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
      static inline constexpr physic_constants::Acceleration auto g_0 = units::isq::si::si2019::standard_gravity<>;
      ///
      /// @brief Earth Mass
      ///
      static inline constexpr auto mass = 5.9722 * std::pow(10,24) * kg;
      ///
      /// @brief Earth Radius
      ///
      static inline constexpr quantity_kind<radius, units::isq::si::kilometre, double> radius(6371 * km);
    }
  }
}

#endif
