typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned int uz;

extern char __bss[], __bss_end[], __stack_top[];




void *memset(void *buf, u8 c, uz n)
{
    u8 *p = (u8 *)buf;
    while(n--) {
        *p++ = c;
    }

    return buf;
}

void kernel_main(void)
{
    memset(__bss, 0, (uz)__bss_end - (uz)__bss);
    for (;;);
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n"
        "j kernel_main\n"
        :
        : [stack_top] "r" (__stack_top)
    );
}
