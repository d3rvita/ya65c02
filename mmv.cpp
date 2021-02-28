#include "MMV.hpp"


static void lcd2str(const uint8_t *lcd, char *lcd_str)
{
	static const char lcd_lut[128] = {
		' ', '?', '?', '?', '?', '?', '1', '7', '_', '?', '?', '?', '?', '?', '?', '?',
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
		'?', 'T', '?', '?', '?', '?', '?', 'M', 'L', 'C', '?', '?', '?', 'G', '?', '0',
		'-', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '3',
		'r', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '2', '?', '?', 'D', '?',
		'?', '?', '?', '?', '?', '?', '4', '?', '?', '?', '?', '?', '?', '5', 'Y', '9',
		'?', 'F', '?', 'P', '?', '?', 'H', 'A', '?', 'E', 'K', '?', 'B', '6', '?', '8'};

	lcd_str[0] = lcd_lut[lcd[0] & 0x7f];
	lcd_str[1] = (lcd[0] & 0x80)? '.' : ' ';

	lcd_str[2] = lcd_lut[lcd[1] & 0x7f];
	lcd_str[3] = (lcd[1] & 0x80)? '.' : ' ';

	lcd_str[4] = lcd_lut[lcd[2] & 0x7f];
	lcd_str[5] = (lcd[2] & 0x80)? '.' : ' ';

	lcd_str[6] = lcd_lut[lcd[3] & 0x7f];
	lcd_str[7] = (lcd[3] & 0x80)? '.' : ' ';

	lcd_str[8] = 0;
}


static void flags2str(uint8_t flags, char *flags_str)
{
	flags_str[0] = 0;

	if (flags & FLG_BUZZER)		strcat(flags_str, "BUZ ");
	if (flags & FLG_LED_PLAY)	strcat(flags_str, "PLAY ");
	if (flags & FLG_LED_POS)	strcat(flags_str, "POS ");
	if (flags & FLG_LED_MEM)	strcat(flags_str, "MEM ");
	if (flags & FLG_LED_INFO)	strcat(flags_str, "INFO ");
	if (flags & FLG_LED_WHITE)	strcat(flags_str, "WHITE ");
	if (flags & FLG_LED_BLACK)	strcat(flags_str, "BLACK ");
}


uint8_t MMVIO::read(uint16_t addr)
{
	uint8_t val = 0xff;

	if (key == KEY_NONE && key_cnt >= 24) {

		key = pop_key();
		key_cnt = 0;

		if (key == KEY_NONE)
			key_cnt = 24;
	}

	if (addr >= 0x2c00 && addr <= 0x2c07) {

		if ( key != KEY_NONE && 
			(key & FLG_KEY_MUX) == (raw.flags & FLG_KEY_MUX) &&
			(key & 0xf) == (addr & 0xf)) {

			val = 0x7f;

			if (key_cnt >= 16) {
				key = KEY_NONE;
				key_cnt = 0;
			}
		}
		key_cnt++;
	}
	else if (addr == 0x3000) {

		// reed-contacts
	}

	return val;
}


void MMVIO::write(uint16_t addr, uint8_t val)
{
	if (addr >= 0x3400 && addr <= 0x3407) {

		uint8_t flag = 1 << (addr & 0x7);

		if (val & 0x80)
			raw.flags |= flag;
		else
			raw.flags &= ~flag;

		curr.flags = raw.flags & 0x3f;	// ignore BUZZER and MUX

		// update lcd on mux change
		if (addr == 0x3407) {

			uint8_t eor = (raw.flags & FLG_KEY_MUX)? 0xff : 0;
			curr.lcd[0] = raw.lcd[0] ^ eor;
			curr.lcd[1] = raw.lcd[1] ^ eor;
			curr.lcd[2] = raw.lcd[2] ^ eor;
			curr.lcd[3] = raw.lcd[3] ^ eor; 
		}
	}
	else if (addr == 0x2000) {

		raw.lcd[lcd_idx] = val;
		lcd_idx = (lcd_idx+255) % 4;
	}
	else if (addr == 0x2400) {

		// LEDs
	}
	else if (addr == 0x2800) {

		// rank selection
	}

	if (curr != prev) {

		print_disp();
		prev = curr;
	}
}


void MMVIO::print_disp()
{
	char lcd_str[10];
	char flags_str[40];
	lcd2str(curr.lcd, lcd_str);
	flags2str(curr.flags, flags_str);

	printf("%02x %02x %02x %02x (%02x) >%s< %s\n",
			curr.lcd[0], curr.lcd[1], curr.lcd[2], curr.lcd[3], curr.flags, lcd_str, flags_str);
	fflush(stdout);
}


void MMVIO::push_key(MMV_KEY key)
{
	if (key != KEY_NONE) {

		key_mutex.lock();
		key_buffer.push(key);
		key_mutex.unlock();
	}
}


MMV_KEY MMVIO::pop_key()
{
	MMV_KEY key = KEY_NONE;

	key_mutex.lock();

	if (!key_buffer.empty()) {
		key = key_buffer.front();
		key_buffer.pop();
	}

	key_mutex.unlock();

	return key;
}



