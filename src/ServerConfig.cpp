#include "ServerConfig.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "HttpException.hpp"

ServerConfig::ServerConfig(int port, size_t max_clients, size_t timeout, size_t max_req) : _port(port), _max_clients(max_clients), _timeout(timeout),_max_req(max_req) {
	_error_pages[400] = "/html/400.html";
	_error_pages[404] = "/html/404.html";
	_error_pages[408] = "/html/408.html";
	_error_pages[405] = "/html/405.html";
	_error_pages[204] = "/html/204.html";
	_error_pages[301] = "/html/301.html";
	_error_pages[500] = "/html/500.html";
}

ServerConfig &ServerConfig::set_max_req(const size_t max_req) {

	if (this->_max_req && max_req)
		this->_max_req = (max_req > 100) ? 100: max_req;
	return *this;
}

ServerConfig &ServerConfig::set_timeout(const size_t timeout) {
	if (this->_timeout && timeout)
	this->_timeout = (timeout > 5) ? 5: timeout;
	return *this;
}

ServerConfig &ServerConfig::set_port(const size_t port) {

	_port=port;
	return *this;
}

ServerConfig &ServerConfig::set_server_name(const std::string &server_name) {
	_server_name = server_name;
	return *this;
}

ServerConfig &ServerConfig::setMaxClients(const int max_cients) {
	_max_clients = max_cients;
	return *this;
}

ServerConfig &ServerConfig::add_location(const Location &location) {
	std::map<std::string, Location>::iterator it = _locations_conf.find(location.get_path());
	if (it != _locations_conf.end())
		throw ServerConfig::LocationRepeateException();
	if (location.get_path().empty())
		throw Location::LocationNotContainPathException();
	_locations_conf[location.get_path()] = location;
	return *this;
}
ServerConfig &ServerConfig::set_error_page(const int code, std::string path) {
	std::map<int, std::string>::iterator it = _error_pages.find(code);
	//TODO: Debo verificar si hay un archivo en ese path y si no hay lanzo un error
	if (it != _error_pages.end())
		_error_pages[code] = path;
	return *this;
}

std::string ServerConfig::get_error_page_by_code(const int code) {
	return _error_pages[code];
}

int countOccurrences(const std::string& str, const std::string& sub) {
	int count = 0;
	size_t pos = 0;

	// Usamos find para buscar subcadenas
	while ((pos = str.find(sub, pos)) != std::string::npos) {
		count++;  // Encontró una coincidencia
		pos += sub.length();  // Avanza la posición para buscar la siguiente coincidencia
	}

	return count;
}

/**
 * @brief Busca la Location que coincide con el path dado, comprobando coincidencias exactas y prefijos.
 * 
 * Si se encuentra una coincidencia exacta, se devuelve esa Location. Si no, se buscan las Location que coincidan 
 * con el prefijo más largo en la configuración y se devuelve la que tenga más coincidencias.
 *
 * @param path La ruta de la URL a comprobar.
 * @return Location La Location que coincide con el path.
 * @example prefixs:["/home/","/home/user/", "/"] path: "/home/user/pepe.py" -> return "/home/user/"
 */
Location ServerConfig::findMatchingLocation(const std::string path) {
	std::map<std::string, Location>::iterator it = _locations_conf.find(path);
	Location loc_ocurrence;
	int tmp_max_ocurrences = 0;
	if (it == _locations_conf.end()) {
		for (it = _locations_conf.begin(); it != _locations_conf.end(); it++) {
			std::string prefix = it->first;
			if (path.size() < prefix.size())
				continue ;
			int ocurrences = countOccurrences(path, prefix);
			if ( ocurrences > tmp_max_ocurrences) {
				tmp_max_ocurrences = ocurrences;
				loc_ocurrence = it->second;
			}
		}
		if (loc_ocurrence.empty())
			throw HttpException::NotFoundException();
		return loc_ocurrence;
	}
	return it->second;
}

int ServerConfig::get_port() const{
	return _port;
}

int ServerConfig::get_max_clients() const{
	return _max_clients;
}

std::string ServerConfig::get_server_name() const {
	return _server_name;
}


size_t ServerConfig::get_timeout() const {
	return _timeout;
}

size_t ServerConfig::get_max_req()const {
	return _max_req;
}

std::map<std::string, Location> ServerConfig::get_locations() {
	return _locations_conf;
}