#include "Utils.hpp"

std::string generate_index_html(std::vector<std::string> files, std::string dir_path) {
	std::string index_file;
    // Escribir el encabezado HTML
	index_file = "Content-Type: text/html\r\n\r\n";
    index_file.append("<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n<title>Index of ");
	index_file.append(dir_path);
	index_file.append("</title>\n</head>\n<body>\n");
    index_file.append( "<h1>Index of ");
	index_file.append(dir_path);
	index_file.append("</h1>\n<ul>\n");

    // Leer los archivos y directorios
	std::vector<std::string>::iterator it;
    for (it = files.begin(); it != files.end(); it ++) {
        std::string entry_path = *it;
        // Ignorar los directorios "." y ".."
        if (entry_path == "." || entry_path == "..") {
            continue;
        }
        // Escribir cada archivo/directorio en la lista HTML
        index_file.append( "<li><a href=\"");
		index_file.append(entry_path);
		index_file.append("\">");

		std::string entry_name(entry_path);
		entry_name = extractStrREnd(entry_name, "/");
	
		index_file.append(entry_name);
		index_file.append("</a></li>\n");
    }

    // Escribir el pie de página HTML
    index_file.append("</ul>\n</body>\n</html>\n");
	return index_file;
}
