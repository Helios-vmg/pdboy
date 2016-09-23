#include "RegisterStore.h"
#include "MemoryController.h"

class GameboyCpu{
	RegisterStore registers;
	MemoryController memory_controller;
public:
	void take_time(unsigned cycles);
	void interrupt_toggle(bool);
	void stop();
};
