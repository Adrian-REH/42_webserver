#include "Server.hpp"
#include "ParserServer.hpp"

pid_t create_server(Server &srv) {
	pid_t pid = fork();

	if (pid < 0)
		return -1;
	else if (pid == 0)
	{
		srv.init();
		srv.start();
		srv.stop();
		exit(1);
	}
	return pid;
}

int main(int argc, char **argv, char **env) {
	ParserServer parserSrv;
	size_t n_server;

	if (argc < 2 || argc > 2)
		std::cerr << "[ERROR] Wrong number of arguments " << argv[0] << std::endl;

	std::vector<Server> srvs =  parserSrv.execute(env);
	std::vector<Server>::iterator it;

	n_server = srvs.size();
	pid_t *pid = (pid_t *)malloc(sizeof(pid_t) * n_server);
	for (size_t i = 0; i < n_server; i ++) {
		pid[i] = create_server(srvs[i]);
	}
	for (size_t i = 0; i < n_server; i ++) {
		waitpid(pid[i], NULL, 0);
	}
}