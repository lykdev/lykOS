
void driver_load()
{
    __asm__ volatile ("int $0x3");

    while (1)
        ;
}