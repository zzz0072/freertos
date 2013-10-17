#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <unistd.h>
#include <stdarg.h> /* For variable list */

#include "fio.h"
#include "filesystem.h"
#include "osdebug.h"
#include "hash-djb2.h"
#include "util.h"


/* send_byte() is in main.c
 * TODO: Separate USART function out of main.c */
extern void send_byte(char ch);
extern char receive_byte();


static struct fddef_t fio_fds[MAX_FDS];

static ssize_t stdin_read(void * opaque, void * buf, size_t count) {
    int i;
    char * data = buf;

    for (i = 0; i < count; i++)
        data[i] = receive_byte();

    return count;
}

static ssize_t stdout_write(void * opaque, const void * buf, size_t count) {
    int i;
    const char * data = (const char *) buf;

    for (i = 0; i < count; i++)
        send_byte(data[i]);

    return count;
}

static xSemaphoreHandle fio_sem = NULL;

__attribute__((constructor)) void fio_init() {
    memset(fio_fds, 0, sizeof(fio_fds));
    fio_fds[0].fdread = stdin_read;
    fio_fds[1].fdwrite = stdout_write;
    fio_fds[2].fdwrite = stdout_write;
    fio_sem = xSemaphoreCreateMutex();
}

struct fddef_t * fio_getfd(int fd) {
    if ((fd < 0) || (fd >= MAX_FDS))
        return NULL;
    return fio_fds + fd;
}

static int fio_is_open_int(int fd) {
    if ((fd < 0) || (fd >= MAX_FDS))
        return 0;
    int r = !((fio_fds[fd].fdread == NULL) &&
              (fio_fds[fd].fdwrite == NULL) &&
              (fio_fds[fd].fdseek == NULL) &&
              (fio_fds[fd].fdclose == NULL) &&
              (fio_fds[fd].opaque == NULL));
    return r;
}

static int fio_findfd() {
    int i;

    for (i = 0; i < MAX_FDS; i++) {
        if (!fio_is_open_int(i))
            return i;
    }

    return -1;
}

int fio_is_open(int fd) {
    int r = 0;
    xSemaphoreTake(fio_sem, portMAX_DELAY);
    r = fio_is_open_int(fd);
    xSemaphoreGive(fio_sem);
    return r;
}

int fio_open(fdread_t fdread, fdwrite_t fdwrite, fdseek_t fdseek, fdclose_t fdclose, void * opaque) {
    int fd;
//    DBGOUT("fio_open(%p, %p, %p, %p, %p)\r\n", fdread, fdwrite, fdseek, fdclose, opaque);
    xSemaphoreTake(fio_sem, portMAX_DELAY);
    fd = fio_findfd();

    if (fd >= 0) {
        fio_fds[fd].fdread = fdread;
        fio_fds[fd].fdwrite = fdwrite;
        fio_fds[fd].fdseek = fdseek;
        fio_fds[fd].fdclose = fdclose;
        fio_fds[fd].opaque = opaque;
    }
    xSemaphoreGive(fio_sem);

    return fd;
}

ssize_t fio_read(int fd, void * buf, size_t count) {
    ssize_t r = 0;
//    DBGOUT("fio_read(%i, %p, %i)\r\n", fd, buf, count);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdread) {
            r = fio_fds[fd].fdread(fio_fds[fd].opaque, buf, count);
        } else {
            r = -3;
        }
    } else {
        r = -2;
    }
    return r;
}

ssize_t fio_write(int fd, const void * buf, size_t count) {
    ssize_t r = 0;
//    DBGOUT("fio_write(%i, %p, %i)\r\n", fd, buf, count);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdwrite) {
            r = fio_fds[fd].fdwrite(fio_fds[fd].opaque, buf, count);
        } else {
            r = -3;
        }
    } else {
        r = -2;
    }
    return r;
}

off_t fio_seek(int fd, off_t offset, int whence) {
    off_t r = 0;
//    DBGOUT("fio_seek(%i, %i, %i)\r\n", fd, offset, whence);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdseek) {
            r = fio_fds[fd].fdseek(fio_fds[fd].opaque, offset, whence);
        } else {
            r = -3;
        }
    } else {
        r = -2;
    }
    return r;
}

int fio_close(int fd) {
    int r = 0;
//    DBGOUT("fio_close(%i)\r\n", fd);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdclose)
            r = fio_fds[fd].fdclose(fio_fds[fd].opaque);
        xSemaphoreTake(fio_sem, portMAX_DELAY);
        memset(fio_fds + fd, 0, sizeof(struct fddef_t));
        xSemaphoreGive(fio_sem);
    } else {
        r = -2;
    }
    return r;
}

void fio_set_opaque(int fd, void * opaque) {
    if (fio_is_open_int(fd))
        fio_fds[fd].opaque = opaque;
}

#define stdin_hash 0x0BA00421
#define stdout_hash 0x7FA08308
#define stderr_hash 0x7FA058A3

static int devfs_open(void * opaque, const char * path, int flags, int mode) {
    uint32_t h = hash_djb2((const uint8_t *) path, -1);
//    DBGOUT("devfs_open(%p, \"%s\", %i, %i)\r\n", opaque, path, flags, mode);
    switch (h) {
    case stdin_hash:
        if (flags & (O_WRONLY | O_RDWR))
            return -1;
        return fio_open(stdin_read, NULL, NULL, NULL, NULL);
        break;
    case stdout_hash:
        if (flags & O_RDONLY)
            return -1;
        return fio_open(NULL, stdout_write, NULL, NULL, NULL);
        break;
    case stderr_hash:
        if (flags & O_RDONLY)
            return -1;
        return fio_open(NULL, stdout_write, NULL, NULL, NULL);
        break;
    }
    return -1;
}

void register_devfs() {
    DBGOUT("Registering devfs.\r\n");
    register_fs("dev", devfs_open, NULL);
}

int puts(const char *msg)
{
    if (!msg) {
        return -1;
    }

    return (int)fio_write(1, msg, strlen(msg));
}

static int printf_cb(char *dest, const char *src)
{
    return puts(src);
}

static int sprintf_cb(char *dest, const char *src)
{
    return (int)strcat(dest, src);
}

typedef int (*proc_str_func_t)(char *, const char *);

/* Common body for sprintf and printf */
static int base_printf(proc_str_func_t proc_str, \
                char *dest, const char *fmt_str, va_list param)
{
    char  param_chr[] = {0, 0}; 
    int   param_int = 0;
    
    long int param_lint = 0;

    char *str_to_output = 0;
    int   curr_char  = 0;

    /* Make sure strlen(dest) is 0
     * for first strcat */
    if (dest) {
        dest[0] = 0;
    }

    /* Let's parse */
    while (fmt_str[curr_char]) {
        /* Deal with normal string
         * increase index by 1 here  */
        if (fmt_str[curr_char++] != '%') {
            param_chr[0]  = fmt_str[curr_char - 1];
            str_to_output = param_chr;
        }
        /* % case-> retrive latter params */
        else {
            switch (fmt_str[curr_char]) {
                case 'S':
                case 's':
                    {
                        str_to_output = va_arg(param, char *);
                    }
                    break;

                case 'd':
                case 'D':
                    {
                       param_int     = va_arg(param, int);
                       str_to_output = itoa(param_int);
                    }
                    break;

                case 'X':
                case 'x':
                    {
                       param_int     = va_arg(param, int);
                       str_to_output = addrtoa(param_int);
                    }
                    break;

                case 'P':
                case 'p':
                    {
                       param_lint     = va_arg(param, long int);
                       str_to_output = htoa(param_lint);
                    }
                    break;

                case 'c':
                case 'C':
                    {
                        param_chr[0]  = (char) va_arg(param, int);
                        str_to_output = param_chr;
                        break;
                    }

                default:
                    {
                        param_chr[0]  = fmt_str[curr_char];
                        str_to_output = param_chr;
                    }
            } /* switch (fmt_str[curr_char])      */
            curr_char++;
        }     /* if (fmt_str[curr_char++] == '%') */
        proc_str(dest, str_to_output);
    }         /* while (fmt_str[curr_char])       */

    return curr_char;
}

int sprintf(char *str, const char *format, ...)
{
    int rval = 0;
    va_list param = {0};

    va_start(param, format);
    rval = base_printf(sprintf_cb, (char *)str, format, param);
    va_end(param);

    return rval;
}

int printf(const char *format, ...)
{
    int rval = 0;
    va_list param = {0};

    va_start(param, format);
    rval = base_printf(printf_cb, (char *)0, format, param);
    va_end(param);

    return rval;
}
