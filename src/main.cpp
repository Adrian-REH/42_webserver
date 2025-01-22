#include "Server.hpp"

int main(int argc, char **argv, char **env) {
	
	Server srv  = Server().addLocation("/cgi-bin/", "/cgi-bin/app.py");
	
	srv.init();
	srv.start();
	srv.stop();
}