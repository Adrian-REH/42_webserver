#include "Server.hpp"
#include "ParserServer.hpp"

int main(int argc, char **argv, char **env) {
	ParserServer parserSrv;

	std::vector<Server> srvs =  parserSrv.execute();
	std::vector<Server>::iterator it;

	pid_t *pid = (pid_t *)malloc(sizeof(pid_t) * srvs.size());

	//TODO: Hacer que se ejecuten en distintos pids los servidores
	for (int i = 0; i < srvs.size(); i ++) {
		srvs[i].init();
		srvs[i].start();
		srvs[i].stop();
	}
/* 	for (int i = 0; i < srvs.size(); i ++) {
		waitpid(pid[i], NULL, 0);
	} */


}