namespace waypoints_config
{
	static auto save_fill_waypoints(std::ostream& file) noexcept -> void
	{
		const auto& waypoints_vector = vars.waypoints.fill_waypoints_vector;

		if (waypoints_vector.empty() == true)
			return;

		const auto& configs = configs::get();
		configs.write_data_binary(file, configs::save_byte_t::FILL_WAYPOINTS);
		configs.write_data_binary(file, waypoints_vector.size());

		for (const auto& waypoint : waypoints_vector)
			configs.write_data_binary(file, waypoint);
	}

	static auto load_fill_waypoints(std::istream& file) noexcept -> void
	{
		const auto& configs = configs::get();
		auto waypoints_vector_size = configs.read_data_binary<size_t>(file);

		for (size_t index = 0; index != waypoints_vector_size; ++index)
			vars.waypoints.fill_waypoints_vector.emplace_back(configs.read_data_binary<waypoint_t>(file));
	}

	static auto save_waypoints(std::ostream& file) noexcept -> void
	{
		const auto& waypoints_vector = vars.waypoints.waypoints_vector;

		const auto& configs = configs::get();
		configs.write_data_binary(file, configs::save_byte_t::WAYPOINTS);
		configs.write_data_binary(file, waypoints_vector.size());

		for (const auto& waypoint : waypoints_vector)
			configs.write_data_binary(file, waypoint);
	}

	static auto load_waypoints(std::istream& file) noexcept -> void
	{
		const auto& configs = configs::get();
		auto waypoints_vector_size = configs.read_data_binary<size_t>(file);

		for (size_t index = 0; index != waypoints_vector_size; ++index)
			vars.waypoints.waypoints_vector.emplace_back(configs.read_data_binary<waypoint_t>(file));
	}
}