#ifndef COMFORK_H
  #define COMFORK_H

  #ifndef _WIN32
    #include <unistd.h>
    #include <sys/wait.h>
  #else
    #ifndef pid_t
typedef int pid_t;
    #endif

    #define WNOHANG 1
    #define WUNTRACED 2
    #define WEXITSTATUS(status) (((status) >> 8) & 0xFF)
    #define WSTOPSIG(stat)      ((status) & 0x7F)
    #define WIFEXITED(status)   (((status) & 0xFF) == 0)

    #ifdef __cplusplus
extern "C" {
    #endif
    pid_t waitpid(pid_t pid, int* status, int options);
    pid_t wait(int* status);
    pid_t fork();
    #ifdef __cplusplus
}
    #endif
  #endif
#endif