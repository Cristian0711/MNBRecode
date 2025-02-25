class samp_localplayer_t {
public:
	[[nodiscard]] inline auto get_samp_ped() noexcept -> void* {
		return *reinterpret_cast<void**>(this);
	}

	[[nodiscard]] inline auto get_anim_id() noexcept -> uint16_t {
		return *reinterpret_cast<uint16_t*>(this + g_samp_offsets->local_player_anim_id_offset());
	}

	[[nodiscard]] inline auto get_vehicle_id() noexcept -> uint16_t {
		return *reinterpret_cast<uint16_t*>(this + g_samp_offsets->player_local_vehicle_id_offset());
	}

	[[nodiscard]] inline auto get_on_foot_data() noexcept -> on_foot_data_t* {
		return reinterpret_cast<on_foot_data_t*>(this + g_samp_offsets->on_foot_data_offset());
	}

	[[nodiscard]] inline auto get_in_car_data() noexcept -> in_car_data_t* {
		return reinterpret_cast<in_car_data_t*>(this + g_samp_offsets->in_car_data_offset());
	}

	[[nodiscard]] inline auto get_trailer_data() noexcept -> trailer_data_t* {
		return reinterpret_cast<trailer_data_t*>(this + g_samp_offsets->trailer_data_offset());
	}

	inline auto enter_vehicle(uint16_t id) noexcept -> void {
		((void(__thiscall*)(samp_localplayer_t*, uint16_t, uint32_t))(g_samp_module + g_samp_offsets->enter_vehicle()))(this, id, 0);
	}

	inline auto exit_vehicle(uint16_t id) noexcept -> void {
		((void(__thiscall*)(samp_localplayer_t*, uint16_t))(g_samp_module + g_samp_offsets->exit_vehicle()))(this, id);
	}

	auto send(std::string_view message) noexcept -> void {
		static void* g_input = *reinterpret_cast<void**>(g_samp_module + g_samp_offsets->input());
		if (message[0] == '/')
			((void(__thiscall*) (void*, const char*))(g_samp_module + g_samp_offsets->func_send_command()))(g_input, message.data());
		else ((void(__thiscall*) (void*, const char*))(g_samp_module + g_samp_offsets->func_send_message()))(this, message.data());
	}
};