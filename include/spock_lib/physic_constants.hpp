#ifndef PHYSIC_CONSTANTS_HPP
#define PHYSIC_CONSTANTS_HPP

#include <units/isq/si/constants.h> // standard_gravity
#include <units/isq/si/mass.h>      // enables the use of units::isq::si::kg
#include <units/isq/si/length.h>    // enables the use of units::isq::si::metre
#include <units/isq/si/time.h>    // enables the use of units::isq::si::second
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
      using units::isq::si::length_references::m;
      using units::isq::si::time_references::s;
      using units::isq::si::kilometre;

      ///
      /// @brief We need a kind to represent the more specific usage of a length quantity
      ///
      /// @note Quantities of kind radius are now clearly distinct from more generic usages of length quantities.
      ///
      struct radius_q : units::kind<radius_q, units::isq::si::dim_length> {};
    }

    ///
    /// @brief Physic constants for planet Earth
    ///
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
      static inline constexpr Acceleration auto gravity = units::isq::si::si2019::standard_gravity<>;
      ///
      /// @brief Planet Mass
      ///
      static inline constexpr auto mass = 5.9722 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean Radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(6371 * km);
    };

    ///
    /// @brief Physic constants for planet Earth's moon
    ///
    struct moon
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 1.6 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 0.073 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(1737.5 * km);
    };

    ///
    /// @brief Physic constants for planet Mercury
    ///
    struct mercury
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 3.7 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 0.330 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(2439.5 * km);
    };

    ///
    /// @brief Physic constants for planet Venus
    ///
    struct venus
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 8.9 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 4.87 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(6052 * km);
    };

    ///
    /// @brief Physic constants for planet Mars
    ///
    struct mars
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 3.7 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 0.642 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(3396 * km);
    };

    ///
    /// @brief Physic constants for planet Jupiter
    ///
    struct jupiter
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 23.1 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 1898 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(71492 * km);
    };

    ///
    /// @brief Physic constants for planet Saturn
    ///
    struct saturn
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 9.0 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 568 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(60268 * km);
    };

    ///
    /// @brief Physic constants for planet Uranus
    ///
    struct uranus
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 8.7 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 86.8 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(25559 * km);
    };

    ///
    /// @brief Physic constants for planet Neptune
    ///
    struct neptune
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 11.0 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 102 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(24764 * km);
    };

    ///
    /// @brief Physic constants for Pluto
    ///
    struct pluto
    {
      ///
      /// @brief Standard Acceleration of Free Fall
      ///
      /// @details The standard acceleration due to gravity (or standard acceleration
      ///          of free fall), sometimes abbreviated as standard gravity,
      ///          usually denoted by \f$ɡ_0$\f or \f$ɡ_n$\f, is the nominal
      ///          gravitational acceleration of an object in a vacuum near the
      ///          surface of the planet.
      ///
      static inline constexpr Acceleration auto gravity = 0.7 * (m / (s*s));
      ///
      /// @brief Mass
      ///
      static inline constexpr auto mass = 0.0130 * std::pow(10,24) * kg;
      ///
      /// @brief Planet mean radius
      ///
      static inline constexpr auto radius = units::quantity_kind<radius_q, kilometre, double>(1188 * km);
    };
  }
}

#endif
