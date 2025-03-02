
#include "Config.hpp"

Config::Config() {

}

Config &Config::getInstance() {
	static Config instance;
	return instance;
}

void Config::addServerConf(ServerConfig srv_conf) {
	std::map<int, ServerConfig>::iterator it = _srvs_conf.find(srv_conf.get_port());

	if (it != _srvs_conf.end())
		throw Config::ConfigServerNameExistException();
	if (srv_conf.get_port() == 0)
		throw std::runtime_error("The Server not contain server_name");
	_srvs_conf[srv_conf.get_port()] = srv_conf;
}

std::map<int, ServerConfig> Config::getServerConfs() {
	return _srvs_conf;
}

ServerConfig Config::getServerConfByServerName(const int port) {
	std::map<int, ServerConfig>::iterator it = _srvs_conf.find(port);

	if (it != _srvs_conf.end()){
		return it->second;
	}
	throw Config::ConfigNotFoundException();
}