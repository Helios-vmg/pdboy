#include "UserInputController.h"
#include "Gameboy.h"
#include <cstring>

UserInputController::UserInputController(Gameboy &system):
	system(&system),
	input_state(new InputState){
}

UserInputController::~UserInputController(){
	delete this->input_state.load();
}

void UserInputController::set_input_state(InputState *state, bool button_down, bool button_up){
	delete std::atomic_exchange(&this->input_state, state);
	if (button_down)
		this->button_down = true;
}

void UserInputController::request_input_state(byte_t select){
	InputState *copy = nullptr;
	copy = (InputState *)std::atomic_exchange(&this->input_state, copy);
	auto &state = *copy;
	this->saved_state = 0;
	if (!(select & pin14_mask)){
		this->saved_state |= state.right & pin10_mask;
		this->saved_state |= state.left & pin11_mask;
		this->saved_state |= state.up & pin12_mask;
		this->saved_state |= state.down & pin13_mask;
	}
	if (!(select & pin15_mask)){
		this->saved_state |= state.a & pin10_mask;
		this->saved_state |= state.b & pin11_mask;
		this->saved_state |= state.select & pin12_mask;
		this->saved_state |= state.start & pin13_mask;
	}
	InputState *null = nullptr;
	if (!std::atomic_compare_exchange_strong(&this->input_state, &null, copy))
		delete copy;
	this->saved_state |= 0xC0;
}

bool UserInputController::get_button_down(){
	return std::atomic_exchange(&this->button_down, false);
}
