#ifndef PARSERSERVER_HPP
#define PARSERSERVER_HPP

#include "Server.hpp"
#include "utils/Utils.hpp"

class ParserServer {
	private:
		const char *_file_name;
		std::deque<std::string> _content_file;

		LimitExcept parseLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);
		Location parseLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);
		Server parseServer(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);

	public:
		ParserServer(const char *file_name = "ws.conf");
		int dumpRawData(const char *file_name);
		/**
		 * @brief Busco en el archivo la configuracion necesaria para Server
		 */
		std::vector<Server> execute(char **env);
};
#endif