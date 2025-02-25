
class waypoints {
private:
	waypoints() {};
public:
	[[nodiscard]] static auto get() noexcept -> waypoints& {
		static waypoints instance;
		return instance;
	}

	auto setup() noexcept -> void
	{
		callbacks::get().add_callback(callbacks::callback_type::every_frame, [this]() {
			tick();
		});

		callbacks::get().add_callback(callbacks::callback_type::render, [this]() {
			draw();
		});
	}

	[[nodiscard]] inline auto active_waypoint() const noexcept -> size_t {
		return m_active_waypoint;
	}

	inline auto pause() noexcept -> void {
		m_paused = true;
	}

	inline auto resume() noexcept -> void {
		m_paused = false;
	}

	auto record_waypoint() const noexcept -> void;
	auto record_fill_waypoint() const noexcept -> void;

	auto do_settings_menu() noexcept -> void;

	auto enable() noexcept -> void;
	auto disable() noexcept -> void;

private:
	auto draw() const noexcept -> void;
	auto tick() noexcept -> void;
	auto record() const noexcept -> void;

	auto add_waypoint(std::vector<waypoint_t>& waypoints, const CVector& position, float speed) const noexcept -> void;
	auto draw_waypoint(size_t index, const CVector& position, ImColor& color) const noexcept -> void;

	auto start_fill_route() noexcept -> void;
	auto fill_thread() noexcept -> void;

	auto normal_route() noexcept -> void;
	auto fill_route() noexcept -> void;
	auto walk_drive_system() noexcept -> void;

	[[nodiscard]] auto should_start_fill_route() noexcept -> bool;
private:
	size_t m_active_waypoint{ 0u };
	uint8_t m_route_loops{ 0u };

	bool m_fill_route_active{ false };
	bool m_paused{ false };

};