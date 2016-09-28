#include "UserInputController.h"
#include <cstring>

UserInputController::UserInputController(Gameboy &system)
		:system(&system){
	memset(&this->input_state, 0xFF, sizeof(this->input_state));
}

void UserInputController::request_input_state(byte_t select){
	this->saved_state = 0;
	if (!(select & pin14_mask)){
		this->saved_state |= this->input_state.right & pin10_mask;
		this->saved_state |= this->input_state.left & pin11_mask;
		this->saved_state |= this->input_state.up & pin12_mask;
		this->saved_state |= this->input_state.down & pin13_mask;
	}
	if (!(select & pin15_mask)){
		this->saved_state |= this->input_state.a & pin10_mask;
		this->saved_state |= this->input_state.b & pin11_mask;
		this->saved_state |= this->input_state.select & pin12_mask;
		this->saved_state |= this->input_state.start & pin13_mask;
	}
	this->saved_state |= 0xC0;
}
