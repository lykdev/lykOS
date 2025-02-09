void _start()
{
    asm volatile("syscall"                      \
        :                                       \
        : "a" (0)                               \
        : "rcx", "r11", "memory"                \
    );

    while (1)
        ;
}
