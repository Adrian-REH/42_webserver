
#include "Utils.hpp"

std::vector<std::string> get_all_dirs(const char *dir_path) {
	std::vector<std::string> dirs;

	DIR* dir = opendir(dir_path);
	if (dir == NULL) {
		std::string d_path = dir_path;
		throw std::runtime_error("Error opening directory: " + d_path);
	}
	
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string dirname(entry->d_name);
		
		// Ignores "." y ".."
		if (dirname != "." && dirname != ".."){
			std::string base(extractStrREnd(dir_path, "/"));
			if (!ends_with(base, "/") && !base.empty())
				base += "/";
			if (entry->d_type == DT_DIR){
				dirname += "/";
			}
			dirs.push_back(dirname);
		}
	}

	closedir(dir);
	return dirs;
}
