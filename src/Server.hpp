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
#include <set>

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
class  LimitExcept {
    private:
        std::set<std::string> _allowedMethods; // Métodos permitidos
        std::string _denyAction; // Acción a tomar si no está permitido
        std::string _logMessage; // Mensaje de log (opcional)
        std::string _authMessage; // Mensaje de autenticación (opcional)

    public:
        LimitExcept() {}
        // Constructor
        LimitExcept(const std::vector<std::string>& methods, const std::string& denyAction = "deny all")
            : _allowedMethods(methods.begin(), methods.end()), _denyAction(denyAction) {}

        // Agregar un método permitido
        LimitExcept addAllowedMethod(const std::string& method) {
            _allowedMethods.insert(method);
            return *this;
        }

        // Configurar acciones de denegación
        LimitExcept setDenyAction(const std::string& action) {
            _denyAction = action;
            return *this;
        }

        // Configurar mensaje de log
        LimitExcept setLogMessage(const std::string& message) {
            _logMessage = message;
            return *this;
        }

        // Configurar mensaje de autenticación
        LimitExcept setAuthMessage(const std::string& message) {
            _authMessage = message;
            return *this;
        }

        // Verificar si un método está permitido
        bool isMethodAllowed(const std::string& method) const {
            return _allowedMethods.find(method) != _allowedMethods.end();
        }

};
class Location {
    private:
        
        std::string _path;
        LimitExcept _limit_except;
        std::string _redirect_url;  // Si hay redirección, ejemplo: http://localhost/new-page
        std::string _index;//El archivo base que se ejecutara en caso de que me den solo _root_directory
        std::vector<std::string> _files; // Todos los archivos con la _type_extension.
        std::string _root_directory;  // Directorio o archivo que se sirve.
        bool _auto_index;  // Para listar directorios.
        int _client_max_body_size;  // Tamaño máximo del cuerpo de la solicitud (si se aplica).
        std::string _path_upload_directory;  // Para subir archivos.
    public:
        Location() {}/**
        * @brief Set the root directory for this location.
        * 
        * This function assigns the root directory path to be used for the current location.
        * 
        * @param root The absolute or relative path to the root directory.
        * @return A reference to the current Location object for method chaining.
        */
        Location &set_root_directory(const std::string &root) {
            _root_directory = root;
            return *this;
        }

        /**
         * @brief Set the default index file for this location.
         * 
         * This function specifies the index file to be served when a directory is requested.
         * 
         * @param index The name of the index file (e.g., "index.html").
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_index(const std::string &index) {
            _index = index;
            return *this;
        }

        /**
         * @brief Set the limit_except configuration for this location.
         * 
         * This function assigns the allowed HTTP methods and corresponding rules (e.g., deny all).
         * 
         * @param lim A `LimitExcept` object containing the HTTP method restrictions.
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_limit_except(const LimitExcept &lim) {
            _limit_except = lim;
            return *this;
        }

        /**
         * @brief Set the path for this location.
         * 
         * This function defines the URL path for which this location configuration applies.
         * 
         * @param path The path associated with this location (e.g., "/images").
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_path(const std::string &path) {
            _path = path;
            return *this;
        }

        /**
         * @brief Set the path for this location.
         * 
         * This function defines the URL path for which this location configuration applies.
         * 
         * @param path The path associated with this location (e.g., "/images").
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_auto_index(const bool &auto_index) {
            _auto_index = auto_index;
            return *this;
        }
        /**
         * @brief Set the upload directory for this location.
         * 
         * This function specifies the path where files will be uploaded for this location.
         * 
         * @param str The path to the upload directory (absolute or relative).
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_path_upload_directory(const std::string &str) {
            _path_upload_directory = str;
            return *this;
        }

        /**
         * @brief Set the maximum size for the client's request body.
         * 
         * This function defines the maximum size (in bytes) for the request body that the client can send to this location.
         * 
         * @param cli_max_body_size The maximum body size allowed for client requests.
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_client_max_body_size(const int &cli_max_body_size) {
            _client_max_body_size = cli_max_body_size;
            return *this;
        }

        /**
         * @brief Set the redirect URL for this location.
         * 
         * This function specifies the URL to which requests for this location will be redirected.
         * 
         * @param str The redirect URL (e.g., "https://example.com").
         * @return A reference to the current Location object for method chaining.
         */
        Location &set_redirect_url(const std::string &str) {
            _redirect_url = str;
            return *this;
        }

        /**
         * @brief Finalize the configuration and build the Location object.
         * 
         * This function ensures that all mandatory fields are set before the `Location` object can be used. 
         * It also populates the list of files and directories within the root directory.
         * 
         * @throws std::runtime_error If the root directory is not set.
         * @return A fully configured Location object.
         */
        Location build() {
            if (_root_directory.empty())
                throw std::runtime_error("Error: no existe un directorio root");
            
            _files = (get_all_dirs(_root_directory.c_str() + 1)); // Adjust for directories without leading '/'
            return *this;
        }
        /**
         * @brief Finds the full script path based on the given request path.
         * 
         * This function resolves the script path for a request, based on the given `path` and the
         * configuration of the `Location` object. It handles cases where the path matches `_index`,
         * `_path`, or starts with `_path`, and attempts to map it to a valid file under `_root_directory`.
         * 
         * @param path The request path to resolve.
         * @return The full path to the script if found.
         * @throws std::runtime_error If the file cannot be resolved based on the request path.
         */
        std::string findScriptPath(std::string &path) {
            std::cout << path << ":" << _path << std::endl;

            // Case 1: If the path ends with the configured index file name, return it directly.
            if (ends_with(path, _index))
                return path;

            // Case 2: If the path exactly matches the configured location path (_path).
            else if ((path == _path)) {
                std::string result;

                // Ensure the root directory starts with a leading '/'.
                if (!starts_with(_root_directory, "/"))
                    result = "/";
                result.append(_root_directory);

                // Ensure the root directory ends with a trailing '/'.
                if (!ends_with(result, "/"))
                    result.append("/");

                // Append the index file name.
                result.append(_index);
                return result;
            }

            // Case 3: If the path starts with the configured location path (_path).
            else if (starts_with(path, _path)) {
                std::vector<std::string>::iterator it;

                // Search for a matching file in the `_files` vector.
                for (it = _files.begin(); it != _files.end(); it++) {
                    if (ends_with(path, *it)) {
                        std::string result;

                        // Ensure the root directory starts with a leading '/'.
                        if (!starts_with(_root_directory, "/"))
                            result = "/";
                        result.append(_root_directory);

                        // Ensure the root directory ends with a trailing '/'.
                        if (!ends_with(result, "/"))
                            result.append("/");

                        // Append the matching file name.
                        result.append(*it);
                        return result;
                    }
                }

                // If no matching file is found, throw an error.
                throw std::runtime_error("Not found file, by path indicated in start line");
            }

            // Default case: Return an empty string if no conditions are met.
            return "";
        }

};

class Server {
private:
    int _port;
    std::string _server_name;
    std::map<int, Client*> clients;
    std::vector<Location> _locations;
    int server_fd;
    int epoll_fd;
    struct sockaddr_in address;
    int _opt;
    int _max_clients;
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
        std::string path = req.get_path(); // Método para obtener el path del script CGI
        std::string method = req.get_method();    // Método para obtener el método HTTP
        std::string body = req.get_body();        // Método para obtener el cuerpo del request
        try {
            std::cout << "Execute CGI" << std::endl;
            std::vector<Location>::iterator it;

            //Si me envian un path: /cgi-php/ lo busco en locations y verifico a que index pertenece
            for (it = _locations.begin(); it != _locations.end(); it++){
                path = it->findScriptPath(path);
                if (!path.empty())
                    break ;
            }
   
            /* Intentar que cuando el path termine con  / se determine utilice el index en ese path en base al parseo.
            * root: /cgi-php/, index: app.php
            * path: /cgi-php/ -> interpreto que el index es app.php
            */
            CGI _cgi(path, method, body, _env); 

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
    Server &set_server_name(const std::string &server_name) {
        _server_name = server_name;
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