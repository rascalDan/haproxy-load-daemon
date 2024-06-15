#include "haproxy-load.h"
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

namespace haproxy_load {
	void
	send_weight(boost::asio::ip::tcp::socket socket, const Agent & agent)
	{
		std::array<double, 3> load {};
		if (getloadavg(load.data(), load.size()) < 1) {
			boost::asio::write(socket, boost::asio::buffer(drain));
			return;
		}
		boost::asio::write(socket, boost::asio::buffer(agent.getResponse(load.front())));
	}
}

int
main(int argc, char ** argv)
{
	double base {}, minimum {}, limit {}, crit {};
	boost::asio::ip::port_type port {};

	try {
		po::options_description opts("HA Proxy Load Agent");
		auto hwcm = [hwc = static_cast<double>(std::thread::hardware_concurrency())](double multiplier) {
			return hwc * multiplier;
		};
		opts.add_options()(
				// Show help
				"help,h", "Show help")
				// Port to listen on
				("port,p", po::value(&port)->required(), "Listen port")
				// Base acceptable load
				("base,b", po::value(&base)->default_value(hwcm(0.5)), "Base acceptable load for full service")
				// Load minimum limit
				("minimum,m", po::value(&minimum)->default_value(hwcm(1.5)),
						"Load limit for minimum new connections without disabling the backend")
				// Load limit
				("limit,l", po::value(&limit)->default_value(hwcm(2.5)),
						"Load limit for no further connections (Drain)")
				// Critical load limit
				("crit,c", po::value(&crit)->default_value(hwcm(3.5)),
						"Critical load for disconnecting existing clients (Maint)");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);

		if (vm.contains("help")) {
			std::cout << opts << '\n';
			return 0;
		}
		po::notify(vm);
	}
	catch (const std::exception & ex) {
		std::cerr << ex.what() << '\n';
		return 1;
	}

	haproxy_load::Agent agent {base, minimum, limit, crit};

	boost::asio::io_service io_service;

	boost::asio::ip::tcp::acceptor listener(
			io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

	while (true) {
		haproxy_load::send_weight(listener.accept(), agent);
	}

	return 0;
}
