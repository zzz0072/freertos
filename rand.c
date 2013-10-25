static unsigned int g_lfsr = 0xACE1;

int rand(void)
{
    /* Info: http://en.wikipedia.org/wiki/Linear_feedback_shift_register */
    /* A ASM version. The original code were:                            */
    /* taps: 16 14 13 11; characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1  */
    /* bit  = ((g_lfsr >> 0) ^ (g_lfsr >> 2) ^ (g_lfsr >> 3) ^ (g_lfsr >> 5) ) & 1; */
    /* g_lfsr =  (g_lfsr >> 1) | (bit << 15);                                       */
    __asm__ ("mov r0, %[pre_lfsr]    \n" /* r0 = g_lfsr             */ \
             "eor r1, r0, r0, lsr #2 \n" /* r1 = r0 ^ (g_lfsr >> 2) */ \
             "eor r1, r1, r0, lsr #3 \n" /* r1 = r1 ^ (g_lfsr >> 3) */ \
             "eor r1, r1, r0, lsr #5 \n" /* r1 = r1 ^ (g_lfsr >> 5) */ \
             "and r1, r1, #1         \n" /* bit = r1 = r1 & 1       */ \
             "lsl r1, r1, #15        \n" /* bit << 15               */ \
             "orr r1, r1, r0, lsr #1 \n" /* g_lfsr = r1 | r2        */ \
             "mov %[result], r1      \n" /* Store result            */ \
             : [result]  "=r" (g_lfsr)   /* Output              */
             : [pre_lfsr] "r" (g_lfsr)   /* Input               */
             : "r0", "r1"                /* clobbered registers */);

    return g_lfsr & 0xffff;
}

void srand(unsigned int seed)
{
    g_lfsr = seed;
}

/* FIXME: Reentrant problem */
unsigned int get_seed(void)
{
    return g_lfsr;
}
