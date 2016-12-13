#pragma once

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
			throw std::exception("!Maybe<T>::is_initialized()");
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

template <typename T>
class PublishingResource{
	std::vector<std::unique_ptr<T>> allocated;
	std::vector<T *> ready;
	std::mutex ready_mutex;
	//Invariant: private_resource is valid at all times.
	T *private_resource;
	std::atomic<T *> public_resource;

	T *reuse_or_allocate(){
		{
			std::lock_guard<std::mutex> lg(this->ready_mutex);
			if (this->ready.size()){
				auto ret = this->ready.back();
				this->ready.pop_back();
				return ret;
			}
		}
		return this->allocate();
	}
	T *allocate(){
		this->allocated.push_back(std::make_unique<T>());
		return this->allocated.back().get();
	}
public:
	PublishingResource(){
		this->private_resource = this->allocate();
		this->public_resource = nullptr;
	}
	~PublishingResource(){}
	void publish(){
		this->private_resource = (RenderedFrame *)std::atomic_exchange(&this->public_resource, this->private_resource);
		if (!this->private_resource)
			this->private_resource = this->reuse_or_allocate();
	}
	T *get_private_resource(){
		return this->private_resource;
	}
	T *get_public_resource(){
		return (T *)std::atomic_exchange(&this->public_resource, nullptr);
	}
	void return_resource_as_ready(T *r){
		std::lock_guard<std::mutex> lg(this->ready_mutex);
		this->ready.push_back(r);
	}
	void return_resource_as_private(T *r){
		T *null_T = nullptr;
		if (std::atomic_compare_exchange_strong(&this->public_resource, &null_T, r))
			return;
		this->return_resource_as_ready(r);
	}
	void clear_public_resource(){
		auto frame = (T *)std::atomic_exchange(&this->public_resource, nullptr);
		if (frame){
			std::lock_guard<std::mutex> lg(this->ready_mutex);
			this->ready.push_back(frame);
		}
	}
};
