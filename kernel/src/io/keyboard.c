#include <io/keyboard.h>

#include <io/screen.h>

typedef enum {
	KBD_CMD_SET_LEDS			= 0xED,
	KBD_CMD_ECHO				= 0xEE,
	KBD_CMD_SET_SCANCODE_TABLE	= 0xF0,
	KBD_CMD_GET_ID				= 0xF2,
	KBD_CMD_SET_REPEAT_SPEED	= 0xF3,
	KBD_CMD_ALLOW_SCAN			= 0xF4,
	KBD_CMD_RESET_NO_SCAN		= 0xF5,
	KBD_CMD_RESET_SCAN			= 0xF6,
	KBD_CMD_RESEND				= 0xFE,
	KBD_CMD_RESET				= 0xFF,
} KeyboardCommand_t;

typedef enum {
    KBD_RET_ERROR0   			= 0x00,
    KBD_RET_BAT 				= 0xAA,
    KBD_RET_ECHO				= 0xEE,
    KBD_RET_ACK					= 0xFA,
    KBD_RET_RESEND				= 0xFE,
    KBD_RET_ERROR1   			= 0xFF,
} KeyboardRet_t;

uint8_t kbdus[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b','\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,	/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',	'm', ',', '.', '/', 0,	/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

cyclic_buffer_t* command_buffer;
cyclic_buffer_t* char_buffer;

uint8_t keyboardGetChar() {
	while (char_buffer->count <= 0);

	uint8_t ch;
	cyclic_buffer_dequeue(char_buffer, &ch);
	return ch;
}

bool keyboardReadReady() {
	return char_buffer->count > 0;
}

uint8_t keyboardReadCmd() {
	while (command_buffer->count <= 0);

	uint8_t cmd;
	cyclic_buffer_dequeue(command_buffer, &cmd);
	return cmd;
}

void keyboardSendCmd(uint8_t cmd) {
  outb(0x60, cmd);
}

bool keyboardWaitACK(uint32_t timeout_ms) {
	uint32_t startTick = getSysTimerTicks();
	while (getSysTimerTicks() - startTick < timeout_ms) {
		if (keyboardReadCmd() == KBD_RET_ACK)
			return true;

		yield();
	}

	return false;
}

void keyboardHandler(CPURegisters_t* regs) {
	uint8_t scancode = inb(0x60);
	
	if (scancode >= 0xE0) {
		cyclic_buffer_enqueue(command_buffer, scancode);
	} else if (scancode & 0x80) {

	} else {
		cyclic_buffer_enqueue(char_buffer, kbdus[scancode]);
	}
}

uint8_t initKeyboard() {
	command_buffer = cyclic_buffer_init(KEYBOARD_COMMAND_BUFFER_SIZE);
	char_buffer = cyclic_buffer_init(KEYBOARD_CHAR_BUFFER_SIZE);

	registerInterruptHandler(IRQ1, keyboardHandler);

	// keyboardSendCmd(KBD_CMD_SET_LEDS);
	// if (!keyboardWaitACK(100)) goto _error;

	// keyboardSendCmd(0x0);
	// if (!keyboardWaitACK(100)) goto _error;

	// keyboardSendCmd(KBD_CMD_SET_LEDS);
	// if (!keyboardWaitACK(100)) goto _error;

	// keyboardSendCmd(0x2);
	// if (!keyboardWaitACK(100)) goto _error;

	// keyboardSendCmd(KBD_CMD_SET_REPEAT_SPEED);
	// if (!keyboardWaitACK(100)) goto _error;

	// keyboardSendCmd(0x20);
	// if (!keyboardWaitACK(100)) goto _error;

	// keyboardSendCmd(KBD_CMD_ALLOW_SCAN);
	// if (!keyboardWaitACK(100)) goto _error;

	return 0;
_error:
	unregisterInterruptHandler(IRQ1);

	cyclic_buffer_free(command_buffer);
	cyclic_buffer_free(char_buffer);
	return -1;
}
