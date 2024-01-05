#include <stdio.h>
#include <stdlib.h>
#include "comfork.h"

int main(int argc, char* argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Failed to clone the current process: 0x%x\r\n", pid);
    } if (pid > 0) {
        // parent will wait
        printf("Parent successfully cloned itself with PID: %d\r\n", pid);
        int status = 0;
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret == 0) {
            printf("Parent didn't wait\r\n");
        } else {
            printf("Clone exit with raw status: 0x%x\r\n", (status));
            if (WIFEXITED(status)) {
                printf("Clone exit with error status: 0x%x\r\n", WEXITSTATUS(status));
            }
        }
    } else {
        // child will waste time
        printf("Clone is wasting time!\r\n");
        for (int i = 0; i < 100000; i++);
        printf("Clone is returning!\r\n");

        // Terminate without clean-up
        _exit(5);
    }
    return 0;
}