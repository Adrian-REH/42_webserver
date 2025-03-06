#ifndef PARSERCONFIG_HPP
#define PARSERCONFIG_HPP

#include "Server.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <set>
#include <cstdlib>  // Para strtol y strtoul
#include "Config.hpp"

template <typename T, class Class>
struct SetterType {
	typedef Class &(Class::*Type)(T);
};

template <typename Class>
struct SetterStorage {
	Class& (Class::*intSetter)(int);
	Class& (Class::*sizeTSetter)(size_t);
	Class& (Class::*stringSetter)(const std::string &);
	Class& (Class::*mapIntStrSetter)(int, std::string );
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

		enum Type { INT, SIZE_T, STRING, MAP_INT_STR } type;

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
	
		void setSetter(Class& (Class::*method)(int , std::string), Type t) {
			setter.mapIntStrSetter = method;
			type = t;
		}
		// Método para ejecutar el setter
		void execute(Class &srv, std::string val) {
			//srv.set_port(8080);
			std::deque<std::string> args = split(val, ' ');

			char *endp = NULL;
			int resultInt = 0;
			size_t resultSizeT = 0;
			switch (type) {
				case Setter::INT:
					resultInt = static_cast<int>(std::strtod(val.c_str(), &endp));
					if (*endp != '\0')
						throw Config::ConfigNotFoundException();
					(void)resultInt;
					if (setter.intSetter)
						(srv.*setter.intSetter)(resultInt);
					break;
				case Setter::STRING:
					if (val.empty())
						throw Config::ConfigNotFoundException();
					if (setter.stringSetter)
						(srv.*setter.stringSetter)(val);
					break;
				case Setter::SIZE_T:
					resultSizeT = static_cast<size_t>(std::strtod(val.c_str(), &endp));
					if (*endp != '\0') {
						if (*endp == 'M' && !std::isinf(static_cast<size_t>(resultSizeT * 1000000)) && !std::isnan(static_cast<size_t>(resultSizeT * 1000000)) )
							resultSizeT = static_cast<size_t>(resultSizeT * 1000000);
						else
							throw Config::ConfigNotFoundException();
					}
					if (setter.sizeTSetter)
						(srv.*setter.sizeTSetter)(resultSizeT);
					break;
				case Setter::MAP_INT_STR:
					if (args.size() != 2)
						throw Config::ConfigNotFoundException();
					resultInt = static_cast<int>(std::strtod(args[0].c_str(), &endp));
					if (setter.mapIntStrSetter)
						(srv.*setter.mapIntStrSetter)(resultInt, args[1]);
					break;
				default:
					break;
			}
		}
};

class ParserConfig {

	private:
		const char *_file_name;
		std::deque<std::string> _content_file;
		std::deque<std::string>::iterator _it;

		LimitExcept parserLimitExcept(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);
		Location parserLocation(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);
		ServerConfig parserServerConfig(std::deque<std::string>::iterator &it, std::deque<std::string>::iterator end);

		std::map<std::string, Setter<ServerConfig> > _automata_srv;
		std::map<std::string, Setter<Location > > _automata_loc;
		std::map<std::string, Setter<LimitExcept> > _automata_limexc;
		
	public:
		void init_automata();
		ParserConfig(const char *file_name);
		std::string get_last_lane_parser();
		int dumpRawData(const char *file_name);
		/**
		 * @brief Busco en el archivo la configuracion necesaria para ServerConfig
		 */
		void execute(char **env);
};

#endif