//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#define BOOST_TEST_MODULE gravity_field

#include <boost/test/unit_test.hpp>

#include <units/isq/si/force.h>  // enables the use of units::isq::si::newton
#include <units/isq/si/mass.h>   // enables the use of units::isq::si::kilogram
#include <units/isq/si/length.h> // enables the use of units::isq::si::metre

#include <iostream>

namespace utf = boost::unit_test;

#include <spock_lib/spock.h>

BOOST_AUTO_TEST_SUITE( gravity_field )

BOOST_AUTO_TEST_CASE( perfect_sphere )
{
  using namespace units::isq;

  using namespace si::mass_references; // enables the use of kg
  using namespace si::force_references; // enables the use of N
  using namespace si::length_references; // enables the use of m

  using earth = spock::physic_constant::earth;
  using gravity_type = spock::gravity::model::perfect_sphere<earth>;
  
  // absolute altitude value measured from the mean sea level
  const perfect_sphere::altitude sea_level(6371000 * m);

  constexpr auto computed = gravity_type::acceleration_at(sea_level);

  const auto expected = 9.81 * (N * m * m / (kg*kg));

  const auto error = computed -expected;

  double tolerance = 0.1; // percent
  BOOST_CHECK_SMALL(computed - expected, tolerance);
  std::cout << error << std:endl;
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
