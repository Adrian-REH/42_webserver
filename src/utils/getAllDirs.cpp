
#include "Utils.hpp"

std::vector<std::string> get_all_dirs(const char *dir_path ) {
	std::vector<std::string> dirs;
	std::cout << "[INFO] Read dir: '" << dir_path <<"'"<< std::endl;

	// Abrir el directorio
	DIR* dir = opendir(dir_path);
	if (dir == NULL) {
		std::cout << "[ERROR] Error al abrir el directorio: " << dir_path<< std::endl;
		return dirs;
	}
	
	// Leer entradas del directorio
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string dirname(entry->d_name);
		// Ignorar las entradas "." y ".."
		if (dirname != "." && dirname != ".."){
			dirs.push_back(dirname);
			}
	}
	
	// Cerrar el directorio
	closedir(dir);
	std::cout << dirs.size() << std::endl;
	return dirs;
}
