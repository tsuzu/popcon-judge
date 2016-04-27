#include <cpprest/http_client.hpp>
#include <random>
#include <string>

namespace docker {
	using uint = unsigned int;

	struct DockerContainerSetting {
		std::string image, cgroup;
		uint mem; // MB
		uint cpu; // %

		struct Mount {
			std::string from, to;
			bool writable;
		};
		std::vector<Mount> mounts;

		std::map<std::string, int> ulimit; // Hard and Soft is the same value
		
		using DCS = DockerContainerSetting;

		DCS& image(const std::string& s) {
			image = i;

			return *this;
		}
		DCS& cgroup(const std::string& s) {
			cgroup = s;

			return *this;
		}
		DCS& mem(uint v) {
			mem = v;

			return *this;
		}
		DCS& cpu(uint v) {
			cpu = v;

			return *this;
		}
		DCS& addMount(const Mount& m) {
			mounts.emplace_back(m);

			return *this;
		}
		DCS& addUlimit(const std::string& n, int v) {
			ulimit[n] = v;

			return *this;
		}
	};

	class DockerContainer {
		std::string docker_name;
		DockerContainerSetting setting;

	public:
		DockerContainer() {
			std::mt19937 rd{std::random_device{}()};
			std::uniform_int_distribution<int> dist(0, 61);

			docker_name.reserve(30);
			for(int i = 0; i < 30; ++i) {
				int r = dist(rd);

				// Only ASCII and UTF-8 Support
				if(r < 26) {
					docker_name.push_back('a' + r);
				}else if (r < 52) {
					docker_name.push_back('A' + r - 26);
				}else {
					docker_name.push_back('0' + r - 52);
				}
			}
		}

		DockerContainer(const DockerContainer&) = delete;
		DockerContainer(DockerContainer&&) = default;
	
		DockerContainer operator = (const DockerContainer&) = delete;
		DockerContainer operator = (DockerContainer&&) = default;

		void create(const DockerContainerSettting& opt) {
			this->setting = opt;

			
			web::http::client client(U("h"));
		}
	};

}
