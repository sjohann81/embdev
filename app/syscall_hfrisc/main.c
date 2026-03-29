#include <stdio.h>
#include <stdint.h>
#include <interrupt.h>

int main(void)
{
    printf("syscall test\n");
    syscall((void *)0x123, 0x456);
    
    return 0;
}

