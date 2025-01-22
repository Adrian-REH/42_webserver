#ifndef LOCATION_HPP
#define LOCATION_HPP
#include <string>
#include <vector>
#include "LimitExcept.hpp"

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
        LimitExcept _limit_except;
        std::string _redirect_url;  // Si hay redirección, ejemplo: http://localhost/new-page
        std::string _index;//El archivo base que se ejecutara en caso de que me den solo _root_directory
        std::vector<std::string> _files; // Todos los archivos con la _type_extension.
        std::string _root_directory;  // Directorio o archivo que se sirve.
        bool _auto_index;  // Para listar directorios.
        int _client_max_body_size;  // Tamaño máximo del cuerpo de la solicitud (si se aplica).
        std::string _path_upload_directory;  // Para subir archivos.
    public:
        Location() {}
		/**
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


#endif