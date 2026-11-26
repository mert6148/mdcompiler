#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

static void print_usage(void) {
    puts("mdcompiler-client - C client for mdcompiler");
    puts("");
    puts("Usage:");
    puts("  mdcompiler-client <mdcompiler-path> <build|compile|run> <source> [options] [-- program-args]");
    puts("  mdcompiler-client --help");
    puts("");
    puts("The client adds --protocol and forwards compiler output and exit status.");
}

static int is_action(const char *action) {
    return strcmp(action, "build") == 0 ||
           strcmp(action, "compile") == 0 ||
           strcmp(action, "run") == 0;
}

#ifdef _WIN32
static void append_quoted(char *destination, size_t *position, const char *value) {
    size_t slashes = 0;
    destination[(*position)++] = '"';

    while (*value != '\0') {
        if (*value == '\\') {
            ++slashes;
            ++value;
            continue;
        }

        if (*value == '"') {
            while (slashes > 0) {
                destination[(*position)++] = '\\';
                destination[(*position)++] = '\\';
                --slashes;
            }
            destination[(*position)++] = '\\';
            destination[(*position)++] = '"';
        } else {
            while (slashes > 0) {
                destination[(*position)++] = '\\';
                --slashes;
            }
            destination[(*position)++] = *value;
        }
        slashes = 0;
        ++value;
    }

    while (slashes > 0) {
        destination[(*position)++] = '\\';
        destination[(*position)++] = '\\';
        --slashes;
    }
    destination[(*position)++] = '"';
}

static char *make_command_line(char *const arguments[], size_t count) {
    size_t capacity = 1;
    for (size_t index = 0; index < count; ++index) {
        size_t length = strlen(arguments[index]);
        if (length > (SIZE_MAX - capacity - 4) / 2) {
            return NULL;
        }
        capacity += length * 2 + 4;
    }

    char *command_line = (char *)malloc(capacity);
    if (command_line == NULL) {
        return NULL;
    }

    size_t position = 0;
    for (size_t index = 0; index < count; ++index) {
        if (index != 0) {
            command_line[position++] = ' ';
        }
        append_quoted(command_line, &position, arguments[index]);
    }
    command_line[position] = '\0';
    return command_line;
}

static int run_compiler(char *const arguments[], size_t count) {
    char *command_line = make_command_line(arguments, count);
    if (command_line == NULL) {
        fputs("mdcompiler-client: failed to allocate process command line\n", stderr);
        return 2;
    }

    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    ZeroMemory(&process_info, sizeof(process_info));
    startup_info.cb = sizeof(startup_info);

    BOOL started = CreateProcessA(
        arguments[0],
        command_line,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_info
    );
    free(command_line);

    if (!started) {
        fprintf(stderr, "mdcompiler-client: could not start compiler (Windows error %lu)\n",
                (unsigned long)GetLastError());
        return 127;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code = 1;
    BOOL got_exit_code = GetExitCodeProcess(process_info.hProcess, &exit_code);
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);

    if (!got_exit_code) {
        fputs("mdcompiler-client: could not read compiler exit code\n", stderr);
        return 1;
    }
    return (int)exit_code;
}
#else
static int run_compiler(char *const arguments[], size_t count) {
    (void)count;
    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "mdcompiler-client: could not create process: %s\n",
                strerror(errno));
        return 1;
    }

    if (child == 0) {
        execvp(arguments[0], arguments);
        fprintf(stderr, "mdcompiler-client: could not start '%s': %s\n",
                arguments[0], strerror(errno));
        _exit(errno == ENOENT ? 127 : 126);
    }

    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) {
            fprintf(stderr, "mdcompiler-client: could not wait for compiler: %s\n",
                    strerror(errno));
            return 1;
        }
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return 1;
}
#endif

int main(int argc, char **argv) {
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 ||
                      strcmp(argv[1], "-h") == 0)) {
        print_usage();
        return 0;
    }

    if (argc < 4) {
        print_usage();
        return 2;
    }
    if (!is_action(argv[2])) {
        fprintf(stderr, "mdcompiler-client: unsupported action '%s'\n", argv[2]);
        print_usage();
        return 2;
    }
    if (argv[3][0] == '\0') {
        fputs("mdcompiler-client: source path must not be empty\n", stderr);
        return 2;
    }

    char **compiler_arguments = (char **)calloc((size_t)argc + 1, sizeof(char *));
    if (compiler_arguments == NULL) {
        fputs("mdcompiler-client: failed to allocate argument list\n", stderr);
        return 2;
    }

    size_t count = 0;
    compiler_arguments[count++] = argv[1];
    compiler_arguments[count++] = argv[2];
    compiler_arguments[count++] = argv[3];
    compiler_arguments[count++] = "--protocol";
    for (int index = 4; index < argc; ++index) {
        compiler_arguments[count++] = argv[index];
    }
    compiler_arguments[count] = NULL;

    int exit_code = run_compiler(compiler_arguments, count);
    free(compiler_arguments);
    return exit_code;
}