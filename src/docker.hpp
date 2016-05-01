#pragma once

#include <cpprest/http_client.h>
#include <json.hpp>
#include <random>
#include <string>
#include <initializer_list>

namespace docker {
	std::string docker_api = "http://127.0.0.1:4321";

	using uint = unsigned int;

	struct DockerContainerSetting {
		std::string image, cgroup;
		uint cpu; // %

		struct Mount {
			std::string from, to;
			bool writable;
		};
		std::vector<Mount> mounts;

		std::map<std::string, int> ulimit; // Both of hard and soft limit are the same value
		
		std::vector<std::string> cmd;

		using DCS = DockerContainerSetting;

		DCS& setImage(const std::string& s) {
			image = s;

			return *this;
		}
		DCS& setCgroup(const std::string& s) {
			cgroup = s;

			return *this;
		}
		DCS& setCPU(uint v) {
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
		DCS& setCmd(std::initializer_list<std::string> list) {
			cmd.clear();
			cmd.reserve(list.size());

			for(auto&& s : list) {
				cmd.emplace_back(s);
			}
		}
		template<typename Iterator>
		DCS& setCmd(const Iterator& begin, const Iterator& end) {
			cmd = std::vector<std::string>(begin, end);
		}
	};

	class DockerContainer {
		std::string dockerName;
		DockerContainerSetting setting;
		bool isCreated = false;
	
	public:
		DockerContainer() {
			std::mt19937 rd{std::random_device{}()};
			std::uniform_int_distribution<int> dist(0, 61);

			dockerName.reserve(30);
			for(int i = 0; i < 30; ++i) {
				int r = dist(rd);

				// Only ASCII and UTF-8 Support
				if(r < 26) {
					dockerName.push_back('a' + r);
				}else if(r < 52) {
					dockerName.push_back('A' + r - 26);
				}else {
					dockerName.push_back('0' + r - 52);
				}
			}
		}

		DockerContainer(const DockerContainer&) = delete;
		DockerContainer(DockerContainer&& dc) {
			this->dockerName = std::move(dc.dockerName);
			this->setting = std::move(setting);
			
			this->isCreated = dc.isCreated;
			dc.isCreated = false;
		}
	
		DockerContainer& operator = (const DockerContainer&) = delete;
		DockerContainer& operator = (DockerContainer&& dc) {
			this->dockerName = std::move(dc.dockerName);
			this->setting = std::move(dc.setting);

			this->isCreated = dc.isCreated;
			dc.isCreated = false;

			return *this;
		}

		~DockerContainer() {
			if(isCreated) {
				web::http::client::http_client cl(U(docker_api));

				cl.request(web::http::methods::DEL, U("/containers/" + dockerName + "?force=1")).wait();
			}
		}

		bool create(const DockerContainerSetting& opt) {
			this->setting = opt;

			using json = nlohmann::json;

			json j;

//			j["Hostname"] = this->dockerName;
			j["Image"] = opt.image;
			j["AttachStdin"] = true;
			j["AttachStdout"] = true;
			j["AttachStderr"] = true;
			j["Tty"] = true;
			j["OpenStdin"] = true;
			j["StdinOnce"] = false;
			j["NetworkDisabled"] = true;
			j["HostConfig"]["CpuPeriod"] = 100000;
			j["HostConfig"]["CpuQuota"] = 1000 * opt.cpu;
			j["HostConfig"]["CgroupParent"] = opt.cgroup;
			j["Cmd"] = opt.cmd;
			
			std::vector<json> ulimit_vec;
			ulimit_vec.reserve(opt.ulimit.size());

			for(auto&& v : opt.ulimit) {
				json j;
				j["Name"] = v.first;
				j["Soft"] = j["Hard"] = v.second;

				ulimit_vec.emplace_back(j);
			}
			j["HostConfig"]["Ulimits"] = ulimit_vec;

			std::vector<std::string> volume_vec;
			volume_vec.reserve(opt.mounts.size());

			for(auto&& v : opt.mounts) {
				volume_vec.emplace_back(v.from + ":" + v.to + (v.writable ? "" : ":ro"));
			}

			j["HostConfig"]["Binds"] = volume_vec;
			
			using namespace utility;

			try {
				web::http::client::http_client cl(U(docker_api));

				cl.request(web::http::methods::POST, U("/containers/create?name=" + dockerName), j.dump(), "application/json")
					  .then([=](pplx::task<web::http::http_response> task)
					{
						auto resp = task.get();
	
						if(resp.status_code() / 100 != 2) {
							// TODO: Add Logger
							
							std::cout << resp.status_code() << std::endl;
							return false;
						}else {
							isCreated = true;
	
							return true;
						}
					})
					.wait();
			}catch(std::exception& e) {
				return false;
			}
		}
	};

};
