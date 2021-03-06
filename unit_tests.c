#include "fio.h"
#include "string.h"
#include "util.h"
#include "FreeRTOS.h"
#include "task.h"
#include "memtest.h"

void unit_test_task(void *pvParameters)
{
    char msg1[] = "Start\n\r";
    char msg2[128] = "Start testing...\n\r";
    puts(msg1);
    puts(msg2);

    if (strcmp(msg1, msg2)) {
        puts("msg1 and msg2 are diff\n\r");
    }
    else {
        puts("strcpy result is not match\n\r");
    }

    if (strncmp(msg1, msg2, 5) == 0) {
        puts("msg1 and msg2 first 5 chars are the same\n\r");
    }
    else {
        puts("strncpy result is not match\n\r");
    }

    /* Dirty? Only last atoi buf need to be set to \0 */
    msg2[MAX_ITOA_CHARS - 1] = 0;
    printf("test htoi(255):%s\n\r", htoa(255, msg2));
    printf("test htoi(-18):%s\n\r", htoa(-18, msg2));
    printf("test addrtoa(-1):%s\n\r", addrtoa(-1, msg2));
    printf("test atoi(100):%s\n\r", itoa(100, msg2));
    printf("\n");

    strcat(msg2, msg1);
    printf("strcat:%s\n\r", msg2);
    sprintf(msg2, "==>test sprintf %s:%s:%d\n", msg1, "zzz", 122);
    printf("sprintf:%s\n\r", msg2);

    vTaskList((signed char*)msg2);
    printf("vTaskList:%s\n\r", msg2);
    mem_test();
    printf("End of unit test\n\n\n\r");
}
