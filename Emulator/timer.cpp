#include "timer.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::uint64_t get_timer_resolution(){
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}

std::uint64_t get_timer_count(){
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return count.QuadPart;
}
