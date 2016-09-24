#include "GameboyCpu.h"
#include <Windows.h>
#include <iostream>

const unsigned gb_frequency = 4194304;

int main(){
	GameboyCpu cpu;
	cpu.initialize();
	LARGE_INTEGER frequency, start, end;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	for (int i = 10000; i--;){
		cpu.rerun();
	}
	QueryPerformanceCounter(&end);
	double real_time = (double)(end.QuadPart - start.QuadPart) / (double)frequency.QuadPart * 1e+6;
	double emulated_time = (double)cpu.total_cycles / (double)gb_frequency;
	std::cout <<
		"Real time    : " << real_time << " us\n"
		"Emulated time: " << emulated_time << " us\n";
}
