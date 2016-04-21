#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[]) {
	po::options_description desc("Options");
	boost::system::error_code ec;
	std::string wdir, server;

	desc.add_options()
		("help,h", "Display options")
		("wdir,w", po::value<std::string>(&wdir)->default_value("/tmp/pj"), "A directory to execute programs")
		("server,s", po::value<std::string>(&server)->default_value("ws://192.168.2.1:8080/"), "'popcon' server address");
	
	
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		
		if(vm.count("help")) {
			std::cout << desc << std::endl;

			return 0;
		}

		po::notify(vm);

		if(!fs::create_directories(wdir, ec) || ec) {
			std::cerr << "error: failed to create '" << wdir << "'" << std::endl;

			return 1;
		}


	}catch(std::exception& e) {
		std::cout << e.what() << std::endl;

		return 1;
	}
}
