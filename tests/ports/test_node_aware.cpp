#include <boost/test/unit_test.hpp>

#include <ports/ports.hpp>
#include "../../src/ports/region_aware_port.hpp"

using namespace fc;

BOOST_AUTO_TEST_CASE(test_node_aware_connection)
{
	typedef region_aware_port<event_in_port<int>> test_in_port;
	typedef region_aware_port<event_out_port<int>> test_out_port;
	auto region = std::make_shared<parallel_region>();

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	test_in_port test_in(region, write_param);
	test_out_port test_out(region);

	auto connection = test_out >> test_in;
	static_assert(is_instantiation_of<node_aware_connection, decltype(connection)>::value,
			"connection should be node_aware_connection");

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_value, 1);

	auto tmp = test_out >> [](int i ){ return ++i;};
	static_assert(is_instantiation_of<node_aware_connection, decltype(tmp)>::value,
			"tmp should be node_aware_connection");
	tmp >> test_in;

	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_value, 2);
}
