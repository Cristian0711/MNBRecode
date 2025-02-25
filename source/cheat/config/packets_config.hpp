namespace packets_config
{
	static auto save_individual_packet(std::ostream& file, const packet_sync_data_t& current_packet) noexcept -> void
	{
		const auto& configs = configs::get();
		
		auto packet_id = current_packet.packet.id;
		configs.write_data_binary<uint8_t>(file, packet_id);

		switch (packet_id)
		{
		case ID_PLAYER_SYNC:
			configs.write_data_binary<on_foot_data_t>(file, current_packet.packet.on_foot_data);
			break;
		case ID_VEHICLE_SYNC:
			configs.write_data_binary<uint16_t>(file, current_packet.packet.model_id);
			configs.write_data_binary<in_car_data_t>(file, current_packet.packet.in_car_data);
			break;
		}
	}

	static auto save_individual_sent_rpc(std::ostream& file, const packet_sync_data_t& sent_rpc) noexcept -> void
	{
		const auto& configs = configs::get();

		auto sent_rpc_id = sent_rpc.rpc.id;
		configs.write_data_binary<uint8_t>(file, sent_rpc_id);

		switch (sent_rpc_id)
		{
		case RPC_Chat:
			configs.write_data_binary<uint8_t>(file, sent_rpc.rpc.send_message.text_length);
			configs.write_string_binary(file, sent_rpc.rpc.send_message.text);
			break;
		case RPC_ServerCommand:
			configs.write_data_binary<uint32_t>(file, sent_rpc.rpc.send_command.text_length);
			configs.write_string_binary(file, sent_rpc.rpc.send_command.text);
			break;
		case RPC_DialogResponse:
			configs.write_data_binary<uint8_t>(file, sent_rpc.rpc.send_dialog_response.text_length);
			configs.write_string_binary(file, sent_rpc.rpc.send_dialog_response.text);

			configs.write_data_binary<uint16_t>(file, sent_rpc.rpc.send_dialog_response.id);
			configs.write_data_binary<uint8_t>(file, sent_rpc.rpc.send_dialog_response.response);
			configs.write_data_binary<uint16_t>(file, sent_rpc.rpc.send_dialog_response.list_item);
			break;
		case RPC_ClickTextDraw:
			configs.write_data_binary<uint16_t>(file, sent_rpc.rpc.send_text_draw.id);
			break;
		}
	}

	static auto save_individual_received_rpc(std::ostream& file, const packet_sync_data_t& sent_rpc) noexcept -> void
	{
		const auto& configs = configs::get();

		auto received_rpc_id = sent_rpc.rpc.id;
		configs.write_data_binary<uint8_t>(file, received_rpc_id);

		if (received_rpc_id == RPC_TogglePlayerControllable || received_rpc_id == RPC_SetPlayerPos ||
			received_rpc_id == RPC_SetVehiclePos || received_rpc_id == RPC_PutPlayerInVehicle) {
			configs.write_data_binary<CVector>(file, sent_rpc.rpc.position);
		}
	}

	static auto save_packets(std::ostream& file) noexcept -> void
	{
		const auto& packets_vector = vars.packets.packets_vector;

		if (packets_vector.empty() == true)
			return;

		const auto& configs = configs::get();

		configs.write_data_binary(file, configs::save_byte_t::PACKETS);
		configs.write_data_binary(file, packets_vector.size());

		for (const auto& sync_data : packets_vector)
		{
			configs.write_data_binary<uint8_t>(file, sync_data.type);
			switch (sync_data.type)
			{
			case packets::PACKET:
				save_individual_packet(file, sync_data);
				break;
			case packets::SENT_RPC:
				save_individual_sent_rpc(file, sync_data);
				break;
			case packets::RECEIVED_RPC:
				save_individual_received_rpc(file, sync_data);
				break;
			}
		}
	}

	static auto load_individual_packet(std::istream& file, packet_sync_data_t& current_packet) noexcept -> void
	{
		const auto& configs = configs::get();

		current_packet.packet.id = configs.read_data_binary<uint8_t>(file);
		switch (current_packet.packet.id)
		{
		case ID_PLAYER_SYNC:
			current_packet.packet.on_foot_data = configs.read_data_binary<on_foot_data_t>(file);
			break;
		case ID_VEHICLE_SYNC:
			current_packet.packet.model_id = configs.read_data_binary<uint16_t>(file);
			current_packet.packet.in_car_data = configs.read_data_binary<in_car_data_t>(file);
			break;
		}
	}

	static auto load_individual_sent_rpc(std::istream& file, packet_sync_data_t& current_rpc) noexcept -> void
	{
		const auto& configs = configs::get();
		current_rpc.rpc.id = configs.read_data_binary<uint8_t>(file);
		switch (current_rpc.rpc.id)
		{
		case RPC_Chat:
			current_rpc.rpc.send_message.text_length = configs.read_data_binary<uint8_t>(file);
			std::memmove(current_rpc.rpc.send_message.text, configs.read_string_binary(file).c_str(),
				current_rpc.rpc.send_message.text_length);
			break;
		case RPC_ServerCommand:
			current_rpc.rpc.send_command.text_length = configs.read_data_binary<uint32_t>(file);
			std::memmove(current_rpc.rpc.send_command.text, configs.read_string_binary(file).c_str(),
				current_rpc.rpc.send_command.text_length);
			break;
		case RPC_DialogResponse:
			current_rpc.rpc.send_dialog_response.text_length = configs.read_data_binary<uint8_t>(file);
			std::memmove(current_rpc.rpc.send_dialog_response.text, configs.read_string_binary(file).c_str(),
				current_rpc.rpc.send_dialog_response.text_length);

			current_rpc.rpc.send_dialog_response.id = configs.read_data_binary<uint16_t>(file);
			current_rpc.rpc.send_dialog_response.response = configs.read_data_binary<uint8_t>(file);
			current_rpc.rpc.send_dialog_response.list_item = configs.read_data_binary<uint16_t>(file);
			break;
		case RPC_ClickTextDraw:
			current_rpc.rpc.send_text_draw.id = configs.read_data_binary<uint16_t>(file);
			break;
		}
	}

	static auto load_individual_received_rpc(std::istream& file, packet_sync_data_t& current_rpc) noexcept -> void
	{
		const auto& configs = configs::get();

		current_rpc.rpc.id = configs.read_data_binary<uint8_t>(file);

		if (current_rpc.rpc.id == RPC_TogglePlayerControllable || current_rpc.rpc.id == RPC_SetPlayerPos ||
			current_rpc.rpc.id == RPC_SetVehiclePos || current_rpc.rpc.id == RPC_PutPlayerInVehicle) {

			current_rpc.rpc.position = configs.read_data_binary<CVector>(file);
		}
	}

	static auto load_packets(std::istream& file) noexcept -> void
	{
		const auto& configs = configs::get();
		auto packets_vector_size = configs.read_data_binary<size_t>(file);

		for (size_t index = 0; index != packets_vector_size; ++index)
		{
			packet_sync_data_t sync_data;
			sync_data.type = configs.read_data_binary<uint8_t>(file);

			switch (sync_data.type)
			{
			case packets::PACKET:
				load_individual_packet(file, sync_data);
				break;
			case packets::SENT_RPC:
				load_individual_sent_rpc(file, sync_data);
				break;
			case packets::RECEIVED_RPC:
				load_individual_received_rpc(file, sync_data);
				break;
			}

			vars.packets.packets_vector.emplace_back(std::move(sync_data));
		}
	}
}