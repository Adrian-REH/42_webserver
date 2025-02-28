
#include "Config.hpp"

Config::Config() {

}

Config &Config::getInstance() {
	static Config instance;
	return instance;
}

void Config::addServerConf(ServerConfig srv_conf) {
	std::map<std::string, ServerConfig>::iterator it = _srvs_conf.find(srv_conf.get_server_name());

	if (it != _srvs_conf.end())
		throw Config::ConfigNotFoundException();
	if (srv_conf.get_server_name().empty())
		throw std::runtime_error("The Server not contain server_name");
	_srvs_conf[srv_conf.get_server_name()] = srv_conf;
}

std::map<std::string, ServerConfig> Config::getServerConfs() {
	return _srvs_conf;
}

ServerConfig Config::getServerConfByServerName(const std::string srv_name) {
	std::map<std::string, ServerConfig>::iterator it = _srvs_conf.find(srv_name);

	if (it != _srvs_conf.end()){
		return it->second;
	}
	throw Config::ConfigNotFoundException();
}