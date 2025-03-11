#ifndef COOKIE_HPP
#define COOKIE_HPP

#include "utils/Utils.hpp"

struct Cookie {
	std::string name;
	std::string value;
	std::time_t expiration;

	Cookie() : name(""), value(""), expiration(0) {}

	bool isEmpty() {
		return (value.empty() || name.empty() ) && expiration == 0;
	}
};


#endif