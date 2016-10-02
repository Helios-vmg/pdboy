#include "HostSystem.h"
#include <iostream>

int main(int, char**){
	SdlHostSystem system;
	system.get_guest().get_storage_controller().load_cartridge("tetris.gb");
	try{
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
