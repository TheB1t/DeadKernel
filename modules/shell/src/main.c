#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

#define INFO "shell v0.1 by Bit\n"

bool _running = true;

void print_prompt() {
    printf("shell(%d)> ", getPID());
}

void handle_command(char* command) {
    char* token = strtok(command, " ");

    if (!token)
        return;

    if (strcmp(token, "info") == 0) {
        printf(INFO);
    } else if (strcmp(token, "getpid") == 0) {
        printf("%d\n", getPID());
    } else if (strcmp(token, "getring") == 0) {
        printf("%d\n", getRing());
    } else if (strcmp(token, "exit") == 0) {
        _running = false;
    } else {
        printf("Command %s not found\n", token);
    }
}

int32_t main() {
    uint32_t commandSize = 128;
    char* command = (char*)malloc(commandSize);
    char* ptr = command;


    printf(INFO);
    print_prompt();

    while (_running) {
        char ch = getch();

        switch (ch) {
            case '\n':
                screenPutChar(ch);

                *ptr++ = '\0';
                ptr = command;

                handle_command(command);
                print_prompt();
                break;
            
            case '\b':
                if (ptr > command) {
                    screenPutChar(ch);
                    ptr--;
                }
                break;

            case '\0':
                break;

            default:
                screenPutChar(ch);
                *ptr++ = ch;  
        }
    }

    printf("\nExiting...\n");
    free(command);
	return 0xDEADBEEF;
}