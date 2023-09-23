#include <io/keyboard.h>

char getch() {
    while (!keyboardReadReady());
        yield();

    return keyboardGetChar();
}