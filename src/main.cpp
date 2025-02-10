#include "Server.hpp"
#include "ParserServer.hpp"
#include "HttpServerManager.hpp"
bool Logger::isEnableDebug = false;

int main(int argc, char **argv, char **env) {
	ParserServer parserSrv;

	if (argc > 2)
	{
		std::cerr << "[ERROR] Wrong number of arguments: " << argv[0] << " configuration_filename" << std::endl;
		return 1;
	}
	else if (argc == 2 && !parserSrv.dumpRawData(argv[1]))
		return 1;
	
	std::vector<Server> srvs =  parserSrv.execute(env);

	HttpServerManager httpManager;
	httpManager.start(srvs);

}