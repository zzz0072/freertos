
/* FIXME: There are re-entry issus */
char* itoa_base(int val, int base)
{
    static char buf[32] = { 0 };
    char has_minus = 0;
    int i = 30;

    /* Sepecial case: 0 */
    if (val == 0) {
        buf[1] = '0';
        return &buf[1];
    }

    if (val < 0) {
        val = -val;
        has_minus = 1;
    }

    for (; val && (i - 1) ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];

    if (has_minus) {
        buf[i] = '-';
        return &buf[i];
    }
    return &buf[i + 1];
}
