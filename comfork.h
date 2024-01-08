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

    // Some Windows have issues with using exit and _exit, replace them with proper Nt API
    #define exit(status)        (ntexit((status)))
    #define _exit(status)       (ntexit((status)))

    #ifdef __cplusplus
extern "C" {
    #endif
    pid_t waitpid(pid_t pid, int* status, int options);
    pid_t wait(int* status);
    pid_t fork();
    void ntexit(int status);
    #ifdef __cplusplus
}
    #endif
  #endif
#endif