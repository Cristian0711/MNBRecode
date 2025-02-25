class collisions {
private:
	collisions() {};
public:
	[[nodiscard]] static auto get() noexcept -> collisions& {
		static collisions instance;
		return instance;
	}

	[[nodiscard]] auto do_hooks() noexcept -> bool {
		const auto& samp = modules::samp();

		process_collision_retn = samp.get_offsets()->process_collisions();
		if (detours_wrapper::get().hook(process_collision_retn, hook_process_car_collision) == false)
			return false;

		o_process_surfing = reinterpret_cast<process_surfing_t>(samp.get_base() + samp.get_offsets()->process_surfing());
		if (detours_wrapper::get().hook(o_process_surfing, hook_process_surfing) == false)
			return false;

		return true;
	}

	static auto hook_process_car_collision() noexcept -> void;
	static auto __fastcall hook_process_surfing(void* _this, void* edx) -> int32_t;

private:
	typedef int(__thiscall* process_surfing_t)(void*);

	process_surfing_t o_process_surfing{ nullptr };
	uint32_t process_collision_retn{ 0 };
};