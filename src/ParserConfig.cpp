
#include "ParserConfig.hpp"

const char* is_file_conf(const char *str) {
	std::string filename(str);
	if (ends_with(filename, ".conf"))
		return str;
	throw std::runtime_error("Parsing error: Invalid file extension, only '.conf' is valid.");
}

ParserConfig::ParserConfig(const char *file_name): _file_name(file_name), _content_file(readFileName(is_file_conf(_file_name))) {}

void ParserConfig::init_automata() {
	_automata_srv["listen "].setSetter(&ServerConfig::set_port, Setter<ServerConfig>::SIZE_T);
	_automata_srv["server_name "].setSetter(&ServerConfig::set_server_name, Setter<ServerConfig>::STRING);
	_automata_srv["keepalive_timeout "].setSetter(&ServerConfig::set_timeout, Setter<ServerConfig>::SIZE_T);
	_automata_srv["keepalive_requests "].setSetter(&ServerConfig::set_max_req, Setter<ServerConfig>::SIZE_T);
	_automata_srv["error_page "].setSetter(&ServerConfig::set_error_page, Setter<ServerConfig>::MAP_INT_STR);

	_automata_loc["autoindex "].setSetter(&Location::set_auto_index, Setter<Location>::STRING);
	_automata_loc["return "].setSetter(&Location::set_redirect_url, Setter<Location>::STRING);
	_automata_loc["index "].setSetter(&Location::set_index, Setter<Location>::STRING);
	_automata_loc["root "].setSetter(&Location::set_root_directory, Setter<Location>::STRING);
	_automata_loc["client_max_body_size "].setSetter(&Location::set_client_max_body_size, Setter<Location>::SIZE_T);
	_automata_loc["upload_store "].setSetter(&Location::set_path_upload_directory, Setter<Location>::STRING);
	_automata_limexc["deny"].setSetter(&LimitExcept::setDenyAction, Setter<LimitExcept>::STRING);

}

LimitExcept ParserConfig::parserLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	LimitExcept limExc;
	size_t lmtPos = it->find("limit_except ") + 13; // Jumps "limit_except " (13 chars)
	size_t bracketPos = it->find("{", lmtPos); // Finds '}' after "limit_except "
	std::map<std::string, Setter<LimitExcept> >::iterator aut_it;
	std::string strmethods = it->substr(lmtPos, bracketPos - lmtPos);
	std::deque<std::string> methods = split(strmethods, ' ');
	std::deque<std::string>::iterator its;
	if (methods.size() <= 0)
		throw Config::ConfigNotFoundException();
	for (its = methods.begin(); its != methods.end(); its++)
		limExc.addAllowedMethod(*its);

	for (++it; it != end; ++it) { //Finds properties for the methods
		std::string line = *it;
		if (line.find("}") != std::string::npos) { // End of limit_except
			break;
		}
		else {
			for (aut_it = _automata_limexc.begin(); aut_it != _automata_limexc.end(); aut_it++){
				if (line.find(aut_it->first) != std::string::npos){
					if (aut_it != _automata_limexc.end()) {
						if (line.find(";") == std::string::npos) throw std::runtime_error("Error: Missing ';'");
						std::string val = extractStrBetween(line, aut_it->first, ";");
						aut_it->second.execute(limExc, val);
					}
					break;
				}
			}
		}
	}
	return limExc;
}

Location ParserConfig::parserLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	Location loc;
	std::map<std::string, Setter<Location> >::iterator aut_it;
	loc.set_path(extractStrBetween(*it, "location ", " {"));
	for (++it; it != end; ++it) {
		std::string line = (*it);
		if (line.find("limit_except") != std::string::npos && line.find("{") != std::string::npos) {
			LimitExcept limExp = parserLimitExcept(it, end);
			loc.set_limit_except(limExp);
		} else if (line.find("}") != std::string::npos) { // End of Location
			return loc;
		} else {

			for (aut_it = _automata_loc.begin(); aut_it != _automata_loc.end(); aut_it++){
				if (line.find(aut_it->first) != std::string::npos){
					if (aut_it != _automata_loc.end()) {
						if (line.find(";") == std::string::npos) throw std::runtime_error("Error: Missing ';'");
						std::string val = extractStrBetween(line, aut_it->first, ";");
						aut_it->second.execute(loc, val);
					}
					break;
				}
			}
		}
	}
	if (loc.get_path().empty())
		throw std::runtime_error("Error on Location configuration");
	return loc;
}

ServerConfig ParserConfig::parserServerConfig(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
	ServerConfig srv;
	std::map<std::string, Setter<ServerConfig> >::iterator aut_it;

	for (++it; it != end; ++it) {
		std::string line = (*it);
		if (line.find("location") != std::string::npos && line.find("{") != std::string::npos) {
			Location loc = parserLocation(it, end);
			srv.add_location(loc);
		} else if (line.find("}") != std::string::npos) { // End of server
			return srv;
		} else {
			for (aut_it = _automata_srv.begin(); aut_it!= _automata_srv.end(); aut_it++) {
				if (line.find(aut_it->first) != std::string::npos) {
					if (aut_it != _automata_srv.end()) {
						if (line.find(";") == std::string::npos) throw std::runtime_error("Error: Missing ';'");
						std::string val = extractStrBetween(line, aut_it->first, ";");
						aut_it->second.execute(srv, val);
					}
					break;
				}
			}
		
		}
	}
	std::map<std::string, Location> locs = srv.get_locations();
	if (srv.get_port() ==0  || srv.get_server_name().empty() || locs.empty())
		throw std::runtime_error("Error on Srv configuration");
	return srv;

}

int ParserConfig::dumpRawData(const char *file_name)
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
 * @brief Iters _content_file and parses the content of the config server
 * Cliente or LimitException, for the program executiong
 */
void ParserConfig::execute() {
	Config &conf = Config::getInstance();
	init_automata();
	if (_content_file.size() <= 0)
		throw std::runtime_error("Error not config");
	for (_it = _content_file.begin(); _it != _content_file.end(); ++_it) {
		std::string line = (*_it);
		if (line.find("server ") != std::string::npos && line.find("{") != std::string::npos) {
			conf.addServerConf(parserServerConfig(_it, _content_file.end()));
		}
	}
}

std::string ParserConfig::get_last_lane_parser() {
	std::deque<std::string>::iterator it_content;
	int line_n = 1;
	for (it_content = _content_file.begin(); it_content != _it ; it_content++) {
		line_n++;
	}
	return std::string(_file_name) + ":" + to_string(line_n)+ ", \n line: " + *_it;
}