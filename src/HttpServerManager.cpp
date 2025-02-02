#include "Server.hpp"
#include <list>
#define MAX_CLIENTS 30
#include "HttpServerManager.hpp"
#include "Logger.hpp"

HttpServerManager::HttpServerManager() {}

int HttpServerManager::start(std::vector<Server> srvs) {
	struct epoll_event ev;
	std::vector<Server>::iterator it;
	int socket_fd;
	_max_events = 0;
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		Logger::log(Logger::ERROR,"HttpServerManager.cpp", "Error al crear epoll");
		std::vector<int>::iterator it;
		for (it = _srv_sockets.begin(); it != _srv_sockets.end(); it++)
			close(*it);
		return -1;
	}
	for (it = srvs.begin(); it != srvs.end(); it++) {
		socket_fd = create_socket_fd(it->getPort());
		if (socket_fd < 0) {
			Logger::log(Logger::WARN,"HttpServerManager.cpp", "Invalid create socket_fd to port: " + to_string(it->getPort()));
			continue ;
		}
		it->setSocketFd(socket_fd);
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
			Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to register server socket with epoll: " + to_string(_epoll_fd) + ", socket_fd: " + to_string(socket_fd));
			close(socket_fd);
			continue ;
		}
		Logger::log(Logger::INFO,"HttpServerManager.cpp", "Server socket registered with epoll");

		_sock_srvs[socket_fd] = *it;
		_max_events += MAX_CLIENTS;
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
	for (it = _srv_sockets.begin(); it != _srv_sockets.end(); it++)
		close(*it);
	close(_epoll_fd);
}


void HttpServerManager::handle_epoll()
{
	struct epoll_event events[_max_events];
	while (true) {
		int nfds = epoll_wait(_epoll_fd, events, _max_events, -1);
		if (nfds == -1) {
			Logger::log(Logger::ERROR,"HttpServerManager.cpp", "epoll_wait failed");
			stop();
			return ;
		}
		
		for (int i = 0; i < nfds; ++i) {
		Logger::log(Logger::INFO,"HttpServerManager.cpp", "Number of events received: " + nfds);
			//TODO: comprobar error control antes que todo
			std::map<int, Server>::iterator it = _sock_srvs.find(events[i].data.fd); 
			if (it != _sock_srvs.end()) {
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "New incoming connection detected");
				if (accept_connections(it->second)) {
					Logger::log(Logger::INFO,"HttpServerManager.cpp", "Connection accepted successfully");
				} else {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to accept connection");
				}
			}
			else if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				Logger::log(Logger::ERROR,"HttpServerManager.cpp", "Failed event");
				close(events[i].data.fd);
				_cli_srvs.erase(events[i].data.fd);
			}
			else if ((events[i].events & EPOLLOUT) )//&& events[i].data.fd != server_fd)
			{
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "Handling output client with FD: " + events[i].data.fd);
				if (_cli_srvs[events[i].data.fd].handle_output_client(events[i].data.fd) < 0){
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling output client with FD: " + events[i].data.fd);
					close(events[i].data.fd);
					_cli_srvs.erase(events[i].data.fd);
				}
				if (set_event_action(events[i].data.fd, EPOLLIN) < 0){
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error set event action EPOLLIN to client with FD: " + events[i].data.fd);
					close(events[i].data.fd);
					_cli_srvs.erase(events[i].data.fd);
				}
			}
			else if ((events[i].events & EPOLLIN))// && events[i].data.fd != server_fd)
			{
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "Handling input client with FD: " + events[i].data.fd);
				if (_cli_srvs[events[i].data.fd].handle_input_client(events[i].data.fd) < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling input client with: " + events[i].data.fd);
					close(events[i].data.fd);
					_cli_srvs.erase(events[i].data.fd);
				}
				if (set_event_action(events[i].data.fd, EPOLLOUT) < 0){
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error set event action EPOLLOUT to client with FD: " + events[i].data.fd);
					close(events[i].data.fd);
					_cli_srvs.erase(events[i].data.fd);
				}
			}
			else{
				std::cerr << "[ERROR] epoll error" << std::endl;
				return ;
			}
		}
	}
}

int HttpServerManager::create_socket_fd(int port) {
	int opt = 1;
	struct sockaddr_in addr;
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("Error al crear el socket");
		return (-1);
	}
	Logger::log(Logger::INFO,"HttpServerManager.cpp", "Created socket_fd: " + socket_fd);

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Fail to bind socket_fd: " + socket_fd);
		close(socket_fd);
		return (-1);
	}
	if (listen(socket_fd, MAX_CLIENTS) < 0) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Fail to listen socket_fd: " + socket_fd);
		close(socket_fd);
		return (-1);
	}
	Logger::log(Logger::DEBUG,"HttpServerManager.cpp", "Max-Clients: " + to_string(MAX_CLIENTS) + ", socket_fd: " + to_string(socket_fd));
	return socket_fd;
}

bool HttpServerManager::accept_connections(Server& srv) {
	struct sockaddr_in client_address;
	struct epoll_event ev;
	socklen_t client_len = sizeof(client_address);
	int client_fd = accept(srv.getSocketFd(), (struct sockaddr *)&client_address, &client_len);
	if (client_fd == -1) {
		Logger::log(Logger::ERROR,"HttpServerManager.cpp", "Error al aceptar la conexión");
		return 0;
	}
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = client_fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

	_cli_srvs[client_fd] = srv.addClient(Client(client_fd));
	return 1;
}

int HttpServerManager::set_event_action(int client_fd, uint32_t action)
{
	struct epoll_event ev;
	ev.events = action | EPOLLET;
	ev.data.fd = client_fd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to register client socket with epoll" + to_string(_epoll_fd) + ", client_fd: " + to_string(client_fd));
		return (-1);
	}
	return 0;
}
