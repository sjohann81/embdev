#include <stdio.h>

int main(void)
{
    char buf[50];
    
    printf("hello world!\n");
    printf("type something, I will echo back...\n");
    while (1) {
        scanf("%49[^\n]", buf);
        printf(buf);
    }
    
    return 0;
}

