#ifndef COOKIE_HPP
#define COOKIE_HPP

#include "utils/Utils.hpp"

struct Cookie {
	private:
		std::string _name;
		std::string _value;
		std::time_t _expiration;

	public:
		Cookie();
		std::string get_value() const;
		std::string get_name() const;
		int get_expiration() const;
		void set_value(std::string v);
		void set_name(std::string n);
		void set_expiration(int e);
		bool isEmpty();
};

#endif
