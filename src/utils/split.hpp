#ifndef SPLIT_HPP
#define SPLIT_HPP
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

/**
 * @brief Divide una cadena en partes utilizando un delimitador.
 * 
 * @param str La cadena que se desea dividir.
 * @param delimiter El carácter delimitador.
 * @return std::deque<std::string> Partes de la cadena dividida.
 */
std::deque<std::string> split(const std::string &str, char delimiter) {
	std::stringstream		sstr(str);
	std::string				token;
	std::deque<std::string>	result;
	while (std::getline(sstr, token, delimiter)){
		result.push_back(token);
	}
	return result;
}

#endif