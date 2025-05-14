
#include "Config.hpp"

Config::Config() {
    _mimetypes[".doc"] = "application/msword";
    _mimetypes[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    _mimetypes[".xls"] = "application/vnd.ms-excel";
    _mimetypes[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    _mimetypes[".ppt"] = "application/vnd.ms-powerpoint";
    _mimetypes[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    _mimetypes[".pdf"] = "application/pdf";
    _mimetypes[".html"] = "text/html";
    _mimetypes[".htm"] = "text/html";
    _mimetypes[".css"] = "text/css";
    _mimetypes[".js"] = "application/javascript";
    _mimetypes[".json"] = "application/json";
    _mimetypes[".xml"] = "application/xml";
    _mimetypes[".csv"] = "text/csv";
    _mimetypes[".txt"] = "text/plain";
    _mimetypes[".rtf"] = "application/rtf";
    _mimetypes[".zip"] = "application/zip";
    _mimetypes[".gz"] = "application/gzip";
    _mimetypes[".tar"] = "application/x-tar";
    _mimetypes[".bz2"] = "application/x-bzip2";
    _mimetypes[".bin"] = "application/octet-stream"; // Type MIME for binary files
    _mimetypes[".exe"] = "application/octet-stream";
    _mimetypes[".dll"] = "application/octet-stream";
    _mimetypes[".iso"] = "application/x-iso9660-image";
    _mimetypes[".mp3"] = "audio/mpeg";
    _mimetypes[".ogg"] = "audio/ogg";
    _mimetypes[".wav"] = "audio/wav";
    _mimetypes[".flac"] = "audio/flac";
    _mimetypes[".mp4"] = "video/mp4";
    _mimetypes[".avi"] = "video/x-msvideo";
    _mimetypes[".mov"] = "video/quicktime";
    _mimetypes[".mkv"] = "video/x-matroska";
    _mimetypes[".webm"] = "video/webm";
    _mimetypes[".jpg"] = "image/jpeg";
    _mimetypes[".jpeg"] = "image/jpeg";
    _mimetypes[".png"] = "image/png";
    _mimetypes[".gif"] = "image/gif";
    _mimetypes[".bmp"] = "image/bmp";
    _mimetypes[".svg"] = "image/svg+xml";
    _mimetypes[".ico"] = "image/x-icon";
    _mimetypes[".webp"] = "image/webp";
    _mimetypes[".tiff"] = "image/tiff";
    _mimetypes[".eps"] = "application/postscript";
    _mimetypes[".ps"] = "application/postscript";
    _mimetypes[".jsonld"] = "application/ld+json";
    _mimetypes[".wasm"] = "application/wasm";
    _mimetypes[".mpg"] = "video/mpeg";
    _mimetypes[".mpeg"] = "video/mpeg";
    _mimetypes[".swf"] = "application/x-shockwave-flash";
    _mimetypes[".jar"] = "application/java-archive";
    _mimetypes[".7z"] = "application/x-7z-compressed";
    _mimetypes[".apk"] = "application/vnd.android.package-archive";
    _mimetypes[".json"] = "application/json";
    _mimetypes[".csv"] = "text/csv";
    _mimetypes[".yaml"] = "application/x-yaml";
    _mimetypes[".yml"] = "application/x-yaml";
}

Config &Config::getInstance() {
	static Config instance;
	return instance;
}

void Config::addServerConf(ServerConfig srv_conf) {
	std::map<int, ServerConfig>::iterator it = _srvs_conf.find(srv_conf.get_port());

	if (it != _srvs_conf.end())
		throw Config::ConfigServerPortExistException( to_string(srv_conf.get_port()));
	_srvs_conf[srv_conf.get_port()] = srv_conf;
}

std::map<int, ServerConfig> Config::getServerConfs() {
	return _srvs_conf;
}

ServerConfig Config::getServerConfByPort(const int port) {
	std::map<int, ServerConfig>::iterator it = _srvs_conf.find(port);

	if (it != _srvs_conf.end()){
		return it->second;
	}
	throw Config::ConfigNotFoundException();
}

std::string Config::getMimeTypeByExtension(const std::string extention){
	std::map<std::string, std::string>::iterator it = _mimetypes.find(extention);
	if (it != _mimetypes.end())
		return it->second;
	return "application/octet-stream"; 
}