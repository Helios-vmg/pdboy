#include "GameboyCpu.h"
#include <iostream>
#include <Windows.h>
#include <timeapi.h>

#pragma comment(lib, "winmm.lib")

int main(){
	GameboyCpu cpu;
	cpu.initialize();
}
