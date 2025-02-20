#ifndef PARSERSERVER_HPP
#define PARSERSERVER_HPP

#include "Server.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <set>
#include <cstdlib>  // Para strtol y strtoul

template <typename T, class Class>
struct SetterType {
	typedef Class &(Class::*Type)(T);
};

template <typename Class>
struct SetterStorage {
	Class& (Class::*intSetter)(int);
	Class& (Class::*sizeTSetter)(size_t);
	Class& (Class::*stringSetter)(const std::string &);
};


// Estructura que almacena un puntero a función miembro genérico
template <typename Class>
class Setter {
	public:
		// Plantilla de SetterType para obtener el tipo de la función
		template <typename T>
		struct SetterType {
			typedef void (Class::*Type)(T);
		};
	    SetterStorage<Class> setter;

		enum Type { INT, SIZE_T, STRING, BOOL } type;

		Setter() : type(INT) {}
	
		// Método para asignar un setter a una función miembro
		void setSetter(Class& (Class::*method)(int), Type t) {
			setter.intSetter = method;
			type = t;
		}
		void setSetter(Class& (Class::*method)(size_t), Type t) {
			setter.sizeTSetter = method;
			type = t;
		}
		void setSetter(Class& (Class::*method)(const std::string &), Type t) {
			setter.stringSetter = method;
			type = t;
		}
	
		// Método para ejecutar el setter
		void execute(Class &srv, std::string val) {
			//srv.set_port(8080);

			char *endp = NULL;
			int resultInt = 0;
			size_t resultSizeT = 0;
			switch (type) {
				case Setter::INT:
					resultInt = static_cast<int>(std::strtod(val.c_str(), &endp));
					(void)resultInt;
					if (setter.intSetter)
						(srv.*setter.intSetter)(resultInt);
					break;
				case Setter::STRING:
					if (setter.stringSetter)
						(srv.*setter.stringSetter)(val);
					break;
				case Setter::SIZE_T:
					resultSizeT = static_cast<size_t>(std::strtod(val.c_str(), &endp));
					if (setter.sizeTSetter)
						(srv.*setter.sizeTSetter)(resultSizeT);
					break;
				default:
					break;
			}
		}
	};

class ParserServer {

	private:
		const char *_file_name;
		std::deque<std::string> _content_file;

		LimitExcept parseLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);
		Location parseLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);
		Server parseServer(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);

		std::map<std::string, Setter<Server> > _automata_srv;
		std::map<std::string, Setter<Location > > _automata_loc;
		std::map<std::string, Setter<LimitExcept> > _automata_limexc;
		
	public:
		void init_automata();
		ParserServer(const char *file_name = "ws.conf");
		int dumpRawData(const char *file_name);
		/**
		 * @brief Busco en el archivo la configuracion necesaria para Server
		 */
		std::vector<Server> execute(char **env);
};
#endif