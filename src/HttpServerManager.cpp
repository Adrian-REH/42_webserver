#include "Server.hpp"
#include <list>
#define MAX_CLIENTS 30
#include "HttpServerManager.hpp"
#include "Logger.hpp"
#include "Config.hpp"

HttpServerManager::HttpServerManager(): _sock_srvs(), _srv_sockets(0), _cli_srvs(), _epoll_fd(0),_max_events(0) {}


int HttpServerManager::start() {
	std::map<int, ServerConfig>::iterator it;
	std::map<int, ServerConfig>  srvs_conf = Config::getInstance().getServerConfs();
	int socket_fd;
	size_t max_clients_srv = (1024 / srvs_conf.size());
	struct epoll_event ev;

	_epoll_fd = epoll_create(1);
	if (_epoll_fd == -1) {
		Logger::log(Logger::ERROR,"HttpServerManager.cpp", "Error al crear epoll");
		std::vector<int>::iterator it;
		for (it = _srv_sockets.begin(); it != _srv_sockets.end(); it++)
			close(*it);
		return -1;
	}
	fcntl(_epoll_fd, F_SETFD, FD_CLOEXEC);

	for (it = srvs_conf.begin(); it != srvs_conf.end(); it++) {
		socket_fd = create_socket_fd(it->second.get_port());
		if (socket_fd < 0) {
			Logger::log(Logger::WARN,"HttpServerManager.cpp",  it->second.get_server_name()+ ":" + to_string(it->second.get_port()) + " failed: Address already in use");
			continue ;
		}
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
			Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to register server socket with epoll: " + to_string(_epoll_fd) + ", socket_fd: " + to_string(socket_fd));
			close(socket_fd);
			continue ;
		}
		Logger::log(Logger::INFO,"HttpServerManager.cpp", "Server socket registered: host:port http://localhost:"+ to_string(it->second.get_port()) +" socket_fd: " + to_string(socket_fd));
		
		Server *srv = new Server(it->first, max_clients_srv, it->second.get_server_name());
		srv->setSocketFd(socket_fd);
		_sock_srvs[socket_fd] = srv;
		_max_events += srv->getMaxClients();
	}
	if (_sock_srvs.empty()) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Number of server is 0");
		return -1;
	}
	handle_epoll();
	return 0;
}

void HttpServerManager::stop() {
	std::vector<int>::iterator it;
	std::map<int, Server*> ::iterator it_clifd_srv;
	for (it = _srv_sockets.begin(); it != _srv_sockets.end(); it++){
		close(*it);
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, *it, NULL);
		_srv_sockets.erase(it);
		it = _srv_sockets.begin();
	}
	for (it_clifd_srv = _cli_srvs.begin(); it_clifd_srv != _cli_srvs.end(); it_clifd_srv++) {
		it_clifd_srv->second->deleteClients();
		close(it_clifd_srv->first);
	}
	_cli_srvs.clear();
	close(_epoll_fd);

	for (it_clifd_srv = _sock_srvs.begin(); it_clifd_srv != _sock_srvs.end(); it_clifd_srv++) {
		close(it_clifd_srv->second->getSocketFd());
		it_clifd_srv->second->deleteClients();
		delete it_clifd_srv->second;
	}
	_sock_srvs.clear();
}

std::map<int, Server *>::iterator HttpServerManager::deleteClient(int client_fd) {
	std::map<int, Server *>::iterator it = _cli_srvs.find(client_fd);
	if (it != _cli_srvs.end()){
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
		_cli_srvs[client_fd]->deleteClient(client_fd);
		_cli_srvs.erase(it);
	}
	return it;
}

int HttpServerManager::manageIdleClients(struct epoll_event *events, int nfds) {
	std::map<int, Server*>::iterator it ;
	std::map<int, Server*>::iterator it_event ;
	for (int i = 0; i < nfds; i++) {
		it_event = _cli_srvs.find(events[i].data.fd);
		if (it_event != _cli_srvs.end() && it_event->second->hasClientTimedOut(it_event->first)) {
			Logger::log(Logger::INFO, "HttpServerManager.cpp", "has client_fd: "+ to_string(it_event->first)+" timed out");
				deleteClient(it_event->first);
				events[i].data.fd = 0;
				events[i].events = 0;
				nfds--;
		}
	}
	for (it = _cli_srvs.begin(); it != _cli_srvs.end(); ) {
		if (it->second->hasClientTimedOut(it->first)) {
			deleteClient(it->first);
			it = _cli_srvs.begin();
			Logger::log(Logger::INFO, "HttpServerManager.cpp", "has client_fd: "+ to_string(it->first)+" timed out");
		} else {
			it++;
		}
	}
	return nfds;
}

void HttpServerManager::handle_epoll()
{
	struct epoll_event events[_max_events];
	while (true) {
		int nfds = epoll_wait(_epoll_fd, events, _max_events, 3000);
		if (nfds == -1) {
			Logger::log(Logger::ERROR,"HttpServerManager.cpp", "epoll_wait failed");
			stop();
			return ;
		}
		Logger::log(Logger::INFO,"HttpServerManager.cpp", "Number of events received: " + to_string(nfds) + ", Clients conected: " + to_string(_cli_srvs.size()));

		nfds = manageIdleClients(events, nfds);
		for (int i = 0; i < nfds; ++i) {

			Logger::log(Logger::INFO, "HttpServerManager.cpp", "Manage fd: " + to_string(events[i].data.fd));
			std::map<int, Server*>::iterator it_srv = _sock_srvs.find(events[i].data.fd); 
			std::map<int, Server *>::iterator it_cli = _cli_srvs.find(events[i].data.fd);

			if (it_srv != _sock_srvs.end()) {
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "New incoming connection detected by server host:port " + it_srv->second->get_server_name() + ":"+ to_string(it_srv->second->getPort()));
				try {
					std::pair<Server*, int > srv_clifd = it_srv->second->accept_connections(_epoll_fd);
					if (srv_clifd.second < 0) // Couldnt accept connection
						continue ;
					_cli_srvs[srv_clifd.second] = srv_clifd.first;
					Logger::log(Logger::INFO,"HttpServerManager.cpp", "Connection accepted successfully: new client_fd: " + to_string(srv_clifd.second));
				} catch (std::exception &e) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to accept connection");
				}
			}
			else if (it_cli != _cli_srvs.end() && ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))) {
				Logger::log(Logger::ERROR,"HttpServerManager.cpp", "Failed event");
				deleteClient(it_cli->first);
			}
			else if (it_cli != _cli_srvs.end() &&  (events[i].events & EPOLLOUT))
			{
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "Handling output client with FD: " + to_string(events[i].data.fd));
				int code_res = it_cli->second->handle_output_client(it_cli->first);
				if (code_res < 0){
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling output client with FD: " + to_string(it_cli->first));
					deleteClient(it_cli->first);
					continue ;
				}
				if (set_event_action(it_cli->first, EPOLLIN) < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error set event action EPOLLIN to client with FD: " + to_string(it_cli->first));
					deleteClient(it_cli->first);
					continue;
				}
				if (code_res)
					deleteClient(it_cli->first);
			}
			else if (it_cli != _cli_srvs.end() && (events[i].events & EPOLLIN))
			{
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "Handling input client_fd: " + to_string(events[i].data.fd));
				int result = it_cli->second->handle_input_client(it_cli->first);
				Client* client = it_cli->second->get_client(it_cli->first);
				if (result < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling input client with: " + to_string(it_cli->first));
					deleteClient(it_cli->first);
					continue ;
				} else if (result == 1) {
					Logger::log(Logger::INFO,"HttpServerManager.cpp", "Conection close by client_fd: " + to_string(it_cli->first));
					deleteClient(it_cli->first);
					continue ;
				}
				if (client && client->get_request().get_state() == 3 && set_event_action(it_cli->first, EPOLLOUT) < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error set event action EPOLLOUT to client with FD: " + to_string(it_cli->first));
					deleteClient(it_cli->first);
				}
			}
			else {
				Logger::log(Logger::ERROR,"HttpServerManager.cpp", "epoll error.");
				stop();
				return ;
			}
		}
	}
}

int HttpServerManager::create_socket_fd(int port) {
	int opt = 1;
	struct sockaddr_in addr;
	int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (socket_fd == -1) {
		perror("Error al crear el socket");
		return (-1);
	}
	Logger::log(Logger::INFO,"HttpServerManager.cpp", "Created socket_fd: " + to_string(socket_fd));

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Fail to bind socket_fd: " + to_string(socket_fd));
		close(socket_fd);
		return (-1);
	}
	if (listen(socket_fd, MAX_CLIENTS) < 0) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Fail to listen socket_fd: " + to_string(socket_fd));
		close(socket_fd);
		return (-1);
	}
	Logger::log(Logger::DEBUG,"HttpServerManager.cpp", "Max-Clients: " + to_string(MAX_CLIENTS) + ", socket_fd: " + to_string(socket_fd));
	return socket_fd;
}

int HttpServerManager::set_event_action(int client_fd, uint32_t action)
{
	struct epoll_event ev;
	ev.events = action | EPOLLET;
	ev.data.fd = client_fd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to register client socket with epoll " + to_string(_epoll_fd) + ", client_fd: " + to_string(client_fd));
		return (-1);
	}
	return 0;
}
