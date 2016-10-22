#pragma once

#include "CommonTypes.h"
#include "GeneralString.h"
#include "threads.h"
#include <memory>
#include <vector>
#include <atomic>

struct RenderedFrame;
struct InputState;

extern std::atomic<bool> slow_mode;

class StorageProvider{
public:
	virtual ~StorageProvider() = 0;
	virtual std::unique_ptr<std::vector<byte_t>> load_file(const path_t &path, size_t maximum_size);;
	virtual bool save_file(const path_t &path, const std::vector<byte_t> &);
	virtual path_t get_save_location();
};

class StdStorageProvider : public StorageProvider{
public:
};

class TimingProvider{
protected:
	Event *periodic_event = nullptr;
public:
	virtual ~TimingProvider(){}
	virtual void register_periodic_notification(Event &) = 0;
	virtual void unregister_periodic_notification() = 0;
};

class EventProvider{
public:
	virtual ~EventProvider(){}
	struct HandleEventsResult{
		InputState *input_state = nullptr;
		bool button_down = false;
		bool button_up = false;
	};
	virtual bool handle_events(HandleEventsResult &) = 0;
};

class GraphicsOutputProvider{
public:
	virtual ~GraphicsOutputProvider(){}
	virtual void render(const RenderedFrame *) = 0;
};

class NetworkProtocol{
public:
	virtual ~NetworkProtocol(){}
	virtual void initiate_as_master() = 0;
	virtual void initiate_as_slave() = 0;
};

class NetworkProvider{
	std::unique_ptr<NetworkProtocol> protocol;
public:
	NetworkProvider(std::unique_ptr<NetworkProtocol> &protocol): protocol(std::move(protocol)){}
	virtual ~NetworkProvider(){}
	virtual void initiate_as_master() = 0;
	virtual void initiate_as_slave() = 0;
};
