#include "Server.hpp"
#include "HttpServerManager.hpp"
#include "ParserConfig.hpp"

void	sigint_handler(int signum)
{
	if (signum == SIGINT)
		std::cout << "SIGINT"<< std::endl;
	if (signum == SIGQUIT)
		std::cout << "SIGQUIT"<< std::endl;
	if (signum == SIGKILL)
		std::cout << "SIGKILL"<< std::endl;
}

int main(int argc, char **argv, char **env) {
	signal(SIGINT, sigint_handler); //TODO: revisar
	signal(SIGKILL, sigint_handler);
	signal(SIGPIPE, sigint_handler);
	if (argc != 2)
		return 1;
	ParserConfig parserSrv;
	try
	{
		std::cout << argv[1] << std::endl;
		parserSrv = argv[1];
		if (argc > 2)
		{
			std::cerr << "[ERROR] Wrong number of arguments: " << argv[0] << " configuration_filename" << std::endl;
			return 1;
		}
		else if (argc == 2 && !parserSrv.dumpRawData(argv[1]))
			return 1;
		parserSrv.execute(env);
		HttpServerManager httpManager;
		httpManager.start();
	}
	catch(const std::exception& e)
	{
		Logger::log(Logger::ERROR, "main.cpp", std::string(e.what()) + ", File: " + parserSrv.get_last_lane_parser());
		
	}
}
