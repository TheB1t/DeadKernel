#include <io/keyboard.h>

char getch() {
    while (!keyboardReadReady()) {
        // Double check
        if (keyboardReadReady())
            break;

        yield();
    }

    return keyboardGetChar();
}