#include "RegisterStore.h"

struct MemoryController{
	unsigned char *memory;
	static void fix_up_address(integer_type &address);
public:
	MemoryController();
	~MemoryController();
	integer_type load8(integer_type address) const;
	void store8(integer_type address, integer_type value);
	integer_type load16(integer_type address) const;
	void store16(integer_type address, integer_type value) const;
	void copy_out_memory_at(void *dst, size_t length, integer_type address);
	void load_rom_at(const void *buffer, size_t length, integer_type address);
	void clear_memory_at(integer_type address, size_t length);
};
