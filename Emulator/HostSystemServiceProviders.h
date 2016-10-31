#pragma once

#include "CommonTypes.h"
#include "GeneralString.h"
#include "threads.h"
#include <memory>
#include <vector>
#include <atomic>

struct RenderedFrame;
struct InputState;
class HostSystem;

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

struct DateTime{
	std::uint16_t year;
	std::uint8_t month;
	std::uint8_t day;
	std::uint8_t hour;
	std::uint8_t minute;
	std::uint8_t second;

	static DateTime from_posix(posix_time_t);
	posix_time_t to_posix() const;
};

class DateTimeProvider{
public:
	virtual DateTime local_now() = 0;
	double get_double_timestamp(){
		return this->date_to_double_timestamp(this->local_now());
	}
	static DateTime double_timestamp_to_date(double);
	static double date_to_double_timestamp(DateTime);
};

class StdDateTimeProvider : public DateTimeProvider{
public:
	DateTime local_now() override;
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
	HostSystem *host = nullptr;
public:
	virtual ~EventProvider(){}
	struct HandleEventsResult{
		InputState *input_state = nullptr;
		bool button_down = false;
		bool button_up = false;
	};
	virtual bool handle_events(HandleEventsResult &) = 0;
	void set_host(HostSystem &host){
		this->host = &host;
	}
	void toggle_fastforward(bool);
	void toggle_slowdown(bool);
	void toggle_pause(int);
};

class GraphicsOutputProvider{
public:
	virtual ~GraphicsOutputProvider(){}
	virtual void render(const RenderedFrame *) = 0;
	virtual void write_frame_to_disk(std::string &path, const RenderedFrame &){}
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
