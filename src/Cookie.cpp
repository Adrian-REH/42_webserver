
#include "Cookie.hpp"

Cookie::Cookie() : _name(""), _value(""), _expiration(0) {}

std::string Cookie::get_value() const {
	return _value;
}

std::string Cookie::get_name() const {
	return _name;
}

int Cookie::get_expiration() const {
	return _expiration;
}

void Cookie::set_value(std::string v){
	_value = v;
}

void Cookie::set_name(std::string n){
	_name = n;
}

void Cookie::set_expiration(int e){
	_expiration = e;
}

bool Cookie::isEmpty() {
	return (_value.empty() || _name.empty() ) && _expiration == 0;
}
