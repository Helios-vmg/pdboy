#pragma once

#include "CommonTypes.h"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

template <typename T>
class Maybe{
	bool initialized;
	T data;
public:
	Maybe(): initialized(false), data(){}
	Maybe(const Maybe<T> &b){
		*this = b;
	}
	Maybe(const T &b){
		*this = b;
	}
	const Maybe<T> &operator=(const Maybe<T> &b){
		this->initialized = b.initialized;
		this->data = b.data;
		return *this;
	}
	const Maybe<T> &operator=(const T &b){
		this->initialized = true;
		this->data = b;
		return *this;
	}
	const T &operator*() const{
		return this->value();
	}
	const T *operator->() const{
		return &this->value();
	}
	const T &value() const{
		if (!this->initialized)
			throw std::runtime_error("!Maybe<T>::is_initialized()");
		return this->data;
	}
	const T &value_or(const T &v) const{
		if (!this->initialized)
			return v;
		return this->data;
	}
	bool is_initialized() const{
		return this->initialized;
	}
	void clear(){
		this->initialized = false;
	}
};

constexpr unsigned bit(unsigned i){
	return 1U << i;
}

template <typename T, size_t N>
size_t array_size(T (&)[N]){
	return N;
}

struct DateTime{
	std::uint16_t year;
	std::uint8_t month;
	std::uint8_t day;
	std::uint8_t hour;
	std::uint8_t minute;
	std::uint8_t second;

	static DateTime from_posix(posix_time_t);
	static posix_time_t double_timestamp_to_posix(double);
	posix_time_t to_posix() const;
	double to_double_timestamp();
};
