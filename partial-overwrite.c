#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int exec_something(char *what)
{
    char *const args[] = {what, "-l", NULL};
    execv(what, args);
    return 0;
}

int orig_fun(const char *arg)
{
    puts("in original target");
    fflush(stdout);
    return 0;
}

static inline __attribute__((always_inline)) void stack_dump(void *start, size_t depth)
{
    void **base = (void**) start;

    for (size_t i = 0; i < depth; ++i)
    {
        uint64_t ptr = (uint64_t) base[i];
        printf("%08lx%c", ptr, (i + 1) % 4 == 0? '\n' : ' ');
    }

    if (depth % 4 != 0)
        putchar('\n');

    fflush(stdout);
}

typedef int(*puts_t)(const char*);

struct something
{
    char buffer[16];
    puts_t ptr;
};

int exec_overflow(const char *argv1)
{
    struct something target = {0xFF};
    size_t payload_len = strlen(argv1);

    target.ptr = &orig_fun;

    puts("=================== [     info      ] ==================");
    printf("payload len: %zu\n", payload_len);
    printf("target: %08lx\n", (uint64_t) &exec_something);
    puts("==================================================\n");

    puts("=================== [ pre overflow ] ===================");
    printf("funptr: %08lx\n", &target);
    stack_dump(&target.buffer, 16);
    puts("========================================================\n");

    memcpy(target.buffer, argv1, payload_len);

    puts("=================== [ pst overflow ] ===================");
    printf("funptr: %08lx\n", &target);
    stack_dump(&target.buffer, 16);
    puts("========================================================\n");

    fflush(stdout);
    target.ptr("/bin/sh");
    return 0;
}

int show_usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s <run|overflow:bytes>\n", argv0);
    return 1;
}

int main(int argc, char **argv)
{
    const char *overflow = "overflow:";
    const size_t overflow_len = strlen(overflow);

    if (argc < 2)
        return show_usage(argv[0]);

    if (strcmp(argv[1], "run") == 0)
    {
        exec_something((char*) "/bin/ls");
        return 0;
    }

    if (strncmp(argv[1], overflow, overflow_len) == 0)
    {
        exec_overflow(argv[1] + overflow_len);
        puts("returned successfully to main");
    }

    return show_usage(argv[0]);
}
