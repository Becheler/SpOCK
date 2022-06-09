//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#define BOOST_TEST_MODULE gravity_field

#include <boost/test/unit_test.hpp>
#include <units/isq/si/force.h>  // 'N' (Newton) shadows a template parameter traditionally used as a size of the array
# include <iostream>
namespace utf = boost::unit_test;

#include <spock_lib/spock.h>

BOOST_AUTO_TEST_SUITE( gravity_field )

BOOST_AUTO_TEST_CASE( perfect_sphere )
{
  using namespace units::isq;
  using namespace si::mass_references;
  using namespace si::volume_references;
  using namespace units::isq::si::references;
  using namespace units::references;

  // declares a policy type
  using gravity_type = spock::gravity::model::perfect_sphere;

  // absolute altitude value measured from the mean sea level
  const perfect_sphere::altitude sea_level = 6371000;

  constexpr auto computed = gravity_type::field_at(sea_level);

  const auto expected = 9.8 * (N * m * m / (kg*kg));

  const auto error = computed -expected;

  double tolerance = 0.02; // percent
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
