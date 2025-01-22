#ifndef PARSER_HPP
#define PARSER_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <map>
#include "Client.hpp"
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include "CGI.hpp"



class Server {
private:
    int server_fd;
    int epoll_fd;
    struct sockaddr_in address;
    int _opt;
    int port;
    int _max_clients;
    std::map<int, Client*> clients;
    std::map<std::string, std::string> _locations;
    char **_env;
    void accept_connections() {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
        if (client_fd == -1) {
            perror("Error al aceptar la conexión");
            return;
        }

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

        Client* new_client = new Client(client_fd);
        clients[client_fd] = new_client;
    }
    void handle_client(int client_fd) {
        Client* client = clients[client_fd];
        std::cout << "Receive Request" << std::endl;
        client->receive_data();
        execute(*client);
        std::cout << "End Request" << std::endl;
    }

    void execute(Client &client) {
        Request req = client.get_request();
        std::string script_path = req.get_path(); // Método para obtener el path del script CGI
        std::string method = req.get_method();    // Método para obtener el método HTTP
        std::string body = req.get_body();        // Método para obtener el cuerpo del request
        try {
            std::cout << "Execute CGI" << std::endl;

            //Si me envian un script_path: /cgi-php/ lo busco en locations y verifico a que index pertenece
            std::string index = _locations[script_path];
            if (!index.empty())
                script_path = index;
            /* Intentar que cuando el path termine con  / se determine utilice el index en ese path en base al parseo.
            * root: /cgi-php/, index: app.php
            * path: /cgi-php/ -> interpreto que el index es app.php
            */
            CGI _cgi(script_path, method, body, _env); 

            std::string result = _cgi.execute();

            std::cout << "Send Response" << std::endl;
            // Enviar respuesta al cliente
            client.send_response(result);

        } catch (const std::exception& e) {
            std::cerr << "Error ejecutando CGI: " << e.what() << std::endl;
            client.send_error(500, "Internal Server Error");
        }

    }

public:
    Server(int port = 8080, int opt = 1, int max_clients = 10 ) : port(port), _opt(opt), _max_clients(max_clients){
    }
    Server &addLocation(const std::string &key, const std::string &value) {
        _locations[key] = value;
        return *this;
    }
    void init() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("Error al crear el socket");
            exit(EXIT_FAILURE);
        }

        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(_opt));

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Error al hacer bind");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, _max_clients) < 0) {
            perror("Error al hacer listen");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        std::cout << "Listen from: " << "INADDR_ANY" << ":" << port << std::endl;
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            perror("Error al crear epoll");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        std::cout << "Max-Clients: " << _max_clients << std::endl;
    }

    void start() {
        struct epoll_event ev;
        struct epoll_event events[_max_clients];

        ev.events = EPOLLIN;
        ev.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
            perror("Error al registrar el socket del servidor en epoll");
            close(server_fd);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }
        while (1) {
            int nfds = epoll_wait(epoll_fd, events, _max_clients, -1);
            if (nfds == -1) {
                perror("Error en epoll_wait");
                close(server_fd);
                close(epoll_fd);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < nfds; ++i) {
                if (events[i].data.fd == server_fd) {
                    accept_connections();
                    std::cout << "Accpet Conection" << std::endl;
                } else {
                    std::cout << "Handle Client" << std::endl;
                    handle_client(events[i].data.fd);
                }
            }
        }
    }

    void stop() {
        close(server_fd);
        close(epoll_fd);
    }
};



#endif