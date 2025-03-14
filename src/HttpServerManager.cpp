#include "Server.hpp"
#include <list>
#define MAX_CLIENTS 30
#include "HttpServerManager.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "CGIManager.hpp"
#include "ClientManager.hpp"
#include "ServerManager.hpp"

HttpServerManager::HttpServerManager(): _srv_sockets(0), _epoll_fd(0),_max_events(0) {}


int HttpServerManager::start() {
	
	ServerManager& srv_m= ServerManager::getInstance();
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
		srv_m.save_server_type(socket_fd, 0, srv);
		//_sock_srvs[socket_fd] = srv;
		_max_events += srv->getMaxClients();
	}
	if (srv_m.get_sock_srvs().empty()) {
		Logger::log(Logger::WARN,"HttpServerManager.cpp", "Number of server is 0");
		return -1;
	}
	srv_m.set_epoll_fd(_epoll_fd);
	handle_epoll();
	return 0;
}

void HttpServerManager::stop() {
	ServerManager& srv_m= ServerManager::getInstance();
	std::vector<int>::iterator it;
	std::map<int, Server*> ::iterator it_clifd_srv;
	for (it = _srv_sockets.begin(); it != _srv_sockets.end(); it++){
		close(*it);
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, *it, NULL);
		_srv_sockets.erase(it);
		it = _srv_sockets.begin();
	}
	srv_m.clear_clis(_epoll_fd);
	srv_m.clear_srvs(_epoll_fd);
	close(_epoll_fd);
}





void HttpServerManager::handle_epoll()
{
	struct epoll_event events[_max_events];
	ServerManager& srv_m = ServerManager::getInstance();
	while (true) {
		int nfds = epoll_wait(_epoll_fd, events, _max_events, 3000);
		if (nfds == -1) {
			Logger::log(Logger::ERROR,"HttpServerManager.cpp", "epoll_wait failed");
			stop();
			return ;
		}
		Logger::log(Logger::INFO,"HttpServerManager.cpp", "Number of events received: " + to_string(nfds) + ", Clients conected: " + to_string(srv_m.get_clis_srvs().size()));

		nfds = srv_m.manageIdleClients(events, nfds, _epoll_fd);
		for (int i = 0; i < nfds; ++i) {

			std::pair<int, Server *> srv_type =  srv_m.find_server_type(events[i].data.fd);
			Logger::log(Logger::INFO, "HttpServerManager.cpp", "Manage fd: " + to_string(events[i].data.fd) + ", type: " + to_string(srv_type.first));
			
			if (srv_type.first == 0) {
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "New incoming connection detected by server host:port " + srv_type.second->get_server_name() + ":"+ to_string(srv_type.second->getPort()));
				try {
					std::pair<Server*, int > srv_clifd = srv_type.second->accept_connections(_epoll_fd);
					if (srv_clifd.second < 0) // Couldnt accept connection
						continue ;
					srv_m.save_server_type(srv_clifd.second, 1 ,srv_clifd.first);
					Logger::log(Logger::INFO,"HttpServerManager.cpp", "Connection accepted successfully: new client_fd: " + to_string(srv_clifd.second));
				} catch (std::exception &e) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Failed to accept connection");
				}
			}
			else if ((srv_type.first == 1 || srv_type.first == 2) && ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))) {
				Logger::log(Logger::ERROR,"HttpServerManager.cpp", "Failed event");
				srv_m.delete_client(events[i].data.fd, _epoll_fd);
			}
			else if (srv_type.first == 1 &&  (events[i].events & EPOLLOUT))
			{
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "Handling output client with FD: " + to_string(events[i].data.fd));
				int code_res = srv_type.second->handle_output_client(events[i].data.fd);
				if (code_res < 0){
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling output client with FD: " + to_string(events[i].data.fd));
					srv_m.delete_client(events[i].data.fd, _epoll_fd);
					continue ;
				}
				if (set_event_action(events[i].data.fd, EPOLLIN) < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error set event action EPOLLIN to client with FD: " + to_string(events[i].data.fd));
					srv_m.delete_client(events[i].data.fd, _epoll_fd);
					continue;
				}
				if (code_res)
					srv_m.delete_client(events[i].data.fd, _epoll_fd);
			}
			else if (srv_type.first == 1 && (events[i].events & EPOLLIN))
			{
				Logger::log(Logger::INFO,"HttpServerManager.cpp", "Handling input client_fd: " + to_string(events[i].data.fd));
				int result = srv_type.second->handle_input_client(events[i].data.fd);
				if (result < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling input client with: " + to_string(events[i].data.fd));
					srv_m.delete_client(events[i].data.fd, _epoll_fd);
					continue ;
				} else if (result == 1) {
					Logger::log(Logger::INFO,"HttpServerManager.cpp", "Conection close by client_fd: " + to_string(events[i].data.fd));
					srv_m.delete_client(events[i].data.fd, _epoll_fd);
					continue ;
				}
				Client* client = srv_type.second->get_client(events[i].data.fd);
				if (client && client->get_request().get_state() == 3 && set_event_action(events[i].data.fd, EPOLLOUT) < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error set event action EPOLLOUT to client with FD: " + to_string(events[i].data.fd));
					srv_m.delete_client(events[i].data.fd, _epoll_fd);
				}
			}
			else if (srv_type.first == 2 && (events[i].events & EPOLLIN)){
				int code_res = srv_type.second->handle_output_cgi(events[i].data.fd);
				if (code_res < 0) {
					Logger::log(Logger::WARN,"HttpServerManager.cpp", "Error handling cgi resolve client with: " + to_string(events[i].data.fd));
					srv_m.delete_cli_by_cgi(events[i].data.fd, _epoll_fd);
					continue ;
				} else if (code_res == 1) {
					Logger::log(Logger::INFO,"HttpServerManager.cpp", "Conection close by client_fd: " + to_string(events[i].data.fd));
					srv_m.delete_cli_by_cgi(events[i].data.fd, _epoll_fd);
					continue ;
				}
				if (code_res)
					srv_m.delete_cli_by_cgi(events[i].data.fd, _epoll_fd);
				srv_m.delete_cgi(events[i].data.fd, _epoll_fd);
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
