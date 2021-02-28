#ifndef _YA65C02_H_
#define _YA65C02_H_

#include <stdint.h>


enum OP {

	ADC, AND, ASL,
	BBR0, BBR1, BBR2, BBR3, BBR4, BBR5, BBR6, BBR7,
	BBS0, BBS1, BBS2, BBS3, BBS4, BBS5, BBS6, BBS7,
	BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRA, BRK, BVC, BVS,
	CLC, CLD, CLI, CLV, CMP, CPX, CPY,
	DEC, DEX, DEY,
	EOR,
	INC, INX, INY,
	JMP, JSR,
	LDA, LDX, LDY, LSR,
	NOP,
	ORA,
	PHA, PHP, PHX, PHY, PLA, PLP, PLX, PLY,
	RMB0, RMB1, RMB2, RMB3, RMB4, RMB5, RMB6, RMB7, 
	ROL, ROR, RTI, RTS,
	SBC, SEC, SED, SEI, 
	SMB0, SMB1, SMB2, SMB3, SMB4, SMB5, SMB6, SMB7, 
	STA, STX, STY, STZ,
	TAX, TAY, TRB, TSB, TSX, TXA, TXS, TYA,

	ERR
};


enum AM {

	// nr. / cycles / inst. size // inst. size [byte], ...
	ABSOLUTE	= 0x043,		// 3
	ABSOLUTE_X	= 0x143,		// 3
	ABSOLUTE_Y	= 0x243,		// 3
	ABS_IND		= 0x363,		// 3, JMP only (absolute indirect)
	ABS_IND_X	= 0x463,		// 3, JMP only
	ACCUMULATOR	= 0x521,		// 1
	IMMEDIATE	= 0x622,		// 2
	IMPLIED		= 0x721,		// 1
	INDIRECT	= 0x852,		// 2, 3 bytes for JMP -> handled as ABS_IND
	INDIRECT_X	= 0x962,		// 2
	INDIRECT_Y	= 0xa52,		// 2
	RELATIVE	= 0xb22,		// 2, branch inst. only
	ZEROPAGE	= 0xc32,		// 2
	ZEROPAGE_X	= 0xd42,		// 2
	ZEROPAGE_Y	= 0xe42,		// 2
	ZEROP_REL 	= 0xf23,		// 3, BBRi and BBSi only (zeropage relative)
};


enum P_FLAG {

	N = 0x80,
	V = 0x40,
	B = 0x10,
	D = 0x08,
	I = 0x04,
	Z = 0x02,
	C = 0x01,
};


const uint16_t NMI_VEC = 0xfffa;
const uint16_t RST_VEC = 0xfffc;
const uint16_t IRQ_VEC = 0xfffe;


struct CMD {
	uint8_t op_code;
	enum OP op;
	enum AM am;
	const char *mnem;
};

extern const CMD cmds[256];


class IOHandler {

	protected:

		bool alive = true;
		uint16_t addr_mask, addr_filter;

	public:

		IOHandler(uint16_t mask, uint16_t filter) : addr_mask(mask), addr_filter(filter) {}

		bool is_alive()	{ return alive; }

		bool is_valid_address(uint16_t addr)	{ return (addr & addr_mask) == addr_filter; }

		virtual uint8_t read(uint16_t addr)				{ return 0; }
		virtual void write(uint16_t addr, uint8_t val)	{}
};


class Memory {

	protected:

		uint8_t a, x, y, s, p;
		uint16_t pc;

		uint8_t mem[64 * 1024];

		IOHandler &io;

	public:

		Memory(IOHandler &io_handler) : io(io_handler) { init(); }

		void init();

		void update_flag(P_FLAG flag, bool set);
		void update_nz_flags(uint8_t val);

		uint8_t *mem_ptr(uint16_t idx)	{ return &mem[idx]; }
		uint8_t *zp_ptr(uint8_t idx)	{ return &mem[idx]; }

		void push(uint8_t val);
		uint8_t pull();

		void push16(uint16_t val);
		uint16_t pull16();

		uint8_t read(uint16_t addr);
		void write(uint16_t addr, uint8_t val);
};


class YA65C02 : public Memory {

	protected:

		uint64_t instr_cnt = 0;
		uint64_t cycle_cnt = 0;

		uint32_t cpu_clock = 1e6;	// in Hz
		uint32_t nmi_clock = 1000;	// in Hz

	public:

		YA65C02(IOHandler &io_handler) : Memory(io_handler) {}

		void set_clock(uint32_t cpu_clk, uint32_t nmi_clk) { cpu_clock = cpu_clk; nmi_clock = nmi_clk; }

		bool load(const uint8_t *rom, size_t rom_size);
		bool load(const char *rom_path);

		void setup_isr();

		uint8_t *decode_operand();

		void tick();
		void run();
};

#endif
