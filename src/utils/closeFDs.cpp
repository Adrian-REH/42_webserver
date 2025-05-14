#include "Utils.hpp"

void closeFDs(int fds[]) {
    
    close(fds[0]);
    close(fds[1]);
}
