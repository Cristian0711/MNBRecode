class waypoints;

class global_utils {
private:
	global_utils() {};
public:
	enum class record_type : uint8_t
	{
		WAYPOINTS = 0,
		FILL_WAYPOINTS,
		PACKETS,
		SMART_WAYPOINTS
	};

	enum class route_type : uint8_t
	{
		WAYPOINTS = 0,
		PACKETS,
		SMART_WAYPOINTS,
		INVALID
	};

	[[nodiscard]] static auto get() noexcept -> global_utils& {
		static global_utils instance;
		return instance;
	}

	[[nodiscard]] inline auto any_route_enabled() const noexcept -> bool {
		return vars.waypoints.enable || vars.packets.enable || vars.smart_waypoints.enable;
	}

	[[nodiscard]] inline auto any_record_enabled() const noexcept -> bool {
		return vars.waypoints.record_waypoints || vars.waypoints.record_fill_waypoints
			|| vars.packets.record_packets || vars.smart_waypoints.record_waypoints;
	}

	[[nodiscard]] inline auto get_active_route_type() const noexcept -> route_type
	{
		bool waypoints_enabled = !vars.waypoints.waypoints_vector.empty();
		bool packets_enabled = !vars.packets.packets_vector.empty();
		bool smart_waypoints_enabled = !vars.smart_waypoints.waypoints_vector.empty();

		int count_enabled = waypoints_enabled + packets_enabled + smart_waypoints_enabled;

		// although impossible you never know :)
		if (count_enabled > 1) {
			return route_type::INVALID;
		}

		if (waypoints_enabled) {
			return route_type::WAYPOINTS;
		}
		else if (packets_enabled) {
			return route_type::PACKETS;
		}
		else if (smart_waypoints_enabled) {
			return route_type::SMART_WAYPOINTS;
		}

		return route_type::INVALID;
	}

	inline auto are_other_waypoints_except_this(record_type type) const noexcept -> bool
	{
		bool waypoints_enabled = !vars.waypoints.waypoints_vector.empty();
		bool fill_waypoints_enabled = !vars.waypoints.fill_waypoints_vector.empty();
		bool packets_enabled = !vars.packets.packets_vector.empty();
		bool smart_waypoints_enabled = !vars.smart_waypoints.waypoints_vector.empty();

		switch (type)
		{
		case record_type::WAYPOINTS:
			return packets_enabled || smart_waypoints_enabled;
		case record_type::PACKETS:
			return waypoints_enabled || smart_waypoints_enabled || fill_waypoints_enabled;
		case record_type::SMART_WAYPOINTS:
			return waypoints_enabled || packets_enabled || fill_waypoints_enabled;
		default:
			return false;
		}
	}

	inline auto record(record_type type) const noexcept -> void
	{
		if (any_record_enabled() == true)
		{
			vars.waypoints.record_waypoints = false;
			vars.waypoints.record_fill_waypoints = false;
			vars.packets.record_packets = false;
			vars.smart_waypoints.record_waypoints = false;

			return;
		}

		if (any_route_enabled() == true) {
			disable();
		}

		if (are_other_waypoints_except_this(type) == true)
			clear_all_waypoints();

		switch (type)
		{
		case record_type::WAYPOINTS:
			vars.waypoints.record_waypoints = true;
			break;

		case record_type::FILL_WAYPOINTS:
			vars.waypoints.record_fill_waypoints = true;
			break;

		case record_type::PACKETS:
			vars.packets.record_packets = true;
			break;

		case record_type::SMART_WAYPOINTS:
			vars.smart_waypoints.record_waypoints = true;
			break;

		default:
			return;
		}
	}

	inline auto stop_all_record_types() const noexcept -> void {
		vars.waypoints.record_waypoints = false;
		vars.waypoints.record_fill_waypoints = false;
		vars.packets.record_packets = false;
		vars.smart_waypoints.record_waypoints = false;
	}

	inline auto enable() const noexcept -> void
	{
		bool waypoints_enabled = !vars.waypoints.waypoints_vector.empty();
		bool packets_enabled = !vars.packets.packets_vector.empty();
		bool smart_waypoints_enabled = !vars.smart_waypoints.waypoints_vector.empty();

		int count_enabled = waypoints_enabled + packets_enabled + smart_waypoints_enabled;

		// although impossible you never know :)
		if (count_enabled > 1) {
			vars.waypoints.waypoints_vector.clear();
			vars.packets.packets_vector.clear();
			vars.smart_waypoints.waypoints_vector.clear();
			return;
		}

		vars.waypoints.enable = false;
		vars.packets.enable = false;
		vars.smart_waypoints.enable = false;

		utils::get().unlock_ped();

		if (vars.waypoints.fill_waypoints_vector.empty() == false
			&& (packets_enabled || smart_waypoints_enabled))
		{
			vars.waypoints.fill_waypoints_vector.clear();
			vars.waypoints.use_fill_route = false;
		}

		if (waypoints_enabled) {
			waypoints::get().enable();
		}
		else if (packets_enabled) {
			packets::get().enable();
		}
		else if (smart_waypoints_enabled) {
			//smart_waypoints::get().enable();
		}

		modules::samp().chat()->add_info_message("PACKETS: {}", vars.packets.enable);
	}

	inline auto disable() const noexcept -> void
	{
		bool waypoints_enabled = !vars.waypoints.waypoints_vector.empty();
		bool packets_enabled = !vars.packets.packets_vector.empty();
		bool smart_waypoints_enabled = !vars.smart_waypoints.waypoints_vector.empty();

		int count_enabled = waypoints_enabled + packets_enabled + smart_waypoints_enabled;

		// although impossible you never know :)
		if (count_enabled > 1) {
			waypoints::get().disable();
			packets::get().disable();
			//smart_waypoints::get().disable();
			return;
		}

		if (waypoints_enabled) {
			waypoints::get().disable();
		}

		if (packets_enabled) {
			packets::get().disable();
		}

		if (smart_waypoints_enabled) {
			//smart_waypoints::get().disable();
		}
	}

	inline auto clear_all_data() const noexcept -> void
	{
		vars.waypoints.waypoints_vector.clear();
		vars.waypoints.fill_waypoints_vector.clear();

		vars.packets.packets_vector.clear();
		vars.smart_waypoints.waypoints_vector.clear();

		if (vars.misc.use_global_keywords_and_colors == true)
			return;

		vars.chat_detect.chat_keywords.clear();
		vars.chat_detect.color_keywords.clear();
	}

	inline auto clear_all_waypoints() const noexcept -> void
	{
		vars.waypoints.waypoints_vector.clear();
		vars.waypoints.fill_waypoints_vector.clear();

		vars.packets.packets_vector.clear();
		vars.smart_waypoints.waypoints_vector.clear();
	}

	template <typename... Args>
	auto send_alert_to_user(std::string_view svMessage, Args&&... args) const noexcept -> void
	{
		if (vars.alarm.enable) {
			alarm::get().start_alarm();
		}

		const std::string message = std::vformat(svMessage, std::make_format_args(args...)).c_str();
		if (vars.misc.use_discord_alerts) {
			socket_commands::get().add_command(message, user_commands_t::ADMIN_DISCORD_ALERT);
		}

		chatlog::get().add_to_chat_log(samp_chat_t::CHAT_TYPE_MNB, message, "", samp_chat_t::chat_alert_color, 0);
		disable();
	}
};