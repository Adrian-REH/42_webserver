#ifndef LOCATION_HPP
#define LOCATION_HPP
#include <string>
#include <vector>
#include "LimitExcept.hpp"

//TODO: extract method to hpp cpp
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

		std::string get_path() const {
			return _path;
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
			//FIXME: En caso de que no haya literalmente un root_dir es necesario que sea resiliente asi que propongo poner "/"
				throw std::runtime_error("Error: no existe un directorio root");
			_files = (get_all_dirs(_root_directory.c_str() + 1)); // Adjust for directories without leading '/'
			return *this;
		}
		// Helper function to build the full path.
		std::string buildFullPath(const std::string &root, const std::string &filename) {
			std::string result;
			if (!starts_with(root, "/"))
				result = "/";
			result.append(root);
			if (!ends_with(result, "/"))
				result.append("/");
			result.append(filename);
			return result;
		}

		/**
		 * @brief Resolves the full script path based on the given request path.
		 * 
		 * This function matches the provided `path` against the configured `_path`, `_index`,
		 * and `_files` in the `Location` object to determine the full path of the requested file.
		 * 
		 * @param path The request path to resolve.
		 * @return The full path to the script if successfully resolved.
		 * @throws std::runtime_error If no matching file is found.
		 */
		std::string findScriptPath(const std::string &path) {
   
			// Case 1: Path ends with the configured index file.
			if (ends_with(path, _index)) {
				return path;
			}

			// Case 2: Path exactly matches the configured location path (_path).
			if (path == _path) {
				return buildFullPath(_root_directory, _index);
			}

			// Case 3: Path starts with _path or vice versa (_path starts with path).
			if (starts_with(path, _path) || starts_with(_path, path)) {
				
				// Handle case where path matches _path with a trailing slash.
				std::string adjustedPath = path + "/";
				if (adjustedPath == _path) {
					return buildFullPath(_root_directory, _index);
				}

				// Check for a matching file in _files.
				for (std::vector<std::string>::iterator it = _files.begin(); it != _files.end(); ++it) {
					if (ends_with(path, *it)) {
						return buildFullPath(_root_directory, *it);
					}
				}

				// Throw error if no matching file is found.
				throw std::runtime_error("Not found file, by path indicated in start line");
			}

			// Default case: Return an empty string if no conditions are met.
			return "";
		}

};


#endif