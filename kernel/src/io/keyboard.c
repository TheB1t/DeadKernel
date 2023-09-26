#include <io/keyboard.h>

#include <io/screen.h>

#define KBD_CMD_SET_LEDS			0xED
#define KBD_CMD_ECHO				0xEE
#define KBD_CMD_SET_SCANCODE_TABLE	0xF0
#define KBD_CMD_GET_ID				0xF2
#define KBD_CMD_SET_REPEAT_SPEED	0xF3
#define KBD_CMD_ALLOW_SCAN			0xF4
#define KBD_CMD_RESET_NO_SCAN		0xF5
#define KBD_CMD_RESET_SCAN			0xF6
#define KBD_CMD_RESEND				0xFE
#define KBD_CMD_RESET				0xFF

#define KBD_RET_ERROR0	0x00
#define KBD_RET_BAT		0xAA
#define KBD_RET_ECHO	0xEE
#define KBD_RET_ACK		0xFA
#define KBD_RET_RESEND	0xFE
#define KBD_RET_ERROR1	0xFF

uint8_t kbdus[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b','\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,	/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',	'm', ',', '.', '/', 0,	/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* 58 - Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* 70 - Scroll Lock */
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

typedef struct {
	uint8_t scroll_lock : 1;
	uint8_t	num_lock : 1;
	uint8_t caps_lock : 1;
	uint8_t reserved : 5;
} KeyboardLEDState_t;

#define KBD_KEY_PRESSED		0x00
#define KBD_KEY_RELEASED	0x80

#define KBD_KEY_SHIFT		0x2A
#define KBD_KEY_CTRL		0x1D
#define KBD_KEY_ALT			0x38

typedef struct {
	uint8_t shift : 1;
	uint8_t	alt : 1;
	uint8_t ctrl : 1;
	uint8_t reserved : 5;
} KeyboardControlsState_t;


KeyboardLEDState_t led_state = { 0 };
KeyboardControlsState_t controls_state = { 0 };
cyclic_buffer_t* command_buffer = NULL;
cyclic_buffer_t* char_buffer = NULL;

uint8_t keyboardGetChar() {
	// TODO: sometimes we have deadlock here
	while (char_buffer->count <= 0);

	uint8_t ch;
	cyclic_buffer_dequeue(char_buffer, &ch);
	return ch;
}

bool keyboardReadReady() {
	return char_buffer->count > 0;
}

void commandPutWithArg(uint8_t cmd, uint8_t arg) {
	cyclic_buffer_enqueue(command_buffer, cmd);
	cyclic_buffer_enqueue(command_buffer, arg);
}

void commandPutWithoutArg(uint8_t cmd) {
	cyclic_buffer_enqueue(command_buffer, cmd);
}

#define ADD_ENTRY(e) case e: return #e; break

uint8_t* kbd_cmdToName(uint8_t cmd) {
	switch (cmd) {
		ADD_ENTRY(KBD_CMD_SET_LEDS);
		ADD_ENTRY(KBD_CMD_ECHO);
		ADD_ENTRY(KBD_CMD_SET_SCANCODE_TABLE);
		ADD_ENTRY(KBD_CMD_GET_ID);
		ADD_ENTRY(KBD_CMD_SET_REPEAT_SPEED);
		ADD_ENTRY(KBD_CMD_ALLOW_SCAN);
		ADD_ENTRY(KBD_CMD_RESET_NO_SCAN);
		ADD_ENTRY(KBD_CMD_RESET_SCAN);
		ADD_ENTRY(KBD_CMD_RESEND);
		ADD_ENTRY(KBD_CMD_RESET);
	}

	return "Unknown";
}

uint8_t* kbd_retToName(uint8_t ret) {
	switch (ret) {
		ADD_ENTRY(KBD_RET_ERROR0);
		ADD_ENTRY(KBD_RET_BAT);
		ADD_ENTRY(KBD_RET_ECHO);
		ADD_ENTRY(KBD_RET_ACK);
		ADD_ENTRY(KBD_RET_RESEND);
		ADD_ENTRY(KBD_RET_ERROR1);
	}

	return "Unknown";
}

void __kbd_write(uint8_t byte) {
	serialprintf(COM1, "[kbd] Write [%c, 0x%02x, %s]\n", byte, byte, kbd_cmdToName(byte));
	outb(0x60, byte);
}

uint8_t __kbd_read() {
	uint8_t byte = inb(0x60);
	serialprintf(COM1, "[kbd] Read [%c, 0x%02x, %s]\n", byte, byte, kbd_retToName(byte));
	return byte;
}

void keyboardHandler(CPURegisters_t* regs) {
	static uint8_t resend_counter = 0;

	uint8_t tmp = 0;
	uint8_t scancode = __kbd_read();

	switch (scancode) {
		case 0x3a:
			led_state.caps_lock = ~led_state.caps_lock;
			commandPutWithArg(KBD_CMD_SET_LEDS, *((uint8_t*)&led_state));
			goto _send;

		case 0x45:
			led_state.num_lock = ~led_state.num_lock;
			commandPutWithArg(KBD_CMD_SET_LEDS, *((uint8_t*)&led_state));
			goto _send;
		
		case 0x46:
			led_state.scroll_lock = ~led_state.scroll_lock;
			commandPutWithArg(KBD_CMD_SET_LEDS, *((uint8_t*)&led_state));
			goto _send;

		case KBD_RET_RESEND:
			if (resend_counter >= 2) {
				cyclic_buffer_dequeue(command_buffer, &tmp);

				switch (tmp) {
					case KBD_CMD_SET_LEDS:
					case KBD_CMD_SET_SCANCODE_TABLE:
					case KBD_CMD_SET_REPEAT_SPEED:
						cyclic_buffer_dequeue(command_buffer, &tmp);
						break;
				}
				resend_counter = 0;
			} else {
				resend_counter++;
				goto _send;
			}
			break;

		case KBD_RET_ACK:
			cyclic_buffer_dequeue(command_buffer, &tmp);
		
			if (command_buffer->count <= 0)
				break;

		_send:
			cyclic_buffer_read(command_buffer, &tmp);
			__kbd_write(tmp);
			break;

		case KBD_RET_ERROR0:
		case KBD_RET_ERROR1:
			break;

		case KBD_KEY_SHIFT | KBD_KEY_PRESSED: controls_state.shift = 1; break;
		case KBD_KEY_SHIFT | KBD_KEY_RELEASED: controls_state.shift = 0; break;
		
		case KBD_KEY_CTRL | KBD_KEY_PRESSED: controls_state.ctrl = 1; break;
		case KBD_KEY_CTRL | KBD_KEY_RELEASED: controls_state.ctrl = 0; break;
		
		case KBD_KEY_ALT | KBD_KEY_PRESSED: controls_state.alt = 1; break;
		case KBD_KEY_ALT | KBD_KEY_RELEASED: controls_state.alt = 0; break;

		default:
			if (scancode & 0x80) {
				
			} else {
				char ch = kbdus[scancode];
				cyclic_buffer_enqueue(char_buffer, controls_state.shift ? toupper(ch) : ch);
			}
	}
}

uint8_t initKeyboard() {
	command_buffer = cyclic_buffer_init(KEYBOARD_COMMAND_BUFFER_SIZE);
	char_buffer = cyclic_buffer_init(KEYBOARD_CHAR_BUFFER_SIZE);

	registerInterruptHandler(IRQ1, keyboardHandler);

	commandPutWithArg(KBD_CMD_SET_LEDS, *((uint8_t*)&led_state));
	led_state.num_lock = 1;
	commandPutWithArg(KBD_CMD_SET_LEDS, *((uint8_t*)&led_state));
	commandPutWithArg(KBD_CMD_SET_REPEAT_SPEED, 0x20);

	uint8_t tmp = 0;
	cyclic_buffer_read(command_buffer, &tmp);
	__kbd_write(tmp);

	return 0;
_error:
	unregisterInterruptHandler(IRQ1);

	cyclic_buffer_free(command_buffer);
	cyclic_buffer_free(char_buffer);
	return -1;
}
