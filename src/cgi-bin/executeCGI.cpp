#include <sys/wait.h>
#include <unistd.h>

pid_t execute_cgi(int *io, char *args[], char **env ) {

    pid_t pid = fork();


    if (pid < 0)
        return 0;
    else if (pid == 0) {
        std::cout << io[0] << std::endl;
        dup2((io[0]), 0);
        close(io[0]);
        dup2((io[1]), 1);
        close(io[1]);
        execve(args[0], args, env);
    }
    return pid;
}