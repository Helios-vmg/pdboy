#pragma once

#include <condition_variable>
#include <mutex>

class Event{
	bool signalled = false;
	std::mutex mutex;
	std::condition_variable cv;
	bool wait_impl();
public:
	void signal();
	void reset_and_wait();
	void wait();
	void wait_for(unsigned ms);
};
