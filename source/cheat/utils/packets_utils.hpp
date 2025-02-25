class packets_utils {
private:
	packets_utils() {};
public:
	enum class SearchDirection {
		REVERSE,
		FORWARD,
		BOTH
	};

public:
	[[nodiscard]] static auto get() -> packets_utils& {
		static packets_utils instance;
		return instance;
	}

	[[nodiscard]] auto get_packet_position(const packet_sync_data_t& sync_data) const noexcept 
		-> std::optional<std::reference_wrapper<const CVector>>
	{
		if (sync_data.packet.id == ID_PLAYER_SYNC) {
			return std::cref(sync_data.packet.on_foot_data.position);
		}
		else if (sync_data.packet.id == ID_VEHICLE_SYNC) {
			return std::cref(sync_data.packet.in_car_data.position);
		}
		else {
			return std::nullopt;
		}
	}

	inline auto reset() noexcept -> void
	{
		m_current_vehicle_id = 0;
		m_enter_car_active = false;
		m_exit_car_active = false;
		m_first_start = true;
		m_wait_to_put_in_vehicle = false;
		m_change_interior_active = false;
	}

	[[nodiscard]] inline auto current_vehicle_id() const noexcept -> uint16_t {
		return m_current_vehicle_id;
	}


	inline auto set_current_vehicle_id(uint16_t id) noexcept -> void {
		m_current_vehicle_id = id;
	}

	[[nodiscard]] inline auto is_enter_car_active() const noexcept -> bool {
		return m_enter_car_active;
	}

	inline auto set_wait_to_put_in_vehicle(bool active) noexcept -> void {
		m_wait_to_put_in_vehicle = active;
	}

	inline auto set_first_start_false() noexcept -> void {
		m_first_start = false;
	}

	[[nodiscard]] inline auto is_wait_to_put_in_vehicle_active() const noexcept -> bool {
		return m_wait_to_put_in_vehicle;
	}

	[[nodiscard]] inline auto is_exit_car_active() const noexcept -> bool {
		return m_exit_car_active;
	}

	[[nodiscard]] inline auto is_change_interior_active() const noexcept -> bool {
		return m_change_interior_active;
	}

	auto enter_closest_vehicle() noexcept -> void
	{
		if (m_wait_to_put_in_vehicle == true || m_enter_car_active == true)
			return;

		auto& utils = utils::get();

		uint16_t closest_vehicle_id = 0;
		if (m_current_vehicle_id != 0 && utils.does_samp_vehicle_exist(m_current_vehicle_id)) {
			closest_vehicle_id = m_current_vehicle_id;
		}
		else {
			closest_vehicle_id = utils.get_closest_vehicle_id();
		}

		if (closest_vehicle_id == 0)
		{
			modules::samp().chat()->add_info_message("2");
			packets::get().disable();
			return;
		}

		if (vars.local_vehicle != nullptr)
		{
			m_current_vehicle_id = modules::samp().local_player()->get_vehicle_id();
			return;
		}

		m_enter_car_active = true;
		std::thread(&packets_utils::enter_vehicle_thread, this, closest_vehicle_id).detach();
	}

	auto exit_current_vehicle() noexcept -> void
	{
		if (vars.local_vehicle == nullptr) {
			return;
		}

		m_exit_car_active = true;
		std::thread(&packets_utils::exit_vehicle_thread, this, m_current_vehicle_id).detach();
	}

	auto sync_data_with_local(in_car_data_t& in_car_data) const noexcept -> void {
		vars.local_vehicle->m_vecMoveSpeed = in_car_data.move_speed;

		float sqw = in_car_data.quaternion[0] * in_car_data.quaternion[0];
		float sqx = in_car_data.quaternion[1] * in_car_data.quaternion[1];
		float sqy = in_car_data.quaternion[2] * in_car_data.quaternion[2];
		float sqz = in_car_data.quaternion[3] * in_car_data.quaternion[3];

		float invs = 1.0f / (sqx + sqy + sqz + sqw);

		auto matrix = vars.local_vehicle->m_matrix;
		matrix->right.x = (sqx - sqy - sqz + sqw) * invs;
		matrix->up.y = (-sqx + sqy - sqz + sqw) * invs;
		matrix->at.z = (-sqx - sqy + sqz + sqw) * invs;

		float tmp1 = in_car_data.quaternion[1] * in_car_data.quaternion[2];
		float tmp2 = in_car_data.quaternion[3] * in_car_data.quaternion[0];
		matrix->up.x = 2.0f * (tmp1 + tmp2) * invs;
		matrix->right.y = 2.0f * (tmp1 - tmp2) * invs;

		tmp1 = in_car_data.quaternion[1] * in_car_data.quaternion[3];
		tmp2 = in_car_data.quaternion[2] * in_car_data.quaternion[0];
		matrix->at.x = 2.0f * (tmp1 - tmp2) * invs;
		matrix->right.z = 2.0f * (tmp1 + tmp2) * invs;

		tmp1 = in_car_data.quaternion[2] * in_car_data.quaternion[3];
		tmp2 = in_car_data.quaternion[1] * in_car_data.quaternion[0];

		matrix->at.y = 2.0f * (tmp1 + tmp2) * invs;
		matrix->up.z = 2.0f * (tmp1 - tmp2) * invs;
	}

	auto search_for_packet(uint8_t type, uint8_t id, size_t active_packet, SearchDirection direction, size_t max_steps = 30u, bool check_active_packet = true) const noexcept
		-> std::optional<std::pair<std::reference_wrapper<const packet_sync_data_t>, size_t>>
	{
		size_t size = vars.packets.packets_vector.size();

		if (size == 0) {
			return std::nullopt;
		}

		// Check if the active packet itself matches the criteria
		if (check_active_packet == true) {
			const packet_sync_data_t& active_sync_data = vars.packets.packets_vector[active_packet];
			if ((active_sync_data.type == packets::PACKET && active_sync_data.packet.id == id) ||
				(active_sync_data.type == packets::RECEIVED_RPC && active_sync_data.rpc.id == id)) {
				return std::make_pair(std::cref(active_sync_data), active_packet);
			}
		}

		// Forward and reverse index variables
		size_t forward_index = (active_packet == size - 1 ? 0 : active_packet + 1); // Start right after the active packet
		size_t reverse_index = (active_packet == 0 ? size - 1 : active_packet - 1); // Start right before the active packet

		size_t steps_taken = 0; // Tracks how many packets have been checked

		while (steps_taken < max_steps && steps_taken < size) {
			// Depending on the search direction, perform the appropriate search
			if (direction == SearchDirection::FORWARD || direction == SearchDirection::BOTH) {
				const packet_sync_data_t& sync_data = vars.packets.packets_vector[forward_index];

				if ((sync_data.type == packets::PACKET && sync_data.packet.id == id) ||
					(sync_data.type == packets::RECEIVED_RPC && sync_data.rpc.id == id)) {
					return std::make_pair(std::cref(sync_data), forward_index);
				}

				// Move forward index
				forward_index = (forward_index == size - 1 ? 0 : forward_index + 1);
			}

			if (direction == SearchDirection::REVERSE || direction == SearchDirection::BOTH) {
				const packet_sync_data_t& sync_data = vars.packets.packets_vector[reverse_index];

				if ((sync_data.type == packets::PACKET && sync_data.packet.id == id) ||
					(sync_data.type == packets::RECEIVED_RPC && sync_data.rpc.id == id)) {
					return std::make_pair(std::cref(sync_data), reverse_index);
				}

				// Move reverse index
				reverse_index = (reverse_index == 0 ? size - 1 : reverse_index - 1);
			}

			steps_taken++;
		}

		return std::nullopt;
	}

	auto search_for_closest_packet(uint8_t id) const noexcept -> size_t
	{
		size_t return_index = 0;
		float  min_distance = std::numeric_limits<float>::max();

		for (size_t index = 0; index != vars.packets.packets_vector.size(); ++index) {
			const auto& sync_data = vars.packets.packets_vector[index];

			if (sync_data.type != packets::PACKET)
				continue;

			if (sync_data.packet.id != id)
				continue;

			const auto current_distance = utils::get().get_distance_to_local_ped(get_packet_position(sync_data).value());
			if (current_distance < min_distance) {
				min_distance = current_distance;
				return_index = index;
			}
		}

		return return_index;
	}

	auto search_for_next_packet_of_type(uint8_t type, uint8_t id, size_t active_packet) const noexcept -> size_t {
		return search_for_packet(type, id, active_packet, SearchDirection::FORWARD, vars.packets.packets_vector.size()).value().second;

	}

	auto is_teleport_permited(const packet_sync_data_t& first_packet, size_t active_packet, const float distance) noexcept -> bool
	{
		if (m_first_start == true) {
			m_first_start = false;
			return true;
		}

		const auto& second_packet = search_for_packet(first_packet.type, first_packet.packet.id, active_packet, SearchDirection::REVERSE, 30u, false);

		if (second_packet == std::nullopt)
			return false;

		const auto first_packet_position = get_packet_position(first_packet);
		const auto second_packet_position = get_packet_position(second_packet.value().first);

		if (first_packet_position == std::nullopt || second_packet_position == std::nullopt)
			return false;

		auto absolute = std::fabsf(utils::get().get_distance_between_two_points(first_packet_position.value(), second_packet_position.value()) - distance);
		if (vars.local_vehicle == nullptr) {
			return absolute < 0.5f;
		}
		else {
			return absolute < vars.local_vehicle->m_vecMoveSpeed.Magnitude() * 3.f;
		}
	}

	auto is_admin_rpc_permited(uint8_t type, uint8_t id, size_t active_packet) noexcept -> bool
	{
		const auto& searched_packet = search_for_packet(type, id, active_packet, SearchDirection::BOTH);

		if (searched_packet != std::nullopt)
			return true;

		if (does_packet_exist_within_distance(type, id, active_packet) == true)
			return true;

		return false;
	}

	auto set_change_interior_active(size_t m_active_packet, bool enable) noexcept -> void
	{
		m_change_interior_tick = GetTickCount64();
		m_change_interior_active = enable;
		m_change_interior_packet_index = m_active_packet;
	}

	auto handle_change_interior() noexcept -> void
	{
		if (GetTickCount64() > m_change_interior_tick + 1000) {
			on_foot_data_t on_foot_data = *modules::samp().local_player()->get_on_foot_data();
			on_foot_data.m_samp_keys.keys_enterExitCar = 1;

			BitStream data;
			data.Write<uint8_t>(ID_PLAYER_SYNC);
			data.Write(reinterpret_cast<const char*>(&on_foot_data), sizeof(on_foot_data_t));

			rakclient::get().send_packet(&data);

			m_change_interior_tick = GetTickCount64();
		}
	}

private:
	uint16_t m_current_vehicle_id{ 0u };
	bool m_enter_car_active{ false };
	bool m_exit_car_active{ false };
	bool m_first_start{ true }; // used for detecting if teleport was from first packet
	bool m_wait_to_put_in_vehicle{ false };
	bool m_change_interior_active{ false };
	size_t m_change_interior_packet_index{ 0u };
	uint64_t m_change_interior_tick{ 0u };

	auto enter_vehicle_thread(uint16_t id) noexcept -> void
	{
		auto& utils = utils::get();
		utils.sync_vehicle_enter(id);
		if (utils.enter_vehicle(id) == false) {
			packets::get().disable();
			return;
		}
		std::this_thread::sleep_for(200ms);

		auto current_tick = GetTickCount64();
		auto current_enter_tick = GetTickCount64();
		while (vars.local_vehicle == nullptr && vars.packets.enable)
		{
			if (GetTickCount64() > current_tick + 10000) {
				packets::get().disable();
				return;
			}
			else if (GetTickCount64() > current_enter_tick + 3000) {
				if (utils.enter_vehicle(id) == false)
				{
					packets::get().disable();
					return;
				}

				current_enter_tick = GetTickCount64();
			}
			std::this_thread::sleep_for(10ms);
		}
		std::this_thread::sleep_for(200ms);

		m_current_vehicle_id = modules::samp().local_player()->get_vehicle_id();
		m_enter_car_active = false;
	}

	auto exit_vehicle_thread(uint16_t id) noexcept -> void
	{
		auto& utils = utils::get();
		utils.sync_vehicle_exit(id);
		utils.exit_vehicle(id);

		auto current_tick = GetTickCount64();
		auto current_exit_tick = GetTickCount64();
		while (vars.local_vehicle != nullptr && vars.packets.enable)
		{
			if (GetTickCount64() > current_tick + 10000) {
				packets::get().disable();
				return;
			}
			else if (GetTickCount64() > current_exit_tick + 3000) {
				utils.exit_vehicle(id);
				current_exit_tick = GetTickCount64();
			}
			std::this_thread::sleep_for(10ms);
		}
		std::this_thread::sleep_for(200ms);
		m_exit_car_active = false;
	}

	auto does_packet_exist_within_distance(uint8_t type, uint8_t id, size_t active_packet, float max_distance = 5.0f) const noexcept -> bool
	{
		size_t size = vars.packets.packets_vector.size();

		if (size == 0) {
			return false;
		}

		size_t forward_index = active_packet + 1;
		size_t reverse_index = active_packet;

		bool forward_done = false;
		bool reverse_done = false;

		while (!forward_done || !reverse_done) {
			// Forward search
			if (!forward_done) {
				const auto& packet = vars.packets.packets_vector[forward_index];

				if (packet.type == type && packet.rpc.id == id) {
					if (utils::get().get_distance_to_local_ped(packet.rpc.position) < max_distance) {
						return true;
					}
				}

				// Move forward and wrap around if needed
				if (++forward_index >= size) {
					forward_done = true;
				}
			}

			// Reverse search
			if (!reverse_done && reverse_index > 0) {
				--reverse_index;
				const auto& packet = vars.packets.packets_vector[reverse_index];

				if (packet.type == type && packet.rpc.id == id && utils::get().get_distance_to_local_ped(packet.rpc.position) < max_distance) {
					return true;
				}
			}
			else {
				reverse_done = true;
			}
		}

		return false;
	}
};