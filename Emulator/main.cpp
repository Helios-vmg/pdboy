#include "HostSystem.h"
#include "SdlProvider.h"
#include <iostream>

int main(int argc, char **argv){
	if (argc < 2)
		return 0;
	auto sdl = std::make_unique<SdlProvider>();
	auto dtp = std::make_unique<StdDateTimeProvider>();
	HostSystem system(nullptr, sdl.get(), sdl.get(), sdl.get(), sdl.get(), dtp.get());
	auto &storage_controller = system.get_guest().get_storage_controller();
	try{
		if (!storage_controller.load_cartridge(path_t(new StdBasicString<char>(argv[1])))){
			std::cerr << "File not found: " << argv[1] << std::endl;
			return 0;
		}
#ifdef IO_REGISTERS_RECORDING
		bool record = argc >= 3 && !strcmp(argv[2], "-r");
		system.get_guest().get_cpu().get_memory_controller().use_recording("io_recording.bin", record);
#endif
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
