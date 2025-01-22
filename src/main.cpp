#include "Server.hpp"
#include "ParserServer.hpp"

int main(int argc, char **argv, char **env) {
	//ParserServer parserSrv;
	Location loc = Location()
	.set_root_directory("/cgi-bin")
	.set_index("login.py")
	.build();
	Server srv  = Server().addLocation(loc);
	
	srv.init();
	srv.start();
	srv.stop();
}