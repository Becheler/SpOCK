//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#define BOOST_TEST_MODULE gravity_field

#include <boost/test/unit_test.hpp>

#include <units/isq/si/force.h>  // enables the use of units::isq::si::newton
#include <units/isq/si/mass.h>   // enables the use of units::isq::si::kilogram
#include <units/isq/si/length.h> // enables the use of units::isq::si::metre
#include <units/quantity_io.h>   // enable the use of std::cout << ... <<
#include <iostream>

namespace utf = boost::unit_test;

#include <spock_lib/spock.h>

BOOST_AUTO_TEST_SUITE( gravity_field )

BOOST_AUTO_TEST_CASE( perfect_sphere )
{
  using namespace units::isq;

  using namespace si::time_references; // enables the use of s
  using namespace si::length_references; // enables the use of m

  using spock::physic_constants::earth;
  using gravity_type = spock::gravity::model::perfect_sphere<earth>;

  constexpr gravity_type::altitude sea_level(6371000 * m);

  constexpr Acceleration auto computed = gravity_type::acceleration_at(sea_level);

  constexpr Acceleration auto expected = 9.81 * (m / (s*s));

  constexpr Acceleration auto error = computed - expected;

  constexpr auto tolerance = units::dimensionless<units::one>(1); // percent, dimensionless

  BOOST_CHECK_SMALL(error.number(), tolerance.number());

  std::cout << error << std::endl;
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
