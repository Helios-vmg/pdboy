#include "HostSystem.h"
#include <iostream>

int main(int argc, char **argv){
	if (argc < 2)
		return 0;
	SdlHostSystem system;
	system.get_guest().get_storage_controller().load_cartridge(argv[1]);
	try{
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
