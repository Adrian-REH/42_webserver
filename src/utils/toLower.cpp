#include "Utils.hpp"
/**
 * @brief Convierte una cadena a lower case.
 * 
 * @param value El valor entero a convertir.
 */
void toLower(std::string& str) {
    for (std::string::size_type i = 0; i < str.size(); ++i){
        str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
    }
}
