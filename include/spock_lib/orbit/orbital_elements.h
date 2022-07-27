//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#ifndef __ORBITAL_ELEMENTS_H_INCLUDED__
#define __ORBITAL_ELEMENTS_H_INCLUDED__

#include <units/format.h>
#include <units/generic/angle.h>
#include <units/generic/dimensionless.h> // enables the use of units::Dimensionless
#include <units/generic/angle.h> // enables the use of units::radian
#include <units/isq/si/length.h> // enables the use of units::isq::si::metre
#include <units/quantity_io.h> // enable units io
#include <iostream>

namespace spock::orbit
{
  // Namespace alias
  namespace si = units::isq::si;
  ///
  /// @brief Parameters required to uniquely identify a specific orbit.
  ///
  /// @details These elements are useful in two-bodies systems using a Kepler orbit.
  ///          An elleptical orbit can be described with two plans (the orbital plan
  ///          and the reference plan) associated with six orbital parameters.
  /// @note    A real orbit and its elements change over time due to gravitational
  ///          perturbations by other objects and the effects of general relativity.
  /// @note    A Kepler orbit is an approximation of the orbit at a particular time.
  ///
  template<typename T=double>
  class orbital_elements
  {
  public:
    ///
    /// @brief Type used to represent values
    ///
    /// @details Template parameter so that precision on computations can be changed,
    ///          for example replacing default paramter with float or boost::multiprecision::cpp_float.
    ///
    using value_type = T;
    ///
    /// @brief Distance units used in orbital elements parameters
    ///
    using distance_type = si::length<si::metre>;
    ///
    /// @brief Dimensionless units of measure used in orbital elements parameters
    ///
    using dimensionless_type = units::dimensionless<units::one, value_type>;
    ///
    /// @brief Angular units used in orbital elements parameters
    ///
    using angle_type =units::angle<units::radian, value_type>;
  private:
    dimensionless_type _eccentricity;
    distance_type _semimajor_axis;
    angle_type _inclination;
    angle_type _longitude_on_the_ascending_node;
    angle_type _argument_of_periapsis;
    angle_type _true_anomaly;
  public:
    ///
    /// @brief Constructor
    ///
    orbital_elements( dimensionless_type eccentricity,
                      distance_type semimajor_axis,
                      angle_type inclination,
                      angle_type longitude_on_the_ascending_node,
                      angle_type argument_of_periapsis,
                      angle_type true_anomaly ) :
      _eccentricity(eccentricity),
      _semimajor_axis(semimajor_axis),
      _inclination(inclination),
      _longitude_on_the_ascending_node(longitude_on_the_ascending_node),
      _argument_of_periapsis(argument_of_periapsis),
      _true_anomaly(true_anomaly){}
      ///
      /// @brief Shape of the ellipse.
      ///
      /// @details Describes how much it is elongated compared to a circle.
      ///          Value of 0 is a circular orbit, values between 0 and 1 form an
      ///          elliptic orbit, 1 is a parabolic escape orbit (or capture orbit),
      ///          and greater than 1 is a hyperbola.
      ///
      /// @note Dimensionless.
      ///
      inline dimensionless_type eccentricity() const { return _eccentricity; }
      ///
      /// @brief The sum of the periapsis and apoapsis distances divided by two.
      ///
      /// @details For classic two-body orbits, the semimajor axis is the distance
      ///          between the centers of the bodies, not the distance of the
      ///          bodies from the center of mass.
      ///
      /// @note Length given in meters.
      ///
      inline distance_type semimajor_axis() const { return _semimajor_axis; }
      ///
      /// @brief Tilt of an object's orbit around a celestial body, in radian.
      ///
      /// @details  Expressed as the angle between a reference plane and the orbital
      ///           plane or axis of direction of the orbiting object.
      ///           - An inclination of 0° means the orbiting body has a prograde orbit in the planet's equatorial plane.
      ///           - An inclination greater than 0° and less than 90° also describes a prograde orbit.
      ///           - An inclination of 63.4° is often called a critical inclination, when describing artificial satellites orbiting the Earth, because they have zero apogee drift.[3]
      ///           - An inclination of exactly 90° is a polar orbit, in which the spacecraft passes over the poles of the planet.
      ///           - An inclination greater than 90° and less than 180° is a retrograde orbit.
      ///           - An inclination of exactly 180° is a retrograde equatorial orbit.
      ///
      /// @note Angle given in radian.
      ///
      inline angle_type inclination() const { return _inclination; }
      ///
      /// @brief Angle from the origin of longitude to the direction of the ascending node.
      ///
      /// @details  It is the angle from a specified reference direction, called the
      ///           origin of longitude, to the direction of the ascending node, as
      ///           measured in a specified reference plane. The ascending node is the
      ///           point where the orbit of the object passes through the plane of reference.
      inline angle_type longitude_on_the_ascending_node() const { return _longitude_on_the_ascending_node; }
      ///
      /// @brief Orientation of the ellipse in the orbital plane.
      ///
      /// @details The argument of periapsis is the angle from the body's ascending
      ///          node to its periapsis, measured in the direction of motion.
      ///          - An argument of periapsis of 0° means that the orbiting body will
      ///            be at its closest approach to the central body at the same moment
      ///            that it crosses the plane of reference from South to North.
      ///          - An argument of periapsis of 90° means that the orbiting body
      ///            will reach periapsis at its northmost distance from the plane of reference.
      ///
      inline angle_type argument_of_periapsis() const { return _argument_of_periapsis; }
      ///
      /// @brief Defines the position of the orbiting body along the ellipse at a specific time (the "epoch").
      ///
      /// @details True anomaly is an angular parameter that defines the position
      ///          of a body moving along a Keplerian orbit. It is the angle between
      ///          the direction of periapsis and the current position of the body,
      ///          as seen from the main focus of the ellipse (the point around which the object orbits).
      ///
      inline angle_type true_anomaly() const { return _true_anomaly; }

  };
} // namespace spock::orbit

///
/// @brief Ostream operator
///
template<typename T>
std::ostream& operator<<(std::ostream& os, const spock::orbit::orbital_elements<T>& oe)
{
  os << "eccentricity" << "\t" << oe.eccentricity() << "\n"
     << "semi-major axis" << "\t" << oe.semimajor_axis() << "\n"
     << "inclination" << "\t" << oe.inclination() << "\n"
     << "longitude on the ascending node" << "\t" << oe.longitude_on_the_ascending_node() << "\n"
     << "argument of periapsis" << "\t" << oe.argument_of_periapsis() << "\n"
     << "true anomaly" << "\t" << oe.true_anomaly()
     << "\n";
    return os;
}
#endif
