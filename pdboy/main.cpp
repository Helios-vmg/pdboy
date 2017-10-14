#include "SdlProvider.h"
#include "HostSystem.h"
#include <iostream>

int main(int argc, char **argv){
	if (argc < 2)
		return 0;
	try{
		SdlProvider sdl;
		HostSystem host(sdl, sdl, sdl, sdl);
		host.run(argv[1]);
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
