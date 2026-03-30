#include <stdio.h>
#include <stdint.h>
#include <interrupt.h>
#include <bsp.h>

void *timer1ctc_handler(void)
{
    printf("time: %d us\n", (uint32_t)bsp_read_us());
    
    return 0;
}

int main(void)
{
    bsp_timer_init(10);
    bsp_timer_start();
    while (1);
    
    return 0;
}

