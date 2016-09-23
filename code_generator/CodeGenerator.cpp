#include "CodeGenerator.h"
#include "CpuDefinition.h"

void CodeGenerator::generate(){
	std::pair<unsigned, unsigned> ranges[] = {
		{ 0, 211 },
		{ 212, 219 },
		{ 220, 221 },
		{ 223, 227 },
		{ 229, 235 },
		{ 238, 244 },
		{ 245, 252 },
		{ 254, 256 },
	};
	//for (auto r : ranges){
	//	for (unsigned i = r.first; i < r.second; i++){
	for (int i = 0; i < 0x100; i++){
			this->begin_opcode_definition(i);
			//this->load_program_counter8();
			this->definition->generate(i, *this);
			this->end_opcode_definition(i);
	}
	//	}
	//}
}

void CodeGenerator::double_opcode(unsigned first_opcode){
	std::pair<unsigned, unsigned> ranges[] = {
		{ 0, 72 },
		{ 128, 136 },
		{ 192, 200 },
	};
	for (auto r : ranges){
		for (unsigned i = r.first; i < r.second; i++){
			this->begin_double_opcode_definition(first_opcode, i);
			//this->load_program_counter8();
			this->definition->generate(first_opcode, i, *this);
			this->end_double_opcode_definition(first_opcode, i);
		}
	}
}
