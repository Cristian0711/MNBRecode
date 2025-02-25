#include "core.h"

auto conditional_routes::change_current_route() noexcept -> void
{
	if (vars.conditional_routes.use_random_routes) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> dist(0, loops_configs.size() - 1);

		if (vars.conditional_routes.do_not_use_last_route) {
			uint8_t new_route = m_current_route;

			while (new_route == m_current_route && loops_configs.size() > 1) {
				new_route = static_cast<uint8_t>(dist(gen));
			}
			m_current_route = new_route;
		}
		else {
			// no restrictions
			m_current_route = dist(gen);
		}

		modules::samp().chat()->add_info_message("Random route: {}", m_current_route);
	}
	else {
		++m_current_route;
		modules::samp().chat()->add_info_message("Normal route: {}", m_current_route);
	}

	if (m_current_route >= loops_configs.size())
		m_current_route = 0;
}

auto conditional_routes::change_to_next_route() noexcept -> void {
	change_current_route();

	if (loops_configs[m_current_route].config_name == configs::get().active_config_name()) {
		modules::samp().chat()->add_info_message("ROUTE REMAINS: {}", loops_configs[m_current_route].config_name);
		return;
	}

	configs::get().load_route_config(loops_configs[m_current_route].filename, true);
	global_utils::get().enable();

	m_last_route_was_conditional = false;
	modules::samp().chat()->add_info_message("ROUTE CHANGED TO: {}", loops_configs[m_current_route].config_name);
}

auto conditional_routes::change_to_conditional_route(std::string_view config_name) noexcept -> void {
	change_current_route();

	configs::get().load_route_config(config_name, true);
	global_utils::get().enable();

	m_last_route_was_conditional = true;
	modules::samp().chat()->add_info_message("CONDITIONAL ROUTE CHANGED TO: {}", config_name);
}

auto conditional_routes::any_condition_valid() noexcept -> std::optional<std::string_view>
{
	if (conditional_configs.empty() == true)
		return std::nullopt;

	for (const auto& config : conditional_configs) {
		switch (config.condition) {
		case HEALTH:
			if (m_last_health_value <= config.value) {
				return config.filename;
			}
			break;
		case FUEL:
			if (m_last_fuel_value == -1) {
				return std::nullopt;
			}
			if (m_last_fuel_value <= config.value) {
				return config.filename;
			}
			break;
		}
	}

	return std::nullopt;
}

auto conditional_routes::increment_route_loops_and_check() noexcept -> void {
	if (vars.conditional_routes.enable == false)
		return;

	if (global_utils::get().any_route_enabled() == false)
		return;

	m_last_health_value = vars.local_ped->m_fHealth;
	m_last_fuel_value = get_fuel_value();

	auto condition_valid = any_condition_valid();
	if(condition_valid != std::nullopt){
		change_to_conditional_route(condition_valid.value());
		m_route_loops = 0;
		return;
	}

	++m_route_loops;
	if (loops_configs.empty() == true)
		return;

	modules::samp().chat()->add_info_message("CURRENT ROUTE: {}, LOOPS NEEDED: {}", m_route_loops, loops_configs[m_current_route].loops_count);
	if (m_route_loops >= loops_configs[m_current_route].loops_count || vars.conditional_routes.use_random_routes == true ||
		m_last_route_was_conditional == true) {
		change_to_next_route();
		m_route_loops = 0;
	}
}

auto conditional_routes::find_fill_textdraw_id() const noexcept -> uint16_t
{
	for (size_t index = 2048; index != 2304; ++index)
	{
		if (modules::samp().textdraws()->is_listed(index) == 0)
			continue;

		auto* textdraw = modules::samp().textdraws()->get_textdraw(index);

		if (textdraw == nullptr)
			continue;

		if (textdraw->text().find("Fuel") == std::string::npos)
			continue;

		return index;
	}

	return 0u;
}

auto conditional_routes::get_fuel_value() noexcept -> float
{
	if (m_fuel_textdraw_id == 0)
		m_fuel_textdraw_id = find_fill_textdraw_id();

	if (m_fuel_textdraw_id == 0)
		return -1.f;


	auto* textdraw = modules::samp().textdraws()->get_textdraw(m_fuel_textdraw_id);

	if (textdraw == nullptr)
		return -1.f;

	size_t first_digit_pos = textdraw->text().find_first_of("0123456789", textdraw->text().find("Fuel"));
	if (first_digit_pos == std::string::npos) 
		return -1;

	return std::stof(textdraw->text().substr(first_digit_pos, 3).data());
}

auto conditional_routes::refresh_data() noexcept -> void
{
	m_last_health_value = vars.local_ped->m_fHealth;
	m_last_fuel_value = get_fuel_value();
}

auto conditional_routes::do_settings_menu() noexcept -> void
{
	ImGui::draw_content_box("Route randomizer", [this]() {
		static int config_select_index = 0;

		auto& route_configs = configs::get().routes();
		if (config_select_index >= route_configs.size())
			config_select_index = route_configs.size() - 1;

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::ComboBoxConfigs("##random_config", &config_select_index, route_configs);
		ImGui::PopItemWidth();

		ImVec2 half_size_button = { (ImGui::GetContentRegionAvail().x - 4.f) / 2.f, 20.f };

		ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - 4.f) / 2.f);
		static int type_switch = 0;
		const char* type_items[] = { "LOOPS", "CONDITIONAL" };
		ImGui::Combo("##type_combo", &type_switch, type_items, 2);
		ImGui::SameLine();
		static int condition_switch = 0;
		const char* condition_items[] = { "HEALTH", "FUEL" };
		ImGui::Combo("##condition_combo", &condition_switch, condition_items, 2);
		ImGui::PopItemWidth();

		if (ImGui::Button("ADD CONFIG", half_size_button))
		{
			if (type_switch == LOOPS_CONFIG)
				loops_configs.emplace_back(route_configs[config_select_index], 1);
			else
				conditional_configs.emplace_back(route_configs[config_select_index], condition_switch, 10.f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Refresh data", half_size_button)) {
			refresh_data();
		}
		ImGui::Checkbox("Conditional routes", &vars.conditional_routes.enable);
		ImGui::Checkbox("Use route randomizer", &vars.conditional_routes.use_random_routes);
		ImGui::Checkbox("Do not include last route in random", &vars.conditional_routes.do_not_use_last_route);

		ImGui::Separator();
		ImGui::BeginColumns("##data_items", 2, ImGuiColumnsFlags_NoResize);
		ImGui::Text2("Health: {:.2f}", m_last_health_value);
		ImGui::NextColumn();
		ImGui::Text2("Fuel: {:.2f}", m_last_fuel_value);
		ImGui::EndColumns();
		ImGui::Separator();

		ImGui::Text("Loops configs:");
		for (size_t index = 0; index != loops_configs.size(); ++index)
		{
			auto& config = loops_configs[index];
			ImGui::PushID(std::vformat("loops{}{}", std::make_format_args(config.uuid, index)).c_str());

			ImGui::BeginColumns("##loops_items", 3, ImGuiColumnsFlags_NoResize);
			ImGui::Text2(config.config_name);
			ImGui::NextColumn();
			ImGui::PushItemWidth(40);
			ImGui::InputScalar("Loops count", ImGuiDataType_U8, &config.loops_count);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			if (ImGui::Button("DELETE")) {

				loops_configs.erase(loops_configs.begin() + index);

				ImGui::PopID();
				if (index == 0)
					return;

				--index;
			}
			ImGui::EndColumns();
			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::Text("Conditional configs:");
		for (size_t index = 0; index != conditional_configs.size(); ++index)
		{
			auto& config = conditional_configs[index];
			ImGui::PushID(std::vformat("conditional{}{}", std::make_format_args(config.uuid, index)).c_str());

			ImGui::BeginColumns("##loops_items", 3, ImGuiColumnsFlags_NoResize);
			ImGui::Text2(config.config_name);
			ImGui::NextColumn();
			ImGui::PushItemWidth(40);
			ImGui::InputFloat(config.condition == HEALTH ? "HEALTH" : "FUEL", &config.value);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			if (ImGui::Button("DELETE")) {

				conditional_configs.erase(conditional_configs.begin() + index);

				ImGui::PopID();
				if (index == 0)
					return;

				--index;
			}

			ImGui::EndColumns();
			ImGui::PopID();
		}
	});
}