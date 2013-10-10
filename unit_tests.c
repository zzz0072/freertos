#include "fio.h"

void unit_test_task(void *pvParameters)
{
    char msg[] = "\n\nStart testing...\n";
    fio_write(1, msg, sizeof(msg));
}
