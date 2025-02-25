class captcha {
private:
	captcha() {};
public:
	[[nodiscard]] static auto get() noexcept -> captcha& {
		static captcha instance;
		return instance;
	}

	inline auto reset() noexcept -> void {
		m_captcha_active = false;
	}

	inline auto setup() noexcept -> void {
		callbacks::get().add_callback(callbacks::callback_type::every_frame, [this]() {
			tick();
		});
	}

	auto tick() noexcept -> void;
	auto handle_received_rpc(uint8_t id, RPCParameters* rpc_params) noexcept -> void;

private:
	bool m_captcha_active { false };
	uint64_t m_captcha_tick{ 0 };
	std::string vehicle_name;
};