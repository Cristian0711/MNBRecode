#include "core.h"

class hooks {
private:
	hooks() {};
public:
	[[nodiscard]] static auto get() noexcept -> hooks& {
		static hooks instance;
		return instance;
	}

	[[nodiscard]] auto do_hooks() noexcept -> bool {
		rendering::get().set_d3d_device(*reinterpret_cast<IDirect3DDevice9**>(0xC97C28));

		const auto& detour_wrapper = detours_wrapper::get();

		if (!detour_wrapper.start_transaction())
			return false;

		if (!rendering::get().do_hooks())
			return false;

		if (!chat_detect::get().do_hooks())
			return false;

		if (!rakclient::get().do_hooks())
			return false;

		if (!packets::get().do_hooks())
			return false;

		if (!collisions::get().do_hooks())
			return false;

		if (!detour_wrapper.commit_transaction())
			return false;

		return true;
	}
};