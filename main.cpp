#include <iostream>
#include <string>
#include <thread>

#include "mmv.hpp"


void input_thread(MMV &mmv)
{
	while (mmv.io.is_alive()) {

		std::string token;
		std::cin>>token;

		if (token == "KEY_CL")		mmv.io.push_key(KEY_CL);
		if (token == "KEY_POS")		mmv.io.push_key(KEY_POS);
		if (token == "KEY_MEM")		mmv.io.push_key(KEY_MEM);
		if (token == "KEY_INFO")	mmv.io.push_key(KEY_INFO);
		if (token == "KEY_LEV")		mmv.io.push_key(KEY_LEV);
		if (token == "KEY_ENT")		mmv.io.push_key(KEY_ENT);
		if (token == "KEY_RIGHT")	mmv.io.push_key(KEY_RIGHT);
		if (token == "KEY_LEFT")	mmv.io.push_key(KEY_LEFT);

		if (token == "KEY_A1")		mmv.io.push_key(KEY_A1);
		if (token == "KEY_B2")		mmv.io.push_key(KEY_B2);
		if (token == "KEY_C3")		mmv.io.push_key(KEY_C3);
		if (token == "KEY_D4")		mmv.io.push_key(KEY_D4);
		if (token == "KEY_E5")		mmv.io.push_key(KEY_E5);
		if (token == "KEY_F6")		mmv.io.push_key(KEY_F6);
		if (token == "KEY_G7")		mmv.io.push_key(KEY_G7);
		if (token == "KEY_H8")		mmv.io.push_key(KEY_H8);
	}
}


int main(int argc, const char *argv[])
{
	MMV mmv;

	const char *rom_filename = "mephisto5.rom";

	if (!mmv.load(rom_filename)) {
		std::cout<<"failed to load file: "<<rom_filename<<std::endl;
		return 1;
	}

	std::thread inp_th(input_thread, std::ref(mmv));

	mmv.run();

	inp_th.join();

	return 0;
}


