
#include "ParserServer.hpp"


LimitExcept ParserServer::parseLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	LimitExcept limExc;
	size_t lmtPos = it->find("limit_except ") + 13; // Salta "limit_except " (13 caracteres)
	size_t bracketPos = it->find("{", lmtPos); // Encuentra el '}' después de "limit_except "
	std::string strmethods = it->substr(lmtPos, bracketPos - lmtPos);
	std::deque<std::string> methods = split(strmethods, ' ');
	std::deque<std::string>::iterator its;
	for (its = methods.begin(); its != methods.end(); its++)
		limExc.addAllowedMethod(*its);
	for (++it; it != end; ++it) {
		std::string line = *it;
		if (line.find("}") != std::string::npos) { // Fin de limit_except
			std::cout << line << std::endl;
			break;
		}
		else if (line.find("deny ") != std::string::npos && line.find(";") != std::string::npos) {
			limExc.setDenyAction(extractStrBetween(line, "deny ", ";"));
			continue ;
		}
		//std::cout << line << std::endl; // Procesar línea
	}
	return limExc;
}

Location ParserServer::parseLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	Location loc;
	loc.set_path(extractStrBetween(*it, "location ", "{"));
	for (++it; it != end; ++it) {
		std::string line = (*it);
		if (line.find("limit_except") != std::string::npos && line.find("{") != std::string::npos) {
			std::cout << line << std::endl;
			loc.set_limit_except(parseLimitExcept(it, end));
		} else if (line.find("}") != std::string::npos) { // Fin de location
			std::cout << line << std::endl;
			return loc;
		} else {
			if (line.find("index ") != std::string::npos && line.find(";") != std::string::npos){
				loc.set_index(extractStrBetween(line, "index ", ";"));
				continue;
			}
			if (line.find("root ") != std::string::npos && line.find(";") != std::string::npos){
				loc.set_root_directory(extractStrBetween(line, "root ", ";"));
				loc.build();
				continue ;
			}
			if (line.find("autoindex ") != std::string::npos && line.find(";") != std::string::npos){
				loc.set_auto_index(line.find("on") != std::string::npos);
				continue ;
			}
			if (line.find("client_max_body_size ") != std::string::npos && line.find("M;") != std::string::npos){
				char *endp = NULL;
				loc.set_client_max_body_size(static_cast<int>(strtod(extractStrBetween(line, "client_max_body_size ", "M;").c_str(), &endp)));
				continue;
			}
			if (line.find("upload_store ") != std::string::npos && line.find(";") != std::string::npos){
				loc.set_path_upload_directory(extractStrBetween(line, "upload_store ", ";"));
				continue;
			}
			//TODO: Tratar los permisos de path_upload_directory
			if (line.find("return ") != std::string::npos && line.find(";") != std::string::npos){
				loc.set_redirect_url(extractStrBetween(line, "return ", ";"));
				continue;
			}
		}
	}
	return loc;
}

Server ParserServer::parseServer(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	Server srv;
	for (++it; it != end; ++it) {
		std::string line = (*it);
		if (line.find("location") != std::string::npos && line.find("{") != std::string::npos) {
			std::cout << line << std::endl;
			srv.addLocation(parseLocation(it, end));
		} else if (line.find("}") != std::string::npos) { // Fin de server
			std::cout << line << std::endl;
			return srv;
		} else {
			if (line.find("listen ") != std::string::npos && line.find(";") != std::string::npos) {
				char *endp;
				std::string port = extractStrBetween(line, "listen ", ";");
				srv.set_port(static_cast<int>(std::strtod(port.c_str(), &endp)));
				continue ;
			}
			if (line.find("server_name ") != std::string::npos && line.find(";") != std::string::npos) {
				//char *endp; TODO
				//srv.set_server_name(extractStrBetween(line, "server_name ", ";"));
				continue ;
			}
			//std::cout << line << std::endl; // Procesar línea
			//Identificar propiedades de Server y procesarlo. srv.set_propertie
		}
	}
	return srv;
}

ParserServer::ParserServer(const char *file_name): _file_name(file_name), _content_file(readFileName(_file_name)) {}

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
 * @brief Busco en el archivo la configuracion necesaria para Server
 */
std::vector<Server> ParserServer::execute(char **env) {
	std::vector<Server> srvs;
	std::deque<std::string>::iterator it;

	std::cout<< "env " << env[0] << std::endl; // TODO: borrar linea
	//Verificar sintaxis y 
	std::cout<< "n_lines of file: " << _content_file.size() << std::endl;
	if (_content_file.size() <= 0)
		throw std::runtime_error("Error not config");
	for (it = _content_file.begin(); it != _content_file.end(); ++it) {
		std::string line = (*it);
		if (line.find("server ") != std::string::npos && line.find("{") != std::string::npos) {
			std::cout << line << std::endl;
			// srvs.push_back(parseServer(it, _content_file.end()));
			parseServer(it, _content_file.end());
			
		}
	}

	srvs.push_back(Server()
	.set_port(8080)
	.addLocation(Location()
		.set_path("/cgi-bin/")
		.set_limit_except(LimitExcept()
			.addAllowedMethod("GET")
			.addAllowedMethod("POST")
		)
		.set_auto_index(true)
		.set_root_directory("/cgi-bin")
		.set_index("login.py")
		.build()));
	Server s;
	s.set_port(8081);
	s.addLocation(Location()
		.set_limit_except(LimitExcept()
			.addAllowedMethod("GET"))
		.set_path("/")
		.set_auto_index(true)
		.set_root_directory("/html")
		.set_index("index.html")
		.build());
	s.addLocation(Location()
		.set_path("/cgi-bin/")
		.set_auto_index(true)
		.set_root_directory("/cgi-bin")
		.set_index("login.py")
		.build());
	s.addLocation(Location()
		.set_path("/images/")
		.set_auto_index(true)
		.set_root_directory("/img/www")
		.build());
	
	srvs.push_back(s);
	return srvs;
}
