#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <vector>
#include <map>
#include <string>
#include "Cookie.hpp"
#include "Location.hpp"

class ServerConfig {
	private:
		int _port;
		std::string _server_name;
		size_t _max_clients;
		size_t _timeout;
		size_t _max_req;
		std::map<std::string, Location> _locations_conf;
		std::map<int, std::string> _error_pages;

	public:
		ServerConfig(int port = 8080, size_t max_clients = 1024, size_t timeout = 1, size_t max_req = 100);
		ServerConfig &set_port(const size_t port);
		ServerConfig &set_server_name(const std::string &server_name);
		ServerConfig &setMaxClients(const int max_cients);
		ServerConfig &add_location(const Location &location);
		ServerConfig &set_timeout(const size_t timeout);
		ServerConfig &set_max_req(const size_t max_req);
		ServerConfig &set_error_page(const int code, std::string index);
		Location findMatchingLocation(const std::string);
		int get_port() const;
		int get_max_clients() const;
		size_t get_timeout() const;
		std::string get_server_name() const;
		std::string get_error_page_by_code(const int key);
		size_t get_max_req()const;
		std::map<std::string, Location> get_locations();

	class LocationRepeateException : public std::exception {
		public:
			virtual const char *what() {
				return "Repeate Location paths";
			}
	};
};

#endif