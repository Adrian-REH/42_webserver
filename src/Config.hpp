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
		std::map<int, ServerConfig> _srvs_conf;
		Config();
		Config(const Config&);
		Config& operator=(const Config&);
	public:
		static Config &getInstance();
		void addServerConf(ServerConfig);
		std::map<int, ServerConfig> getServerConfs();
		ServerConfig getServerConfByServerName(const int);

	class ConfigNotFoundException : public std::exception {
		public:
			const char* what() const throw() {
				return "Configuration not found";
			}
	};

	class ConfigServerPortExistException : public std::exception {
		private:
			std::string _message;
		public:
			ConfigServerPortExistException(std::string message = ""):_message("The server port: " + message + " already exist") {}
			virtual ~ConfigServerPortExistException() throw() {}
			const char* what() const throw() {
				return (_message).c_str();
			}
	};
};

#endif