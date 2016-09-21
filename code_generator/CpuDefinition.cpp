#include "CpuDefinition.h"
#include "CodeGenerator.h"
#include "FlagSetting.h"
#include <memory>
#include <cassert>
#include <cstring>
#include <array>

bool match_opcode(unsigned opcode, const char *pattern){
	auto length = strlen(pattern);
	assert(opcode < 0x100 && length == 8);
	for (unsigned i = length; i--;){
		auto match = pattern[i];
		if (match != '0' && match != '1')
			continue;
		bool match_value = match == '1';
		bool bit = (opcode & 1) == 1;
		opcode >>= 1;
		if (bit != match_value)
			return false;
	}
	return true;
}

void CpuDefinition::generate(unsigned opcode, CodeGenerator &generator){
	switch (opcode){
		case 0x00:
			// Handle no-op.
			generator.take_time(4);
			return;
		case 0x08:
			{
				// Handle SP -> (imm16)16
				auto imm = generator.load_program_counter16();
				auto mem = generator.load_mem16(imm);
				generator.write_register16(Register16::SP, mem);
				generator.take_time(20);
				return;
			}
		case 0x18:
			{
				// Handle imm8 + PC -> PC
				auto imm = generator.load_program_counter8();
				generator.add_PC(imm);
				generator.take_time(8);
				return;
			}
		case 0x27:
			{
				// Handle DAA A
				auto val = generator.get_register_value8(Register8::A);
				auto pair = generator.perform_decimal_adjustment(val);
				val = pair.first;
				generator.write_register8(Register8::A, val);
				// TODO: Flag carry = !!value_of(pair.second)
				generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Keep, FlagSetting::Reset, FlagSetting::IfNonZero(pair.second) });
				generator.take_time(4);
				return;
			}
		case 0x2F:
			// Handle ~A -> A
			generator.flip_A();
			generator.take_time(4);
			return;
		case 0x37:
			// Handle 1 -> CarryFlag
			generator.set_flags({ FlagSetting::Keep, FlagSetting::Reset, FlagSetting::Reset, FlagSetting::Set });
			generator.take_time(4);
			return;
		case 0x3F:
			// Handle !CarryFlag -> CarryFlag
			generator.set_flags({ FlagSetting::Keep, FlagSetting::Reset, FlagSetting::Reset, FlagSetting::Flip });
			generator.take_time(4);
			return;
		case 0x76:
			generator.halt();
			generator.take_time(4);
			return;
		case 0xC3:
			{
				// Handle imm16 -> PC
				auto imm = generator.load_program_counter16();
				generator.write_register16(Register16::PC, imm);
				generator.take_time(12);
				return;
			}
		case 0xCB:
			generator.double_opcode(opcode);
			return;
		case 0xC9:
		case 0xD9:
			{
				// Handle RET, RETI
				auto addr = generator.get_register_value16(Register16::SP);
				addr = generator.load_mem16(addr);
				generator.inc2_SP();
				generator.write_register16(Register16::PC, addr);
				if (opcode == 0xD9)
					generator.enable_interrupts();
				generator.take_time(8);
				return;
			}
		case 0xCD:
			{
				// Handle CALL imm16
				auto imm = generator.load_program_counter16();
				generator.push_PC();
				generator.write_register16(Register16::PC, imm);
				generator.take_time(12);
				return;
			}
		case 0xE8:
			{
				// Handle imm16 + SP -> SP
				auto imm = generator.load_program_counter16();
				generator.add_SP(imm);
				generator.take_time(16);
				return;
			}
		case 0xE9:
			{
				// Handle (HL)16 -> SP
				auto reg = generator.get_register_value16(Register16::HL);
				auto addr = generator.load_mem16(reg);
				generator.write_register16(Register16::PC, addr);
				generator.take_time(4); // Shouldn't this be 12?
				return;
			}
		case 0xEA:
			{
				// Handle A -> (imm16)8
				auto imm = generator.load_program_counter16();
				auto reg = generator.get_A();
				generator.store_mem8(imm, reg);
				auto mem = generator.load_mem8(imm);
				generator.take_time(16);
				return;
			}
		case 0xF3:
			generator.disable_interrupts();
			generator.take_time(4);
			return;
		case 0xF8:
			{
				// Handle (SP + imm8)16 -> HL
				auto imm = generator.load_program_counter8();
				auto mem = generator.load_sp_offset16(imm);
				generator.write_register16(Register16::HL, mem);
				generator.take_time(12);
				return;
			}
		case 0xF9:
			// Handle HL -> SP
			generator.write_register16(Register16::SP, generator.get_register_value16(Register16::HL));
			generator.take_time(8);
			return;
		case 0xFB:
			generator.enable_interrupts();
			generator.take_time(4);
			return;
		case 0xFA:
			{
				// Handle (imm16)8 -> A
				auto imm = generator.load_program_counter16();
				auto mem = generator.load_mem8(imm);
				generator.write_A(mem);
				generator.take_time(16);
				return;
			}
	}

	// 000xx010
	if (match_opcode(opcode, "000xx010")){
		auto operand = (Register16A)((opcode >> 4) & 1);
		auto operation = (opcode >> 3) & 1;
		if (operation){
			// Handle (BC)8 -> A, (DE)8 -> A.
			auto reg = generator.get_register_value16(operand);
			auto mem = generator.load_mem8(reg);
			generator.write_A(mem);
		} else{
			// Handle A -> (BC)8, A -> (DE)8.
			auto dst = generator.get_register_value16(operand);
			auto src = generator.get_A();
			generator.store_mem8(dst, src);
		}
		generator.take_time(8);
		return;
	}

	// 000xx111
	if (match_opcode(opcode, "000xx111")){
		// Handle rotations
		bool left = (opcode >> 3) & 1;
		bool through_carry = (opcode >> 4) & 1;
		auto old_a = generator.get_register_value8(Register8::A);
		generator.rotate_A(left, through_carry);
		auto new_a = generator.get_register_value8(Register8::A);
		auto old_bit_7 = generator.get_bit_value(old_a, 7);
		auto old_bit_0 = generator.get_bit_value(old_a, 0);
		FlagSettings flags[] = {
			{ FlagSetting::IfZero(new_a), FlagSetting::Reset, FlagSetting::Reset, FlagSetting::IfNonZero(old_bit_7) },
			{ FlagSetting::IfZero(new_a), FlagSetting::Reset, FlagSetting::Reset, FlagSetting::IfNonZero(old_bit_0) },
		};
		generator.set_flags(flags[(int)!left]);
		generator.take_time(4);
	}

	// 00xxx011
	if (match_opcode(opcode, "00xxx011")){
		// Handle INC reg16, DEC reg16
		auto reg = (Register16A)((opcode >> 4) & 3);
		bool decrement = (opcode >> 3) & 1;
		if (decrement)
			generator.dec_register16(reg);
		else
			generator.inc_register16(reg);
		generator.take_time(8);
		return;
	}

	// 00xx0001
	if (match_opcode(opcode, "00xx0001")){
		// Handle imm16 -> reg16
		auto reg = (Register16A)((opcode >> 4) & 3);
		auto imm = generator.load_program_counter16();
		generator.write_register16(reg, imm);
		generator.take_time(12);
		return;
	}

	// 00xxx110
	if (match_opcode(opcode, "00xxx110")){
		// Handle imm8 -> reg8
		auto dst = (Register8)((opcode >> 3) & 7);
		auto imm = generator.load_program_counter8();
		if (dst == Register8::None){
			generator.store_hl8(imm);
			generator.take_time(12);
			return;
		}
		generator.write_register8(dst, imm);
		generator.take_time(8);
		return;
	}

	// 00xx1001
	if (match_opcode(opcode, "00xx1001")){
		// Handle reg16 + HL -> HL
		auto reg = (Register16A)((opcode >> 4) & 3);
		generator.add16(reg);
		generator.take_time(8);
		return;
	}

	// 00xxx10x
	if (match_opcode(opcode, "00xxx10x")){
		//Handle INC reg8, DEC reg8
		bool decrement = (opcode & 1) == 1;
		auto operand = (Register8)((opcode >> 3) & 7);
		unsigned time;
		uintptr_t val;
		if (operand == Register8::None){
			auto mem = generator.load_hl8();
			if (decrement)
				val = generator.dec_temp(mem);
			else
				val = generator.inc_temp(mem);
			generator.store_mem8(mem, val);
			time = 12;
		} else{
			generator.inc_register8(operand);
			val = generator.get_register_value8(operand);
			time = 8;
		}
		generator.set_flags(
		{
			FlagSetting::IfZero(val),
			(decrement ? FlagSetting::Set : FlagSetting::Reset),
			FlagSetting::DontCare,
			FlagSetting::Keep
		}
		);
		generator.take_time(time);
	}

	// 001xx010
	if (match_opcode(opcode, "001xx010")){
		bool decrement = (opcode >> 4) & 1;
		bool load = (opcode >> 3) & 1;

		if (load)
			generator.write_A(generator.load_hl8());
		else
			generator.store_hl8(generator.get_A());

		if (decrement)
			generator.dec_register16(Register16::HL);
		else
			generator.inc_register16(Register16::HL);

		generator.take_time(8);
		return;
	}

	// 01xxxxxx
	if (match_opcode(opcode, "01xxxxxx")){
		assert(opcode != 0x76);
		// Handle reg8 -> reg8, reg8 -> (HL)8, and (HL)8 -> reg8.
		auto dst = (Register8)((opcode >> 3) & 7);
		auto src = (Register8)(opcode & 7);
		if (dst == src){
			assert(dst != Register8::None && src != Register8::None);
			generator.noop();
			generator.take_time(4);
			return;
		}
		if (src == Register8::None){
			assert(dst != Register8::None);
			auto mem = generator.load_hl8();
			generator.write_register8(dst, mem);
			generator.take_time(8);
			return;
		} else if (dst == Register8::None){
			assert(src != Register8::None);
			auto reg = generator.get_register_value8(src);
			generator.store_hl8(reg);
			generator.take_time(8);
			return;
		}
		auto reg = generator.get_register_value8(src);
		generator.write_register8(dst, reg);
		generator.take_time(4);
		return;
	}

	// 10xxxxxx
	// 11xxx110
	if (match_opcode(opcode, "10xxxxxx") || match_opcode(opcode, "11xxx110")){
		auto operation = (opcode >> 3) & 7;
		auto operand = (Register8)(opcode & 7);
		auto immediate = (opcode & 0x40) == 0x40;
		uintptr_t val;
		if (!immediate){
			if (operand == Register8::None)
				val = generator.load_hl8();
			else
				val = generator.get_register_value8(operand);
		} else
			val = generator.load_program_counter8();
		std::array<uintptr_t, 3> array = { 0, 0, 0 };
		switch (operation){
			case 0:
				array = generator.add8(val);
				val = array[0];
				break;
			case 1:
				array = generator.add_carry(val);
				val = array[0];
				break;
			case 2:
				array = generator.sub(val);
				val = array[0];
				break;
			case 3:
				array = generator.sub_carry(val);
				val = array[0];
				break;
			case 4:
				val = generator. and (val);
				break;
			case 5:
				val = generator. xor (val);
				break;
			case 6:
				val = generator. or (val);
				break;
			case 7:
				array = generator.cmp(val);
				val = array[0];
				break;
		}
		auto if_zero = FlagSetting::IfZero(val);
		FlagSetting half_carry = FlagSetting::Keep,
			carry = FlagSetting::Reset;
		if (array[0]){
			half_carry = FlagSetting::IfNonZero(array[1]);
			carry = FlagSetting::IfNonZero(array[2]);
		}
		FlagSettings fs = { FlagSetting::Keep, FlagSetting::Keep, FlagSetting::Keep, FlagSetting::Keep };
		switch (operation){
			case 0:
			case 1:
				fs = { if_zero, FlagSetting::Reset, half_carry, carry };
				break;
			case 2:
			case 3:
				fs = { if_zero, FlagSetting::Set, half_carry, carry };
				break;
			case 4:
				fs = { if_zero, FlagSetting::Reset, FlagSetting::Set, carry };
				break;
			case 5:
			case 6:
				fs = { if_zero, FlagSetting::Reset, FlagSetting::Reset, carry };
				break;
			case 7:
				fs = { if_zero, FlagSetting::Set, half_carry, carry };
				break;
		}
		generator.set_flags(fs);
		generator.take_time(operand == Register8::None ? 8 : 4);
	}

	// 11xx0x01
	if (match_opcode(opcode, "11xx0x01")){
		// Handle PUSH, POP
		auto reg = (Register16B)((opcode >> 4) & 3);
		auto push = (opcode & 4) == 4;
		if (push)
			generator.dec2_SP();
		auto addr = generator.get_register_value16(Register16::SP);
		if (push){
			auto val = generator.get_register_value16(reg);
			generator.store_mem16(addr, val);
		} else{
			auto val = generator.load_mem16(addr);
			generator.write_register16(Register16::SP, val);
			generator.inc2_SP();
		}
		generator.take_time(push ? 16 : 12);
		return;
	}

	if (match_opcode(opcode, "11xxx111")){
		// Handle RST
		auto addr = ((opcode >> 3) & 7) * 8;
		generator.push_PC();
		generator.write_register16_literal(Register16::SP, addr);
		generator.take_time(32);
		return;
	}

	// 110xx000
	if (match_opcode(opcode, "110xx000")){
		// Handle conditional RET
		auto type = (ConditionalJumpType)((opcode >> 3) & 3);
		auto addr = generator.get_register_value16(Register16::SP);
		addr = generator.load_mem16(addr);
		generator.inc2_SP();
		generator.set_PC_if(addr, type);
		generator.take_time(8);
		return;
	}

	// 110xx010
	// 001xx000
	if (match_opcode(opcode, "110xx010") || match_opcode(opcode, "001xx000")){
		// Handle conditional relative and absolute jumps.
		bool absolute = (opcode & 0x80) == 0x80;
		auto type = (ConditionalJumpType)((opcode >> 3) & 3);
		if (absolute){
			auto imm = generator.load_program_counter16();
			generator.set_PC_if(imm, type);
		} else{
			auto imm = generator.load_program_counter8();
			generator.add_PC_if(imm, type);
		}
		generator.take_time(absolute ? 12 : 8);
		return;
	}

	// 110xx100
	if (match_opcode(opcode, "110xx100")){
		// Handle conditional calls.
		auto type = (ConditionalJumpType)((opcode >> 3) & 3);
		auto imm = generator.load_program_counter16();
		generator.push_PC();
		generator.set_PC_if(imm, type);
		generator.take_time(12);
		return;
	}

	// 111x00x0
	if (match_opcode(opcode, "111x00x0")){
		// Handle (0xFF00 + C)8 -> A, (0xFF00 + imm8)8 -> A
		// Handle A -> (0xFF00 + C)8, A -> (0xFF00 + imm8)8

		bool using_C = (opcode & 2) == 2;
		bool load = (opcode & 0x10) == 0x10;
		uintptr_t offset;
		if (using_C)
			offset = generator.get_register_value8(Register8::C);
		else
			offset = generator.load_program_counter8();

		if (load)
			generator.write_A(generator.load_ff00_offset8(offset));
		else
			generator.store_mem_ff008(offset, generator.get_A());

		generator.take_time(using_C ? 8 : 12);
		return;
	}

}

void CpuDefinition::generate(unsigned first_opcode, unsigned opcode, CodeGenerator &generator){
	if (first_opcode == 0xCB){
		// 00110xxx
		if (match_opcode(opcode, "00110xxx")){
			auto operand = (Register8)(opcode & 7);
			unsigned time = 8;
			uintptr_t val;
			if (operand == Register8::None){
				val = generator.load_hl8();
				time = 16;
			} else
				val = generator.get_register_value8((Register8)operand);
			val = generator.swap_nibbles(val);
			if (operand == Register8::None)
				generator.store_hl8(val);
			else
				generator.write_register8((Register8)operand, val);
			generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Reset, FlagSetting::Reset, FlagSetting::Reset });
			generator.take_time(time);
			return;
		}
		// 00xxxxxx
		if (match_opcode(opcode, "00xxxxxx")){
			auto operation = (BitwiseOps)((opcode >> 3) & 7);
			auto operand = (Register8)(opcode & 7);
			assert(operation != BitwiseOps::None);
			unsigned time = 8;
			uintptr_t val;
			if (operand == Register8::None){
				val = generator.load_hl8();
				time = 16;
			} else
				val = generator.get_register_value8(operand);
			auto carry = FlagSetting::Reset;
			auto old_bit_7 = FlagSetting::IfNonZero(generator.get_bit_value(val, 7));
			auto old_bit_0 = FlagSetting::IfNonZero(generator.get_bit_value(val, 0));
			switch (operation){
				case BitwiseOps::RotateLeft:
					val = generator.rotate_left(val);
					carry = old_bit_7;
					break;
				case BitwiseOps::RotateRight:
					val = generator.rotate_right(val);
					carry = old_bit_0;
					break;
				case BitwiseOps::RotateLeftThroughCarry:
					val = generator.rotate_left_through_carry(val);
					carry = old_bit_7;
					break;
				case BitwiseOps::RotateRightThroughCarry:
					val = generator.rotate_right_through_carry(val);
					carry = old_bit_0;
					break;
				case BitwiseOps::ShiftLeft:
					val = generator.shift_left(val);
					carry = old_bit_7;
					break;
				case BitwiseOps::ArithmeticShiftRight:
					val = generator.arithmetic_shift_right(val);
					carry = old_bit_0;
					break;
				case BitwiseOps::None:
					assert(false);
				case BitwiseOps::BitwiseShiftRight:
					val = generator.bitwise_shift_right(val);
					carry = old_bit_0;
					break;
			}
			if (operand == Register8::None)
				generator.store_hl8(val);
			else
				generator.write_register8(operand, val);
			generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Reset, FlagSetting::Reset, carry });
			generator.take_time(time);
			return;
		}

		auto operation = (BitfieldOps)((opcode >> 6) & 3);
		auto register_operand = (Register8)(opcode & 7);
		auto bit_operand = (opcode >> 3) & 7;
		unsigned time = 8;
		uintptr_t val;
		if (register_operand == Register8::None){
			val = generator.load_hl8();
			time = 16;
		} else
			val = generator.get_register_value8(register_operand);
		FlagSettings fs = { FlagSetting::Keep, FlagSetting::Keep, FlagSetting::Keep, FlagSetting::Keep };
		switch (operation){
			case BitfieldOps::BitCheck:
				val = generator.get_bit_value(val, bit_operand);
				fs = { FlagSetting::IfZero(val), FlagSetting::Reset, FlagSetting::Set, FlagSetting::Keep };
				break;
			case BitfieldOps::BitReset:
				val = generator.set_bit_value(val, bit_operand, false);
				break;
			case BitfieldOps::BitSet:
				val = generator.set_bit_value(val, bit_operand, true);
				break;
			default:
				assert(false);
				break;
		}
		generator.set_flags(fs);
		generator.take_time(time);
		return;
	}
	if (first_opcode == 0x10 && opcode == 0x00){
		generator.stop();
		generator.take_time(4);
		return;
	}
}
