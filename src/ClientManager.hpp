#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP
#include "CGI.hpp"
#include "ServerConfig.hpp"
#include "HttpStatus.hpp"
#include "Client.hpp"

class ClientManager {
	private:
		std::map<int, Client *> _pfd_cli;// Pipe de respuesta en base a CGI
		std::map<int, Client *> _sock_cli;// Pipe de respuesta en base a CGI
		ClientManager(){}
		~ClientManager(){}
		ClientManager& operator=(ClientManager&){return *this;}
	public:
		static ClientManager& getInstance(){
			static ClientManager instance;
			return instance;
		}

		void clear(){
			_pfd_cli.clear();
			_sock_cli.clear();
		}
		Client* save_cli_by_pfd(std::pair<int, Client*> pair){
			_pfd_cli[pair.first] = pair.second;
			return pair.second;
		}
		Client* save_cli_by_sock(std::pair<int, Client*> pair){
			_sock_cli[pair.first] = pair.second;
			return pair.second;
		}
		Client *get_cli_by_pfd(int fd){
			std::map<int, Client *>::iterator it = _pfd_cli.find(fd);
			if (it != _pfd_cli.end())
				return it->second;
			return NULL;
		}
		Client *get_cli_by_sock(int fd){
			std::map<int, Client *>::iterator it = _sock_cli.find(fd);
			if (it != _sock_cli.end())
				return it->second;
			return NULL;
		}
};

#endif