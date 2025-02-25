class chat_detect {
private:
	chat_detect() {};
public:
	[[nodiscard]] static auto get() noexcept -> chat_detect& {
		static chat_detect instance;
		return instance;
	}

	[[nodiscard]] inline auto do_hooks() noexcept -> bool
	{
		auto samp = modules::samp();

		o_add_entry = reinterpret_cast<add_entry_t>(samp.get_base() + samp.get_offsets()->add_entry());
		if (detours_wrapper::get().hook(o_add_entry, hook_add_entry) == false)
			return false;

		return true;
	}

	inline auto setup() noexcept -> void
	{
		callbacks::get().add_callback(callbacks::callback_type::every_frame, [this]() {
			tick();
		});
	}

	inline auto reset() noexcept -> void
	{
		m_ignore_last_chat = false;
		m_has_line_changed = false;
		m_send_response = false;
		m_should_play_alarm = false;

		response_delay = 0;

		last_keyword = { nullptr, 0 };
	}

	auto do_settings_menu() noexcept -> void;
private:
	auto tick() noexcept -> void;
	auto scan_chat() noexcept -> void;
	auto send_response() noexcept -> void;
	auto start_alarm() noexcept -> void;

	static auto __fastcall hook_add_entry(void* _this, void* _edx, int32_t type, const char* text, const char* prefix, uint32_t text_color, uint32_t prefix_color) noexcept -> void;
	typedef void(__thiscall* add_entry_t)(void*, int32_t, const char*, const char*, uint32_t, uint32_t);
	add_entry_t o_add_entry;

private:
	bool m_ignore_last_chat { false };
	bool m_has_line_changed { false };
	bool m_send_response { false };
	bool m_should_play_alarm { false };

	uint32_t response_delay { 0 };

	std::string_view last_keyword { nullptr, 0 };
};