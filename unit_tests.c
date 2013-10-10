#include "fio.h"
#include "string.h"
#include "util.h"
void unit_test_task(void *pvParameters)
{
    char msg1[] = "Start testing...\n\r";
    char msg2[] = "Start\n\r";
    my_puts(msg1);
    my_puts(msg2);
    
    if (strcmp(msg1, msg2)) {
        my_puts("msg1 and msg2 are diff\n\r");
    }
    else {
        my_puts("strcpy result is not match\n\r");
    }
    
    if (strncmp(msg1, msg2, 5) == 0) {
        my_puts("msg1 and msg2 first 5 chars are the same\n\r");
    }
    else {
        my_puts("strncpy result is not match\n\r");
    }
    
    printf("test htoi(255):%s\n\r", htoa(255));
    printf("test atoi(100):%s\n\r", itoa(100));
}
