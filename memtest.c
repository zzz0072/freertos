#include "FreeRTOS.h"
#include "fio.h"
#include "task.h"
#include "rand.h"
extern int receive_byte_noblock(char *ch);
struct slot {
    void *pointer;
    unsigned int size;
    unsigned int seed;
};

#define CIRCBUFSIZE 1024
unsigned int g_write_pointer, g_read_pointer;
static struct slot g_slots[CIRCBUFSIZE];

static unsigned int circbuf_size(void)
{
    return (g_write_pointer + CIRCBUFSIZE - g_read_pointer) % CIRCBUFSIZE;
}

static void write_cb(struct slot foo)
{
    if (circbuf_size() == CIRCBUFSIZE - 1) {
        printf("circular buffer overflow\n\r");
        vTaskDelete(NULL);
    }
    g_slots[g_write_pointer++] = foo;
    g_write_pointer %= CIRCBUFSIZE;
}

static struct slot read_cb(void)
{
    struct slot foo;
    if (g_write_pointer == g_read_pointer) {
        /* circular buffer is empty */
        return (struct slot){ .pointer=NULL, .size=0, .seed=0 };
    }
    foo = g_slots[g_read_pointer++];
    g_read_pointer %= CIRCBUFSIZE;
    return foo;
}


void mem_test(void)
{
    unsigned int seed;
    int i, size;
    char *p;
    char ch;

    /* Set seed from tick count */
    seed = get_current_systick();
    srand(seed);
    while (1) {
        size = rand() & 0x7FF;
        seed = get_seed();
        printf("try to allocate %d bytes. Press any key to stop testing\n\r", size);
        p = (char *) pvPortMalloc(size);
        printf("malloc returned %p\n\r", p);
        if (p == NULL) {
            /* can't do new allocations until we free some older ones */
            while (circbuf_size() > 0) {
                /* confirm that data didn't get trampled before freeing */
                struct slot foo = read_cb();
                p = foo.pointer;
                srand(foo.seed);  /* reset the seed to generate the same sequence */
                size = foo.size;
                printf("free a block, size %d\n\r", size);

                /* Verify test patterns */
                for (i = 0; i < size; i++) {
                    unsigned char u = p[i];
                    unsigned char v = (unsigned char) rand();
                    if (u != v) {
                        printf("OUCH: u=%02X, v=%02X\n\r", u, v);
                        return;
                    }
                }
                vPortFree(p);
                if ((rand() & 1) == 0) break;
            }
        } else {
            printf("allocate a block, size %d\n\r", size);
            write_cb((struct slot){.pointer=p, .size=size, .seed=seed});

            /* Write test patterns */
            for (i = 0; i < size; i++) {
                p[i] = (unsigned char) rand();
            }
        }
        if (receive_byte_noblock(&ch) == pdTRUE) {
            break;
        }
    }
}
