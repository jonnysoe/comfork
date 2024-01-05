# Common-fork library

A simple `fork` and `wait` header and library for Windows to be compatible with UNIX-based C/C++ open source softwares.

## Background
There is not much libraries available for Windows to use `fork` and `wait`, this is one of the key missing features when porting UNIX-based FOSS codes into Windows, especially when developing on a pure Windows platform without UNIX-compatibility environments like `WSL`, `MSYS2` or `Cygwin`.

While threads as lightweight processes have been widely available and is the preferred approach for multi-tasking, `fork` and `wait` functions still have their values when it comes to performing tasks that may kill the main process - there are libraries that actually calls `_exit` on a spawned thread, the parent process will be exit as well.

## Description
This library is meant emulate the basic capabilities of `fork` and `wait`, so that it can be a drop-in header and library replacement for code bases that use these functions without being overly intrusive.

What is available right now is barebone minimal as `waitpid` only supports `0` and `>0` pids, `-1` and `<-1` range were supposed to target the process group that belongs to the specified pid where process group is not supported - they're behaving like `0` and `>0` respectively. The only `option` supported at the moment is `WNOHANG`.

Check out the [wait manpage](https://manpages.ubuntu.com/manpages/jammy/en/man2/wait.2.html) for more information on the `pid` argument for `waitpid`.

## Getting started

### Requirements
- [Git](https://git-scm.com/downloads)
- [CMake](https://cmake.org/download/)
- [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
- C/C++ compiler (tested on [MSVC](https://visualstudio.microsoft.com/visual-cpp-build-tools/) and [LLVM/Clang](https://releases.llvm.org/download.html))

Optional:
- [Visual Studio Code](https://code.visualstudio.com/download)
- [Ninja](https://ninja-build.org/)
- [CCache](https://ccache.dev/download.html)

### OS Compatibility
- Windows 10 x64 (technically Windows Vista) and above, because it uses `NtCreateUserProcess`.

NOTE: Linux and Mac OS are not included as they're just a wrapper, this code should still compile and run on those platforms.

### Usage
#### Demo
When building as a standalone program to test the waters, there will be a few targets available to showcase how `fork` and `waitpid` works.

Either hit `F5` in vscode, or issue the following commands in the project directory to try out the programs:
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 && cmake --build build
bin\main_c
bin\main_cpp
bin\main_nowait_c
bin\main_nowait_cpp
```

#### Module
This library can be added into any code base as a module, its easier if the code base uses modern build tools, eg. CMake, Bazel, Meson, etc.; basically those that support interoperability with CMake.

The easiest approach to use [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html#typical-case)
```
FetchContent_Declare(
    comfork
    GIT_REPOSITORY https://github.com/jonnysoe/comfork.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(comfork)
add_subdirectory(${comfork_SOURCE_DIR})
link_libraries(libcomfork)
```

#### Mechanism
Most of these codes were adapted from `Hunt & Hackett`'s documentation in [The Definitive Guide To Process Cloning on Windows](https://github.com/huntandhackett/process-cloning?tab=readme-ov-file#the-definitive-guide-to-process-cloning-on-windows). The code is basically being refactored to behave like UNIX `fork` and `wait`. Check out that page for more information and understanding the caveats of this library.

The `fork` function has been widely used since ages, it was achieved by invoking an undocumented native API (`ntdll`) - `RtCreateUserProcess`, which is the lowest API in user space that can bypass detection controls - refer to [Capt. Meelo's blog page](https://captmeelo.com/redteam/maldev/2022/05/10/ntcreateuserprocess.html) for detailed experiments and explanations.

This native API however does not come with a function prototype in any of the headers in `Windows SDK`, thankfully most of these APIs have been experimented on thoroughly and compiled into the [phnt](https://github.com/winsiderss/phnt) headers by the Process Hacker Project.

NOTE: Despite not encountering any major issues, there may be missing steps that was used in the [Winnie-AFL](https://github.com/sslab-gatech/winnie)'s [fork](https://github.com/sslab-gatech/winnie/blob/master/forklib/fork.cpp) library that is worth adapting.

## Acknowledgement
`Hunt & Hackett` consolidated a lot of information which is helpful to prepare this library.
`Winsider` for the [phnt](https://github.com/winsiderss/phnt) native API headers for the elegant native API function call implementation.
`Capt. Meelo`'s blog page for detailed understanding of the native API.

## License
Licensed under the MIT License. See [LICENSE.md](./LICENSE.md) for more information.

### Third Party
The [phnt](https://github.com/winsiderss/phnt) headers for `ntdll` is licensed under the [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) License.