#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

#define INFO "shell v0.1 by Bit\n"

bool _running = true;
char command[128];
char* ptr = command;

void print_prompt() {
    printf("shell(%d)> ", getPID());
}

void handle_command(char* cmd) {
    if (strcmp(cmd, "info") == 0) {
        printf(INFO);
    } else if (strcmp(cmd, "getpid") == 0) {
        printf("%d\n", getPID());
    } else if (strcmp(cmd, "getring") == 0) {
        printf("%d\n", getRing());
    } else if (strcmp(cmd, "exit") == 0) {
        _running = false;
    } else {
        printf("Command %s not found\n", cmd);
    }
}

int32_t main() {
    printf(INFO);
    print_prompt();

    while (_running) {
        char ch = getch();

        if (ch == '\n') {
            screenPutChar(ch);

            *ptr++ = '\0';
            ptr = command;

            handle_command(command);
            print_prompt();
        } else if (ch == '\b') {
            if (ptr > command) {
                screenPutChar(ch);
                ptr--;
            }
        } else {
            screenPutChar(ch);
            *ptr++ = ch;
        }      
    }

    printf("Exiting...\n");
	return 0xDEADBEEF;
}