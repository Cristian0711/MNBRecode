
#define DEFINE_OFFSET(name) \
	virtual constexpr uint32_t name() = 0;

#define SET_OFFSET(name, ...) \
	constexpr uint32_t name() override { return __VA_ARGS__; };

inline struct offsets_data_t {
	uint32_t info;
	uint32_t chat;
	uint32_t pools;
	uint32_t settings;
	uint32_t rak_client_interface;
	uint32_t dialog;
	uint32_t textdraws;
	uint32_t input;

	uint32_t player_pool_offset;
	uint32_t vehicles_pool_offset;

	uint32_t gta_vehicle_list_offset;
	uint32_t gta_vehicle_offset;

	uint32_t local_player_offset;
	uint32_t player_local_id_offset;
	uint32_t player_local_vehicle_id_offset;

	uint32_t enter_vehicle;
	uint32_t exit_vehicle;

	uint32_t chat_entry_offset;

	uint32_t local_player_anim_id_offset;
	uint32_t on_foot_data_offset;
	uint32_t in_car_data_offset;
	uint32_t trailer_data_offset;

	uint32_t func_chat_msg;
	uint32_t func_hide_dialog;

	uint32_t ac_patch_1;
	uint32_t ac_patch_2;
	uint32_t ac_patch_3;

	uint32_t func_set_cursor_mode;
	uint32_t func_lock_actor_cam;
	uint32_t func_unlock_actor_cam;

	uint32_t func_get_player_name;
	uint32_t func_screenshot;
	uint32_t func_get_documents_path;

	uint32_t func_send_message;
	uint32_t func_send_command;

	uint32_t func_get_vehicle;
	uint32_t func_enter_vehicle;

	uint32_t add_entry;
	uint32_t send_rpc;
	uint32_t receive_rpc1;
	uint32_t receive_rpc2;

	uint32_t send_on_foot_packet;
	uint32_t send_in_car_packet;
	uint32_t send_trailer_packet;

	uint32_t process_collisions;
	uint32_t process_surfing;

	uint32_t present;
	uint32_t reset;
	uint32_t wndproc;

	auto set_offsets(std::string_view json_data) noexcept -> void {
		json json = json::parse(json_data);

		info = std::stoul(json["info"].get<std::string>(), nullptr, 16);
		chat = std::stoul(json["chat"].get<std::string>(), nullptr, 16);
		pools = std::stoul(json["pools"].get<std::string>(), nullptr, 16);
		settings = std::stoul(json["settings"].get<std::string>(), nullptr, 16);
		rak_client_interface = std::stoul(json["rak_client_interface"].get<std::string>(), nullptr, 16);
		dialog = std::stoul(json["dialog"].get<std::string>(), nullptr, 16);
		textdraws = std::stoul(json["textdraws"].get<std::string>(), nullptr, 16);
		input = std::stoul(json["input"].get<std::string>(), nullptr, 16);
		player_pool_offset = std::stoul(json["player_pool_offset"].get<std::string>(), nullptr, 16);
		vehicles_pool_offset = std::stoul(json["vehicles_pool_offset"].get<std::string>(), nullptr, 16);
		gta_vehicle_list_offset = std::stoul(json["gta_vehicle_list_offset"].get<std::string>(), nullptr, 16);
		gta_vehicle_offset = std::stoul(json["gta_vehicle_offset"].get<std::string>(), nullptr, 16);
		local_player_offset = std::stoul(json["local_player_offset"].get<std::string>(), nullptr, 16);
		player_local_id_offset = std::stoul(json["player_local_id_offset"].get<std::string>(), nullptr, 16);
		player_local_vehicle_id_offset = std::stoul(json["player_local_vehicle_id_offset"].get<std::string>(), nullptr, 16);
		enter_vehicle = std::stoul(json["enter_vehicle"].get<std::string>(), nullptr, 16);
		exit_vehicle = std::stoul(json["exit_vehicle"].get<std::string>(), nullptr, 16);
		chat_entry_offset = std::stoul(json["chat_entry_offset"].get<std::string>(), nullptr, 16);
		local_player_anim_id_offset = std::stoul(json["local_player_anim_id_offset"].get<std::string>(), nullptr, 16);
		on_foot_data_offset = std::stoul(json["on_foot_data_offset"].get<std::string>(), nullptr, 16);
		in_car_data_offset = std::stoul(json["in_car_data_offset"].get<std::string>(), nullptr, 16);
		trailer_data_offset = std::stoul(json["trailer_data_offset"].get<std::string>(), nullptr, 16);
		func_chat_msg = std::stoul(json["func_chat_msg"].get<std::string>(), nullptr, 16);
		func_hide_dialog = std::stoul(json["func_hide_dialog"].get<std::string>(), nullptr, 16);
		ac_patch_1 = std::stoul(json["ac_patch_1"].get<std::string>(), nullptr, 16);
		ac_patch_2 = std::stoul(json["ac_patch_2"].get<std::string>(), nullptr, 16);
		ac_patch_3 = std::stoul(json["ac_patch_3"].get<std::string>(), nullptr, 16);
		func_set_cursor_mode = std::stoul(json["func_set_cursor_mode"].get<std::string>(), nullptr, 16);
		func_lock_actor_cam = std::stoul(json["func_lock_actor_cam"].get<std::string>(), nullptr, 16);
		func_unlock_actor_cam = std::stoul(json["func_unlock_actor_cam"].get<std::string>(), nullptr, 16);
		func_get_player_name = std::stoul(json["func_get_player_name"].get<std::string>(), nullptr, 16);
		func_screenshot = std::stoul(json["func_screenshot"].get<std::string>(), nullptr, 16);
		func_get_documents_path = std::stoul(json["func_get_documents_path"].get<std::string>(), nullptr, 16);
		func_send_message = std::stoul(json["func_send_message"].get<std::string>(), nullptr, 16);
		func_send_command = std::stoul(json["func_send_command"].get<std::string>(), nullptr, 16);
		func_get_vehicle = std::stoul(json["func_get_vehicle"].get<std::string>(), nullptr, 16);
		func_enter_vehicle = std::stoul(json["func_enter_vehicle"].get<std::string>(), nullptr, 16);
		add_entry = std::stoul(json["add_entry"].get<std::string>(), nullptr, 16);
		send_rpc = std::stoul(json["send_rpc"].get<std::string>(), nullptr, 16);
		receive_rpc1 = std::stoul(json["receive_rpc1"].get<std::string>(), nullptr, 16);
		receive_rpc2 = std::stoul(json["receive_rpc2"].get<std::string>(), nullptr, 16);
		send_on_foot_packet = std::stoul(json["send_on_foot_packet"].get<std::string>(), nullptr, 16);
		send_in_car_packet = std::stoul(json["send_in_car_packet"].get<std::string>(), nullptr, 16);
		send_trailer_packet = std::stoul(json["send_trailer_packet"].get<std::string>(), nullptr, 16);
		process_collisions = std::stoul(json["process_collisions"].get<std::string>(), nullptr, 16);
		process_surfing = std::stoul(json["process_surfing"].get<std::string>(), nullptr, 16);
		present = std::stoul(json["present"].get<std::string>(), nullptr, 16);
		reset = std::stoul(json["reset"].get<std::string>(), nullptr, 16);
		wndproc = std::stoul(json["wndproc"].get<std::string>(), nullptr, 16);
	}

} offsets;

class samp_offsets_i {
public:
	DEFINE_OFFSET(info)
	DEFINE_OFFSET(chat)
	DEFINE_OFFSET(pools)
	DEFINE_OFFSET(settings)
	DEFINE_OFFSET(rak_client_interface)
	DEFINE_OFFSET(dialog)
	DEFINE_OFFSET(textdraws)
	DEFINE_OFFSET(input)

	DEFINE_OFFSET(player_pool_offset)
	DEFINE_OFFSET(vehicles_pool_offset)

	DEFINE_OFFSET(gta_vehicle_list_offset)
	DEFINE_OFFSET(gta_vehicle_offset)

	DEFINE_OFFSET(local_player_offset)
	DEFINE_OFFSET(player_local_id_offset)
	DEFINE_OFFSET(player_local_vehicle_id_offset)

	DEFINE_OFFSET(enter_vehicle)
	DEFINE_OFFSET(exit_vehicle)

	DEFINE_OFFSET(chat_entry_offset)

	DEFINE_OFFSET(local_player_anim_id_offset)
	DEFINE_OFFSET(on_foot_data_offset)
	DEFINE_OFFSET(in_car_data_offset)
	DEFINE_OFFSET(trailer_data_offset)

	DEFINE_OFFSET(func_chat_msg)
	DEFINE_OFFSET(func_hide_dialog)

	DEFINE_OFFSET(ac_patch_1)
	DEFINE_OFFSET(ac_patch_2)
	DEFINE_OFFSET(ac_patch_3)

	DEFINE_OFFSET(func_set_cursor_mode)
	DEFINE_OFFSET(func_lock_actor_cam)
	DEFINE_OFFSET(func_unlock_actor_cam)

	DEFINE_OFFSET(func_get_player_name)
	DEFINE_OFFSET(func_screenshot)
	DEFINE_OFFSET(func_get_documents_path)

	DEFINE_OFFSET(func_send_message)
	DEFINE_OFFSET(func_send_command)

	DEFINE_OFFSET(func_get_vehicle)
	DEFINE_OFFSET(func_enter_vehicle)

	DEFINE_OFFSET(add_entry)
	DEFINE_OFFSET(send_rpc)
	DEFINE_OFFSET(receive_rpc1)
	DEFINE_OFFSET(receive_rpc2)

	DEFINE_OFFSET(send_on_foot_packet)
	DEFINE_OFFSET(send_in_car_packet)
	DEFINE_OFFSET(send_trailer_packet)

	DEFINE_OFFSET(process_collisions)
	DEFINE_OFFSET(process_surfing)

	DEFINE_OFFSET(present)
	DEFINE_OFFSET(reset)
	DEFINE_OFFSET(wndproc)
};

class samp_offsets_r1 : samp_offsets_i {
public:
	static samp_offsets_r1* get() noexcept { return new samp_offsets_r1(); }

	SET_OFFSET(info, offsets.info);
	SET_OFFSET(chat, offsets.chat);
	SET_OFFSET(pools, offsets.pools);
	SET_OFFSET(settings, offsets.settings);
	SET_OFFSET(rak_client_interface, offsets.rak_client_interface);
	SET_OFFSET(dialog, offsets.dialog);
	SET_OFFSET(textdraws, offsets.textdraws);
	SET_OFFSET(input, offsets.input);

	SET_OFFSET(player_pool_offset, offsets.player_pool_offset);
	SET_OFFSET(vehicles_pool_offset, offsets.vehicles_pool_offset);

	SET_OFFSET(gta_vehicle_list_offset, offsets.gta_vehicle_list_offset);
	SET_OFFSET(gta_vehicle_offset, offsets.gta_vehicle_offset);

	SET_OFFSET(local_player_offset, offsets.local_player_offset);
	SET_OFFSET(player_local_id_offset, offsets.player_local_id_offset);
	SET_OFFSET(player_local_vehicle_id_offset, offsets.player_local_vehicle_id_offset);

	SET_OFFSET(enter_vehicle, offsets.enter_vehicle);
	SET_OFFSET(exit_vehicle, offsets.exit_vehicle);

	SET_OFFSET(chat_entry_offset, offsets.chat_entry_offset);

	SET_OFFSET(local_player_anim_id_offset, offsets.local_player_anim_id_offset);
	SET_OFFSET(on_foot_data_offset, offsets.on_foot_data_offset);
	SET_OFFSET(in_car_data_offset, offsets.in_car_data_offset);
	SET_OFFSET(trailer_data_offset, offsets.trailer_data_offset);

	SET_OFFSET(func_chat_msg, offsets.func_chat_msg);
	SET_OFFSET(func_hide_dialog, offsets.func_hide_dialog);

	SET_OFFSET(ac_patch_1, offsets.ac_patch_1);
	SET_OFFSET(ac_patch_2, offsets.ac_patch_2);
	SET_OFFSET(ac_patch_3, offsets.ac_patch_3);

	SET_OFFSET(func_set_cursor_mode, offsets.func_set_cursor_mode);
	SET_OFFSET(func_lock_actor_cam, offsets.func_lock_actor_cam);
	SET_OFFSET(func_unlock_actor_cam, offsets.func_unlock_actor_cam);

	SET_OFFSET(func_get_player_name, offsets.func_get_player_name);
	SET_OFFSET(func_screenshot, offsets.func_screenshot);
	SET_OFFSET(func_get_documents_path, offsets.func_get_documents_path);

	SET_OFFSET(func_send_message, offsets.func_send_message);
	SET_OFFSET(func_send_command, offsets.func_send_command);

	SET_OFFSET(func_get_vehicle, offsets.func_get_vehicle);
	SET_OFFSET(func_enter_vehicle, offsets.func_enter_vehicle);

	SET_OFFSET(add_entry, offsets.add_entry);
	SET_OFFSET(send_rpc, offsets.send_rpc);
	SET_OFFSET(receive_rpc1, offsets.receive_rpc1);
	SET_OFFSET(receive_rpc2, offsets.receive_rpc2);

	SET_OFFSET(send_on_foot_packet, offsets.send_on_foot_packet);
	SET_OFFSET(send_in_car_packet, offsets.send_in_car_packet);
	SET_OFFSET(send_trailer_packet, offsets.send_trailer_packet);

	SET_OFFSET(process_collisions, offsets.process_collisions);
	SET_OFFSET(process_surfing, offsets.process_surfing);

	SET_OFFSET(present, offsets.present);
	SET_OFFSET(reset, offsets.reset);
	SET_OFFSET(wndproc, offsets.wndproc);
};

extern uint32_t g_samp_module;
extern samp_offsets_i* g_samp_offsets;