#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

#define INFO "shell v0.1 by Bit\n"

bool _running = true;

void memPrint(uint8_t* mem, uint32_t size) {
	#define OUT_W 16
	
	printf("---- memory start at 0x%08x ----\n", mem);
	for (uint32_t i = 0; i < size / OUT_W; i++) {
		printf("%08x | ", mem + (OUT_W * i));
		for (uint32_t j = 0; j < OUT_W; j++) {
			printf("%02x ", mem[(OUT_W * i) + j]);
		}
		printf("| ");
		for (uint32_t j = 0; j < OUT_W; j++) {
			if (mem[(OUT_W * i) + j]  > 31)
				printf("%.1c", mem[(OUT_W * i) + j]);
			else
				printf(".");
		}
		printf("\n");
	}
	printf("---- memory end at 0x%08x ----\n", mem + size);
}

void print_prompt() {
    printf("shell(%d)> ", getPID());
}

void handle_command(char* command) {
    char* argv[16];
    uint32_t argc = 0;

    char* cmd = strtok(command, " ");

    if (!cmd)
        return;

    for (; argc < 16; argc++) {
        char* arg = strtok(NULL, " ");

        if (arg == NULL)
            break;

        argv[argc] = arg;
    }

    #define COMMAND0(c)   if (strcmp(cmd, #c) == 0) {
    #define COMMAND(c)    } else if (strcmp(cmd, #c) == 0) {
    #define BAD_COMMAND()   } else {
    #define END()           }

    COMMAND0(info)
        printf(INFO);
   
    COMMAND(getpid)
        printf("%d\n", getPID());
   
    COMMAND(getring)
        printf("%d\n", getRing());
    
    COMMAND(readmem)        
        if (argc < 2)
            return;

        int32_t address = strtoi(argv[0], NULL, 16);
        int32_t count = atoi(argv[1]);
        
        memPrint((uint8_t*)address, (uint32_t)count);

    COMMAND(echo)
        for (uint32_t i = 0; i < argc; i++)
            printf("%s ", argv[i]);

        printf("\n");
        
    COMMAND(exit)
        _running = false;

    BAD_COMMAND()
        printf("Command %s not found\n", cmd);
    
    END()
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