#ifndef PARSERSERVER_HPP
#define PARSERSERVER_HPP
#include "Server.hpp"
#include <string>
#include <deque>
#include <vector>
#include "utils/Utils.hpp"
class ParserServer {
	private:
		const char *_file_name;
		std::deque<std::string> _content_file;

		LimitExcept parseLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
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
					Logger::log(Logger::DEBUG, "ParserServer.cpp", line);

					break;
				}
				else if (line.find("deny ") != std::string::npos && line.find(";") != std::string::npos) {
					limExc.setDenyAction(extractStrBetween(line, "deny ", ";"));
					continue ;
				}
			}
			return limExc;
		}

		Location parseLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
			Location loc;
			loc.set_path(extractStrBetween(*it, "location ", "{"));
			for (++it; it != end; ++it) {
				std::string line = (*it);
				if (line.find("limit_except") != std::string::npos && line.find("{") != std::string::npos) {
					Logger::log(Logger::DEBUG, "ParserServer.cpp", line);
					loc.set_limit_except(parseLimitExcept(it, end));
				} else if (line.find("}") != std::string::npos) { // Fin de location
					Logger::log(Logger::DEBUG, "ParserServer.cpp", line);
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

		Server parseServer(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end) {
			Server srv;
			for (++it; it != end; ++it) {
				std::string line = (*it);
				if (line.find("location") != std::string::npos && line.find("{") != std::string::npos) {
					Logger::log(Logger::DEBUG, "ParserServer.cpp", line);
					srv.addLocation(parseLocation(it, end));
				} else if (line.find("}") != std::string::npos) { // Fin de server
					Logger::log(Logger::DEBUG, "ParserServer.cpp", line);
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
						srv.setServerName(extractStrBetween(line, "server_name ", ";"));
						continue ;
					}
					//Identificar propiedades de Server y procesarlo. srv.set_propertie
				}
			}
			return srv;
		}
	public:
		ParserServer(const char *file_name = "ws.conf"):
		_file_name(file_name),
		_content_file(readFileName(_file_name)) {}

		int dumpRawData(const char *file_name)
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
		std::vector<Server> execute(char **env) {
			std::vector<Server> srvs;
			std::deque<std::string>::iterator it;
			
			std::string env_init(env[0]);
			Logger::log(Logger::DEBUG, "ParserServer.cpp", "env: " + env_init);
			//Verificar sintaxis y 
			Logger::log(Logger::DEBUG, "ParserServer.cpp", "n_lines of file: " + to_string(_content_file.size()));

			if (_content_file.size() <= 0)
				throw std::runtime_error("Error not config");
			for (it = _content_file.begin(); it != _content_file.end(); ++it) {
				std::string line = (*it);
				if (line.find("server ") != std::string::npos && line.find("{") != std::string::npos) {
					Logger::log(Logger::DEBUG, "ParserServer.cpp", line);
					// srvs.push_back(parseServer(it, _content_file.end()));
					parseServer(it, _content_file.end());
					
				}
			}

			srvs.push_back(Server()
			.set_port(8080)
			.setServerName("principal.com")
			.setMaxClients(10)
			.addLocation(Location()
				.set_path("/cgi-bin/")
				.set_auto_index(true)
				.set_root_directory("/cgi-bin")
				.set_index("login.py")
				.build()));
			srvs.push_back(Server()
			.set_port(8081)
			.setMaxClients(10)
			.setServerName("secundario.com")
			.addLocation(Location()
				.set_path("/")
				.set_auto_index(true)
				.set_root_directory("/")
				.build()));
			return srvs;
		}
};
#endif