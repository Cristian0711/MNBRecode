#include "core.h"

auto chatlog::add_color_to_line_data(line_data_t& data, uint32_t color) noexcept -> void
{
	uint8_t red = ((color >> 16) & 0xff), green = ((color >> 8) & 0xff), blue = (color & 0xff);
	ImVec4 imvec4_color{ red / 255.f, green / 255.f, blue / 255.f, 1.00f };
	std::string hex_color = std::format("{:X}", color & 0x00FFFFFF);

	if (hex_color.length() != 6) {
		hex_color = "{00" + hex_color + "}";
	}
	else {
		hex_color = "{" + hex_color + "}";
	}

	data.emplace_back(chat_log_type::TYPE_COLOR, std::move(imvec4_color), std::move(hex_color));
}

auto chatlog::add_color_to_line_data(line_data_t& data, const std::string_view color) noexcept -> void
{
	auto int_color = std::stoi(color.data(), nullptr, 16);

	uint8_t red = ((int_color >> 16) & 0xff), green = ((int_color >> 8) & 0xff), blue = (int_color & 0xff);
	ImVec4 imvec4_color{ red / 255.f, green / 255.f, blue / 255.f, 1.00f };

	auto string_color = std::format("{{{}}}", color);
	std::transform(string_color.begin(), string_color.end(), string_color.begin(),
		::toupper);

	data.emplace_back(chat_log_type::TYPE_COLOR, std::move(imvec4_color), std::move(string_color));
}

auto chatlog::add_text_to_line_data(line_data_t& data, const std::string_view text) noexcept -> void
{
	std::string forward_text{ text };

	if (forward_text.find('%') != std::string::npos)
		std::replace(forward_text.begin(), forward_text.end(), '%', '#');

	data.emplace_back(chat_log_type::TYPE_TEXT, std::move(forward_text));
}

auto chatlog::add_to_chat_log(int32_t type, const std::string_view text, const std::string_view prefix, uint32_t text_color, uint32_t prefix_color) noexcept -> void
{
	line_data_t line_data;

	if (type == samp_chat_t::CHAT_TYPE_CHAT)
	{
		add_color_to_line_data(line_data, prefix_color);
		add_text_to_line_data(line_data, prefix);
		add_text_to_line_data(line_data, ": ");
	}

	add_color_to_line_data(line_data, text_color);

	if (type == samp_chat_t::CHAT_TYPE_MNB)
	{
		add_text_to_line_data(line_data, text);
		chatlog_data.emplace_back(std::move(line_data));
		return;
	}

	static const std::regex regex { "\\{([a-fA-F0-9]{6})\\}" };
	std::cregex_iterator it { text.data(), text.data() + text.size(), regex };
	std::cregex_iterator end_it;

	size_t last_pos = 0;
	while (it != end_it)
	{
		std::cmatch color_match = *it;
		size_t match_pos = color_match.position();
		size_t match_len = color_match.length();

		if (match_pos > last_pos) {
			const auto segment = text.substr(last_pos, match_pos - last_pos);
			add_text_to_line_data(line_data, segment);
		}

		add_color_to_line_data(line_data, color_match[1].str());

		last_pos = match_pos + match_len;
		++it;
	}

	// this will add the entire chat line if no hex color was found
	if (last_pos < text.size()) {
		add_text_to_line_data(line_data, text.substr(last_pos));
	}

	chatlog_data.emplace_back(std::move(line_data));

	if (chatlog_data.size() > 50)
		chatlog_data.erase(chatlog_data.begin());
}

auto chatlog::draw_color_tree_list() const noexcept -> void
{
	auto& colors_vector = vars.chat_detect.color_keywords;
	const auto tree_text = std::format("Color entries: {}", colors_vector.size());

	if (!ImGui::TreeNode("##color_entries", tree_text.c_str()))
		return;

	for (size_t index = 0; index != colors_vector.size(); ++index)
	{
		const auto& color_entry = colors_vector[index];

		ImGui::PushID(static_cast<int>(index));

		if (ImGui::Button("Delete", ImVec2(90.f, 20.f)))
		{
			colors_vector.erase(colors_vector.begin() + index);
			ImGui::PopID();

			if (index == 0)
				return;

			--index;
		}

		ImGui::SameLine();

		const auto entry_text = std::format("Entry #{:d}", index + 1);
		if (!ImGui::TreeNode(entry_text.c_str()))
		{
			ImGui::PopID();
			continue;
		}

		ImGui::Separator();

		ImGui::BeginColumns("##menu_items", 2, ImGuiColumnsFlags_NoResize);
		if (color_entry.ignore_mode == chat_block::DELETE_CHAT)
			ImGui::Text("Delete");
		else
			ImGui::Text("Ignore");

		ImGui::NextColumn();

		ImGui::Text2(color_entry.color);

		ImGui::EndColumns();
		ImGui::Separator();

		ImGui::TreePop();
		ImGui::PopID();
	}

	ImGui::TreePop();
}

auto chatlog::draw_chat_log_child() const noexcept -> void
{
	ImGui::BeginChild("chat_log_child", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar);

	for (const auto& chat_entry : chatlog_data)
	{
		ImVec4 last_color{ 0.00f, 0.00f, 0.00f, 0.00f };

		std::unordered_set<std::string_view> delete_colors, ignore_colors;
		for (const auto& entry_data : chat_entry)
		{
			if (entry_data.type != chat_log_type::TYPE_COLOR)
				continue;

			auto ignore_mode = chat_block::get().is_color_in_vector(entry_data.data);
			if (ignore_mode == std::nullopt)
				continue;

			switch (ignore_mode.value())
			{
			case chat_block::DELETE_CHAT:
				delete_colors.insert(entry_data.data);
				break;
			case chat_block::IGNORE_CHAT:
				ignore_colors.insert(entry_data.data);
				break;
			}
		}

		if (delete_colors.size() != 0)
		{
			ImGui::vectored_tooltip({ 1.f, 0.f, 0.f, 1 }, "This chat is deleted!", delete_colors);
			ImGui::SameLine();
		}

		if (ignore_colors.size() != 0)
		{
			ImGui::vectored_tooltip({ 128 / 255.f,  128 / 255.f,  128 / 255.f, 1 }, "This chat is ignored!", ignore_colors);
			ImGui::SameLine();
		}

		if (delete_colors.empty() == true && ignore_colors.empty() == true)
		{
			ImGui::colored_tooltip({ 0.f, 1.f, 0.f, 1 }, "This chat is good!");
			ImGui::SameLine();
		}

		for (const auto& entry_data : chat_entry)
		{
			ImGui::PushID(entry_data.data.c_str());
			switch (entry_data.type)
			{
			case chat_log_type::TYPE_TEXT:
				ImGui::TextColored(last_color, entry_data.data.c_str());
				ImGui::SameLine(0, 0);
				break;
			case chat_log_type::TYPE_COLOR:
				ImGui::TextColored(entry_data.color, entry_data.data.c_str());
				last_color = entry_data.color;
				ImGui::SameLine(0, 0);
				break;
			}

			// proceed to handle the right click || left click
			if (entry_data.type != chat_log_type::TYPE_COLOR || ImGui::IsItemHovered() == false)
			{
				ImGui::PopID();
				continue;
			}

			auto ignore_mode = chat_block::DEFAULT;
			if (ImGui::IsMouseClicked(ImGuiPopupFlags_MouseButtonLeft))
				ignore_mode = chat_block::DELETE_CHAT;
			else if (ImGui::IsMouseClicked(ImGuiPopupFlags_MouseButtonRight))
				ignore_mode = chat_block::IGNORE_CHAT;

			// 2 is an invalid value, we can detect if mouse was pressed
			if (ignore_mode == chat_block::DEFAULT)
			{
				ImGui::PopID();
				continue;
			}

			if (chat_block::get().is_color_in_vector(entry_data.data) != std::nullopt)
				chat_block::get().remove_color(entry_data.data);
			else
				chat_block::get().add_color(ignore_mode, entry_data.data);

			ImGui::PopID();
		}

		ImGui::NewLine();
	}

	ImGui::EndChild();
}

auto chatlog::do_chatlog_menu() const noexcept -> void
{
	ImGui::colored_tooltip({ 1, 1, 1, 1 }, "Left Click to add Deleted / Right Click to add Ignored");
	ImGui::SameLine();

	draw_color_tree_list();
	ImGui::Separator();
	draw_chat_log_child();
}