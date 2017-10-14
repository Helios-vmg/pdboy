#pragma once
#include <sstream>
#include <functional>

#ifdef stdout
#undef stdout
#endif

class StdOutEmulator{
	std::stringstream stream;
	std::function<void(const std::string &)> on_destruction;
public:
	StdOutEmulator() = default;
	template <typename T>
	StdOutEmulator(const T &f): on_destruction(f){}
	StdOutEmulator(const StdOutEmulator &other): stream(other.stream.str()), on_destruction(other.on_destruction){}
	StdOutEmulator(StdOutEmulator &&other){
		*this = std::move(other);
	}
	~StdOutEmulator(){
		if (this->on_destruction)
			this->on_destruction(this->stream.str());
	}
	const StdOutEmulator &operator=(const StdOutEmulator &other){
		this->stream = std::stringstream(other.stream.str());
		this->on_destruction = other.on_destruction;
		return *this;
	}
	const StdOutEmulator &operator=(StdOutEmulator &&other){
		this->stream = std::move(other.stream);
		this->on_destruction = std::move(other.on_destruction);
		return *this;
	}
	template <typename T>
	StdOutEmulator &operator<<(const T &x){
		this->stream << x;
		return *this;
	}
};

template <typename T>
StdOutEmulator &&operator<<(StdOutEmulator &&stdout, const T &x){
	return std::move(stdout << x);
}
