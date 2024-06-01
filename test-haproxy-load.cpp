#define BOOST_TEST_MODULE haproxy_load
#include "haproxy-load.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <format>

using responses = std::tuple<double, // load
		haproxy_load::Agent,
		std::string_view>; // expected

namespace haproxy_load {
	std::ostream &
	operator<<(std::ostream & out, const Agent & agent)
	{
		out << std::format("[ base: {}, limit: {}, crit: {} ]", agent.base, agent.limit, agent.crit);
		return out;
	}
}

static constexpr haproxy_load::Agent single {.25, 1.5, 1.9}, multi {0.2, 6.0, 9.0}, large {0.1, 24.0, 47.0};

BOOST_DATA_TEST_CASE(response,
		boost::unit_test::data::make<responses>({
				// Singles
				{0.0, single, "up ready 100%"},
				{0.25, single, "up ready 100%"},
				{0.35, single, "up ready 92%"},
				{1.4, single, "up ready 8%"},
				{1.5, single, "up ready 0%"},
				{1.6, single, "up drain"},
				{1.89, single, "up drain"},
				{1.91, single, "stopped maint"},
				{1.89, single, "up drain"},
				{1.91, single, "stopped maint"},
				// Multis
				{0.0, multi, "up ready 100%"},
				{0.2, multi, "up ready 100%"},
				{0.35, multi, "up ready 97%"},
				{1.4, multi, "up ready 79%"},
				{5.3, multi, "up ready 12%"},
				{6.1, multi, "up drain"},
				{9.1, multi, "stopped maint"},
				// Larges
				{0.0, large, "up ready 100%"},
				{0.1, large, "up ready 100%"},
				{0.35, large, "up ready 99%"},
				{1.4, large, "up ready 95%"},
				{5.3, large, "up ready 78%"},
				{23.3, large, "up ready 3%"},
				{24.1, large, "up drain"},
				{47.1, large, "stopped maint"},
		}),
		load, agent, expected)
{
	const auto resp = agent.getResponse(load);
	BOOST_REQUIRE_GE(resp.length(), 2);
	BOOST_REQUIRE_EQUAL(resp.back(), '\n');
	BOOST_CHECK_EQUAL(std::string_view {resp}.substr(0, resp.length() - 1), expected);
}
