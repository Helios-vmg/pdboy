#pragma once

#include <condition_variable>
#include <mutex>

typedef std::lock_guard<std::mutex> automutex_t;

class Event{
	bool signalled = false;
	std::mutex mutex;
	std::condition_variable cv;
	bool wait_impl();
public:
	void signal();
	void reset_and_wait();
	void reset_and_wait_for(unsigned ms);
	void wait();
	void wait_for(unsigned ms);
};
