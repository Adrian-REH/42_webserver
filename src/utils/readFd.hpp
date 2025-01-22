#ifndef READFD_HPP
#define READFD_HPP
#include <string>

std::string readFd(int fd) {
	std::string result;
	char buffer[1024];
	ssize_t bytes_read;

	while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
		result.append(buffer, bytes_read);
	}
	return result;
}

#endif