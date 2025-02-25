class utils {
private:
	utils() {};
public:
	[[nodiscard]] static auto get() -> utils& {
		static utils instance;
		return instance;
	}

	[[nodiscard]] auto get_distance_to_local_ped(const CVector& position) const noexcept -> float {
		if (vars.local_vehicle != nullptr) {
			return (position - vars.local_vehicle->GetPosition()).Magnitude();
		}
		else if (vars.local_ped != nullptr) {
			return (position - vars.local_ped->GetPosition()).Magnitude();
		}
		else {
			return -1.f;
		}
	}

	[[nodiscard]] auto get_distance_to_local_ped_2d(const CVector& position) const noexcept -> float {
		if (vars.local_vehicle != nullptr) {
			return (position - vars.local_vehicle->GetPosition()).Magnitude2D();
		}
		else if (vars.local_ped != nullptr) {
			return (position - vars.local_ped->GetPosition()).Magnitude2D();
		}
		else {
			return -1.f;
		}
	}

	[[nodiscard]] auto get_distance_between_two_points(const CVector& first, const CVector& second) const noexcept -> float {
		return (first - second).Magnitude();
	}

	[[nodiscard]] auto are_vectors_equal (const CVector& first, const CVector& second) const noexcept -> float {
		return first.x == second.x && first.y == second.y && first.z == second.z;
	}

	[[nodiscard]] auto get_local_ped_position() const noexcept -> const CVector& {
		if (vars.local_vehicle != nullptr) {
			return vars.local_vehicle->GetPosition();
		}
		else if (vars.local_ped != nullptr) {
			return vars.local_ped->GetPosition();
		}
		else {
			return { 0.f, 0.f, 0.f };
		}
	}

	[[nodiscard]] auto does_ped_send_input() const noexcept -> bool
	{
		return (ImGui::IsKeyDown('W') || ImGui::IsKeyDown('A') || ImGui::IsKeyDown('S') || ImGui::IsKeyDown('D'));
	}

	[[nodiscard]] inline auto is_ped_locked() const noexcept -> bool {
		return m_ped_locked;
	}

	inline auto set_ped_locked() noexcept -> void {
		m_ped_locked = true;
	}

	inline auto unlock_ped() noexcept -> void
	{
		if (vars.local_vehicle != nullptr) {
			plugin::Command<plugin::Commands::SET_CAR_STATUS>(vars.local_vehicle, NULL);
		}
		else if (vars.local_ped != nullptr) {
			plugin::Command<plugin::Commands::CLEAR_CHAR_TASKS>(vars.local_ped);
		}
		m_ped_locked = false;
	}

	[[nodiscard]] auto get_vehicle_model_name(uint16_t model_id) const noexcept -> std::string_view
	{
		if (model_id < 400)
			return "INVALID";

		return vehicle_model_names[model_id - 400];
	}

	auto sync_vehicle_enter(uint16_t id) const noexcept -> void {
		BitStream data;
		data.Write<uint16_t>(id);
		data.Write<uint8_t>(0);
		rakclient::get().send_rpc(RPC_EnterVehicle, &data);
	}

	auto sync_vehicle_exit(uint16_t id) const noexcept -> void {
		BitStream data;
		data.Write<uint16_t>(id);
		rakclient::get().send_rpc(RPC_ExitVehicle, &data);
	}

	[[nodiscard]] auto enter_vehicle(uint16_t id) const noexcept -> bool
	{
		auto* gta_vehicle = modules::samp().vehicles()->get_vehicle(id);
		if (gta_vehicle == nullptr || gta_vehicle->m_nType != ENTITY_TYPE_VEHICLE) {
			return false;
		}

		auto vehicle = ((uint32_t(__thiscall*)(samp_vehicles_t*, uint32_t))(g_samp_module + g_samp_offsets->func_get_vehicle()))(modules::samp().vehicles(), id);
		
		if (vehicle == 0)
			return false;

		((CPed * (__thiscall*)(void*, uint32_t, uint32_t))(g_samp_module + +g_samp_offsets->func_enter_vehicle()))(modules::samp().local_player()->get_samp_ped(), 
			*(uint32_t*)(vehicle + +g_samp_offsets->gta_vehicle_offset()), 0);

		return true;
	}

	auto exit_vehicle(uint16_t id) const noexcept -> void
	{
		plugin::Command<plugin::Commands::TASK_LEAVE_ANY_CAR>(vars.local_ped);
	}

	[[nodiscard]] auto does_samp_vehicle_exist(uint16_t id) const noexcept -> bool {
		return modules::samp().vehicles()->get_vehicle(id) != nullptr;
	}

	[[nodiscard]] auto get_closest_vehicle_id() const noexcept -> uint16_t
	{
		LOG("fngetClosestVehicleID");
		auto distance = 8.f;
		uint16_t closest_vehicle_id = 0;

		for (uint16_t index = 0; index != 2000; ++index)
		{
			auto* gta_vehicle = modules::samp().vehicles()->get_vehicle(index);

			if (gta_vehicle == nullptr)
				continue;

			CVector& position = reinterpret_cast<CVehicle*>(gta_vehicle)->GetPosition();

			auto currentDistance = get_distance_to_local_ped(position);
			if (currentDistance < distance)
			{
				distance = currentDistance;
				closest_vehicle_id = index;
			}
		}

		return closest_vehicle_id;
	}

	[[nodiscard]] auto get_vehicle_model_id_from_name(const std::string& vehicle_name) const noexcept -> uint16_t
	{
		for (size_t index = 0; index != vehicle_model_names.size(); ++index)
		{
			if (lower_string(vehicle_model_names[index]) != lower_string(vehicle_name))
				continue;

			return index + 400;
		}
		return -1;
	}

private:
	[[nodiscard]] auto lower_string(const std::string& str) const noexcept -> std::string
	{
		std::string data = str;
		std::transform(data.begin(), data.end(), data.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return data;
	}

	private:
	bool m_ped_locked{ false };

	std::vector<const char*> vehicle_model_names = {
	"Landstalker", "Bravura", "Buffalo", "Linerunner", "Perrenial", "Sentinel",
	"Dumper", "Firetruck", "Trashmaster", "Stretch", "Manana", "Infernus",
	"Voodoo", "Pony", "Mule", "Cheetah", "Ambulance", "Leviathan", "Moonbeam",
	"Esperanto", "Taxi", "Washington", "Bobcat", "Whoopee", "BF Injection",
	"Hunter", "Premier", "Enforcer", "Securicar", "Banshee", "Predator", "Bus",
	"Rhino", "Barracks", "Hotknife", "Trailer", "Previon", "Coach", "Cabbie",
	"Stallion", "Rumpo", "RC Bandit", "Romero", "Packer", "Monster", "Admiral",
	"Squalo", "Seasparrow", "Pizzaboy", "Tram", "Trailer", "Turismo", "Speeder",
	"Reefer", "Tropic", "Flatbed", "Yankee", "Caddy", "Solair", "Berkley's RC Van",
	"Skimmer", "PCJ-600", "Faggio", "Freeway", "RC Baron", "RC Raider", "Glendale",
	"Oceanic","Sanchez", "Sparrow", "Patriot", "Quad", "Coastguard", "Dinghy",
	"Hermes", "Sabre", "Rustler", "ZR-350", "Walton", "Regina", "Comet", "BMX",
	"Burrito", "Camper", "Marquis", "Baggage", "Dozer", "Maverick", "News Chopper",
	"Rancher", "FBI Rancher", "Virgo", "Greenwood", "Jetmax", "Hotring", "Sandking",
	"Blista Compact", "Police Maverick", "Boxville", "Benson", "Mesa", "RC Goblin",
	"Hotring Racer A", "Hotring Racer B", "Bloodring Banger", "Rancher", "Super GT",
	"Elegant", "Journey", "Bike", "Mountain Bike", "Beagle", "Cropduster", "Stunt",
	 "Tanker", "Roadtrain", "Nebula", "Majestic", "Buccaneer", "Shamal", "Hydra",
	 "FCR-900", "NRG-500", "HPV1000", "Cement Truck", "Tow Truck", "Fortune",
	 "Cadrona", "FBI Truck", "Willard", "Forklift", "Tractor", "Combine", "Feltzer",
	 "Remington", "Slamvan", "Blade", "Freight", "Streak", "Vortex", "Vincent",
	"Bullet", "Clover", "Sadler", "Firetruck", "Hustler", "Intruder", "Primo",
	"Cargobob", "Tampa", "Sunrise", "Merit", "Utility", "Nevada", "Yosemite",
	"Windsor", "Monster", "Monster", "Uranus", "Jester", "Sultan", "Stratum",
	"Elegy", "Raindance", "RC Tiger", "Flash", "Tahoma", "Savanna", "Bandito",
	"Freight Flat", "Streak Carriage", "Kart", "Mower", "Dune", "Sweeper",
	"Broadway", "Tornado", "AT-400", "DFT-30", "Huntley", "Stafford", "BF-400",
	"News Van", "Tug", "Trailer", "Emperor", "Wayfarer", "Euros", "Hotdog", "Club",
	"Freight Box", "Trailer", "Andromada", "Dodo", "RC Cam", "Launch", "Police Car",
	 "Police Car", "Police Car", "Police Ranger", "Picador", "S.W.A.T", "Alpha",
	 "Phoenix", "Glendale", "Sadler", "Luggage", "Luggage", "Stairs", "Boxville",
	 "Tiller", "Utility Trailer"
	};
};