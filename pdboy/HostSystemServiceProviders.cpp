#include "HostSystemServiceProviders.h"
#include "HostSystem.h"
#include <fstream>
#include <iostream>
#include <cassert>

std::uint32_t NetworkProvider::little_endian_to_native_endian(std::uint32_t n){
#ifndef BIG_ENDIAN
	return n;
#else
	std::uint32_t ret;
	ret = n & 0xFF;
	n >>= 8;
	ret <<= 8;
	ret |= n & 0xFF;
	n >>= 8;
	ret <<= 8;
	ret |= n & 0xFF;
	n >>= 8;
	ret <<= 8;
	ret |= n & 0xFF;
	return ret;
#endif
}

std::uint32_t NetworkProvider::native_endian_to_little_endian(std::uint32_t n){
	return NetworkProvider::little_endian_to_native_endian(n);
}

void AudioOutputProvider::set_callback(get_audio_data_callback_t &&f){
	this->get_audio_data_callback = std::move(f);
}
