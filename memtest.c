#include "FreeRTOS.h"
#include "fio.h"
#include "task.h"
extern int receive_byte_noblock(char *ch);
struct slot {
    void *pointer;
    unsigned int size;
    unsigned int lfsr;
};

#define CIRCBUFSIZE 1024
unsigned int write_pointer, read_pointer;
static struct slot slots[CIRCBUFSIZE];
static unsigned int lfsr = 0xACE1;

static unsigned int circbuf_size(void)
{
    return (write_pointer + CIRCBUFSIZE - read_pointer) % CIRCBUFSIZE;
}

static void write_cb(struct slot foo)
{
    if (circbuf_size() == CIRCBUFSIZE - 1) {
        printf("circular buffer overflow\n\r");
        vTaskDelete(NULL);
    }
    slots[write_pointer++] = foo;
    write_pointer %= CIRCBUFSIZE;
}

static struct slot read_cb(void)
{
    struct slot foo;
    if (write_pointer == read_pointer) {
        /* circular buffer is empty */
        return (struct slot){ .pointer=NULL, .size=0, .lfsr=0 };
    }
    foo = slots[read_pointer++];
    read_pointer %= CIRCBUFSIZE;
    return foo;
}


/* Get a pseudorandom number generator from Wikipedia */
static int prng(void)
{
    static unsigned int bit;
    /* taps: 16 14 13 11; characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    lfsr =  (lfsr >> 1) | (bit << 15);
    return lfsr & 0xffff;
}

void mem_test(void)
{
    int i, size;
    char *p;
    char ch;

    while (1) {
        size = prng() & 0x7FF;
        printf("try to allocate %d bytes. Press any key to stop testing\n\r", size);
        p = (char *) pvPortMalloc(size);
        printf("malloc returned %p\n\r", p);
        if (p == NULL) {
            /* can't do new allocations until we free some older ones */
            while (circbuf_size() > 0) {
                /* confirm that data didn't get trampled before freeing */
                struct slot foo = read_cb();
                p = foo.pointer;
                lfsr = foo.lfsr;  /* reset the PRNG to its earlier state */
                size = foo.size;
                printf("free a block, size %d\n\r", size);
                for (i = 0; i < size; i++) {
                    unsigned char u = p[i];
                    unsigned char v = (unsigned char) prng();
                    if (u != v) {
                        printf("OUCH: u=%02X, v=%02X\n\r", u, v);
                        return;
                    }
                }
                vPortFree(p);
                if ((prng() & 1) == 0) break;
            }
        } else {
            printf("allocate a block, size %d\n\r", size);
            write_cb((struct slot){.pointer=p, .size=size, .lfsr=lfsr});
            for (i = 0; i < size; i++) {
                p[i] = (unsigned char) prng();
            }
        }
        if (receive_byte_noblock(&ch) == pdTRUE) {
            break;
        }
    }
}
