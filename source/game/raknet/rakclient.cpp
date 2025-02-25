#include "core.h"

auto rakclient::send_rpc(uint8_t id, BitStream* bitStream, PacketPriority priority, PacketReliability reliability, int8_t ordering_chanel, bool shift_timestamp) const noexcept -> bool
{
	return (send_rpc_t(v_table[25]))(rak_client_interface, &id, bitStream, priority, reliability, ordering_chanel, shift_timestamp);
}

auto rakclient::send_packet(BitStream* bitStream, PacketPriority priority, PacketReliability reliability, int8_t ordering_chanel) const noexcept -> bool
{
	return (send_packet_t(v_table[6]))(rak_client_interface, bitStream, priority, reliability, ordering_chanel);
}


auto rakclient::handle_send_rpc(uint8_t id, BitStream* bitStream) noexcept -> void
{
	packets::get().record_sent_rpc(id, bitStream);
}

auto rakclient::handle_receive_rpc(uint8_t id, RPCParameters* rpc_params) noexcept -> bool
{
	if (rpc_params == nullptr)
		return true;

	packets::get().handle_received_rpc(id, rpc_params);
	captcha::get().handle_received_rpc(id, rpc_params);

	return true;
}


__declspec(naked) auto rakclient::hook_send_rpc() noexcept -> void
{
	__asm pushad;
	static uint32_t jmp_back = (uint32_t)(modules::samp().get_base() + 0x30B89);
	static BitStream* bitStream = nullptr;
	static int32_t* id = nullptr;
	__asm popad;

	__asm
	{
		mov bitStream, eax;
		mov id, edx;
		pushad;
	}

	handle_send_rpc(*id, bitStream);

	__asm
	{
		popad;
		push edx;
		add ecx, 0xFFFFF222;
		jmp[jmp_back];
	}
}

__declspec(naked) auto rakclient::hook_receive_rpc1() noexcept -> void
{
	__asm pushad;
	static RPCParameters* rpc_params = nullptr;
	static RPCNode* rpc_node = nullptr;
	static uint32_t jmp_back = (uint32_t)(modules::samp().get_base() + 0x373C9);
	__asm popad;

	__asm
	{
		push ecx;
		mov[ebp + 0x44], 0;
		mov rpc_params, eax;
		mov rpc_node, edi;
		pushad;
	}

	if(handle_receive_rpc(rpc_node->uniqueIdentifier, rpc_params) == false)
		jmp_back = (uint32_t)(modules::samp().get_base() + 0x374CC);
	else
		jmp_back = (uint32_t)(modules::samp().get_base() + 0x373C9);

	__asm
	{
		popad;
		jmp[jmp_back];
	}
}

__declspec(naked) auto rakclient::hook_receive_rpc2() noexcept -> void
{
	__asm pushad;
	static RPCParameters* rpc_params = nullptr;
	static RPCNode* rpc_node = nullptr;
	static uint32_t jmp_back = (uint32_t)(modules::samp().get_base() + 0x3743D);
	__asm popad;

	__asm
	{
		lea eax, [ebp + 0x44];
		push eax;
		mov[ebp + 0x44], esi;
		mov rpc_params, eax;
		mov rpc_node, edi;
		pushad;
	}

	if (handle_receive_rpc(rpc_node->uniqueIdentifier, rpc_params) == false)
		jmp_back = (uint32_t)(modules::samp().get_base() + 0x37440);
	else
		jmp_back = (uint32_t)(modules::samp().get_base() + 0x3743D);

	__asm
	{
		popad;
		jmp[jmp_back];
	}
}