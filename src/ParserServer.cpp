
#include "ParserServer.hpp"

bool hasDuplicates(const std::vector<Location>& locations) {
    std::set<Location> uniqueLocations;
    
    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
        if (!uniqueLocations.insert(*it).second)
            return true;
    return false;
}

ParserServer::ParserServer(const char *file_name): _file_name(file_name), _content_file(readFileName(_file_name)) {}

void ParserServer::init_automata() {
	_automata_srv["listen "].setSetter(&Server::set_port, Setter<Server>::SIZE_T);
	_automata_srv["server_name "].setSetter(&Server::set_server_name, Setter<Server>::STRING);
	_automata_srv["keepalive_timeout "].setSetter(&Server::set_timeout, Setter<Server>::SIZE_T);
	_automata_srv["keepalive_requests "].setSetter(&Server::set_max_req, Setter<Server>::SIZE_T);

	_automata_loc["autoindex "].setSetter(&Location::set_auto_index, Setter<Location>::STRING);
	_automata_loc["return "].setSetter(&Location::set_redirect_url, Setter<Location>::STRING);
	_automata_loc["index "].setSetter(&Location::set_index, Setter<Location>::STRING);
	_automata_loc["root "].setSetter(&Location::set_root_directory, Setter<Location>::STRING);
	_automata_loc["client_max_body_size "].setSetter(&Location::set_client_max_body_size, Setter<Location>::INT);
	_automata_loc["upload_store "].setSetter(&Location::set_path_upload_directory, Setter<Location>::STRING);
	_automata_limexc["deny"].setSetter(&LimitExcept::setDenyAction, Setter<LimitExcept>::STRING);


}

LimitExcept ParserServer::parseLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	LimitExcept limExc;
	size_t lmtPos = it->find("limit_except ") + 13; // Salta "limit_except " (13 caracteres)
	size_t bracketPos = it->find("{", lmtPos); // Encuentra el '}' después de "limit_except "
	std::string strmethods = it->substr(lmtPos, bracketPos - lmtPos);
	std::map<std::string, Setter<LimitExcept> >::iterator aut_it;
	std::deque<std::string> methods = split(strmethods, ' ');
	std::deque<std::string>::iterator its;
	for (its = methods.begin(); its != methods.end(); its++)
		limExc.addAllowedMethod(*its);


	for (++it; it != end; ++it) { //Busco propiedades para los methods
		std::string line = *it;
		if (line.find("}") != std::string::npos) { // Fin de limit_except
			std::cout << line << std::endl;
			break;
		}
		else {
			for (aut_it = _automata_limexc.begin(); aut_it != _automata_limexc.end(); aut_it++){
				if (line.find(aut_it->first) != std::string::npos){
					if (aut_it != _automata_limexc.end()) {
						if (line.find(";") == std::string::npos) throw std::runtime_error("Error: Falta ';'");
						std::string val = extractStrBetween(line, aut_it->first, ";");
						aut_it->second.execute(limExc, val);
					}
					break;
				}
			}
		}
	}
	/**TODO: Errores
	 * 	 LimitException Vacio -> limit_except GET {}
	 */
	//TODO: SI no hay ciertos datos para LimitExcept que lance un error
	return limExc;
}

Location ParserServer::parseLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	Location loc;
	std::map<std::string, Setter<Location> >::iterator aut_it;
	loc.set_path(extractStrBetween(*it, "location ", " {"));
	for (++it; it != end; ++it) {
		std::string line = (*it);
		if (line.find("limit_except") != std::string::npos && line.find("{") != std::string::npos) {
			std::cout << line << std::endl;
			LimitExcept limExp = parseLimitExcept(it, end);
			loc.set_limit_except(limExp);
		} else if (line.find("}") != std::string::npos) { // Fin de location
			std::cout << line << std::endl;
			return loc;
		} else {
			std::cout << line << std::endl;

			for (aut_it = _automata_loc.begin(); aut_it != _automata_loc.end(); aut_it++){
				if (line.find(aut_it->first) != std::string::npos){
					if (aut_it != _automata_loc.end()) {
						if (line.find(";") == std::string::npos) throw std::runtime_error("Error: Falta ';'");
						std::string val = extractStrBetween(line, aut_it->first, ";");
						aut_it->second.execute(loc, val);
					}
					break;
				}
			}
			//TODO: Tratar los permisos de path_upload_directory
		}
	}
	if (loc.get_path().empty())
		throw std::runtime_error("Error en la configuracion de Location");
	return loc;
}

Server ParserServer::parseServer(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	Server srv;
	std::map<std::string, Setter<Server> >::iterator aut_it;

	for (++it; it != end; ++it) {
		std::string line = (*it);
		if (line.find("location") != std::string::npos && line.find("{") != std::string::npos) {
			std::cout << line << std::endl;
			Location loc = parseLocation(it, end);
			srv.addLocation(loc);
		} else if (line.find("}") != std::string::npos) { // Fin de server
			std::cout << line << std::endl;
			return srv;
		} else {
			for (aut_it = _automata_srv.begin(); aut_it!= _automata_srv.end(); aut_it++) {
				if (line.find(aut_it->first) != std::string::npos) {
					if (aut_it != _automata_srv.end()) {
						if (line.find(";") == std::string::npos) throw std::runtime_error("Error: Falta ';'");
						std::string val = extractStrBetween(line, aut_it->first, ";");
						//srv.set_port(8080);

						aut_it->second.execute(srv, val);
					}
					break;
				}
			}
		
		}
	}
	std::vector<Location> locs = srv.get_locations();
	if (srv.getPort() ==0  || srv.get_server_name().empty() || locs.empty() || hasDuplicates(locs))
		throw std::runtime_error("Error en la configuracion de Srv");
	return srv;

}

int ParserServer::dumpRawData(const char *file_name)
{
	_file_name = file_name;
	try {
		_content_file = readFileName(_file_name);
	} catch (const std::exception&  e) {
		std::cerr << e.what() << std::endl;
		return 0;
	}
	return 1;
}

/**
 * @brief Itero sobre _content_file y parseando el contenido a configuracion del server
 * Cliente o LimitException, para el funcionamiento del programa
 */
std::vector<Server> ParserServer::execute(char **env) {
	std::vector<Server> srvs;
	std::deque<std::string>::iterator it;
	init_automata();
	(void)env; // FIXME: Se precisa el env para cada ejecucion de CGI?, o es posible hacerlo sin utilizar el ENV que se envie, de igual forma se puede utilizar 
	if (_content_file.size() <= 0)
		throw std::runtime_error("Error not config");
	for (it = _content_file.begin(); it != _content_file.end(); ++it) {
		std::string line = (*it);
		if (line.find("server ") != std::string::npos && line.find("{") != std::string::npos) {
			std::cout << line << std::endl;
			srvs.push_back(parseServer(it, _content_file.end()));
		}
	}
	return srvs;
}
