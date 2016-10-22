#pragma once

#include "CommonTypes.h"
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
	virtual std::unique_ptr<std::vector<byte_t>> load_file(const char *path, size_t maximum_size);
	virtual std::unique_ptr<std::vector<byte_t>> load_file(const wchar_t *path, size_t maximum_size);
	virtual bool save_file(const char *path, const std::vector<byte_t> &);
	virtual bool save_file(const wchar_t *path, const std::vector<byte_t> &);
	virtual std::string get_save_location();
	virtual std::wstring get_save_location_wide();
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
