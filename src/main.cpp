#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <json.hpp>

#include "docker.hpp"
#include "cgroup.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[]) {
	std::cout << sys::exec({"/bin/ls", "-la", "/"}) << std::endl;

	return 0;

	po::options_description desc("Options");
	boost::system::error_code ec;
	std::string wdir, server, setting;
	fs::path set;

	int parallelism;


	desc.add_options()
		("help,h", "Display options")
		("wdir,w", po::value<std::string>(&wdir)->default_value("/tmp/pj"), "A directory to execute programs")
		("server,s", po::value<std::string>(&server)->default_value("ws://192.168.2.1:8080/"), "'popcon' server address")
		("setting", po::value<std::string>(&setting)->default_value("./pj.json", "the path of the setting file"));
	
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		
		if(vm.count("help")) {
			std::cout << desc << std::endl;

			return 0;
		}

		po::notify(vm);

		if((!fs::exists(wdir) || !fs::is_directory(wdir)) &&  !fs::create_directories(wdir, ec) || ec) {
			std::cerr << "error: failed to create '" << wdir << "'" << std::endl;

			return 1;
		}

		set = fs::path{setting};

		if(!fs::exists(set) || !fs::is_regular_file(set)) {
			fs::ofstream ofs(set);

			if(ofs) {
				std::cerr << "created a setting file at '" << set.string() << "'" << std::endl;
				
				ofs << 
R"({
	"parallelism": 2
})";

				return 1;
			} else {

				std::cerr << "unable to create a setting file" << std::endl;

				return 1;
			}
		}

		fs::ifstream ifs(set);
		
		if(!ifs) {
			std::cerr << "unable to load the setting file" << std::endl;

			return 1;
		}

		auto jsonStr = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		
		using json = nlohmann::json;

		try {
			auto j = json::parse(jsonStr);
			
			parallelism = j["parallelism"].get<int>();	

		}catch(std::exception& e) {
			std::cerr << "Failed to parse the json(" << e.what() << ")" << std::endl;

			return 1;
		}
	}catch(std::exception& e) {
		std::cout << e.what() << std::endl;

		return 1;
	}

	docker::DockerContainerSetting dcs;

	dcs .setImage("ubuntu:16.04")
		.setCPU(80)
		.addMount({"/tmp", "/hosttmp", true})
		.setCmd({"ls", "-la", "/"});

	docker::DockerContainer container;

	std::cout << container.create(dcs) << std::endl;

}
