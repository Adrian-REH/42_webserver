#ifndef CGI_HPP
#define CGI_HPP

#include <cstring>
#include <sys/wait.h>
#include <map>
#include "utils/Utils.hpp"
#include "Logger.hpp"
#include "HttpException.hpp"
#include "Request.hpp"
#include "Cookie.hpp"

/**
 * @brief Clase para gestionar la ejecución de scripts CGI.
 * 
 * La clase `CGI` permite configurar y ejecutar scripts CGI en el servidor, 
 * determinando el intérprete adecuado según el tipo de archivo.
 */
class CGI {
	private:
		std::string _working_dir;
		std::string _script_path;
		Request _request;
		Cookie _cookie;
		char** _env;
		size_t _exec_timeout;
		size_t _exec_time;
		std::string _interpreter;
		int	_status_code;
		pid_t _pid;
		int _cgi_fd;
		int _status;

	public:
		CGI(){}
		/**
		 * @brief Constructor de la clase CGI.
		 * 
		 * @param script_path Ruta al script a ejecutar.
		 * @param method Método HTTP asociado al script.
		 * @param body Cuerpo de la solicitud HTTP.
		 * @param env Variables de entorno para la ejecución.
		 */
		CGI(const std::string& working_dir, const std::string& script_path, Request request, Cookie cookie = Cookie(), char** env = NULL, size_t exec_timeout = 5);
		~CGI();
		/**
		 * @brief Determina el intérprete adecuado para el script según su extensión.
		 * 
		 * @return Ruta al ejecutable del intérprete correspondiente.
		 * @throws std::runtime_error Si el tipo de script no está soportado.
		 */
		std::string determine_interpreter() const;

		int resolve_cgi_env(Request, std::string);
		int cgi_kill();


		/**
		 * @brief Ejecuta el script CGI y devuelve su salida.
		 * 
		 * Crea un proceso hijo que ejecuta el script CGI utilizando el intérprete correspondiente.
		 * Captura la salida del script y la devuelve como una respuesta HTTP.
		 * 
		 * @return Respuesta HTTP generada por el script CGI.
		 * @throws std::runtime_error Si falla la creación del pipe o el fork.
		 */
		void execute();
		int istimeout();
		void verify_timeout();
		std::string resolve_response();
		int get_pfd() const;
		int get_pid() const;
		Cookie get_cookie() const;
		int get_status_code();
};

#endif
