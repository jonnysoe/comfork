#include "phnt_windows.h"
#include "phnt.h"

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <errno.h>

#include "comfork.h"

namespace {
std::unordered_map<pid_t, std::pair<HANDLE, HANDLE>> handles;

LARGE_INTEGER instant = {0};
LARGE_INTEGER polling = {50};

pid_t waitsinglepid(pid_t pid, int& status, LARGE_INTEGER* timeout) {
    HANDLE hProcess = handles[pid].first;
    HANDLE hThread = handles[pid].second;

    // Initialize to make WIFEXITED return false
    status = 0;

    int ret = NtWaitForSingleObject(hProcess, false, timeout);
    if (ret == WAIT_TIMEOUT) {
        // Didn't wait for clone
        return 0;
    }

    // Save exit code before closing the process handle
    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);

    NtClose(hProcess);
    NtClose(hThread);
    handles.erase(pid);

    if (!NT_SUCCESS(ret)) {
        // Failed to wait for the clone
        status = ret << 8;
    } else if (exitCode != NO_ERROR) {
        // Clone exit status
        status = exitCode << 8;
    }

    return pid;
}
}

pid_t waitpid(pid_t pid, int* status, int options) {
    auto timeout = !!(options & WNOHANG) ? &instant : nullptr;
    int dummy = 0;
    if (status == nullptr) {
        status = &dummy;
    }

    pid_t ret = 0;
    int err = 0;
    if (!!(options & ~(WNOHANG | WUNTRACED))) {
        err = EINVAL;
    } else if ((pid > 0) || (pid < -1)) {
        // targetted child, there is no distinction between Process group and non-process group for now
        pid = abs(pid);
        if (handles.count(pid) == 0) {
            // PID is not a child of the current process
            err = ECHILD;
        } else {
            ret = waitsinglepid(pid, *status, timeout);
        }
    } else if (pid >= -1) {
        // any child, there is no distinction between Process group and non-process group for now
        bool const blocking = (timeout == nullptr) && !handles.empty();
        if (blocking) {
            // Switch to polling mode as we don't want to block on a single PID
            ///@todo it should be less clunky with WaitForMultipleObjects
            timeout = &polling;
        }
        do {
            for (const auto& handle : handles) {
                ret = waitsinglepid(handle.first, *status, timeout);
                if (ret != 0) {
                    break;
                }
            }
        } while (blocking && (ret == 0));
    }

    // Error overrides return value to -1 and set errno
    if (err != 0) {
        // fork will set errno
        errno = err;
        ret = -1;
    }
    return ret;
}

pid_t wait(int* status) {
    return waitpid(-1, status, 0);
}

pid_t fork() {
    NTSTATUS status;
    HANDLE hProcess;
    HANDLE hThread;

    PS_CREATE_INFO createInfo = { 0 };
    createInfo.Size = sizeof(createInfo);

    status = NtCreateUserProcess(
        &hProcess,
        &hThread,
        PROCESS_ALL_ACCESS,
        THREAD_ALL_ACCESS,
        NULL,
        NULL,
        PROCESS_CREATE_FLAGS_INHERIT_HANDLES,
        0,
        NULL,
        &createInfo,
        NULL
    );

    pid_t pid = 0;

    if (status == STATUS_PROCESS_CLONED) {
        // Executing inside the clone...

        // Re-attach to the parent's console to be able to write to it
        FreeConsole();
        AttachConsole(ATTACH_PARENT_PROCESS);

        // Newer child process should not have access to older sibling processes of the same parent process,
        // so that they don't wait for the wrong processes.
        handles.clear();
    } else {
        // Executing inside the original (parent) process...

        if (!NT_SUCCESS(status)) {
            pid = status;
            return pid;
        }

        pid = GetProcessId(hProcess);

        handles[pid] = { hProcess, hThread };
    }

    return pid;
}