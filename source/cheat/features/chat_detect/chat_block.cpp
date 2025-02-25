#include "core.h"

auto chat_block::add_color(uint8_t ignore_mode, const std::string_view color) noexcept -> void
{
	remove_color(color);
	vars.chat_detect.color_keywords.emplace_back(ignore_mode, color);
}

auto chat_block::remove_color(const std::string_view color) noexcept -> void
{
	auto& color_keywords = vars.chat_detect.color_keywords;

	color_keywords.erase(
		std::remove_if(color_keywords.begin(), color_keywords.end(),
			[&color](const auto& entry) { return entry.color == color; }),
		color_keywords.end()
	);
}

auto chat_block::is_color_in_vector(const std::string_view color) const noexcept -> std::optional<uint8_t> 
{
	auto& color_keywords = vars.chat_detect.color_keywords;
	auto it = std::find_if(color_keywords.begin(), color_keywords.end(),
		[&color](const auto& entry) {
			return entry.color == color;
		});

	if (it != color_keywords.end()) {
		return it->ignore_mode;
	}

	return std::nullopt;
}

auto chat_block::should_delete_last_chat() noexcept -> bool
{
	const auto& last_chat_entry = chatlog::get().get_last_entry();

	if (last_chat_entry == std::nullopt)
		return false;

	for (const auto& entry_data : last_chat_entry.value().get())
	{
		if (entry_data.type != chatlog::chat_log_type::TYPE_COLOR)
			continue;

		auto ignore_mode = chat_block::get().is_color_in_vector(entry_data.data);
		if (ignore_mode == std::nullopt)
			continue;

		switch (ignore_mode.value())
		{
		case DELETE_CHAT:
			return true;
		case IGNORE_CHAT:
			m_ignore_last_chat = true;
			return false;
		default:
			break;
		}
	}

	return false;
}