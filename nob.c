#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

int get_executable_path(char *buffer, size_t size)
{
#if defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        return 0;
    }
    return -1;

#elif defined(__APPLE__)
    uint32_t bufsize = (uint32_t)size;
    char temp_path[PATH_MAX];
    if (_NSGetExecutablePath(temp_path, &bufsize) == 0)
    {
        if (realpath(temp_path, buffer) != NULL)
        {
            return 0;
        }
    }
    return -1;

#else
    return -1;
#endif
}

void get_executable_dir(char *path)
{
    char *last_slash = strrchr(path, '/');

    if (last_slash != NULL)
    {
        *last_slash = '\0';
    }
}

#define nullptr NULL

void *global;

[[gnu::constructor(0)]] void __a()
{
    global = __builtin_frame_address(0);
}

int main(int argc, char *argv[])
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};
    __auto_type offset = __builtin_frame_address(0) - global;
    char *b = malloc(snprintf(nullptr, 0, "-D_OFFSOFCAM=%ld", offset));
    sprintf(b, "-D_OFFSOFCAM=%ld", offset);
    nob_cmd_append(&cmd, "clang", "-std=c2y", "-fno-inline", "-fblocks", b);
    for (int i = 1; i < (argc); ++i)
    {
        nob_cmd_append(&cmd, argv[i]);
    }

    char *buf = malloc(2048);

    get_executable_path(buf, 2048);
    get_executable_dir(buf);
    buf = realloc(buf, strlen(buf) + strlen("/anvilimpl.c") + 1);
    strcat(buf, "/anvilimpl.c");
    nob_cmd_append(&cmd, buf);
#ifdef __linux__
    nob_cmd_append(&cmd, "-lBlocksRuntime");
#endif
    nob_cmd_run_sync(cmd);
    free(buf);
    free(b);
}
