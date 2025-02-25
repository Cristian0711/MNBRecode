class conditional_routes {
private:
	conditional_routes() {};

	enum configs_type
	{
		LOOPS_CONFIG = 0,
		CONDITIONAL_CONFIG
	};

	enum conditions
	{
		HEALTH = 0,
		FUEL
	};

	class loops_configs_t
	{
	public:
		std::string config_name;
		std::string filename;
		std::string uuid;
		uint8_t loops_count;
		loops_configs_t(configs::route_configs_t& config, uint8_t loops_count)
			: config_name(config.config_name), filename(config.filename), uuid(config.uuid), loops_count(loops_count) { }
	};

	class conditional_configs_t
	{
	public:
		std::string config_name;
		std::string filename;
		std::string uuid;
		uint8_t condition;
		float value;
		conditional_configs_t(configs::route_configs_t& config, uint8_t condition, float value)
			: config_name(config.config_name), filename(config.filename), uuid(config.uuid), condition(condition), value(value) { }
	};

public:
	[[nodiscard]] static auto get() noexcept -> conditional_routes& {
		static conditional_routes instance;
		return instance;
	}

	auto reset() noexcept -> void {
		m_route_loops = 0;
		m_current_route = 0;

		m_last_route_was_conditional = false;
	}

	auto increment_route_loops_and_check() noexcept -> void;
	auto do_settings_menu() noexcept -> void;
private:
	auto any_condition_valid() noexcept -> std::optional<std::string_view>;
	auto change_to_conditional_route(std::string_view config_name) noexcept -> void;
	auto change_to_next_route() noexcept -> void;
	auto find_fill_textdraw_id() const noexcept -> uint16_t;
	auto get_fuel_value() noexcept -> float;
	auto refresh_data() noexcept -> void;
	auto change_current_route() noexcept -> void;
private:
	uint8_t m_route_loops{ 0 };
	uint8_t m_current_route{ 0 };
	uint16_t m_fuel_textdraw_id{ 0 };

	float m_last_fuel_value{ -1.f };
	float m_last_health_value{ -1.f };

	bool m_last_route_was_conditional{ false };

	std::vector<loops_configs_t> loops_configs;
	std::vector<conditional_configs_t> conditional_configs;
};