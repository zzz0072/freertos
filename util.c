enum int_type_t {
    IS_SIGNED_INT,
    IS_UNSIGNED_INT
};


static char* num_to_string(unsigned int val, int base, char *buf, enum int_type_t int_type)
{
    char has_minus = 0;
    int i = 30;

    /* Sepecial case: 0 */
    if (val == 0) {
        buf[1] = '0';
        return &buf[1];
    }

    if (int_type == IS_SIGNED_INT && (int)val < 0) {
        val = (int)-val;
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

char *itoa(int val, char *itoa_buf)
{
    return num_to_string(val, 10, itoa_buf, IS_SIGNED_INT);
}

char *htoa(int val, char *htoa_buf)
{
    return num_to_string(val, 16, htoa_buf, IS_SIGNED_INT);
}

char* addrtoa(long int addr, char *addr_buf)
{
    return num_to_string(addr, 16, addr_buf, IS_UNSIGNED_INT);
}
