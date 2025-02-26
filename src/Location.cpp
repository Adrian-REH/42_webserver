
#include "Location.hpp"

Location::Location() {}

Location &Location::set_root_directory(const std::string &root) {
	_root_directory = root;
	return *this;
}

Location &Location::set_index(const std::string &index) {
	_index = index;
	return *this;
}

Location &Location::set_limit_except(const LimitExcept &lim) {
	_limit_except = lim;
	return *this;
}

Location &Location::set_path(const std::string &path) {
	_path = path;
	return *this;
}

Location &Location::set_auto_index(const std::string &rule) {
	_auto_index = (rule.find("on") != std::string::npos);
	return *this;
}

Location &Location::set_path_upload_directory(const std::string &str) {
	_path_upload_directory = str;
	return *this;
}

Location &Location::set_client_max_body_size(const int cli_max_body_size) {
	_client_max_body_size = cli_max_body_size;
	return *this;
}

Location &Location::set_redirect_url(const std::string &str) {
	_redirect_url = str;
	return *this;
}

Location &Location::set_limit_except(LimitExcept &l) {
	_limit_except = l;
	return *this;
}

std::string Location::get_path() const {
	return _path;
}
int Location::get_auto_index() const {
	return _auto_index;
}
std::vector<std::string> Location::get_files() const {
	return _files;
}

LimitExcept Location::get_limit_except() const {
	return _limit_except;
}

Location Location::build() {
	if (_root_directory.empty())
	//FIXME: En caso de que no haya literalmente un root_dir es necesario que sea resiliente asi que propongo poner "/"
		throw std::runtime_error("Error: no existe un directorio root");
	return *this;
}
		
// Helper function to build the full path.
std::string Location::buildFullPath(const std::string &root,const std::string path, const std::string &filename) {
	std::string result;
	if (!starts_with(root, "/"))
		result = "/";
	result.append(root);
	if (!ends_with(result, "/"))
		result.append("/");
	result.append(path);
	if (!ends_with(result, "/") && !filename.empty())
		result.append("/");
	result.append(filename);
	return result;
}

int Location::findScriptPath(const std::string &url_path, std::string &final_path) {
	std::string path = extractStrEnd(url_path, _path);
	std::string file;
	std::cout << "[DEBUG] begin path " << path << std::endl;
	// Case 1: Path is base root '/'
	if (!_index.empty() && path.empty() && _path == url_path)
		return final_path = buildFullPath(_root_directory, "", _index), 0;
	// Case 2: Path ends with the configured index file.
	if (!_index.empty() && ends_with(path, _index))
		return final_path = buildFullPath(_root_directory, "", path), 0;
	// Check for a matching file in _files.
	size_t dot_pos = path.rfind('.');
	if ((dot_pos != std::string::npos) && (dot_pos != path.length() - 1)) {
		std::string path_tmp  = extractStrStart(path, "/");
		file = extractStrEnd(path, path_tmp); //extractStrEnd
		path = path_tmp;
		if (!_index.empty() && ends_with(path, _index))
			return final_path = buildFullPath(_root_directory, "", path), 0;

		//return buildFullPath(_root_directory, path, ""), 0;
	}
	
	std::string work_dir = buildFullPath(_root_directory, path, "");

	std::cout << "[DEBUG] Work dir, get files: '" << work_dir <<"'"<< std::endl;
	if (work_dir == "/")
		work_dir = ".";
	const char * dir = work_dir.c_str();
	if (work_dir.length() > 1)
		dir++;

	// If directory doesn't exist, it will throw an exception
	_files = get_all_dirs(dir); // TODO: Adjust for directories without leading '/'
	for (std::vector<std::string>::iterator it = _files.begin(); it != _files.end(); ++it) {
		if (!file.empty() && ends_with(file, *it)) {
			return final_path = buildFullPath(dir, "", *it), 0;
		} 
		/*else if (*it == _index){
			return final_path = buildFullPath(dir, "", _index), 0;
		}*/
	}
	//No existe o autoindex
	return (final_path = dir,1);
}
/**
 * FIX: autoindex paths on findScriptPath()
 * FIX: else if (*it == _index){ 
 * 
 * 
 * casos:
 * 	- /cgi-bin y path es /cgi-bin/ (no es el mismo) y existe el base '/'
 * 	- /cgi-bin y no existe path relacionaddo pero exxiste el base /
 * 	- /cgi-bin/upload_file.py y existe path es /cgi-bin/ con su index y otros mas incluido /
 * 	- /images/kpop/ikon.jpeg y existe path /iamges/ con root en /img/www => /img/www/kpop/ikon.jpeg
 */