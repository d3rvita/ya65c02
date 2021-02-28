#include <stdio.h>

#include "ya65c02.hpp"


class YA65C02_TEST : public YA65C02 {

	public:
	
		IOHandler io;

		YA65C02_TEST() : io(0, 1), YA65C02(io) {}

		void check_cmd_array()
		{
			for (int i = 0; i < 256; i++) {
				if (cmds[i].op_code != i)
					printf("cmds[0x%2x].op_code != 0x%2x\n", i, cmds[i].op_code);

				for (int j = i + 1; j < 256; j++) {
					if (cmds[i].op != NOP && cmds[i].op != ERR && cmds[i].op == cmds[j].op && cmds[i].am == cmds[j].am)
						printf("identic cmds: %2x == %2x\n", i, j);
				}
			}
		}

		void run_dormann(const char *testfile, uint16_t test_pass_pc)
		{
			printf("test %-31s running ... ", testfile);
			fflush(stdout);

			load(testfile);
			pc = 0x400;

			for (uint16_t old_pc = 0; pc != old_pc;) {
				old_pc = pc;
				tick();
			}

			if (pc == test_pass_pc)
				printf("passed\n");
			else
				printf("failed: PC: %04x\n", pc);
		}


		void dormann()
		{
			const char *testfile[2] = {"6502_functional_test.bin", "65C02_extended_opcodes_test.bin"};
			uint16_t test_pass_pc[2] = {0x3469, 0x24f1};

			for (int i = 0; i < 2; i++)
				run_dormann(testfile[i], test_pass_pc[i]);
		}
};


int main()
{
	YA65C02_TEST test;
	test.check_cmd_array();
	test.dormann();

	return 0;
}
