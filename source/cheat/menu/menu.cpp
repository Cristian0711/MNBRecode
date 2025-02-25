#include "core.h"

auto menu::setup_style() noexcept -> void 
{
	auto& style = ImGui::GetStyle();

	style.WindowBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(5.f, 5.f);
	style.ChildBorderSize = 0.4f;
	style.ChildRounding = 1.f;
	style.FrameRounding = 1.f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_Button] = ImColor(65, 93, 111, 255);
	style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_ButtonHovered] = ImColor(42, 61, 76, 255);
	style.Colors[ImGuiCol_CheckMark] = ImColor(85, 119, 152, 255);
	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);

	style.Colors[ImGuiCol_WindowBg] = ImColor(18, 23, 27, 250);
	style.Colors[ImGuiCol_Border] = ImColor(156, 161, 165, 255);

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdanab.ttf", 14);

	static const ImWchar szIconRange[] = { ICON_MIN_FA, ICON_MAX_FA, NULL };
	ImFontConfig fontConfig; fontConfig.MergeMode = true; fontConfig.PixelSnapH = true; fontConfig.GlyphOffset = ImVec2(-5, 2);
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(uiFontAwesomeData, uiFontAwesomeSize, 34, &fontConfig, szIconRange);
}

auto menu::setup() noexcept -> void
{
	callbacks::get().add_callback(callbacks::callback_type::every_frame, [this](){
		auto samp = modules::samp();

		if (ImGui::IsKeyPressed(VK_INSERT))
		{
			m_open = !m_open;
			samp.info()->toggle_cursor(m_open);

			if (!m_open)
				samp.info()->unlock_actor_cam();
		}

		if (m_open)
			samp.info()->lock_actor_cam();
	});
}

auto menu::draw() const noexcept -> void 
{
	auto& style = ImGui::GetStyle();

	static int8_t current_tab = 0;
	static ImVec2 main_window_pos = { 0.00f, 0.00f };

	ImGui::SetNextWindowSize(ImVec2(500.f, 417.f), ImGuiCond_Once);
	ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f }, ImGuiCond_Once, { 0.5f, 0.5f });
	if (ImGui::Begin("Make", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		main_window_pos = ImGui::GetWindowPos();
		switch (current_tab)
		{
		case 0:
			waypoints::get().do_settings_menu();
			packets::get().do_settings_menu();
			chat_detect::get().do_settings_menu();
			conditional_routes::get().do_settings_menu();
			break;
		case 1:
			break;
		case 2:
			public_configs::get().do_settings_menu();
			configs::get().do_settings_menu();
			break;
		case 3:
			chatlog::get().do_chatlog_menu();
			break;
		case 4:
			break;
		}
		ImGui::End();

	}

	ImGui::SetNextWindowSize(ImVec2(64.f, 417.f), ImGuiCond_Once);
	ImGui::SetNextWindowPos({ main_window_pos.x - 64, main_window_pos.y });
	if (ImGui::Begin("Tabs", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
	{
		ImVec2 tabs_window_pos = ImGui::GetWindowPos();

		if (ImGui::IsMouseHoveringRect(tabs_window_pos, { tabs_window_pos.x + 64, tabs_window_pos.y + 417.f }) && ImGui::GetWindowSize().x < 146)
			ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x + 2, 417.f));

		if (!ImGui::IsMouseHoveringRect(tabs_window_pos, { tabs_window_pos.x + 146, tabs_window_pos.y + 417.f }) && ImGui::GetWindowSize().x > 64)
			ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x - 2, 417.f));

		ImGui::BeginGroup();

		if (ImGui::Tab(ICON_FA_SLIDERS, "Main", current_tab == 0, {130, 40}))
			current_tab = 0;
		if (ImGui::Tab(ICON_FA_COMMENTS, "Chat", current_tab == 1, {130, 40}))
			current_tab = 1;
		if (ImGui::Tab(ICON_FA_GEAR, "Configs", current_tab == 2, {130, 40}))
			current_tab = 2;
		if (ImGui::Tab(ICON_FA_STICKY_NOTE, "ChatLog", current_tab == 3, {130, 40}))
			current_tab = 3;
		if (ImGui::Tab(ICON_FA_USER_SECRET, "User", current_tab == 4, { 130, 40 }, true))
			current_tab = 4;

		ImGui::EndGroup();

		ImVec2 vec_text_size = ImGui::CalcTextSize("MAKE");
		ImVec2 vec_position = { tabs_window_pos.x + 32, tabs_window_pos.y + 390 };
		vec_position.x -= vec_text_size.x / 2.f + 7;

		ImGui::GetWindowDrawList()->AddText(m_font, 20, vec_position, ImGui::GetColorU32({ 1, 1, 1, 1 }), "MAKE");
		ImGui::End();
	}
}