
class rakclient
{
private:
	rakclient() {};
public:
	enum PacketPriority
	{
		SYSTEM_PRIORITY,
		HIGH_PRIORITY,
		MEDIUM_PRIORITY,
		LOW_PRIORITY,
		NUMBER_OF_PRIORITIES
	};

	enum PacketReliability
	{
		UNRELIABLE = 6,
		UNRELIABLE_SEQUENCED,
		RELIABLE,
		RELIABLE_ORDERED,
		RELIABLE_SEQUENCED
	};
public:

	[[nodiscard]] static auto get() noexcept -> rakclient& {
		static rakclient instance;
		return instance;
	}

	[[nodiscard]] inline auto do_hooks() noexcept -> bool
	{
		auto samp = modules::samp();

		o_hook_send_rpc = static_cast<uint32_t>(samp.get_base() + samp.get_offsets()->send_rpc());
		if (detours_wrapper::get().hook(o_hook_send_rpc, hook_send_rpc) == false)
			return false;

		o_hook_receive_rpc1 = static_cast<uint32_t>(samp.get_base() + samp.get_offsets()->receive_rpc1());
		if (detours_wrapper::get().hook(o_hook_receive_rpc1, hook_receive_rpc1) == false)
			return false;

		o_hook_receive_rpc2 = static_cast<uint32_t>(samp.get_base() + samp.get_offsets()->receive_rpc2());
		if (detours_wrapper::get().hook(o_hook_receive_rpc2, hook_receive_rpc2) == false)
			return false;

		return true;
	}

	inline auto setup() noexcept -> void {
		rak_client_interface = *reinterpret_cast<void**>(modules::samp().info() + modules::samp().get_offsets()->rak_client_interface());
		v_table = *reinterpret_cast<void***>(rak_client_interface);
	}

	auto send_rpc(uint8_t id, BitStream* bitStream, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE_SEQUENCED, int8_t ordering_chanel = 0u, bool shift_timestamp = 0) const noexcept -> bool;
	auto send_packet(BitStream* bitStream, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE_SEQUENCED, int8_t ordering_chanel = 0u) const noexcept -> bool;

private:
	static auto hook_send_rpc() noexcept -> void;
	static auto hook_receive_rpc1() noexcept -> void;
	static auto hook_receive_rpc2() noexcept -> void;

	static auto handle_send_rpc(uint8_t id, BitStream* bitStream) noexcept -> void;
	static auto handle_receive_rpc(uint8_t id, RPCParameters* rpc_params) noexcept -> bool;

	uint32_t o_hook_send_rpc { 0 };
	uint32_t o_hook_receive_rpc1 { 0 };
	uint32_t o_hook_receive_rpc2 { 0 };

	typedef bool(__thiscall* send_rpc_t)(void*, uint8_t*, BitStream*, PacketPriority, PacketReliability, int8_t, bool);
	typedef bool(__thiscall* send_packet_t)(void*, BitStream*, PacketPriority, PacketReliability, int8_t);

	void* rak_client_interface{ nullptr };
	void** v_table{ nullptr };
};