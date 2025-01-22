#ifndef PARSERSERVER_HPP
#define PARSERSERVER_HPP
#include "Server.hpp"
#include "utils/split.hpp"
#include "utils/readFileName.hpp"
#include <string>
#include <deque>

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
		Server* execute() {
			Server *srv = new Server();
			srv->addLocation("", "");
			return srv;
		}
};
#endif