#include <stdio.h>

int main(void)
{
    char buf[80];
    
    printf("hello world!\n");
    printf("type something, I will echo back...\n");
    while (1) {
        scanf("%79[^\n]", buf);
        printf(buf);
    }
    
    return 0;
}

