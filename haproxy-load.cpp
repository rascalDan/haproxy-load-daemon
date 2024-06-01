#include "haproxy-load.h"
#include <format>

namespace haproxy_load {
	static constexpr std::string idle_response {"up ready 100%\n"};
	static constexpr std::string busy_response {"up drain\n"};
	static constexpr std::string overloaded_response {"stopped maint\n"};

	std::string
	Agent::getResponse(double load) const noexcept
	{
		if (load < base) {
			return idle_response;
		}
		else if (load > crit) {
			return overloaded_response;
		}
		else if (load > limit) {
			return busy_response;
		}

		const auto scaled = ((limit - load) * scale);
		return std::format("up ready {:.0f}%\n", scaled);
	}
}
