//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE gravity_field

#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include <spock_lib/spock.h>

BOOST_AUTO_TEST_SUITE( gravity_field )

class CoordinateType {};
class SatelliteType {};

BOOST_AUTO_TEST_CASE( perfect_sphere )
{
  // declares a policy type
  using gravity_type = spock::gravity::model::perfect_sphere;
  double surface_altitude = 6371000; // km
  auto computed_value = gravity_type::get_field(surface_altitude);
  double expected_value = 9.8;
  double epsilon = 0.02;
  BOOST_CHECK_SMALL(computed_value - expected_value, epsilon);
}

//
// BOOST_AUTO_TEST_CASE( j1_term_only )
// {
//   using gravity_type = spock::gravity::model::j1_term_only;
//   auto computer = spock::force_calculator<gravity_type>();
//   auto position = "here";
//   auto gravity_vector = computer.compute_at(position);
// }

BOOST_AUTO_TEST_SUITE_END()
