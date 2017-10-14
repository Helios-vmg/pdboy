#include "libpdboy.hpp"
#include "timer.h"

LIBPDBOY_API libpdboy_state *libpdboy_create(void *user_data){
	return new (std::nothrow) libpdboy(user_data);
}

LIBPDBOY_API void libpdboy_destroy(libpdboy_state *state){
	delete (libpdboy *)state;
}

LIBPDBOY_API void libpdboy_set_datetime_provider(libpdboy_state *state, libpdboy_local_now_f local_now){
	((libpdboy *)state)->public_set_datetime_provider(local_now);
}

LIBPDBOY_API void libpdboy_set_filesystem_provider(libpdboy_state *state, libpdboy_load_file_f load_file, libpdboy_write_file_f write_file, libpdboy_release_buffer_f release_buffer){
	((libpdboy *)state)->public_set_filesystem_provider(load_file, write_file, release_buffer);
}

LIBPDBOY_API void libpdboy_set_timing_provider(libpdboy_state *state, libpdboy_register_periodic_notification_f reg_notification, libpdboy_unregister_periodic_notification_f unreg_notification){
	((libpdboy *)state)->public_set_timing_provider(reg_notification, unreg_notification);
}

LIBPDBOY_API void libpdboy_configure(libpdboy_state *state, int enable_video, int enable_audio){
	((libpdboy *)state)->public_configure(!!enable_video, !!enable_audio);
}

LIBPDBOY_API void libpdboy_load(libpdboy_state *state, const char *path){
	((libpdboy *)state)->public_load(path);
}

LIBPDBOY_API void libpdboy_get_current_frame(libpdboy_state *state, libpdboy_rgba *framebuffer){
	((libpdboy *)state)->public_get_current_frame(framebuffer);
}

LIBPDBOY_API void libpdboy_get_audio_data(libpdboy_state *state, libpdboy_audiosample *dst, size_t dst_sample_count){
	((libpdboy *)state)->public_get_audio_data(dst, dst_sample_count);
}

LIBPDBOY_API void libpdboy_set_input_state(libpdboy_state *state, libpdboy_inputstate *istate, int any_button_down, int any_button_up){
	((libpdboy *)state)->public_set_input_state(istate, any_button_down, any_button_up);
}

LIBPDBOY_API void libpdboy_set_stdout_provider(libpdboy_state *state, libpdboy_stdout_f callback){
	((libpdboy *)state)->public_set_stdout_provider(callback);
}

LIBPDBOY_API const char *libpdboy_get_exception_message(libpdboy_state *state){
	return ((libpdboy *)state)->public_get_exception_message();
}

libpdboy::libpdboy(void *user_data): user_data(user_data){}

libpdboy::~libpdboy(){
	if (this->gameboy)
		this->gameboy->stop();
	this->gameboy.reset();
}

void libpdboy::public_set_datetime_provider(libpdboy_local_now_f local_now){
	this->ln = local_now;
}

void libpdboy::public_set_filesystem_provider(libpdboy_load_file_f load_file, libpdboy_write_file_f write_file, libpdboy_release_buffer_f release_buffer){
	this->lf = load_file;
	this->wf = write_file;
	this->rb = release_buffer;
}

void libpdboy::public_set_timing_provider(libpdboy_register_periodic_notification_f reg_notification, libpdboy_unregister_periodic_notification_f unreg_notification){
	this->rpn = reg_notification;
	this->upn = unreg_notification;
}

void libpdboy::public_configure(bool enable_video, bool enable_audio){
	this->enable_video = enable_video;
	this->enable_audio = enable_audio;
}

void libpdboy::public_load(const char *path){
	this->gameboy.reset(new Gameboy(*this));
	auto &storage_controller = this->gameboy->get_storage_controller();
	if (!storage_controller.load_cartridge(path_t(new StdBasicString<char>(path)))){
		std::cerr << "File not found: " << path << std::endl;
		return;
	}
	this->gameboy->run();
}

void libpdboy::public_get_current_frame(libpdboy_rgba *framebuffer){
	auto frame = this->gameboy->get_current_frame();
	
	if (frame)
		memcpy(framebuffer, frame->pixels, sizeof(frame->pixels));
	else
		memset(framebuffer, 0xFF, sizeof(frame->pixels));
	
	if (frame)
		this->gameboy->return_used_frame(frame);
}

void libpdboy::public_get_audio_data(libpdboy_audiosample *dst, size_t dst_sample_count){
	LOCK_MUTEX(this->audio_mutex);

	auto &sc = this->gameboy->get_sound_controller();
	if (this->old_frame){
		auto n = std::min<size_t>(dst_sample_count, this->old_frame->length - this->old_frame_offset);
		memcpy(dst, this->old_frame->buffer + this->old_frame_offset, n * sizeof(libpdboy_audiosample));
		dst += n;
		dst_sample_count -= n;
		if (this->old_frame->length > this->old_frame_offset + n){
			this->old_frame_offset += n;
			return;
		}
		sc.return_used_frame(this->old_frame);
		this->old_frame = nullptr;
		this->old_frame_offset = 0;
	}

	while (dst_sample_count){
		auto frame = sc.get_current_frame();
		if (!frame){
			memset(dst, 0, dst_sample_count * sizeof(libpdboy_audiosample));
			break;
		}
		if (frame->frame_no < this->next_audio_frame){
			sc.return_used_frame(frame);
			continue;
		}

		this->next_audio_frame = frame->frame_no + 1;
		auto n = std::min<size_t>(dst_sample_count, frame->length);
		memcpy(dst, frame->buffer, n * sizeof(libpdboy_audiosample));
		dst += n;
		dst_sample_count -= n;
		if (frame->length > n){
			this->old_frame = frame;
			this->old_frame_offset = n;
			break;
		}
		sc.return_used_frame(frame);
	}
}

void libpdboy::public_set_input_state(libpdboy_inputstate *istate, int any_button_down, int any_button_up){
	auto input_state = new InputState;
	memcpy(input_state, istate, sizeof(*istate));
	this->gameboy->get_input_controller().set_input_state(input_state, !!any_button_down, !!any_button_up);
}

void libpdboy::public_set_stdout_provider(libpdboy_stdout_f callback){
	this->so = callback;
}

const char *libpdboy::public_get_exception_message(){
	{
		LOCK_MUTEX(this->exception_mutex);
		this->exception_message_copy = this->exception_message;
	}
	if (!this->exception_message_copy.size())
		return nullptr;
	return this->exception_message_copy.c_str();
}

bool libpdboy::load_file(const path_t &path, std::vector<byte_t> &dst, size_t max_size){
	auto casted_path = std::dynamic_pointer_cast<StdBasicString<char>>(path);
	if (!casted_path)
		return false;

	auto string = casted_path->get_std_basic_string();
	//this->stdout() << "Requested file load: \"" << string << "\", maximum size: " << maximum_size << " bytes.\n";

	if (this->lf){
		std::uint8_t *temp;
		std::uint64_t file_size;
		auto result = this->lf(this->user_data, string.c_str(), &temp, &file_size, max_size);
		if (result != Success)
			return false;
		std::unique_ptr<std::uint8_t, std::function<void(std::uint8_t *)>> p(temp, [this](std::uint8_t *p){ this->rb(this->user_data, p); });
		dst.resize(file_size);
		memcpy(&dst[0], temp, file_size);
		return true;
	}
	std::ifstream file(string.c_str(), std::ios::binary);
	if (!file)
		return false;

	file.seekg(0, std::ios::end);
	if ((size_t)file.tellg() > max_size)
		return false;
	dst.resize(file.tellg());
	file.seekg(0);
	file.read((char *)&dst[0], dst.size());
	return true;
}

void libpdboy::register_periodic_notification(Event &event){
	if (this->rpn){
		this->rpn(this->user_data, [](void *private_data){ ((Event *)private_data)->signal(); }, &event);
		return;
	}
	auto e = &event;
	this->run_spinlock_timer_thread = true;
	this->spinlock_timer_thread.reset(new std::thread([this, e](){ this->spinlock_timer_thread_function(e); }));
}

void libpdboy::spinlock_timer_thread_function(Event *event){
	auto resolution = 1000.0 / get_timer_resolution();
	while (this->run_spinlock_timer_thread){
		auto first = get_timer_count();
		while (true){
			auto delta = first - get_timer_count();
			if (delta * resolution > 1)
				event->signal();
		}
	}
}

void libpdboy::unregister_periodic_notification(){
	this->run_spinlock_timer_thread = false;
	if (this->spinlock_timer_thread){
		this->spinlock_timer_thread->join();
		this->spinlock_timer_thread.reset();
	}
	if (this->upn)
		this->upn(this->user_data);
}

DateTime libpdboy::local_now(){
	DateTime ret;
	if (this->ln){
		auto temp = this->ln(this->user_data);
		static_assert(sizeof(ret) == sizeof(temp), "");
		memcpy(&ret, &temp, sizeof(temp));
		return ret;
	}
	auto timestamp = ::time(nullptr);
	tm t = *localtime(&timestamp);
	ret.year = t.tm_year + 1900;
	ret.month = t.tm_mon + 1;
	ret.day = t.tm_mday;
	ret.hour = t.tm_hour;
	ret.minute = t.tm_min;
	ret.second = t.tm_sec;
	return ret;
}

void libpdboy::throw_exception(const char *message){
	LOCK_MUTEX(this->exception_mutex);
	this->exception_message = message;
}

path_t libpdboy::get_save_location(Cartridge &cart, SaveFileType type){
	auto ret = cart.get_path()->get_directory();
	auto name = cart.get_path()->get_filename()->remove_extension();
	const char *extension = nullptr;
	switch (type){
		case SaveFileType::Ram:
			extension = ".sav";
			break;
		case SaveFileType::Rtc:
			extension = ".rtc";
			break;
	}
	assert(extension);
	*name += extension;
	return ret->append_path_part(name);
}

bool libpdboy::save_file(const path_t &path, const std::vector<byte_t> &buffer){
	std::uint8_t b;
	const std::uint8_t *p = &b;
	if (buffer.size())
		p = &buffer[0];
	return this->save_file(path, p, buffer.size());
}

bool libpdboy::save_file(const path_t &path, const byte_t *buffer, size_t size){
	auto casted_path = std::dynamic_pointer_cast<StdBasicString<char>>(path);
	if (!casted_path)
		return false;

	auto string = casted_path->get_std_basic_string();
	//this->stdout() << "Requested file save: \"" << string << "\", size: " << size << " bytes.\n";

	if (this->wf)
		return !!this->wf(this->user_data, string.c_str(), buffer, size);

	std::ofstream file(string.c_str(), std::ios::binary);
	if (!file)
		return false;
	if (size)
		file.write((const char *)buffer, size);
	return true;
}

StdOutEmulator libpdboy::stdout(){
	return StdOutEmulator(
		[this](const std::string &out){
			this->send_to_output(out.c_str());
		}
	);
}

void libpdboy::send_to_output(const char *s){
	if (this->so)
		this->so(this->user_data, s);
}
