#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <map>
#include <string>
#include "Cookie.hpp"
#include "Client.hpp"
#include "Location.hpp"
#include "ServerConfig.hpp"


class Config {
	private:
		size_t _max_clients;
		std::map<std::string, ServerConfig> _srvs_conf;
		Config();
		Config(const Config&);
		Config& operator=(const Config&);
	public:
		static Config &getInstance();
		void addServerConf(ServerConfig);
		std::map<std::string, ServerConfig> getServerConfs();
		ServerConfig getServerConfByServerName(const std::string);

	class ConfigNotFoundException : public std::exception {
		public:
			const char* what() const throw() {
				return "Configuration not found";
			}
	};

	class ConfigServerNameExistException : public std::exception {
		public:
			const char* what() const throw() {
				return "The server_name already exists";
			}
	};
};

#endif