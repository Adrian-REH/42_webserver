#include "Server.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "HttpException.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#define check(expr) if (!(expr)) { perror(#expr); kill(0, SIGTERM); }

Server::Server(int port, size_t max_clients, std::string server_name) : _port(port), _max_clients(max_clients), _server_name(server_name), _clients() { //,_max_clients(max_clients), _env_len(0){
}

Server &Server::set_port(const size_t port) {

	_port=port;
	return *this;
}

Server &Server::setSocketFd(const int sock_fd) {
	_socket_fd = sock_fd;
	return *this;
}
Server &Server::set_server_name(const std::string &server_name) {
	_server_name = server_name;
	return *this;
}

Server &Server::setMaxClients(const int max_cients) {
	_max_clients = max_cients;
	return *this;
}


Server &Server::addClient(const Client &cli) {
	_clients[cli.get_socket_fd()] = new Client(cli);
	return *this;
}


int Server::getSocketFd() const{
	return _socket_fd;
}

int Server::getPort() const{
	return _port;
}

int Server::getMaxClients() const{
	return _max_clients;
}

std::string Server::get_server_name() const {
	return _server_name;
}


void Server::deleteClient(const int client_fd) {
	Logger::log(Logger::DEBUG, "Server.cpp", "Deleting client_fd: "+ to_string(client_fd));

	std::map<int, Client *>::iterator it=  _clients.find(client_fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
	}
}

void Server::deleteClients() {
	Logger::log(Logger::DEBUG, "Server.cpp", "Deleting all clients");

	std::map<int, Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end() ; it++) {
		delete it->second;
	}
	_clients.clear();
}

std::pair<std::string, std::string> client_info(struct sockaddr_in client_address) {
	char client_ip[INET_ADDRSTRLEN];  // Buffer for the IP
	inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);

	int client_port = ntohs(client_address.sin_port); // Converts to a legible format

	std::string ip(client_ip);
	return std::make_pair(ip, to_string(client_port));
}
void Server::enable_keepalive(int sock) {
	ServerConfig srv_conf = Config::getInstance().getServerConfByPort(_port);
    int yes = srv_conf.get_timeout();
    check(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) != -1);

    int idle = 1;
    check(setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) != -1);

    int interval = 1;
    check(setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) != -1);

    int maxpkt = srv_conf.get_max_req();
    check(setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int)) != -1);
}
std::pair<Server*, int> Server::accept_connections(int epoll_fd) {
	struct sockaddr_in client_address;
	struct epoll_event ev;
	socklen_t client_len = sizeof(client_address);
	int client_fd = accept(_socket_fd, (struct sockaddr *)&client_address, &client_len);
	if (client_fd == -1) {
		Logger::log(Logger::WARN, "Server.cpp", "Error accepting conection");
		return std::make_pair(this, -1);
	}
	std::pair<std::string, std::string> data_cli = client_info(client_address);
	size_t n_clients = _clients.size();
	if (n_clients >= _max_clients) {
		Logger::log(Logger::ERROR, "Server.cpp", "Server is full!, n_clients: " + to_string(n_clients) + ", max_clients: " + to_string(_max_clients));
		Logger::log(Logger::DEBUG, "Server.cpp", "The client was rejected: IP: " + data_cli.first + "Port + " + data_cli.second );
		if (client_fd != -1) {
			// Sends response Retry-After 5 seg
			send(client_fd, "HTTP/1.1 503 Service Unavailable\r\nRetry-After: 5\r\n\r\n", 56, 0);
			close(client_fd);
		}
		return std::make_pair(this, -1);
	}
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	fcntl(client_fd, F_SETFD, FD_CLOEXEC);
	enable_keepalive(client_fd);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = client_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
	_clients[client_fd] = new Client(client_fd);
	_clients[client_fd]->set_ip(data_cli.first);
	_clients[client_fd]->set_port(data_cli.second);

	Logger::log(Logger::DEBUG, "Server.cpp", "The client was acepted: IP:PORT " + data_cli.first + ":" + data_cli.second + " fd: " + to_string(client_fd));
	return std::make_pair(this, client_fd);
}


Client* Server::get_client(int client_fd) {
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	Client* client = NULL;
	if (it != _clients.end())
		client = it->second;
	return client;
}

int Server::handle_input_client(int client_fd) {
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	Client* client = NULL;
	if (it != _clients.end())
		client = it->second;
	else {
		Logger::log(Logger::ERROR,"Server.cpp", "Client for FD" + to_string(client_fd) + " doesn't exist.");
		return -1;
	}


	ServerConfig srv_conf = Config::getInstance().getServerConfByPort(_port);

	if (client->handle_request(srv_conf) < 0) {
		return -1;
	}

	return 0;
}

int Server::handle_output_client(int client_fd) {
	Client* client = _clients[client_fd];
	Logger::log(Logger::INFO,"Server.cpp", "Executing read and send request for client_fd: " + to_string(client_fd));
	try {
		Config& conf = Config::getInstance();
		client->handle_response(conf.getServerConfByPort(_port));
		return client->should_close();
	} catch (HttpException::ForbiddenException &e) {
		client->send_error(403, "Forbidden");
		std::string val(e.what());
		Logger::log(Logger::ERROR,"Server.cpp",  e.what());
		return -1;
	} catch (HttpException::NotFoundException &e) {
		client->send_error(404, "Not Found");
		std::string val(e.what());
		Logger::log(Logger::ERROR,"Server.cpp",  e.what());
		return -1;
	} catch (HttpException::InternalServerErrorException &e) {
		client->send_error(500, "Internal Server Error");
		std::string val(e.what());
		Logger::log(Logger::ERROR,"Server.cpp",  e.what());
		return -1;
	} catch (std::exception &e) {
		client->send_error(500, "Internal Server Error");
		std::string val(e.what());
		Logger::log(Logger::ERROR,"Server.cpp",  e.what());
		return -1;
	}
	return 0;
}
