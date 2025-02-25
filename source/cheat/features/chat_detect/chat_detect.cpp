#include "core.h"

auto __fastcall chat_detect::hook_add_entry(void* _this, void* _edx, int32_t type, const char* text, const char* prefix, uint32_t text_color, uint32_t prefix_color) noexcept -> void
{
	// workaround otherwise if prefix or text == nullptr it will throw an unhandled exception
	chatlog::get().add_to_chat_log(type, text != nullptr ? text : "", prefix != nullptr ? prefix : "", text_color, prefix_color);

	// verify if the chat should be deleted ( based on color )
	if (chat_block::get().should_delete_last_chat() == true)
		return;

	auto& chat = chat_detect::get();

	chat.m_has_line_changed = true;
	chat.m_should_play_alarm = false;

	return chat_detect::get().o_add_entry(_this, type, text, prefix, text_color, prefix_color);
}

auto chat_detect::scan_chat() noexcept -> void
{
	if (m_ignore_last_chat == true || m_has_line_changed == false)
		return;

	if (chat_block::get().should_ignore_last_chat() == true)
		return;

	static auto lower_compare = [](char l, char r) {
		return (std::tolower(l) == std::tolower(r));
	};

	const auto last_entry_text = modules::samp().chat()->get_last_entry_text();
	for (const auto& [keyword, response] : vars.chat_detect.chat_keywords)
	{
		auto search_result = std::search(last_entry_text.begin(), last_entry_text.end(), keyword.begin(), keyword.end(), lower_compare);

		if (search_result == last_entry_text.end())
			continue;

		auto response_segment_count = response.length() / 5.f;
		auto tick = static_cast<int32_t>(GetTickCount64());
		response_segment_count <= 1 ? response_delay = tick + 1000 : response_delay = tick + static_cast<int32_t>(response_segment_count) * 1000;

		last_keyword = response;
		m_send_response = true;
		m_should_play_alarm = true;

		if (vars.misc.use_discord_alerts) {
			socket_commands::get().add_command(last_entry_text.data(), user_commands_t::CHAT_DISCORD_ALERT);
		}
	}

	m_has_line_changed = false;
}

auto chat_detect::send_response() noexcept -> void
{
	if (m_send_response == false)
		return;

	if (last_keyword.empty())
		return;

	if (GetTickCount64() < response_delay)
		return;

	modules::samp().local_player()->send(last_keyword);
	m_send_response = false;
}

auto chat_detect::start_alarm() noexcept -> void
{
	if (m_should_play_alarm == false)
		return;

	alarm::get().start_alarm();
}

auto chat_detect::tick() noexcept -> void
{
	if (vars.chat_detect.enable == false)
		return;

	if (global_utils::get().any_route_enabled() == false)
		return;

	scan_chat();
	send_response();
	start_alarm();
}

auto chat_detect::do_settings_menu() noexcept -> void
{
	static char keyword[256];
	static char response[256];

	ImGui::draw_content_box("Chat settings", [this]() {
		/*static ImVec2 TextSize = pFont->CalcTextSizeA(70.f, FLT_MAX, 0.0f, xorstr_("Response"));
		ImVec2 ButtonSize = ImVec2((ImGui::GetContentRegionAvail().x - 9.f) / 2.f, 20.f);*/

		ImGui::Text("Chat search/response");

		ImGui::InputText("Keyword", keyword, IM_ARRAYSIZE(keyword));
		ImGui::InputText("Response", response, IM_ARRAYSIZE(response));

		ImVec2 button_size{ (ImGui::GetContentRegionAvail().x - 9.f) / 2.f, 20.f };

		auto& keywords_vector = vars.chat_detect.chat_keywords;

		if (ImGui::Button("AddEntry", button_size) && strlen(keyword) != 0 && strlen(response) != 0)
		{
			keywords_vector.emplace_back(keyword, response);
			sprintf(keyword, "");
			sprintf(response, "");
		}

		ImGui::SameLine();

		if (ImGui::Button("ClearEntries", button_size))
			keywords_vector.clear();

		const auto tree_text = std::format("Keyword entries: {}", keywords_vector.size());

		if (!ImGui::TreeNode("##color_entries", tree_text.c_str()))
			return;

		for (size_t index = 0; index != keywords_vector.size(); ++index)
		{
			ImGui::PushID(static_cast<int>(index));
			if (ImGui::Button("Delete", ImVec2(90.f, 20.f)))
			{
				keywords_vector.erase(keywords_vector.begin() + index);

				ImGui::PopID();

				if (index == 0)
					return;

				--index;
			}

			ImGui::SameLine();

			const auto second_tree_text = std::format("Entry: #{}", index + 1);
			if (ImGui::TreeNode(second_tree_text.c_str()))
			{
				ImGui::Separator();
				ImGui::BeginColumns("##menu_items", 2, ImGuiColumnsFlags_NoResize);
				ImGui::Text(keywords_vector[index].keyword.c_str());
				ImGui::NextColumn();
				ImGui::Text(keywords_vector[index].response.c_str());
				ImGui::EndColumns();
				ImGui::Separator();

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		ImGui::TreePop();
	});

}