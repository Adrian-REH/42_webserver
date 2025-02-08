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
		Location();
		/**
		* @brief Set the root directory for this location.
		* 
		* This function assigns the root directory path to be used for the current location.
		* 
		* @param root The absolute or relative path to the root directory.
		* @return A reference to the current Location object for method chaining.
		*/
		Location &set_root_directory(const std::string &root);
		/**
		 * @brief Set the default index file for this location.
		 * 
		 * This function specifies the index file to be served when a directory is requested.
		 * 
		 * @param index The name of the index file (e.g., "index.html").
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_index(const std::string &index);

		/**
		 * @brief Set the limit_except configuration for this location.
		 * 
		 * This function assigns the allowed HTTP methods and corresponding rules (e.g., deny all).
		 * 
		 * @param lim A `LimitExcept` object containing the HTTP method restrictions.
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_limit_except(const LimitExcept &lim);

		/**
		 * @brief Set the path for this location.
		 * 
		 * This function defines the URL path for which this location configuration applies.
		 * 
		 * @param path The path associated with this location (e.g., "/images").
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_path(const std::string &path);

		/**
		 * @brief Set the path for this location.
		 * 
		 * This function defines the URL path for which this location configuration applies.
		 * 
		 * @param path The path associated with this location (e.g., "/images").
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_auto_index(const bool &auto_index);

		/**
		 * @brief Set the upload directory for this location.
		 * 
		 * This function specifies the path where files will be uploaded for this location.
		 * 
		 * @param str The path to the upload directory (absolute or relative).
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_path_upload_directory(const std::string &str);

		/**
		 * @brief Set the maximum size for the client's request body.
		 * 
		 * This function defines the maximum size (in bytes) for the request body that the client can send to this location.
		 * 
		 * @param cli_max_body_size The maximum body size allowed for client requests.
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_client_max_body_size(const int &cli_max_body_size);

		/**
		 * @brief Set the redirect URL for this location.
		 * 
		 * This function specifies the URL to which requests for this location will be redirected.
		 * 
		 * @param str The redirect URL (e.g., "https://example.com").
		 * @return A reference to the current Location object for method chaining.
		 */
		Location &set_redirect_url(const std::string &str);

		std::string get_path() const;

		int get_auto_index() const;

		std::vector<std::string> get_files() const;
		/**
		 * @brief Finalize the configuration and build the Location object.
		 * 
		 * This function ensures that all mandatory fields are set before the `Location` object can be used. 
		 * It also populates the list of files and directories within the root directory.
		 * 
		 * @throws std::runtime_error If the root directory is not set.
		 * @return A fully configured Location object.
		 */
		Location build();
		
		// Helper function to build the full path.
		std::string buildFullPath(const std::string &root,const std::string path, const std::string &filename);

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
		int findScriptPath(const std::string &url_path, std::string &final_path);

};


#endif