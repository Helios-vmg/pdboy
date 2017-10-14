#pragma once

#include "CommonTypes.h"
#include <libpdboy.h>
#include <memory>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>

class Cartridge;
struct RenderedFrame;
struct InputState;
class HostSystem;
struct AudioFrame;

class TimingProvider{
protected:
	std::function<void()> event_callback;
public:
	virtual ~TimingProvider(){}
	virtual void register_periodic_notification(const std::function<void()> &) = 0;
	virtual void unregister_periodic_notification() = 0;
};

class EventProvider{
	HostSystem *host = nullptr;
public:
	virtual ~EventProvider(){}
	struct HandleEventsResult{
		libpdboy_inputstate *input_state = nullptr;
		bool button_down = false;
		bool button_up = false;
	};
	virtual bool handle_events(HandleEventsResult &) = 0;
	void set_host(HostSystem &host){
		this->host = &host;
	}
};

class GraphicsOutputProvider{
public:
	virtual ~GraphicsOutputProvider(){}
	virtual std::unique_ptr<libpdboy_rgba, std::function<void(libpdboy_rgba *)>> render() = 0;
	virtual void write_frame_to_disk(std::string &path, const RenderedFrame &){}
};

class AudioOutputProvider{
public:
	typedef std::function<void(void *, size_t)> get_audio_data_callback_t;
protected:
	get_audio_data_callback_t get_audio_data_callback;
	std::mutex mutex;
public:
	virtual ~AudioOutputProvider(){}
	void set_callback(get_audio_data_callback_t &&f);
	virtual void stop_audio() = 0;
};

enum class DisconnectionCause{
	ConnectionAborted,
	LocalUserInitiated,
	RemoteUserInitiated,
	ConnectionDropped,
};

class NetworkProviderConnection;

class NetworkProvider{
protected:
	std::vector<std::unique_ptr<NetworkProviderConnection>> connections;
public:
	virtual ~NetworkProvider(){}
	virtual NetworkProviderConnection *create_connection() = 0;
	static std::uint32_t little_endian_to_native_endian(std::uint32_t);
	static std::uint32_t native_endian_to_little_endian(std::uint32_t);
};

class NetworkProviderConnection{
	NetworkProvider *provider;
	std::function<void()> on_accept;
	std::function<void(DisconnectionCause)> on_disconnection;
	std::function<size_t(const std::vector<byte_t> &)> on_data_receive;
public:
	NetworkProviderConnection(NetworkProvider &provider): provider(&provider){}
	virtual ~NetworkProviderConnection(){}
	virtual bool open() = 0;
	virtual void abort() = 0;
	virtual void send_data(const std::vector<byte_t> &) = 0;
	virtual void send_data(const void *, size_t) = 0;

#define DEFINE_SETTER(x) void set_##x(const decltype(x) &y){ this->x = y; }
	DEFINE_SETTER(on_accept);
	DEFINE_SETTER(on_disconnection);
	DEFINE_SETTER(on_data_receive);
};

class NetworkProtocol{
protected:
	NetworkProviderConnection *connection;
public:
	struct transfer_data{
		bool passive_mode;
		//When operating in DMG mode, this is always false.
		bool fast_mode;
		//When operating in DMG mode, this is always false.
		bool double_speed_mode;
		byte_t data;
	};

	NetworkProtocol(NetworkProviderConnection *connection): connection(connection){}
	virtual ~NetworkProtocol(){}
	virtual int get_default_port() const{
		return -1;
	}
	virtual void send_data(transfer_data) = 0;
	//Warning: This callback may be called from a different thread!
	virtual void set_on_connected(const std::function<void()> &) = 0;
	//Warning: This callback may be called from a different thread!
	virtual void set_on_disconnected(const std::function<void(DisconnectionCause)> &) = 0;
	//Warning: This callback may be called from a different thread!
	virtual void set_on_data_received(const std::function<void(transfer_data)> &) = 0;
};
