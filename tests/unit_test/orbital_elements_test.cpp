//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#define BOOST_TEST_MODULE gravity_field

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

#include <spock_lib/orbit.h>

BOOST_AUTO_TEST_SUITE( orbital_elements )

BOOST_AUTO_TEST_CASE( constructor )
{
  using namespace units::isq::si::references; // enables the use of m
  using orbital_elements = spock::orbit::orbital_elements<float>;

  orbital_elements::dimensionless_type eccentricity(35.0);
  auto semimajor_axis = orbital_elements::distance_type(500000.0 * m);
  auto inclination = orbital_elements::angle_type(0.0);
  auto longitude_on_the_ascending_node = orbital_elements::angle_type(0.0);
  auto argument_of_periapsis = orbital_elements::angle_type(0.0);
  auto true_anomaly = orbital_elements::angle_type(0.0);

  orbital_elements oe(eccentricity, semimajor_axis, inclination, longitude_on_the_ascending_node, argument_of_periapsis, true_anomaly);

}


BOOST_AUTO_TEST_SUITE_END()
