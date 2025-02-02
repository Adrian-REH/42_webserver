#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "utils/Utils.hpp"
#include "LimitExcept.hpp"

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
		int get_auto_index() const {
			return _auto_index;
		}
		std::vector<std::string> get_files() const {
			return _files;
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
			return *this;
		}
		
		// Helper function to build the full path.
		std::string buildFullPath(const std::string &root,const std::string path, const std::string &filename) {
			std::string result;
			if (!starts_with(root, "/"))
				result = "/";
			result.append(root);
			if (!ends_with(result, "/"))
				result.append("/");
			result.append(path);
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
		std::string findScriptPath(const std::string &url_path) {
			std::string path = extractStrEnd(url_path, _path);
			std::string file;
			// Case 1: Path ends with the configured index file.
			if (!_index.empty() && ends_with(path, _index))
				return buildFullPath(_root_directory, "", path);
			// Check for a matching file in _files.
			size_t dot_pos = path.rfind('.');
			if ((dot_pos != std::string::npos) && (dot_pos != path.length() - 1)){
				std::string path_tmp = extractStrStart(path, "/");
				file = extractStrStart(path, path_tmp);
				path = path_tmp;
			}
			std::string work_dir = buildFullPath(_root_directory, path, "");

			std::cout << "[DEBUG] Work dir, get files: '" << work_dir <<"'"<< std::endl;
			if (work_dir == "/")
				work_dir = ".";
			const char * dir = work_dir.c_str();
			if (work_dir.length() > 1)
				dir++;
			_files = get_all_dirs(dir); // Adjust for directories without leading '/'
			for (std::vector<std::string>::iterator it = _files.begin(); it != _files.end(); ++it) {
				if (!file.empty() && ends_with(file, *it)) {
					return buildFullPath(_root_directory, path, *it);
				} else if (*it == _index)
					return buildFullPath(_root_directory, path, _index);
			}
			return buildFullPath(_root_directory, path, "");
		}

};


#endif