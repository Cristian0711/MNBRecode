#include "core.h"

auto waypoints::enable() noexcept -> void
{
	if (vars.waypoints.waypoints_vector.size() < 2)
		return;

	m_active_waypoint = 0;
	vars.waypoints.enable = true;
	m_paused = false;

	menu::get().set_enable_button_text(true);
}

auto waypoints::disable() noexcept -> void
{
	m_active_waypoint = 0;
	vars.waypoints.enable = false;

	m_fill_route_active = false;
	m_route_loops = 0;

	utils::get().unlock_ped();
	menu::get().set_enable_button_text(false);
}

auto waypoints::draw_waypoint(size_t index, const CVector& position, ImColor& color) const noexcept -> void
{
	CVector screen_position;

	if (render_utils::get().get_screen_pos(position, screen_position) == false)
		return;

	if (index == m_active_waypoint)
	{
		render_utils::get().draw_circle_filled(screen_position, 7, ImColor{ 0.00f, 0.00f, 0.00f, 1.00f });
		render_utils::get().draw_circle_filled(screen_position, 5, ImColor{ 1.00f, 1.00f, 0.00f, 1.00f });
	}
	else
	{
		render_utils::get().draw_circle_filled(screen_position, 7, ImColor{ 0.00f, 0.00f, 0.00f, 1.00f });
		render_utils::get().draw_circle_filled(screen_position, 5, ImColor{ 1.00f, 0.00f, 0.00f, 1.00f });
	}

	screen_position.y -= 25;

	std::string waypoint_name = std::vformat("Waypoint {} - {:.1f}", std::make_format_args(index + 1, utils::get().get_distance_to_local_ped(position)));
	render_utils::get().draw_string(waypoint_name, screen_position, 16, color, true);
}

auto waypoints::draw() const noexcept -> void
{
	if (vars.misc.show_count == 0)
		return;

	const auto& waypoints_vector = vars.waypoints.waypoints_vector;
	for (size_t index = 0; index < waypoints_vector.size(); index += vars.misc.show_count)
		draw_waypoint(index, waypoints_vector[index].position, vars.style.waypoint_text_color);
	
	const auto& fill_waypoints_vector = vars.waypoints.fill_waypoints_vector;
	for (size_t index = 0; index < fill_waypoints_vector.size(); index += vars.misc.show_count)
		draw_waypoint(index, fill_waypoints_vector[index].position, vars.style.fill_waypoint_text_color);
}

auto waypoints::add_waypoint(std::vector<waypoint_t>& waypoints, const CVector& position, float speed) const noexcept -> void
{
	if (global_utils::get().are_other_waypoints_except_this(global_utils::record_type::WAYPOINTS) == true)
	{
		global_utils::get().stop_all_record_types();
		global_utils::get().clear_all_waypoints();
	}

	waypoints.emplace_back(position, speed);
}

auto waypoints::record_waypoint() const noexcept -> void
{
	if (vars.local_vehicle != nullptr)
	{
		add_waypoint(vars.waypoints.waypoints_vector, vars.local_vehicle->GetPosition(), (vars.local_vehicle->m_vecMoveSpeed.Magnitude() * 170.0f) / 2.8f);
	}
	else if (vars.local_ped != nullptr)
	{
		add_waypoint(vars.waypoints.waypoints_vector, vars.local_ped->GetPosition(), 1.f);
	}
}

auto waypoints::record_fill_waypoint() const noexcept -> void
{
	if (vars.local_vehicle != nullptr)
	{
		add_waypoint(vars.waypoints.fill_waypoints_vector, vars.local_vehicle->GetPosition(), (vars.local_vehicle->m_vecMoveSpeed.Magnitude() * 170.0f) / 2.8f);
	}
	else if (vars.local_ped != nullptr)
	{
		add_waypoint(vars.waypoints.fill_waypoints_vector, vars.local_ped->GetPosition(), 1.f);
	}
}

auto waypoints::record() const noexcept -> void
{
	static auto current_tick = GetTickCount64();

	if (GetTickCount64() < current_tick + vars.waypoints.record_delay)
		return;

	if (vars.waypoints.record_waypoints)
		record_waypoint(), current_tick = GetTickCount64();
	else if(vars.waypoints.record_fill_waypoints)
		record_fill_waypoint(), current_tick = GetTickCount64();
}

auto waypoints::should_start_fill_route() noexcept -> bool
{
	if (vars.waypoints.use_fill_route == false)
		return false;

	if (vars.waypoints.fill_waypoints_vector.empty() == true)
		return false;

	if (vars.local_vehicle == nullptr)
		return false;

	if (vars.waypoints.fill_routes_loops_count > m_route_loops)
		return false;

	if (m_active_waypoint != 0)
		return false;

	return true;
}

auto waypoints::start_fill_route() noexcept -> void
{
	m_active_waypoint = 0;
	m_fill_route_active = true;
	m_route_loops = 0;
}

auto waypoints::fill_thread() noexcept -> void
{
	// wait for vehicle to stop
	while (vars.local_vehicle->m_vecMoveSpeed.Magnitude() != 0)
		Sleep(200);

	utils::get().unlock_ped();
	modules::samp().local_player()->send("/engine");
	std::this_thread::sleep_for(1s);
	modules::samp().local_player()->send("/fill");
	std::this_thread::sleep_for(1s);
	Sleep(vars.waypoints.fill_route_sleep);
	modules::samp().local_player()->send("/engine");

	resume();
}

auto waypoints::fill_route() noexcept -> void
{
	const auto& waypoints_vector = vars.waypoints.fill_waypoints_vector;
	const auto& local_position = utils::get().get_local_ped_position();
	auto waypoints_utils = waypoints_utils::get();

	if (vars.local_vehicle == nullptr)
	{
		disable();
		return;
	}

	if (waypoints_utils.is_ped_in_waypoints_proximity(waypoints_vector[m_active_waypoint].position) == true)
		++m_active_waypoint;

	if (m_active_waypoint == waypoints_vector.size())
	{
		m_active_waypoint = 0;
		m_fill_route_active = false;

		return;
	}

	auto& waypoint = waypoints_vector[m_active_waypoint];
	if (m_active_waypoint + 1 == vars.waypoints.fill_waypoint_index) 
	{
		waypoints_utils.drive_to_coords(waypoint.position, 1.f);
		std::thread(&waypoints::fill_thread, this).detach();

		++m_active_waypoint;
		pause();
		return;
	}

	waypoints_utils.drive_to_coords(waypoint.position, waypoint.speed);
}

auto waypoints::normal_route() noexcept -> void
{
	const auto& waypoints_vector = vars.waypoints.waypoints_vector;
	const auto& local_position = utils::get().get_local_ped_position();
	auto waypoints_utils = waypoints_utils::get();

	if (waypoints_utils.is_ped_in_waypoints_proximity(waypoints_vector[m_active_waypoint].position) == true)
		++m_active_waypoint;

	if (m_active_waypoint == waypoints_vector.size())
	{
		conditional_routes::get().increment_route_loops_and_check();
		m_active_waypoint = 0;
		++m_route_loops;
		return;
	}

	auto& waypoint = waypoints_vector[m_active_waypoint];
	if (vars.local_vehicle != nullptr) {
		waypoints_utils.drive_to_coords(waypoint.position, waypoint.speed);
	}
	else if (vars.local_ped != nullptr) {
		waypoints_utils.walk_to_coords(waypoint.position);
	}
}

auto waypoints::walk_drive_system() noexcept -> void
{
	const auto& waypoints_vector = vars.waypoints.waypoints_vector;
	if (waypoints_vector.size() < 2)
		return;

	if (m_paused == true)
		return;

	if (should_start_fill_route() == true)
		start_fill_route();

	auto& utils = utils::get();

	if (utils.does_ped_send_input() == true)
	{
		if (utils.is_ped_locked() == true)
			utils.unlock_ped();

		return;
	}

	if (m_fill_route_active == true) {
		fill_route();
	}
	else {
		normal_route();
	}
}

auto waypoints::tick() noexcept -> void
{
	vars.local_ped = FindPlayerPed();
	vars.local_vehicle = FindPlayerVehicle(-1, false);

	record();

	if (vars.waypoints.enable == false)
		return;

	walk_drive_system();
}

auto waypoints::do_settings_menu() noexcept -> void
{
	ImGui::draw_content_box("Main settings", [this]() {
		if (ImGui::Button(menu::get().enable_button_text().data()))
		{
			if (global_utils::get().any_route_enabled() == false) {
				conditional_routes::get().reset();
				global_utils::get().enable();
			}
			else
				global_utils::get().disable();
		}

		ImGui::Checkbox("Use fill route", &vars.waypoints.use_fill_route);
		ImGui::Checkbox("Use chat detect", &vars.chat_detect.enable);
		ImGui::Checkbox("Use sound detect", &vars.alarm.enable);
		ImGui::Checkbox("Use no collision", &vars.collisions.enable);
		ImGui::Checkbox("Use no collision when recording", &vars.collisions.use_when_recording);
		ImGui::Checkbox("Use global keywords", &vars.misc.use_global_keywords_and_colors);
		ImGui::Checkbox("Use captcha solver", &vars.captcha.enable);
		ImGui::Checkbox("Use discord alerts", &vars.misc.use_discord_alerts);
		});

	ImGui::draw_content_box("Record settings", [this]() {
		ImVec2 half_size_button = { (ImGui::GetContentRegionAvail().x - 8.f) / 2.f, 20.f };

		if (ImGui::Button("ADD WAYPOINT", half_size_button))
			record_waypoint();

		ImGui::SameLine();
		if (ImGui::Button("CLEAR WAYPOINTS", half_size_button))
			vars.waypoints.waypoints_vector.clear();

		if (ImGui::Button("RECORD WAYPOINTS", half_size_button))
			global_utils::get().record(global_utils::record_type::WAYPOINTS);

		ImGui::SameLine();
		if (ImGui::Button("RECORD FILL WAYPOINTS", half_size_button))
			global_utils::get().record(global_utils::record_type::FILL_WAYPOINTS);

		ImGui::InputInt("Fill waypoint index", &vars.waypoints.fill_waypoint_index);
		if (vars.waypoints.fill_waypoint_index < 2)
			vars.waypoints.fill_waypoint_index = 2;

		ImGui::InputInt("Fill route sleep", &vars.waypoints.fill_route_sleep);
		ImGui::InputScalar("Fill routes count", ImGuiDataType_U8, &vars.waypoints.fill_routes_loops_count);

		ImGui::InputScalar("Show count", ImGuiDataType_U16, &vars.misc.show_count);
	});
}