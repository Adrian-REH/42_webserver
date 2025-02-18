#include "Utils.hpp"
/**
 * @brief Convierte una cadena de texto a un valor entero en decimal.
 * 
 * @param value La cadena de texto a convertir.
 * @return int Representación en entero de la cadena.
 */
int to_int(std::string value) {
    int result = 0;
    std::stringstream stream;

    stream << value;
    stream >> result;
    return result;
}

unsigned long to_hex_ulong(std::string value) {
    unsigned long result = 0;
    std::stringstream stream;

    stream << value;
    stream >> std::hex >> result;
    return result;
}

unsigned long to_dec_ulong(std::string value) {
    unsigned long result = 0;
    std::stringstream stream;

    stream << value;
    stream >> std::hex >> result;
    return result;
}