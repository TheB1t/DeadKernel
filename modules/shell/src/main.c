#include <common.h>
#include <syscall.h>

#define INFO "shell v0.1 by Bit\n"

char command[128];
char* ptr = command;

void print_prompt() {
    printf("shell> ");
}

void handle_command(char* cmd) {
    if (strcmp(cmd, "info") == 0) {
        printf(INFO);
    } else if (strcmp(cmd, "getpid") == 0) {
        printf("%d\n", getPID());
    } else if (strcmp(cmd, "getcpl") == 0) {
        printf("%d\n", getCPL());
    } else {
        printf("Command %s not found\n", cmd);
    }
}

int32_t main() {
    printf(INFO);
    print_prompt();

    while (1) {
        char ch = getch();

        if (ch == '\n') {
            screenPutChar(ch);

            *ptr++ = '\0';
            ptr = command;

            handle_command(command);
            print_prompt();
        } else if (ch == '\b') {
            if (ptr > command)
                screenPutChar(ch);
        } else {
            screenPutChar(ch);
            *ptr++ = ch;
        }      
    }

	return 0xDEADBEEF;
}
