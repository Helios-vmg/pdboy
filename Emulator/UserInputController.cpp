#include "UserInputController.h"
#include "Gameboy.h"
#include <cstring>

UserInputController::UserInputController(Gameboy &system)
		:system(&system){
	memset(&this->input_state, 0xFF, sizeof(this->input_state));
}

void UserInputController::set_input_state(const InputState &state, bool button_down, bool button_up){
	std::lock_guard<std::mutex> lg(this->system->get_interpreter_thread_mutex());
	if (state != this->input_state)
		this->state_changed = true;
	this->input_state = state;
	if (button_down){
		this->system->get_cpu().joystick_irq();
	}
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
