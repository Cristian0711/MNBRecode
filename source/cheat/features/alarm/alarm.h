class alarm {
private:
	alarm() {};
public:
	[[nodiscard]] static auto get() noexcept -> alarm& {
		static alarm instance;
		return instance;
	}

	inline auto reset() noexcept -> void {
		m_alarm_active = false;
	}

	auto start_alarm() noexcept -> void;

private:
	inline auto beep_thread() noexcept -> void;
	inline auto custom_alarm_thread() noexcept -> void;

private:
	bool m_alarm_active { false };
};