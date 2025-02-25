class samp_dialog_t {
public:
	inline auto hide() noexcept {
		((void*(__thiscall*) (samp_dialog_t*)) (g_samp_module + g_samp_offsets->func_hide_dialog()))(this);
	}
};