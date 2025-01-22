#ifndef PARSERSERVER_HPP
#define PARSERSERVER_HPP
#include "Server.hpp"
#include "utils/split.hpp"
#include "utils/readFileName.hpp"
#include <string>
#include <deque>
#include <vector>

class ParserServer {
	private:
		const char *_file_name;
		std::deque<std::string> _content_file;
	public:
		ParserServer(const char *file_name = "ws.conf"):
		_file_name(file_name),
		_content_file(readFileName(_file_name)) {}
		/**
		 * @brief Busco en el archivo la configuracion necesaria para Server
		 */
		std::vector<Server> execute() {
			std::vector<Server> srvs;
			Location loc;

			std::deque<std::string>::iterator it;
			if (_content_file.size() <= 0)
				throw std::runtime_error("Error not config");
			for (it = _content_file.begin(); it != _content_file.end(); it++ ) {
				//Busco http: std::vector<Server> srv
				//----Busco server: srv.push_back()
				//----------Busco Location
			}
			srvs.push_back(Server().addLocation(loc));
			return srvs;
		}
};
#endif