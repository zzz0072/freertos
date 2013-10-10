#include "fio.h"
#include "string.h"
#include "util.h"
void unit_test_task(void *pvParameters)
{
    char msg1[] = "Start testing...\n";
    char msg2[] = "Start";
    my_puts(msg1);
    my_puts(msg2);
    
    if (strcmp(msg1, msg2)) {
        my_puts("msg1 and msg2 are diff\n");
    }
    else {
        my_puts("strcpy result is not match\n");
    }
    
    if (strncmp(msg1, msg2, 5) == 0) {
        my_puts("msg1 and msg2 first 5 chars are the same\n");
    }
    else {
        my_puts("strncpy result is not match\n");
    }
    
    printf("test htoi(255):%s\n", htoa(255));
    printf("test atoi(100):%s\n", itoa(100));
}
