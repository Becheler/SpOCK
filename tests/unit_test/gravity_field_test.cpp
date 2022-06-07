//
//  Copyright © 2022 Aaron Ridley, Arnaud Becheler, Aidan Kingwell. All rights reserved.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE gravity_field

#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include <spock_lib/spock.h>

BOOST_AUTO_TEST_SUITE( gravity_field )


BOOST_AUTO_TEST_CASE( simple_sphere )
{
  using gravity_type = spock::gravity::model::simple_sphere;
  auto computer = spock::force_calculator<gravity_type>();
  auto position = "here";
  auto gravity_vector = computer.compute_at(position);
}


BOOST_AUTO_TEST_CASE( j1_term_only )
{
  using gravity_type = spock::gravity::model::j1_term_only;
  auto computer = spock::force_calculator<gravity_type>();
  auto position = "here";
  auto gravity_vector = computer.compute_at(position);
}

BOOST_AUTO_TEST_SUITE_END()
