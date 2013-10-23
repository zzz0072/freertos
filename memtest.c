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
unsigned int g_write_pointer, g_read_pointer;
static struct slot g_slots[CIRCBUFSIZE];
static unsigned int g_lfsr = 0xACE1;

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
        return (struct slot){ .pointer=NULL, .size=0, .lfsr=0 };
    }
    foo = g_slots[g_read_pointer++];
    g_read_pointer %= CIRCBUFSIZE;
    return foo;
}


/* Get a pseudorandom number generator from Wikipedia */
static int prng(void)
{
    /* Info: http://en.wikipedia.org/wiki/Linear_feedback_shift_register */
    /* A ASM version. The original code were:                            */
    /* taps: 16 14 13 11; characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1  */
    /* bit  = ((g_lfsr >> 0) ^ (g_lfsr >> 2) ^ (g_lfsr >> 3) ^ (g_lfsr >> 5) ) & 1; */
    /* g_lfsr =  (g_lfsr >> 1) | (bit << 15);                                       */
    __asm__ ("mov r0, %[pre_lfsr]    \n"                                          \
             "eor r1, r0, r0, lsr #2 \n" /* r1 = (g_lfsr >> 0) ^ (g_lfsr >> 2) */ \
             "eor r1, r1, r0, lsr #3 \n" /* r1 = r1 ^ (g_lfsr >> 3) */            \
             "eor r1, r1, r0, lsr #5 \n" /* r1 = r1 ^ (g_lfsr >> 5) */            \
             "and r1, r1, #1         \n" /* bit = r1 = r1 & 1       */            \
             "lsl r1, r1, #15        \n" /* bit << 15               */            \
             "orr r1, r1, r0, lsr #1 \n" /* g_lfsr = r1 | r2        */            \
             "mov %[result], r1      \n" /* Store result            */            \
             : [result]  "=r" (g_lfsr)   /* Output              */
             : [pre_lfsr] "r" (g_lfsr)   /* Input               */
             : "r0", "r1"                /* clobbered registers */);

    return g_lfsr & 0xffff;
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
                g_lfsr = foo.lfsr;  /* reset the PRNG to its earlier state */
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
            write_cb((struct slot){.pointer=p, .size=size, .lfsr=g_lfsr});
            for (i = 0; i < size; i++) {
                p[i] = (unsigned char) prng();
            }
        }
        if (receive_byte_noblock(&ch) == pdTRUE) {
            break;
        }
    }
}
