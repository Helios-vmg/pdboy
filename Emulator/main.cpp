#include "HostSystem.h"
#include <iostream>

int main(int, char**){
	HostSystem system;
	try{
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
