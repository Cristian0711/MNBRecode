class configs {
private:
	configs() {};
public:
	enum class save_byte_t : uint8_t
	{
		WAYPOINTS = 0,
		FILL_WAYPOINTS,
		PACKETS,
		SMART_WAYPOINTS,
		KEYWORDS,
		COLORS
	};

	class route_configs_t
	{
	public:
		std::string config_name;
		std::string filename;
		std::string uuid;
		route_configs_t(std::string_view config_name, std::string_view filename, std::string_view uuid)
			: config_name(config_name), filename(filename), uuid(uuid) { }
	};

	[[nodiscard]] static auto get() noexcept -> configs& {
		static configs instance;
		return instance;
	}

	[[nodiscard]] inline auto routes() noexcept -> std::vector<route_configs_t>& {
		return route_configs;
	}

	[[nodiscard]] inline auto active_config_name() noexcept -> const std::string& {
		return m_active_config_name;
	}


	auto setup() noexcept -> void;

	auto create_route_config(const std::string_view name) noexcept -> void;
	auto load_route_config(const std::string_view name, bool disable) noexcept -> void;
	auto download_route_config(const std::string_view data) noexcept -> void;
	auto remove_route_config(const std::string_view filename)  noexcept -> void;

	auto do_settings_menu() noexcept -> void;

	template <typename T>
	auto write_data_binary(std::ostream& file, const T& data) const noexcept -> void;
	auto write_string_binary(std::ostream& file, const std::string_view string) const noexcept -> void;

	template <typename T>
	[[nodiscard]] auto read_data_binary(std::istream& file) const noexcept -> T;
	[[nodiscard]] auto read_string_binary(std::istream& file) const noexcept -> std::string;

private:
	auto get_configs_list() noexcept -> void;
	auto get_config_data(size_t index) const noexcept -> std::string;

	auto save_keywords(std::ostream& file) const noexcept -> void;
	auto load_keywords(std::istream& file) const noexcept -> void;

	auto save_colors(std::ostream& file) const noexcept -> void;
	auto load_colors(std::istream& file) const noexcept -> void;

	std::vector<route_configs_t> route_configs;
	std::filesystem::path configs_path;
	std::string m_active_config_name;
};