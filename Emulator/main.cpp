#include "HostSystem.h"

int main(int, char**){
	HostSystem system;
	try{
		system.run();
	} catch (std::exception &){
	}
	return 0;
}
