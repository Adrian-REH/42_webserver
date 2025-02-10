
#include "Utils.hpp"

std::vector<std::string> get_all_dirs(const char *dir_path ) {
	std::vector<std::string> dirs;

	// Abrir el directorio
	DIR* dir = opendir(dir_path);
	if (dir == NULL) {
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
	return dirs;
}
