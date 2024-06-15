#pragma once

#include <string>
#include <string_view>

namespace haproxy_load {
	inline constexpr std::string_view drain {"DRAIN"};
	inline constexpr std::string_view maint {"MAINT"};

	class Agent {
	public:
		double base {};
		double minimum {};
		double limit {};
		double crit {};

		[[nodiscard]] std::string getResponse(double load) const noexcept;

		double scale {100. / (minimum - base)};
	};
}
