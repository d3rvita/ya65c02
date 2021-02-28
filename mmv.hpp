#ifndef _MMV_H_
#define _MMV_H_

#include <mutex>
#include <queue>

#include "ya65c02.hpp"


enum MMV_KEY {

	KEY_NONE  = 0x00,

	KEY_CL    = 0x10,
	KEY_POS   = 0x11,
	KEY_MEM   = 0x12,
	KEY_INFO  = 0x13,
	KEY_LEV   = 0x14,
	KEY_ENT   = 0x15,
	KEY_RIGHT = 0x16,	// white, zero
	KEY_LEFT  = 0x17,	// black, nine

	KEY_A1    = 0x83,	// pawn
	KEY_B2    = 0x85,	// knight
	KEY_C3    = 0x86,	// bishop
	KEY_D4    = 0x87,	// rook
	KEY_E5    = 0x80,	// queen
	KEY_F6    = 0x81,	// king
	KEY_G7    = 0x82,
	KEY_H8    = 0x84,
};


enum MMV_FLAGS {

	FLG_LED_BLACK = 0x01,
	FLG_LED_WHITE = 0x02,
	FLG_LED_INFO  = 0x04,
	FLG_LED_MEM   = 0x08,
	FLG_LED_POS   = 0x10,
	FLG_LED_PLAY  = 0x20,
	FLG_BUZZER    = 0x40,
	FLG_KEY_MUX   = 0x80,
};


struct Display {

	uint8_t flags = 0;
	uint8_t lcd[4] = {0, 0, 0, 0};

	bool operator !=(const Display &d) const
	{
		return	flags != d.flags || 
				lcd[0] != d.lcd[0] ||
				lcd[1] != d.lcd[1] ||
				lcd[2] != d.lcd[2] ||
				lcd[3] != d.lcd[3];
	}
};


class MMVIO : public IOHandler {

	protected:

		uint8_t lcd_idx = 3;

		Display raw, curr, prev;

		MMV_KEY key = KEY_NONE;
		uint8_t key_cnt = 0;

		std::mutex key_mutex;
		std::queue<MMV_KEY> key_buffer;

	public:

		MMVIO() : IOHandler(0xe000, 0x2000) {}

		virtual uint8_t read(uint16_t addr);
		virtual void write(uint16_t addr, uint8_t val);

		void print_disp();

		void push_key(MMV_KEY key);
		MMV_KEY pop_key();
};


class MMV : public YA65C02 {

	public:

		MMVIO io;

		MMV() : YA65C02(io) { set_clock(4915200, 600); }
};

#endif
