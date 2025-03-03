
#include "Utils.hpp"

std::vector<std::string> get_all_dirs(const char *dir_path) {
	std::vector<std::string> dirs;

	// Abrir el directorio
	DIR* dir = opendir(dir_path);
	if (dir == NULL) {
		std::string d_path = dir_path;
		throw std::runtime_error("Error al abrir el directorio: " + d_path);
	}
	
	// Leer entradas del directorio
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string dirname(entry->d_name);
		
		// Ignorar las entradas "." y ".."
		if (dirname != "." && dirname != ".."){
			std::string base(extractStrREnd(dir_path, "/"));
			if (starts_with(base, "/"))
				base.erase(0,1);
			if (!ends_with(base, "/") && !base.empty())
				base += "/";
			dirname = base + dirname;
			if (entry->d_type == DT_DIR)
				dirname += "/";
			dirs.push_back(dirname);
		}
	}
	
	// Cerrar el directorio
	closedir(dir);
	return dirs;
}
