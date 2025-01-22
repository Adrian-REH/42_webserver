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
#include "utils/endsWith.hpp"
#include "utils/startsWith.hpp"
#include <dirent.h> // Para opendir, readdir, closedir

std::vector<std::string> get_all_dirs(const char *dir_path ) {
    std::vector<std::string> dirs;
    // Abrir el directorio
    DIR* dir = opendir(dir_path);
    if (dir == NULL) {
        std::cerr << "Error al abrir el directorio" << std::endl;
        return dirs;
    }
    
    // Leer entradas del directorio
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignorar las entradas "." y ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            std::string d_name(entry->d_name);
            dirs.push_back(d_name);
        }
    }
    
    // Cerrar el directorio
    closedir(dir);
    return dirs;
}
class Location {
    private:
        std::string _path;
        std::vector<std::string> _allowed_methods;  // GET, POST, etc.
        std::string _redirect_url;  // Si hay redirección, ejemplo: http://localhost/new-page
        std::string _index;//El archivo base que se ejecutara en caso de que me den solo _root_directory
        std::vector<std::string> _files; // Todos los archivos con la _type_extension.
        std::string _root_directory;  // Directorio o archivo que se sirve.
        bool _autoindex;  // Para listar directorios.
        int _client_max_body_size;  // Tamaño máximo del cuerpo de la solicitud (si se aplica).
        std::string _path_upload_directory;  // Para subir archivos.
    public:
        Location() {}
        Location &set_root_directory(const std::string &root) {
            _root_directory = root;
            return *this;
        }
        Location &set_index(const std::string &index) {
            _index = index;
            return *this;
        }
        Location build() {
            if (_root_directory.empty())
                throw std::runtime_error("Error no existe un directorio root");
            _files = (get_all_dirs(_root_directory.c_str() + 1)); // debe moverse +1 porque precisa  archivos sin inicio '/'
            return *this;
        }
        std::string findScriptPath(std::string & script_path) {
            std::cout << script_path<< ":" << _root_directory<< std::endl;
            if (ends_with(script_path, _index))
                return script_path;
            else if (script_path == _root_directory)
            {
                script_path.append("/");
                script_path.append(_index);
                return script_path;
            }
            else if (starts_with(script_path, _root_directory)){
                //Busco si script_path coincide con algun archivo ejecutable dentro de _root_directory
                //TODO: Verificar si inicia con _root_directory
                
                std::vector<std::string>::iterator it;
                for (it = _files.begin(); it != _files.end(); it++)
                    if (ends_with(script_path, *it))
                        return script_path;
                throw std::runtime_error("Not found file, by path indicated in start line");
            }
            return "";
        }
};

class Server {
private:
    int server_fd;
    int epoll_fd;
    struct sockaddr_in address;
    int _opt;
    int _port;
    int _max_clients;
    std::map<int, Client*> clients;
    std::vector<Location> _locations;
    //std::map<std::string, std::string> _locations;
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
            std::vector<Location>::iterator it;

            //Si me envian un script_path: /cgi-php/ lo busco en locations y verifico a que index pertenece
            for (it = _locations.begin(); it != _locations.end(); it++)
                script_path = it->findScriptPath(script_path);
   
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
    Server(int port = 8080, int opt = 1, int max_clients = 10 ) : _port(port), _opt(opt), _max_clients(max_clients){
    }
    
    Server &set_port(const int &port) {
        _port=port;
        return *this;
    }
    Server &addLocation(const Location &location) {
        _locations.push_back(location);
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
        address.sin_port = htons(_port);

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
        std::cout << "Listen from: " << "INADDR_ANY" << ":" << _port << std::endl;
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