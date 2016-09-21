struct MemoryController{
	unsigned char *memory;
public:
	MemoryController();
	~MemoryController();
	unsigned load8(unsigned address) const;
	void store8(unsigned address, unsigned value);
	unsigned load16(unsigned address) const;
	unsigned store16(unsigned address, unsigned value) const;
};
