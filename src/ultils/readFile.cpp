#include <iostream>
#include <string>

std::string ft_readFile(int fd) {
    int n_byte = 0;
    std::string result = "HTTP/1.1 200 OK\r\n";
    char buffer[1024];
    do {
        n_byte = read(fd, buffer, sizeof(buffer));
        if (n_byte > 0)
            result.append(buffer, n_byte);
        return result;
    } while (n_byte > 0);
    return result;
}