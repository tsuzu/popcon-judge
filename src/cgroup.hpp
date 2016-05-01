#pragma once

#include <cstdlib>
#include <random>
#include <vector>
#include <initializer_list>
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <boost/lexical_cast.hpp>

namespace sys {
	bool exec(const std::initializer_list<std::string>& cmd) {
		pid_t pid = fork();

		if(pid == -1) {
			return false;
		}else if(pid == 0) {
			std::vector<char*> cmdVec;
			cmdVec.reserve(cmd.size() + 1);

			for(auto&& arg : cmd) {
				cmdVec.push_back(const_cast<char*>(&arg[0]));
			}
			cmdVec.push_back(nullptr);

			execv(cmdVec[0], reinterpret_cast<char* const*>(&cmdVec[0]));

			_exit(1);
		}else {
			int status;
			waitpid(pid, &status, 0);

			return WIFEXITED(status) && WEXITSTATUS(status) == 0;
		}
	}
}


namespace cgroup {
	std::string cgroupFS = "/sys/fs/cgroup";

	class CgroupManager {
		std::string groupName;
		std::vector<std::string> subsys;
		bool isCreated = false;

		std::string allSubsysStr() {
			std::string str;

			for(auto&& s : subsys) {
				str += s +  ":";
			}

			return str.substr(0, str.size() - 1);
		}
	public:
		CgroupManager() {
			std::mt19937 rd{std::random_device{}()};
			std::uniform_int_distribution<int> dist{0, 61};

			groupName.reserve(31);
			groupName.push_back('/');
			for(int i = 0; i < 30; ++ i) {
				int r = dist(rd);

				// Only ASCII and UTF-8 Support
				if(r < 26) {
					groupName.push_back('a' + r);
				}else if(r < 52) {
					groupName.push_back('A' + r - 26);
				}else {
					groupName.push_back('0' + r - 52);
				}
			}
		}

		CgroupManager(const CgroupManager&) = delete;
		CgroupManager(CgroupManager&& cm) {
			this->groupName = std::move(cm.groupName);
			this->subsys = std::move(cm.subsys);

			this->isCreated = cm.isCreated;
			cm.isCreated = false;
		}

		CgroupManager& operator = (const CgroupManager&) = delete;
		CgroupManager& operator = (CgroupManager&& cm) {
			this->groupName = std::move(cm.groupName);
			this->subsys = std::move(cm.subsys);

			this->isCreated = cm.isCreated;
			cm.isCreated = false;
			
			return *this;
		}

		~CgroupManager() {
			if(isCreated) {
				sys::exec({"/usr/bin/cgdelete", allSubsysStr() + ":" + groupName});
			}
		}

		CgroupManager& addSubsys(const std::string& s) {
			if(isCreated)
				throw std::runtime_error{"already created"};

			subsys.emplace_back(s);

			return *this;
		}
		CgroupManager& addSubsys(const std::initializer_list<std::string>& list) {
			if(isCreated)
				throw std::runtime_error{"already created"};

			subsys.reserve(subsys.size() + list.size());

			for(auto&& s : list) {
				subsys.emplace_back(s);
			}

			return *this;
		}
		template<typename T>
		CgroupManager& set(const std::string& subsys, const std::string& item, T&& val) {
			auto path = cgroupFS + "/" + subsys + "/" + groupName + "/" + item;
			std::ofstream ofs(path);

			if(!ofs) {
				throw std::runtime_error{"Unable to open '" + path + "' for writing"};
			}

			ofs << val;
		}
		template<typename T>
		bool setter(const std::string& subsys, const std::string& item, T&& val) noexcept {
			std::ofstream ofs(cgroupFS + "/" + subsys + "/" + groupName + "/" + item);

			if(!ofs) {
				return false;
			}
			
			try {
				ofs << val;
			}catch(std::exception& e) {
				return false;
			}

			return true;
		}

		template<typename T = std::string>
		T getter(const std::string& subsys, const std::string& item) {
			auto path = cgroupFS + "/" + subsys + "/" + groupName + "/" + item;
			std::ifstream ifs(path);

			if(!ifs) {
				throw std::runtime_error{"Unable to open '" + path + "' for reading"};
			}

			auto in = std::string{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
			
			return boost::lexical_cast<std::string>(in);
		}
};
}
