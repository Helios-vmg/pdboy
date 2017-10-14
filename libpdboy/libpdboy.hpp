#pragma once
#include "libpdboy.h"
#include "Gameboy.h"
#include "GeneralString.h"
#include "threads.h"
#include "StdOutEmulator.h"
#include <new>
#include <memory>
#include <mutex>

struct DateTime;

enum class SaveFileType{
	Ram,
	Rtc,
};

class libpdboy{
	std::unique_ptr<Gameboy> gameboy;
	void *user_data;
	libpdboy_local_now_f ln = nullptr;
	libpdboy_load_file_f lf = nullptr;
	libpdboy_write_file_f wf = nullptr;
	libpdboy_release_buffer_f rb = nullptr;
	libpdboy_register_periodic_notification_f rpn = nullptr;
	libpdboy_unregister_periodic_notification_f upn = nullptr;
	libpdboy_stdout_f so = nullptr;
	bool enable_video = true;
	bool enable_audio = true;
	std::mutex audio_mutex;
	std::mutex exception_mutex;
	std::uint64_t next_audio_frame = 0;
	AudioFrame *old_frame = nullptr;
	size_t old_frame_offset = 0;
	std::unique_ptr<std::thread> spinlock_timer_thread;
	std::atomic<bool> run_spinlock_timer_thread;
	std::string exception_message;
	std::string exception_message_copy;

	void spinlock_timer_thread_function(Event *event);
public:
	libpdboy(void *user_data);
	~libpdboy();
	Gameboy &get_guest(){
		return *this->gameboy;
	}
	void public_set_datetime_provider(libpdboy_local_now_f local_now);
	void public_set_filesystem_provider(libpdboy_load_file_f load_file, libpdboy_write_file_f write_file, libpdboy_release_buffer_f release_buffer);
	void public_set_timing_provider(libpdboy_register_periodic_notification_f reg_notification, libpdboy_unregister_periodic_notification_f unreg_notification);
	void public_configure(bool enable_video, bool enable_audio);
	void public_load(const char *path);
	void public_get_current_frame(libpdboy_rgba *framebuffer);
	void public_get_audio_data(libpdboy_audiosample *dst, size_t dst_sample_count);
	void public_set_input_state(libpdboy_inputstate *istate, int any_button_down, int any_button_up);
	void public_set_stdout_provider(libpdboy_stdout_f callback);
	const char *public_get_exception_message();

	bool load_file(const path_t &path, std::vector<byte_t> &dst, size_t max_size);
	void register_periodic_notification(Event &);
	void unregister_periodic_notification();
	DateTime local_now();
	void throw_exception(const char *);
	path_t get_save_location(Cartridge &cart, SaveFileType type);
	bool save_file(const path_t &path, const std::vector<byte_t> &buffer);
	bool save_file(const path_t &path, const byte_t *buffer, size_t size);
	void send_to_output(const char *);
	StdOutEmulator stdout();
};
