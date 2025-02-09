#ifndef CGI_HPP
#define CGI_HPP

#include <cstring>
#include <sys/wait.h>
#include "utils/Utils.hpp"

/**
 * @brief Clase para gestionar la ejecución de scripts CGI.
 * 
 * La clase `CGI` permite configurar y ejecutar scripts CGI en el servidor, 
 * determinando el intérprete adecuado según el tipo de archivo.
 */
class CGI {
	private:
		std::string _script_path;
		std::string _method;
		std::string _body;
		char** _env;

	public:
		/**
		 * @brief Constructor de la clase CGI.
		 * 
		 * @param script_path Ruta al script a ejecutar.
		 * @param method Método HTTP asociado al script.
		 * @param body Cuerpo de la solicitud HTTP.
		 * @param env Variables de entorno para la ejecución.
		 */
		CGI(const std::string& script_path, const std::string& method, const std::string& body, char** env);
		/**
		 * @brief Determina el intérprete adecuado para el script según su extensión.
		 * 
		 * @return Ruta al ejecutable del intérprete correspondiente.
		 * @throws std::runtime_error Si el tipo de script no está soportado.
		 */
		std::string determine_interpreter() const;

		/**
		 * @brief Ejecuta el script CGI y devuelve su salida.
		 * 
		 * Crea un proceso hijo que ejecuta el script CGI utilizando el intérprete correspondiente.
		 * Captura la salida del script y la devuelve como una respuesta HTTP.
		 * 
		 * @return Respuesta HTTP generada por el script CGI.
		 * @throws std::runtime_error Si falla la creación del pipe o el fork.
		 */
		std::string execute();
};

#endif
