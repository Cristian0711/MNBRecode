#define PACKET_SEND_CHEAT (void*)6969

class packets {
private:
	packets() {};
public:
	enum sync_data_type : uint8_t
	{
		PACKET = 0,
		SENT_RPC,
		RECEIVED_RPC
	};

public:
	[[nodiscard]] static auto get() noexcept -> packets& {
		static packets instance;
		return instance;
	}

	[[nodiscard]] inline auto do_hooks() noexcept -> bool
	{
		auto samp = modules::samp();

		o_send_on_foot_packet = reinterpret_cast<packet_send_t>(samp.get_base() + samp.get_offsets()->send_on_foot_packet());
		if (detours_wrapper::get().hook(o_send_on_foot_packet, hook_send_on_foot_packet) == false)
			return false;

		o_send_in_car_packet = reinterpret_cast<packet_send_t>(samp.get_base() + samp.get_offsets()->send_in_car_packet());
		if (detours_wrapper::get().hook(o_send_in_car_packet, hook_send_in_car_packet) == false)
			return false;

		o_send_trailer_packet = reinterpret_cast<packet_send_trailer_t>(samp.get_base() + samp.get_offsets()->send_trailer_packet());
		if (detours_wrapper::get().hook(o_send_trailer_packet, hook_send_trailer_packet) == false)
			return false;

		return true;
	}

	inline auto setup() noexcept -> void
	{
		callbacks::get().add_callback(callbacks::callback_type::render, [this]() {
			draw();
		});

		callbacks::get().add_callback(callbacks::callback_type::every_frame, [this]() {
			tick();
		});
	}

	auto enable() noexcept -> void;
	auto disable() noexcept -> void;

	auto record_sent_rpc(uint8_t id, BitStream* data) const noexcept -> void;
	auto handle_received_rpc (uint8_t id, RPCParameters* rpc_params) noexcept -> void;

	auto do_settings_menu() noexcept -> void;
private:
	auto draw() const noexcept -> void;
	auto tick() noexcept -> void;

	auto send_recorded_rpc() noexcept -> void;
	auto record_received_rpc(uint8_t id, BitStream& data) const noexcept -> void;
	auto handle_recorded_received_rpc() noexcept -> void;

	auto send_on_foot_packet(on_foot_data_t& on_foot_data) const noexcept -> void;
	auto send_in_car_packet(in_car_data_t& in_car_data) const noexcept -> void;

	auto record_on_foot_packet() const noexcept -> void;
	auto record_in_car_packet() const noexcept -> void;

	[[nodiscard]] auto handle_on_foot_packet(void* _edx) noexcept -> bool;
	[[nodiscard]] auto handle_in_car_packet(void* _edx) noexcept -> bool;

	typedef void(__thiscall* packet_send_t)(void*);
	typedef void(__thiscall* packet_send_trailer_t)(void*, uint16_t);

	packet_send_t o_send_on_foot_packet{ nullptr };
	packet_send_t o_send_in_car_packet{ nullptr };
	packet_send_trailer_t o_send_trailer_packet{ nullptr };

	static auto __fastcall hook_send_on_foot_packet(void* _this, void* _edx) noexcept -> void;
	static auto __fastcall hook_send_in_car_packet(void* _this, void* _edx) noexcept -> void;
	static auto __fastcall hook_send_trailer_packet(void* _this, void* _edx, uint16_t unused) noexcept -> void;

private:
	size_t m_active_packet{ 0u };

	uint16_t current_vehicle_id{ 0u };
};