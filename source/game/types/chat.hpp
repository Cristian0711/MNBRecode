#include "core.h"

class samp_chat_t {
public:
	enum e_chat_message_type
	{
		CHAT_TYPE_NONE = 0,
		CHAT_TYPE_CHAT = 2,
		CHAT_TYPE_INFO = 4,
		CHAT_TYPE_DEBUG = 8,
		CHAT_TYPE_MNB = 16
	};

	struct chat_entry_t
	{
		uint32_t system_time;
		char prefix[28];
		char text[144];
		uint8_t unknown[64];
		int type;
		D3DCOLOR text_color;
		D3DCOLOR prefix_color;
	};

	template <typename... Args>
	inline auto add_info_message(const std::string_view message, Args&&... args)  const noexcept -> void {
		LOG("fnAddInfoMessage");

		if (message.length() >= 144)
			return;

		add_entry(CHAT_TYPE_DEBUG, std::vformat(message, std::make_format_args(args...)), "", chat_debug_color, 0);
	}

	[[nodiscard]] inline auto get_last_entry_text() const noexcept -> std::string_view {
		return { get_last_entry()->text, 144 };
	}

	static constexpr uint32_t chat_alert_color { 16711680u };

private:
	template <typename... Args>
	inline auto add_message(D3DCOLOR color, const std::string_view message, Args&&... args) const noexcept -> void {
		LOG("fnAddMessageToChat");

		if (message.length() >= 144)
			return;

		add_entry(CHAT_TYPE_DEBUG, std::vformat(message, std::make_format_args(args...)), "", color, 0);
	}

	inline auto add_entry(uint32_t type, const std::string_view text, std::string_view prefix, D3DCOLOR text_color, D3DCOLOR prefix_color) const noexcept -> void
	{
		LOG("fnAddChatEntry");
		((void(__thiscall*)(const samp_chat_t*, uint32_t, const char*, const char*, D3DCOLOR, D3DCOLOR))(g_samp_module + g_samp_offsets->func_chat_msg()))(this, type, text.data(), prefix.data(), text_color, prefix_color);
	}

	[[nodiscard]] inline auto get_entry(uint8_t index) const noexcept -> chat_entry_t*
	{
		static auto entry_base = reinterpret_cast<uint32_t>(this + g_samp_offsets->chat_entry_offset());
		return reinterpret_cast<chat_entry_t*>(entry_base + index * sizeof(chat_entry_t));
	}

	[[nodiscard]] inline auto get_last_entry() const noexcept -> chat_entry_t* {
		return get_entry(max_entries);
	}

private:
	static constexpr uint32_t chat_debug_color { 8956514u };
	static constexpr uint8_t max_entries = { 99u };
};
