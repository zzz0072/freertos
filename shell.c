#include "fio.h"
#include "string.h"
#include "host.h"

#define MAX_MSG_CHARS (32)

#define BACKSPACE (127)
#define ESC        (27)

#define RT_NO  (0)
#define RT_YES (1)

extern char receive_byte();

static void read_token(char *token, int max_token_chars)
{
    char ch = 0;
    char last_char_is_ESC = RT_NO;
    int curr_char;

    curr_char = 0;
    while(1) {
        /* Receive a byte from the stdin. */
        fio_read(0, &ch, 1);

        /* Handle ESC case first */
        if (last_char_is_ESC == RT_YES) {
            last_char_is_ESC = RT_NO;

            if (ch == '[') {
                /* Direction key: ESC[A ~ ESC[D */
                fio_read(0, &ch, 1);

                /* Home:      ESC[1~
                 * End:       ESC[2~
                 * Insert:    ESC[3~
                 * Delete:    ESC[4~
                 * Page up:   ESC[5~
                 * Page down: ESC[6~ */
                if (ch >= '1' && ch <= '6') {
                    fio_read(0, &ch, 1);
                }
                continue;
            }
        }

        /* If the byte is an end-of-line type character, then
         * finish the string and inidcate we are done.
         */
        if (curr_char >= (max_token_chars - 2) || \
            (ch == '\r') || (ch == '\n')) {
            *(token + curr_char) = '\n';
            *(token + curr_char + 1) = '\0';
            break;
        }
        else if(ch == ESC) {
            last_char_is_ESC = RT_YES;
        }
        /* Skip control characters. man ascii for more information */
        else if (ch < 0x20) {
            continue;
        }
        else if(ch == BACKSPACE) { /* backspace */
            if(curr_char > 0) {
                curr_char--;
                printf("\b \b");
            }
        }
        else {
            /* Appends only when buffer is not full.
             * Include \n\0 */
            *(token + curr_char++) = ch;
            printf("%c", ch);
        }
    }
}

typedef void (*cmd_func_t)(void);
struct cmd_t
{
    char *name;
    char *desc;
    cmd_func_t handler;
};

static void help_menu(void);
static void system(void);

typedef struct cmd_t cmd_entry;
static cmd_entry available_cmds[] = {
        {
            .name = "help",
            .desc = "This menu",
            .handler = help_menu
        },
        {
            .name = "system",
            .desc = "system\n\r\t\tRun host command",
            .handler = system
        }
};

static void system(void)
{
    char host_cmd[MAX_MSG_CHARS];

    printf("Enter host command: ");
    read_token(host_cmd, MAX_MSG_CHARS);

    if (strlen(host_cmd) < MAX_MSG_CHARS - 1 && host_cmd[0] != '\n') {
        host_system(host_cmd, strlen(host_cmd));
    }
}

static void help_menu(void)
{
    int i = 0;

    printf("Available Commands:\n\r");
    for (i = 0; i < sizeof(available_cmds)/sizeof(cmd_entry); i++) {
        printf("%s\t\t%s\n\r", available_cmds[i].name, available_cmds[i].desc);
    }
}

static void proc_cmd(char *cmd)
{
    int i = 0;

    /* Lets process command */
    for (i = 0; i < sizeof(available_cmds)/sizeof(cmd_entry); i++) {
        if (strncmp(cmd, available_cmds[i].name, strlen(available_cmds[i].name)) == 0) {
            /* Avoid subset case -> valid cmd: "ps" vs user input: "ps1" */
            if (cmd[strlen(available_cmds[i].name)] != '\n' ) {
                continue;
            }
            available_cmds[i].handler();
            return;
        }
    }
    printf("Command not found.\n\r");
}

void shell_task(void *pvParameters)
{
    char str[MAX_MSG_CHARS];

    help_menu();
    while (1) {
        /* Show prompt */
        printf("\n\r$ ");
        read_token(str, MAX_MSG_CHARS);

        /* Process command */
        if (strlen(str) < MAX_MSG_CHARS - 1 && str[0] != '\n') {
            printf("\n\r");
            proc_cmd(str);
        }
    }
}
