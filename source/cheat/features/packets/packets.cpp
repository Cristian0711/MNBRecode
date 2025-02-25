#include "core.h"

auto packets::enable() noexcept -> void
{
	if (vars.packets.packets_vector.size() < 2)
		return;

	const auto first_packet_id = vars.packets.packets_vector[m_active_packet].packet.id;
	if (vars.local_vehicle != nullptr && first_packet_id == ID_PLAYER_SYNC || 
		vars.local_vehicle == nullptr && first_packet_id == ID_VEHICLE_SYNC)
		return;

	packets_utils::get().reset();
	if (vars.local_vehicle) {
		packets_utils::get().set_current_vehicle_id(modules::samp().local_player()->get_vehicle_id());
	}
	menu::get().set_enable_button_text(true);

	m_active_packet = 0;
	vars.packets.enable = true;

	if (vars.packets.start_from_closest_packet == false)
		return;

	if (vars.local_vehicle != nullptr) {
		m_active_packet = packets_utils::get().search_for_closest_packet(ID_VEHICLE_SYNC);
	} 
	else {
		m_active_packet = packets_utils::get().search_for_closest_packet(ID_PLAYER_SYNC);
	}
}

auto packets::disable() noexcept -> void
{
	packets_utils::get().reset();

	m_active_packet = 0;
	vars.packets.enable = false;

	menu::get().set_enable_button_text(false);
}

auto packets::record_sent_rpc(uint8_t id, BitStream* data) const noexcept -> void
{
	//modules::samp().chat()->add_info_message("SENT ID: {}", id);
	if (vars.packets.record_packets == false)
		return;

	packet_sync_data_t sync_data;
	switch (id)
	{
	case RPC_Chat:
		data->Read(sync_data.rpc.send_message.text_length);
		data->Read(sync_data.rpc.send_message.text, sync_data.rpc.send_message.text_length);
		sync_data.rpc.send_message.text[sync_data.rpc.send_message.text_length] = '\0';
		break;

	case RPC_ServerCommand:
		data->Read(sync_data.rpc.send_command.text_length);
		data->Read(sync_data.rpc.send_command.text, sync_data.rpc.send_command.text_length);
		sync_data.rpc.send_command.text[sync_data.rpc.send_command.text_length] = '\0';
		break;

	case RPC_DialogResponse:
		data->Read(sync_data.rpc.send_dialog_response.id);
		data->Read(sync_data.rpc.send_dialog_response.response);
		data->Read(sync_data.rpc.send_dialog_response.list_item);
		data->Read(sync_data.rpc.send_dialog_response.text_length);
		data->Read(sync_data.rpc.send_dialog_response.text, sync_data.rpc.send_dialog_response.text_length);
		sync_data.rpc.send_dialog_response.text[sync_data.rpc.send_dialog_response.text_length] = '\0';
		break;

	case RPC_ClickTextDraw:
		data->Read(sync_data.rpc.send_text_draw.id);
		break;

	default:
		return;
	}

	sync_data.type = SENT_RPC;
	sync_data.rpc.id = id;
	vars.packets.packets_vector.emplace_back(std::move(sync_data));
}

auto packets::record_received_rpc(uint8_t id, BitStream& data) const noexcept -> void
{
	packet_sync_data_t sync_data;
	sync_data.type = RECEIVED_RPC;
	switch (id)
	{
	case RPC_TogglePlayerControllable:
		uint8_t moveable;
		data.Read(moveable);

		if (moveable != 0)
			return;

		sync_data.rpc.id = RPC_TogglePlayerControllable;
		sync_data.rpc.position = utils::get().get_local_ped_position();
		break;
	case RPC_SetPlayerInterior:
		sync_data.rpc.id = RPC_SetPlayerInterior;
		break;
	case RPC_WorldVehicleRemove:
		uint16_t vehicle_id;
		data.Read(vehicle_id);

		if (vehicle_id < 0 || vehicle_id >= 2000)
			return;

		if (vehicle_id != modules::samp().local_player()->get_vehicle_id())
			return;

		sync_data.rpc.id = RPC_WorldVehicleRemove;
		break;
	case RPC_PutPlayerInVehicle:
		sync_data.rpc.id = RPC_PutPlayerInVehicle;
		break;
	default:
		return;
	}

	vars.packets.packets_vector.emplace_back(std::move(sync_data));
}

auto packets::handle_received_rpc(uint8_t id, RPCParameters* rpc_params) noexcept -> void
{
	//modules::samp().chat()->add_info_message("RECV ID: {}", id);

	if (vars.packets.enable == false && vars.packets.record_packets == false)
		return;

	BitStream data(reinterpret_cast<unsigned char*>(rpc_params->input), ((int)rpc_params->numberOfBitsOfData / 8) + 1, false);

	if (vars.packets.record_packets == true) {
		record_received_rpc(id, data);
		return;
	}

	CVector rpc_position;
	switch (id)
	{
		uint16_t vehicle_id;
		uint8_t moveable;
		float z_delta;

	case RPC_TogglePlayerControllable:
		data.Read(moveable);

		if (moveable != 0)
			return;

		if (packets_utils::get().is_admin_rpc_permited(RECEIVED_RPC, id, m_active_packet) == false) {
			global_utils::get().send_alert_to_user("Cheat Info: You have been frozen!");
		}
		break;
	case RPC_WorldVehicleRemove:
		if (vars.local_vehicle == nullptr)
			return;

		data.Read(vehicle_id);

		if (vehicle_id < 0 || vehicle_id >= 2000)
			return;

		if (vehicle_id != modules::samp().local_player()->get_vehicle_id())
			return;

		if (packets_utils::get().is_admin_rpc_permited(RECEIVED_RPC, id, m_active_packet) == false) {
			global_utils::get().send_alert_to_user("Cheat Info: Your car has been removed by an admin!");
		}
		else {
			m_active_packet = packets_utils::get().search_for_next_packet_of_type(PACKET, ID_PLAYER_SYNC, m_active_packet);
		}
		break;
	case RPC_PutPlayerInVehicle:
		data.Read(vehicle_id);

		packets_utils::get().set_current_vehicle_id(vehicle_id);
		packets_utils::get().set_wait_to_put_in_vehicle(false);
		m_active_packet = packets_utils::get().search_for_next_packet_of_type(PACKET, ID_VEHICLE_SYNC, m_active_packet);

		break;
	case RPC_SetPlayerInterior:
		if (packets_utils::get().is_change_interior_active()) {
			packets_utils::get().set_change_interior_active(m_active_packet, false);
		}

		break;
	case RPC_SetPlayerPos:
		data.Read(rpc_position);

		z_delta = rpc_position.z - utils::get().get_local_ped_position().z;
		if (utils::get().get_distance_to_local_ped_2d(rpc_position) > 4.f || z_delta < 2.f)
			return;

		global_utils::get().send_alert_to_user("Cheat Info: You have been slapped {:.2f}m upwards!", z_delta);
		break;
	case RPC_SetVehiclePos:
		data.Read(vehicle_id);
		data.Read(rpc_position);

		if (vehicle_id != packets_utils::get().current_vehicle_id())
			return;

		if (vars.local_vehicle == nullptr)
			return;

		z_delta = rpc_position.z - utils::get().get_local_ped_position().z;
		if (utils::get().get_distance_to_local_ped_2d(rpc_position) > 4.f || z_delta < 2.f)
			return;

		global_utils::get().send_alert_to_user("Cheat Info: You car has been slapped {:.2f}m upwards!", z_delta);
		break;
	default:
		return;
	}
}

auto packets::handle_recorded_received_rpc() noexcept -> void
{
	if (vars.packets.enable == false)
		return;

	if (vars.packets.packets_vector[m_active_packet].type != RECEIVED_RPC)
		return;

	if (vars.packets.packets_vector[m_active_packet].rpc.id == RPC_PutPlayerInVehicle)
		packets_utils::get().set_wait_to_put_in_vehicle(true);

	++m_active_packet;
}

auto packets::send_recorded_rpc() noexcept -> void
{
	if (vars.packets.enable == false)
		return;

	const packet_sync_data_t& sync_data = vars.packets.packets_vector[m_active_packet];

	if (sync_data.type != SENT_RPC)
		return;

	BitStream data;
	switch (sync_data.rpc.id)
	{
	case RPC_Chat:
		data.Write(reinterpret_cast<const char*>(&sync_data.rpc.send_message), sizeof(send_message_t));
		break;
	case RPC_ServerCommand:
		data.Write(reinterpret_cast<const char*>(&sync_data.rpc.send_command), sizeof(send_command_t));
		break;
	case RPC_DialogResponse:
		data.Write(sync_data.rpc.send_dialog_response.id);
		data.Write(sync_data.rpc.send_dialog_response.response);
		data.Write(sync_data.rpc.send_dialog_response.list_item);
		data.Write(sync_data.rpc.send_dialog_response.text_length);
		data.Write(sync_data.rpc.send_dialog_response.text, sync_data.rpc.send_dialog_response.text_length);

		modules::samp().dialog()->hide();
		modules::samp().info()->toggle_cursor(false);
		break;
	case RPC_ClickTextDraw:
		data.Write(reinterpret_cast<const char*>(&sync_data.rpc.send_text_draw), sizeof(send_textdraw_t));
		break;
	}

	rakclient::get().send_rpc(sync_data.rpc.id, &data);

	++m_active_packet;
}

auto packets::record_on_foot_packet() const noexcept -> void
{
	auto on_foot_data = *modules::samp().local_player()->get_on_foot_data();
	
	// verify if the player is entering car, if so do not record
	if (on_foot_data.current_animation_id == 1043 || on_foot_data.current_animation_id == 1024 || 
		on_foot_data.current_animation_id == 1011 || on_foot_data.current_animation_id == 1009)
		return;

	if (utils::get().get_distance_to_local_ped(on_foot_data.position) > 3.f)
		return;

	packet_sync_data_t on_foot_sync;
	on_foot_sync.type = PACKET;
	on_foot_sync.packet.id = ID_PLAYER_SYNC;
	on_foot_sync.packet.on_foot_data = std::move(on_foot_data);

	vars.packets.packets_vector.emplace_back(std::move(on_foot_sync));
}

auto packets::send_on_foot_packet(on_foot_data_t& on_foot_data) const noexcept -> void
{
	on_foot_data.health = static_cast<uint8_t>(vars.local_ped->m_fHealth);
	on_foot_data.armuor = static_cast<uint8_t>(vars.local_ped->m_fArmour);

	BitStream data;
	data.Write<uint8_t>(ID_PLAYER_SYNC);
	data.Write(reinterpret_cast<const char*>(&on_foot_data), sizeof(on_foot_data_t));

	rakclient::get().send_packet(&data);
}

auto packets::handle_on_foot_packet(void* _edx) noexcept -> bool
{
	if (vars.packets.enable == false && vars.packets.record_packets == false)
		return true;

	if (_edx != PACKET_SEND_CHEAT)
		return false;

	if (vars.packets.record_packets == true)
	{
		record_on_foot_packet();
		return true;
	}

	const packet_sync_data_t& sync_data = vars.packets.packets_vector[m_active_packet];

	if (sync_data.type != PACKET)
		return false;

	if (packets_utils::get().is_exit_car_active() == true)
		return true;

	if (sync_data.packet.id == ID_VEHICLE_SYNC && packets_utils::get().is_enter_car_active() == false &&
		packets_utils::get().is_wait_to_put_in_vehicle_active() == false) {
		packets_utils::get().enter_closest_vehicle();
		return false;
	}

	if (sync_data.packet.id != ID_PLAYER_SYNC)
		return true;

	if (sync_data.packet.on_foot_data.m_samp_keys.keys_enterExitCar == 1) {
		if (packets_utils::get().search_for_packet(RECEIVED_RPC, RPC_SetPlayerInterior, m_active_packet, packets_utils::SearchDirection::BOTH, 15).has_value()) {
			packets_utils::get().set_change_interior_active(m_active_packet, true);

			if (utils::get().get_distance_to_local_ped(packets_utils::get().get_packet_position(sync_data).value()) > 5.f) {
				m_active_packet++;
				return true;
			}
		}
	}
	else {
		if (packets_utils::get().is_change_interior_active() == true) {
			packets_utils::get().handle_change_interior();
			return true;
		}
	}

	const CVector& packet_pos = packets_utils::get().get_packet_position(sync_data).value();
	const auto distance = utils::get().get_distance_to_local_ped(packet_pos);

	if (distance > 5.f && packets_utils::get().is_teleport_permited(sync_data, m_active_packet, distance) == false) {
		global_utils::get().send_alert_to_user("Cheat Info: You have been teleported {:.2f}m away!", distance);
		return true;
	}

	vars.local_ped->Teleport(packet_pos, true);

	on_foot_data_t on_foot_data = sync_data.packet.on_foot_data;

	if (packets_utils::get().is_change_interior_active() == true) {
		on_foot_data.m_samp_keys.keys_enterExitCar = 0;
	}

	send_on_foot_packet(on_foot_data);

	++m_active_packet;

	return false;
}

// TRUE -> send packet, FALSE -> handle packets ourselves
auto __fastcall packets::hook_send_on_foot_packet(void* _this, void* _edx) noexcept -> void
{
	if (packets::get().handle_on_foot_packet(_edx) == false)
		return;

	return packets::get().o_send_on_foot_packet(_this);
}


auto packets::send_in_car_packet(in_car_data_t& in_car_data) const noexcept -> void
{
	in_car_data.player_health = static_cast<uint8_t>(vars.local_ped->m_fHealth);
	in_car_data.player_armour = static_cast<uint8_t>(vars.local_ped->m_fArmour);

	in_car_data.vehicle_health = vars.local_vehicle->m_fHealth;
	in_car_data.vehicle_id = packets_utils::get().current_vehicle_id();

	BitStream data;
	data.Write<uint8_t>(ID_VEHICLE_SYNC);
	data.Write(reinterpret_cast<const char*>(&in_car_data), sizeof(in_car_data_t));

	rakclient::get().send_packet(&data);

	packets_utils::get().sync_data_with_local(in_car_data);
}

auto packets::record_in_car_packet() const noexcept -> void
{
	auto in_car_data = *modules::samp().local_player()->get_in_car_data();

	if (utils::get().get_distance_to_local_ped(in_car_data.position) > vars.local_vehicle->m_vecMoveSpeed.Magnitude() * 5.f)
		return;

	packet_sync_data_t in_car_sync;
	in_car_sync.type = PACKET;
	in_car_sync.packet.id = ID_VEHICLE_SYNC;
	in_car_sync.packet.model_id = vars.local_vehicle->m_nModelIndex;
	in_car_sync.packet.in_car_data = std::move(in_car_data);

	vars.packets.packets_vector.emplace_back(std::move(in_car_sync));
}

auto packets::handle_in_car_packet(void* _edx) noexcept -> bool
{
	if (vars.packets.enable == false && vars.packets.record_packets == false)
		return true;

	if (_edx != PACKET_SEND_CHEAT)
		return false;

	if (vars.packets.record_packets == true)
	{
		record_in_car_packet();
		return true;
	}

	const packet_sync_data_t& sync_data = vars.packets.packets_vector[m_active_packet];

	if (sync_data.type != PACKET)
		return false;

	if (packets_utils::get().is_enter_car_active() == true)
		return true;

	if (sync_data.packet.id == ID_PLAYER_SYNC && packets_utils::get().is_exit_car_active() == false) {
		packets_utils::get().exit_current_vehicle();
		return false;
	}

	if (sync_data.packet.id != ID_VEHICLE_SYNC)
		return true;

	const CVector& packet_pos = packets_utils::get().get_packet_position(sync_data).value();
	const auto distance = utils::get().get_distance_to_local_ped(packet_pos);

	if (distance > 2.f) {
		modules::samp().chat()->add_info_message("DIST: {}", distance);
	}

	if (distance > 5.f && packets_utils::get().is_teleport_permited(sync_data, m_active_packet, distance) == false) {
		global_utils::get().send_alert_to_user("Cheat Info: You car has been teleported {:.2f}m away!", distance);
		return true;
	}

	vars.local_vehicle->Teleport(packet_pos, true);

	auto in_car_data = sync_data.packet.in_car_data;
	send_in_car_packet(in_car_data);

	++m_active_packet;

	return false;
}

auto __fastcall packets::hook_send_in_car_packet(void* _this, void* _edx) noexcept -> void
{
	if (packets::get().handle_in_car_packet(_edx) == false)
		return;

	return packets::get().o_send_in_car_packet(_this);
}

auto __fastcall packets::hook_send_trailer_packet(void* _this, void* _edx, uint16_t unused) noexcept -> void
{

	return packets::get().o_send_trailer_packet(_this, unused);
}

auto packets::draw() const noexcept -> void 
{
	if (vars.misc.show_count == 0)
		return;

	if (vars.packets.packets_vector.size() <= 0)
		return;

	const auto& packets_vector = vars.packets.packets_vector;

	for (size_t index = 0; index < packets_vector.size(); index += vars.misc.show_count)
	{
		const auto& current_packet = packets_vector[index];

		if (current_packet.type != PACKET)
			continue;

		if (current_packet.packet.id != ID_PLAYER_SYNC && current_packet.packet.id != ID_VEHICLE_SYNC)
			continue;

		const auto position = packets_utils::get().get_packet_position(current_packet);

		if (position == std::nullopt)
			continue;

		CVector screen_position;
		const auto& render_utils = render_utils::get();
		if (render_utils.get_screen_pos(position.value(), screen_position) == false)
			continue;

		const auto render_name = std::vformat("Waypoint {} - {:.1f}", std::make_format_args(index + 1, utils::get().get_distance_to_local_ped(position.value())));

		if (index == 0)
		{
			render_utils.draw_string(render_name, { screen_position.x, screen_position.y - 55.f, 0 }, 16, vars.style.waypoint_text_color, true);
			render_utils.draw_string(current_packet.packet.id == ID_PLAYER_SYNC ? "OnFoot" : "InCar", { screen_position.x, screen_position.y - 40.f, 0 }, 16, vars.style.waypoint_text_color, true);

			if (current_packet.packet.id == ID_VEHICLE_SYNC) {
				render_utils.draw_string(utils::get().get_vehicle_model_name(current_packet.packet.model_id), { screen_position.x, screen_position.y - 40.f, 0 }, 16, vars.style.waypoint_text_color, true);
			}
		}
		else
		{
			if (current_packet.packet.id == ID_VEHICLE_SYNC) {
				render_utils.draw_string(render_name, { screen_position.x, screen_position.y - 40.f, 0 }, 16, vars.style.waypoint_text_color, true);
				render_utils.draw_string(utils::get().get_vehicle_model_name(current_packet.packet.model_id), {screen_position.x, screen_position.y - 25.f, 0}, 16, vars.style.waypoint_text_color, true);
			}
			else { 
				render_utils.draw_string(render_name, { screen_position.x, screen_position.y - 25.f, 0 }, 16, vars.style.waypoint_text_color, true);
			}
		}

		if (index == m_active_packet)
		{
			render_utils.draw_circle_filled(screen_position, 7, ImVec4(0.00f, 0.00f, 0.00f, 1.00f));
			render_utils.draw_circle_filled(screen_position, 5, vars.style.waypoint_dot_color);
		}
		else
		{
			render_utils.draw_circle_filled(screen_position, 7, ImColor(ImVec4(0.00f, 0.00f, 0.00f, 1.00f)));
			render_utils.draw_circle_filled(screen_position, 5, ImColor(ImVec4(1.00f, 0.00f, 0.00f, 1.00f)));
		}
	}
}

auto packets::tick() noexcept -> void
{
	if (!vars.packets.enable && !vars.packets.record_packets)
		return;

	static auto current_tick = GetTickCount64() - 50;

	if (GetTickCount64() < current_tick + 40)
		return;

	if (vars.local_ped->IsAlive() == false) {
		global_utils::get().send_alert_to_user("Cheat Info: You died!");
		return;
	}

	if (vars.packets.enable && m_active_packet == vars.packets.packets_vector.size()) {
		conditional_routes::get().increment_route_loops_and_check();
		m_active_packet = 0u;
		return;
	}

	handle_recorded_received_rpc();
	send_recorded_rpc();

	if (vars.local_vehicle == nullptr) {
		hook_send_on_foot_packet(modules::samp().local_player(), PACKET_SEND_CHEAT);
	}
	else {
		hook_send_in_car_packet(modules::samp().local_player(), PACKET_SEND_CHEAT);
		hook_send_trailer_packet(modules::samp().local_player(), PACKET_SEND_CHEAT, 0);
	}

	packets_utils::get().set_first_start_false();
	current_tick = GetTickCount64();
}

auto packets::do_settings_menu() noexcept -> void
{
	ImGui::draw_content_box("Packets settings", [this]() {
		ImVec2 half_size_button = { (ImGui::GetContentRegionAvail().x - 8.f) / 2.f, 20.f };

		if (ImGui::Button("RECORD PACKETS", half_size_button))
			global_utils::get().record(global_utils::record_type::PACKETS);

		ImGui::SameLine();
		if (ImGui::Button("CLEAR PACKETS", half_size_button))
		{
			vars.packets.packets_vector.clear();
		}

		ImGui::Checkbox("Start from closest packet", &vars.packets.start_from_closest_packet);
	});
}