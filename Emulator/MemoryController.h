struct MemoryController{
	unsigned char *memory;
	static void fix_up_address(unsigned &address);
public:
	MemoryController();
	~MemoryController();
	unsigned load8(unsigned address) const;
	void store8(unsigned address, unsigned value);
	unsigned load16(unsigned address) const;
	void store16(unsigned address, unsigned value) const;
	void copy_out_memory_at(void *dst, size_t length, unsigned address);
	void load_rom_at(const void *buffer, size_t length, unsigned address);
	void clear_memory_at(unsigned address, size_t length);
};
