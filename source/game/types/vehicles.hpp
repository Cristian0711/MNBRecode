class samp_vehicles_t {
public:
	[[nodiscard]] inline auto get_vehicle(uint16_t id) noexcept -> CVehicle* {
		return *reinterpret_cast<CVehicle**>(this + g_samp_offsets->gta_vehicle_list_offset() + id * sizeof(CVehicle*));
	}
};