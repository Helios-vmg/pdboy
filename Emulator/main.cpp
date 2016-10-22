#include "HostSystem.h"
#include "SdlProvider.h"
#include <iostream>

int main(int argc, char **argv){
	if (argc < 2)
		return 0;
	auto sdl = new SdlProvider;
	HostSystem system(nullptr, sdl, sdl, sdl);
	auto &storage_controller = system.get_guest().get_storage_controller();
	try{
		storage_controller.load_cartridge(path_t(new StdBasicString<char>(argv[1])));
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
