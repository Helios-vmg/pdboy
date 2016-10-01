#include "HostSystem.h"
#include <iostream>

int main(int, char**){
	SdlHostSystem system;
	try{
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
