#include "core.h"

class samp_info_t {
public:
	inline auto toggle_cursor(bool enabled) const noexcept
	{
		ImGui::GetIO().MouseDrawCursor = enabled;
		((void(__thiscall*) (const samp_info_t*, int, bool)) (g_samp_module + g_samp_offsets->func_set_cursor_mode()))(this, enabled ? 4 : 0, !enabled);
	}

	inline auto lock_actor_cam() const noexcept {
		((void(__stdcall*) (void*)) (g_samp_module + g_samp_offsets->func_lock_actor_cam()));
	}

	inline auto unlock_actor_cam() const noexcept {
		((void(__thiscall*) (const samp_info_t*)) (g_samp_module + g_samp_offsets->func_unlock_actor_cam()))(this);
	}
};