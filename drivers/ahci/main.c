int main()
{
    asm volatile ("int $0x7");

    while (true)
        ;    
}
