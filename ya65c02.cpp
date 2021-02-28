#include <string.h>
#include <assert.h>

#include <thread>
#include <chrono>

#include "YA65C02.hpp"


#define _(op_code, op, am) {op_code, op, am, #op}

const CMD cmds[256] = {

_(0x00, BRK, IMPLIED),    _(0x01, ORA, INDIRECT_X), _(0x02, NOP, INDIRECT),    _(0x03, NOP,  IMPLIED),
_(0x04, TSB, ZEROPAGE),   _(0x05, ORA, ZEROPAGE),   _(0x06, ASL, ZEROPAGE),    _(0x07, RMB0, ZEROPAGE),
_(0x08, PHP, IMPLIED),    _(0x09, ORA, IMMEDIATE),  _(0x0a, ASL, ACCUMULATOR), _(0x0b, NOP,  IMPLIED),
_(0x0c, TSB, ABSOLUTE),   _(0x0d, ORA, ABSOLUTE),   _(0x0e, ASL, ABSOLUTE),    _(0x0f, BBR0, ZEROP_REL),

_(0x10, BPL, RELATIVE),   _(0x11, ORA, INDIRECT_Y), _(0x12, ORA, INDIRECT),    _(0x13, NOP,  IMPLIED),
_(0x14, TRB, ZEROPAGE),   _(0x15, ORA, ZEROPAGE_X), _(0x16, ASL, ZEROPAGE_X),  _(0x17, RMB1, ZEROPAGE),
_(0x18, CLC, IMPLIED),    _(0x19, ORA, ABSOLUTE_Y), _(0x1a, INC, ACCUMULATOR), _(0x1b, NOP,  IMPLIED),
_(0x1c, TRB, ABSOLUTE),   _(0x1d, ORA, ABSOLUTE_X), _(0x1e, ASL, ABSOLUTE_X),  _(0x1f, BBR1, ZEROP_REL),

_(0x20, JSR, ABSOLUTE),   _(0x21, AND, INDIRECT_X), _(0x22, NOP, INDIRECT),    _(0x23, NOP,  IMPLIED),
_(0x24, BIT, ZEROPAGE),   _(0x25, AND, ZEROPAGE),   _(0x26, ROL, ZEROPAGE),    _(0x27, RMB2, ZEROPAGE),
_(0x28, PLP, IMPLIED),    _(0x29, AND, IMMEDIATE),  _(0x2a, ROL, ACCUMULATOR), _(0x2b, NOP,  IMPLIED),
_(0x2c, BIT, ABSOLUTE),   _(0x2d, AND, ABSOLUTE),   _(0x2e, ROL, ABSOLUTE),    _(0x2f, BBR2, ZEROP_REL),

_(0x30, BMI, RELATIVE),   _(0x31, AND, INDIRECT_Y), _(0x32, AND, INDIRECT),    _(0x33, NOP,  IMPLIED),
_(0x34, BIT, ZEROPAGE_X), _(0x35, AND, ZEROPAGE_X), _(0x36, ROL, ZEROPAGE_X),  _(0x37, RMB3, ZEROPAGE),
_(0x38, SEC, IMPLIED),    _(0x39, AND, ABSOLUTE_Y), _(0x3a, DEC, ACCUMULATOR), _(0x3b, NOP,  IMPLIED),
_(0x3c, BIT, ABSOLUTE_X), _(0x3d, AND, ABSOLUTE_X), _(0x3e, ROL, ABSOLUTE_X),  _(0x3f, BBR3, ZEROP_REL),

_(0x40, RTI, IMPLIED),    _(0x41, EOR, INDIRECT_X), _(0x42, NOP, INDIRECT),    _(0x43, NOP,  IMPLIED),
_(0x44, NOP, ZEROPAGE),   _(0x45, EOR, ZEROPAGE),   _(0x46, LSR, ZEROPAGE),    _(0x47, RMB4, ZEROPAGE),
_(0x48, PHA, IMPLIED),    _(0x49, EOR, IMMEDIATE),  _(0x4a, LSR, ACCUMULATOR), _(0x4b, NOP,  IMPLIED),
_(0x4c, JMP, ABSOLUTE),   _(0x4d, EOR, ABSOLUTE),   _(0x4e, LSR, ABSOLUTE),    _(0x4f, BBR4, ZEROP_REL),

_(0x50, BVC, RELATIVE),   _(0x51, EOR, INDIRECT_Y), _(0x52, EOR, INDIRECT),    _(0x53, NOP,  IMPLIED),
_(0x54, NOP, ZEROPAGE),   _(0x55, EOR, ZEROPAGE_X), _(0x56, LSR, ZEROPAGE_X),  _(0x57, RMB5, ZEROPAGE),
_(0x58, CLI, IMPLIED),    _(0x59, EOR, ABSOLUTE_Y), _(0x5a, PHY, IMPLIED),     _(0x5b, NOP,  IMPLIED),
_(0x5c, NOP, ABSOLUTE),   _(0x5d, EOR, ABSOLUTE_X), _(0x5e, LSR, ABSOLUTE_X),  _(0x5f, BBR5, ZEROP_REL),

_(0x60, RTS, IMPLIED),    _(0x61, ADC, INDIRECT_X), _(0x62, NOP, INDIRECT),    _(0x63, NOP,  IMPLIED),
_(0x64, STZ, ZEROPAGE),   _(0x65, ADC, ZEROPAGE),   _(0x66, ROR, ZEROPAGE),    _(0x67, RMB6, ZEROPAGE),
_(0x68, PLA, IMPLIED),    _(0x69, ADC, IMMEDIATE),  _(0x6a, ROR, ACCUMULATOR), _(0x6b, NOP,  IMPLIED),
_(0x6c, JMP, ABS_IND),    _(0x6d, ADC, ABSOLUTE),   _(0x6e, ROR, ABSOLUTE),    _(0x6f, BBR6, ZEROP_REL),

_(0x70, BVS, RELATIVE),   _(0x71, ADC, INDIRECT_Y), _(0x72, ADC, INDIRECT),    _(0x73, NOP,  IMPLIED),
_(0x74, STZ, ZEROPAGE_X), _(0x75, ADC, ZEROPAGE_X), _(0x76, ROR, ZEROPAGE_X),  _(0x77, RMB7, ZEROPAGE),
_(0x78, SEI, IMPLIED),    _(0x79, ADC, ABSOLUTE_Y), _(0x7a, PLY, IMPLIED),     _(0x7b, NOP,  IMPLIED),
_(0x7c, JMP, ABS_IND_X),  _(0x7d, ADC, ABSOLUTE_X), _(0x7e, ROR, ABSOLUTE_X),  _(0x7f, BBR7, ZEROP_REL),

_(0x80, BRA, RELATIVE),   _(0x81, STA, INDIRECT_X), _(0x82, NOP, INDIRECT),    _(0x83, NOP,  IMPLIED),
_(0x84, STY, ZEROPAGE),   _(0x85, STA, ZEROPAGE),   _(0x86, STX, ZEROPAGE),    _(0x87, SMB0, ZEROPAGE),
_(0x88, DEY, IMPLIED),    _(0x89, BIT, IMMEDIATE),  _(0x8a, TXA, IMPLIED),     _(0x8b, NOP,  IMPLIED),
_(0x8c, STY, ABSOLUTE),   _(0x8d, STA, ABSOLUTE),   _(0x8e, STX, ABSOLUTE),    _(0x8f, BBS0, ZEROP_REL),

_(0x90, BCC, RELATIVE),   _(0x91, STA, INDIRECT_Y), _(0x92, STA, INDIRECT),    _(0x93, NOP,  IMPLIED),
_(0x94, STY, ZEROPAGE_X), _(0x95, STA, ZEROPAGE_X), _(0x96, STX, ZEROPAGE_Y),  _(0x97, SMB1, ZEROPAGE),
_(0x98, TYA, IMPLIED),    _(0x99, STA, ABSOLUTE_Y), _(0x9a, TXS, IMPLIED),     _(0x9b, NOP,  IMPLIED),
_(0x9c, STZ, ABSOLUTE),   _(0x9d, STA, ABSOLUTE_X), _(0x9e, STZ, ABSOLUTE_X),  _(0x9f, BBS1, ZEROP_REL),

_(0xa0, LDY, IMMEDIATE),  _(0xa1, LDA, INDIRECT_X), _(0xa2, LDX, IMMEDIATE),   _(0xa3, NOP,  IMPLIED),
_(0xa4, LDY, ZEROPAGE),   _(0xa5, LDA, ZEROPAGE),   _(0xa6, LDX, ZEROPAGE),    _(0xa7, SMB2, ZEROPAGE),
_(0xa8, TAY, IMPLIED),    _(0xa9, LDA, IMMEDIATE),  _(0xaa, TAX, IMPLIED),     _(0xab, NOP,  IMPLIED),
_(0xac, LDY, ABSOLUTE),   _(0xad, LDA, ABSOLUTE),   _(0xae, LDX, ABSOLUTE),    _(0xaf, BBS2, ZEROP_REL),

_(0xb0, BCS, RELATIVE),   _(0xb1, LDA, INDIRECT_Y), _(0xb2, LDA, INDIRECT),    _(0xb3, NOP,  IMPLIED),
_(0xb4, LDY, ZEROPAGE_X), _(0xb5, LDA, ZEROPAGE_X), _(0xb6, LDX, ZEROPAGE_Y),  _(0xb7, SMB3, ZEROPAGE),
_(0xb8, CLV, IMPLIED),    _(0xb9, LDA, ABSOLUTE_Y), _(0xba, TSX, IMPLIED),     _(0xbb, NOP,  IMPLIED),
_(0xbc, LDY, ABSOLUTE_X), _(0xbd, LDA, ABSOLUTE_X), _(0xbe, LDX, ABSOLUTE_Y),  _(0xbf, BBS3, ZEROP_REL),

_(0xc0, CPY, IMMEDIATE),  _(0xc1, CMP, INDIRECT_X), _(0xc2, NOP, INDIRECT),    _(0xc3, NOP,  IMPLIED),
_(0xc4, CPY, ZEROPAGE),   _(0xc5, CMP, ZEROPAGE),   _(0xc6, DEC, ZEROPAGE),    _(0xc7, SMB4, ZEROPAGE),
_(0xc8, INY, IMPLIED),    _(0xc9, CMP, IMMEDIATE),  _(0xca, DEX, IMPLIED),     _(0xcb, ERR,  IMPLIED),
_(0xcc, CPY, ABSOLUTE),   _(0xcd, CMP, ABSOLUTE),   _(0xce, DEC, ABSOLUTE),    _(0xcf, BBS4, ZEROP_REL),

_(0xd0, BNE, RELATIVE),   _(0xd1, CMP, INDIRECT_Y), _(0xd2, CMP, INDIRECT),    _(0xd3, NOP,  IMPLIED),
_(0xd4, NOP, ZEROPAGE),   _(0xd5, CMP, ZEROPAGE_X), _(0xd6, DEC, ZEROPAGE_X),  _(0xd7, SMB5, ZEROPAGE),
_(0xd8, CLD, IMPLIED),    _(0xd9, CMP, ABSOLUTE_Y), _(0xda, PHX, IMPLIED),     _(0xdb, ERR,  IMPLIED),
_(0xdc, NOP, ABSOLUTE),   _(0xdd, CMP, ABSOLUTE_X), _(0xde, DEC, ABSOLUTE_X),  _(0xdf, BBS5, ZEROP_REL),

_(0xe0, CPX, IMMEDIATE),  _(0xe1, SBC, INDIRECT_X), _(0xe2, NOP, INDIRECT),    _(0xe3, NOP,  IMPLIED),
_(0xe4, CPX, ZEROPAGE),   _(0xe5, SBC, ZEROPAGE),   _(0xe6, INC, ZEROPAGE),    _(0xe7, SMB6, ZEROPAGE),
_(0xe8, INX, IMPLIED),    _(0xe9, SBC, IMMEDIATE),  _(0xea, NOP, IMPLIED),     _(0xeb, NOP,  IMPLIED),
_(0xec, CPX, ABSOLUTE),   _(0xed, SBC, ABSOLUTE),   _(0xee, INC, ABSOLUTE),    _(0xef, BBS6, ZEROP_REL),

_(0xf0, BEQ, RELATIVE),   _(0xf1, SBC, INDIRECT_Y), _(0xf2, SBC, INDIRECT),    _(0xf3, NOP,  IMPLIED),
_(0xf4, NOP, ZEROPAGE),   _(0xf5, SBC, ZEROPAGE_X), _(0xf6, INC, ZEROPAGE_X),  _(0xf7, SMB7, ZEROPAGE),
_(0xf8, SED, IMPLIED),    _(0xf9, SBC, ABSOLUTE_Y), _(0xfa, PLX, IMPLIED),     _(0xfb, NOP,  IMPLIED),
_(0xfc, NOP, ABSOLUTE),   _(0xfd, SBC, ABSOLUTE_X), _(0xfe, INC, ABSOLUTE_X),  _(0xff, BBS7, ZEROP_REL)
};


inline uint16_t read16(const uint8_t *mem)
{
	return mem[0] | (mem[1] << 8);
}


inline void write16(uint8_t *mem, uint16_t val)
{
	mem[0] = val & 0xff;
	mem[1] = val >> 8;
}


void Memory::init()
{
	memset(mem, 0xff, sizeof(mem));

	a = x = y = 0;
	s = 0xfd;
	p = 0x20 | B;
	pc = RST_VEC;
}


void Memory::update_flag(P_FLAG flag, bool set)
{
	p = set ? p | flag : p & ~flag;
}


void Memory::update_nz_flags(uint8_t val)
{
	update_flag(N, val & 0x80);
	update_flag(Z, val == 0);
}


void Memory::push(uint8_t val)
{
	mem[s-- + 0x100] = val;
}


uint8_t Memory::pull()
{
	return mem[++s + 0x100];
}


void Memory::push16(uint16_t val)
{
	write16(mem + s + 0x100-1, val);
	s -= 2;
}


uint16_t Memory::pull16()
{
	s += 2;
	return read16(mem + s + 0x100-1);
}


uint8_t Memory::read(uint16_t addr)
{
	if (io.is_valid_address(addr))
		return io.read(addr);

	return mem[addr];
}


void Memory::write(uint16_t addr, uint8_t val)
{
	if (io.is_valid_address(addr))
		io.write(addr, val);
	else
		mem[addr] = val;
}


bool YA65C02::load(const uint8_t *rom, size_t rom_size)
{
	if (rom_size > 64*1024)
		return false;

	Memory::init();

	memcpy(mem + (64*1024 - rom_size), rom, rom_size);

	instr_cnt = 0;
	cycle_cnt = 0;

	return true;
}


bool YA65C02::load(const char *rom_path)
{
	uint8_t rom[64*1024];

	FILE *test = fopen(rom_path, "rb");
	if (test == NULL)
		return false;

	size_t bytes_read = fread(rom, 1, sizeof(rom), test);
	fclose(test);

	return load(rom, bytes_read);
}


void YA65C02::setup_isr()
{
	push16(pc);
	push(p);

	pc = read16(mem + IRQ_VEC);

	update_flag(I, true);
	update_flag(D, false);
}


uint8_t *YA65C02::decode_operand()
{
	uint8_t *operand = NULL;
	uint8_t *addr = mem_ptr(pc + 1);

	switch (cmds[mem[pc]].am) {
		case ABSOLUTE:		{ operand = mem_ptr(read16(addr));						} break;
		case ABSOLUTE_X:	{ operand = mem_ptr(read16(addr) + x);					} break;
		case ABSOLUTE_Y:	{ operand = mem_ptr(read16(addr) + y);					} break;
		case ABS_IND:		{ operand = mem_ptr(read16(mem_ptr(read16(addr))));		} break;
		case ABS_IND_X:		{ operand = mem_ptr(read16(mem_ptr(read16(addr) + x)));	} break;
		case ACCUMULATOR:	{ operand = &a;											} break;
		case IMMEDIATE:		{ operand = addr;										} break;
		case IMPLIED:		{ operand = NULL;										} break;
		case INDIRECT:		{ operand = mem_ptr(read16(zp_ptr(*addr)));				} break;
		case INDIRECT_X:	{ operand = mem_ptr(read16(zp_ptr(*addr + x)));			} break;
		case INDIRECT_Y:	{ operand = mem_ptr(read16(zp_ptr(*addr)) + y);			} break;
		case RELATIVE:		{ operand = addr;										} break;
		case ZEROPAGE:		{ operand = zp_ptr(*addr);								} break;
		case ZEROPAGE_X:	{ operand = zp_ptr(*addr + x);							} break;
		case ZEROPAGE_Y:	{ operand = zp_ptr(*addr + y);							} break;
		case ZEROP_REL:		{ operand = zp_ptr(*addr);								} break;
		default:
			assert(0);
	}

	return operand;
}


void YA65C02::tick()
{
	uint8_t *operand = decode_operand();

	const CMD *cmd = &cmds[mem[pc]];

	pc += cmd->am & 0xf;

	switch(cmd->op) {

		case ADC: {
			if (p & D) {

				uint8_t carry = (a & 0xf) + (*operand & 0xf) + (p & C);
				uint16_t res = a + *operand + (p & C);

				if (carry >= 0xa)
					res += 6;

				if (res >= 0xa0)
					res += 0x60;

				a = res;

				update_flag(V, false);
				update_flag(C, res >= 0xa0);
			}
			else {
				uint16_t res = a + *operand + (p & C);
				bool ovf = ~(a ^ *operand) & (a ^ res) & 0x80;
				a = res;

				update_flag(V, ovf);
				update_flag(C, res & 0x100);
			}
			update_nz_flags(a);
		} break;

		case AND: { update_nz_flags(a &= *operand); } break;

		case ASL: {
			update_flag(C, *operand & 0x80);
			update_nz_flags(*operand <<= 1);
		} break;

		case BBR0: { if (!(*operand & 0x01))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR1: { if (!(*operand & 0x02))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR2: { if (!(*operand & 0x04))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR3: { if (!(*operand & 0x08))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR4: { if (!(*operand & 0x10))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR5: { if (!(*operand & 0x20))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR6: { if (!(*operand & 0x40))	pc += (int8_t)mem[pc - 1]; } break;
		case BBR7: { if (!(*operand & 0x80))	pc += (int8_t)mem[pc - 1]; } break;

		case BBS0: { if (*operand & 0x01)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS1: { if (*operand & 0x02)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS2: { if (*operand & 0x04)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS3: { if (*operand & 0x08)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS4: { if (*operand & 0x10)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS5: { if (*operand & 0x20)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS6: { if (*operand & 0x40)		pc += (int8_t)mem[pc - 1]; } break;
		case BBS7: { if (*operand & 0x80)		pc += (int8_t)mem[pc - 1]; } break;

		case BCC: { if (!(p & C))	pc += (int8_t)*operand; } break;
		case BCS: { if (p & C)		pc += (int8_t)*operand; } break; 
		case BEQ: { if (p & Z)		pc += (int8_t)*operand; } break;

		case BIT: {
			if (cmd->am != IMMEDIATE) {
				update_flag(N, *operand & 0x80);
				update_flag(V, *operand & 0x40);
			}
			update_flag(Z, (*operand & a) == 0);			
		} break;

		case BMI: { if (p & N)		pc += (int8_t)*operand; } break;
		case BNE: { if (!(p & Z))	pc += (int8_t)*operand; } break;
		case BPL: { if (!(p & N))	pc += (int8_t)*operand; } break;

		case BRA: { pc += (int8_t)*operand; } break;

		case BRK: {
			update_flag(B, true);

			push16(pc + 1);
			push(p);

			pc = read16(mem + IRQ_VEC);

			update_flag(I, true);
			update_flag(D, false);
		} break;

		case BVC: { if (!(p & V))	pc += (int8_t)*operand; } break;
		case BVS: { if (p & V)		pc += (int8_t)*operand; } break;

		case CLC: { update_flag(C, false); } break;
		case CLD: { update_flag(D, false); } break;
		case CLI: {	update_flag(I, false); } break;
		case CLV: { update_flag(V, false); } break;

		case CMP: {
			update_nz_flags(a - *operand);
			update_flag(C, a >= *operand);
		} break;

		case CPX: {
			update_nz_flags(x - *operand);
			update_flag(C, x >= *operand);
		} break;

		case CPY: {
			update_nz_flags(y - *operand);
			update_flag(C, y >= *operand);
		} break;

		case DEC: { update_nz_flags(--*operand);	} break;
		case DEX: { update_nz_flags(--x);			} break;
		case DEY: { update_nz_flags(--y);			} break;

		case EOR: { update_nz_flags(a ^= *operand);	} break;

		case INC: { update_nz_flags(++*operand);	} break;
		case INX: { update_nz_flags(++x);			} break;
		case INY: { update_nz_flags(++y);			} break;

		case JMP: { pc = operand - mem; } break;

		case JSR: {
			push16(pc - 1);
			pc = operand - mem;
		} break;

		case LDA: { a = read(operand - mem); update_nz_flags(a); } break;
		case LDX: { x = read(operand - mem); update_nz_flags(x); } break;
		case LDY: { y = read(operand - mem); update_nz_flags(y); } break;

		case LSR: {
			update_flag(C, *operand & 0x01);
			update_nz_flags(*operand >>= 1);
		} break;

		case NOP: {} break;

		case ORA: { update_nz_flags(a |= *operand); } break;

		case PHA: { push(a);						} break;
		case PHP: { update_flag(B, true); push(p);	} break;
		case PHX: { push(x);						} break;
		case PHY: { push(y);						} break;

		case PLA: { a = pull(); update_nz_flags(a);	} break;
		case PLP: { p = pull() | 0x20;				} break;
		case PLX: { x = pull(); update_nz_flags(x);	} break;
		case PLY: { y = pull(); update_nz_flags(y);	} break;

		case RMB0: { *operand &= ~0x01; } break;
		case RMB1: { *operand &= ~0x02; } break;
		case RMB2: { *operand &= ~0x04; } break;
		case RMB3: { *operand &= ~0x08; } break;
		case RMB4: { *operand &= ~0x10; } break;
		case RMB5: { *operand &= ~0x20; } break;
		case RMB6: { *operand &= ~0x40; } break;
		case RMB7: { *operand &= ~0x80; } break;

		case ROL: {
			bool carry = *operand & 0x80;
			*operand = (*operand << 1) | (p & C);
			update_nz_flags(*operand);
			update_flag(C, carry);
		} break;

		case ROR: {
			bool carry = *operand & 0x01;
			*operand = (*operand >> 1) | (p & C)*0x80;
			update_nz_flags(*operand);
			update_flag(C, carry);
		} break;

		case RTI: {
			p = pull() | 0x20;
			pc = pull16();
			update_flag(B, false);
		} break;

		case RTS: { pc = pull16() +1; } break;

		case SBC: {
			if (p & D) {

				uint8_t borrow = (a & 0xf) - (*operand & 0xf) - ((p & C) ^ 1);
				uint16_t res = a - *operand - ((p & C) ^ 1);

				if (borrow >= 0xa)
					res -= 6;

				if (res >= 0xa0)
					res -= 0x60;

				a = res;

				update_flag(V, false);
				update_flag(C, res < 0xa0);
			}
			else {

				uint16_t res = a - *operand - ((p & C)^1);
				bool ovf = (a ^ *operand) & (a ^ res) & 0x80;
				a = res;

				update_flag(V, ovf);
				update_flag(C, !(res & 0x100));
			}
			update_nz_flags(a);
		} break;

		case SEC: { update_flag(C, true); } break;
		case SED: { update_flag(D, true); } break;
		case SEI: { update_flag(I, true); } break;

		case SMB0: { *operand |= 0x01; } break;
		case SMB1: { *operand |= 0x02; } break;
		case SMB2: { *operand |= 0x04; } break;
		case SMB3: { *operand |= 0x08; } break;
		case SMB4: { *operand |= 0x10; } break;
		case SMB5: { *operand |= 0x20; } break;
		case SMB6: { *operand |= 0x40; } break;
		case SMB7: { *operand |= 0x80; } break;

		case STA: { write(operand - mem, a); } break;	
		case STX: { write(operand - mem, x); } break;
		case STY: { write(operand - mem, y); } break;
		case STZ: { write(operand - mem, 0); } break;

		case TAX: { update_nz_flags(x = a); } break;
		case TAY: { update_nz_flags(y = a); } break;

		case TRB: {
			update_flag(Z, (*operand & a) == 0);	
			*operand &= ~a;
		} break;

		case TSB: {
			update_flag(Z, (*operand & a) == 0);	
			*operand |= a;
		} break;

		case TSX: { update_nz_flags(x = s); } break;
		case TXA: { update_nz_flags(a = x); } break;
		case TXS: { s = x;					} break;
		case TYA: { update_nz_flags(a = y); } break;

		default:
			assert(0);
	}

	instr_cnt++;
	cycle_cnt += (cmd->am >> 4) & 0xf;
}


void YA65C02::run()
{
	uint64_t cycle_cnt_nmi = 0;
	uint64_t cycle_cnt_timing = 0;

	auto start = std::chrono::steady_clock::now();

	while (io.is_alive()) {

		tick();

		if (cycle_cnt - cycle_cnt_nmi >= cpu_clock / nmi_clock) {
			setup_isr();
			cycle_cnt_nmi = cycle_cnt;
		}

		// timing check every 50 ms
		if (cycle_cnt - cycle_cnt_timing >= cpu_clock / 20) {

			auto now = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

			uint64_t set = cycle_cnt * 1000 / cpu_clock;

			auto delta = std::chrono::milliseconds(set) - elapsed;
			std::this_thread::sleep_for(delta);

			cycle_cnt_timing = cycle_cnt;
		}
	}
}
