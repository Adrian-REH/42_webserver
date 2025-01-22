#include "Server.hpp"
#include "ParserServer.hpp"

int main(int argc, char **argv, char **env) {
	//ParserServer parserSrv;
	Location loc;
	Server srv  = Server().addLocation(loc);
	
	srv.init();
	srv.start();
	srv.stop();
}