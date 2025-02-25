class rendering {
private:
	rendering() {};
public:
	[[nodiscard]] static auto get() noexcept -> rendering& {
		static rendering instance;
		return instance;
	}

	[[nodiscard]] inline auto d3d_device() const noexcept -> IDirect3DDevice9* {
		return m_d3d_device;
	}

	inline auto set_d3d_device(IDirect3DDevice9* device) noexcept -> void
	{
		m_d3d_device = device;
	}

	[[nodiscard]] auto do_hooks() noexcept -> bool {
		const auto& samp = modules::samp();

		present_retn = samp.get_base() + samp.get_offsets()->present();
		if (detours_wrapper::get().hook(present_retn, present) == false)
			return false;

		reset_retn = samp.get_base() + samp.get_offsets()->reset();
		if (detours_wrapper::get().hook(reset_retn, reset) == false)
			return false;

		original_wndproc = reinterpret_cast<WNDPROC>(samp.get_base() + samp.get_offsets()->wndproc());
		if (detours_wrapper::get().hook(original_wndproc, wndproc) == false)
			return false;

		return true;
	}

private:
	static auto present() noexcept -> void; // __declspec(naked)
	static auto reset() noexcept -> void;	// __declspec(naked)
	static auto __stdcall wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT;

	static auto execute_frame_operations() noexcept -> void;

private:
	bool m_initialized { false };

	WNDPROC  original_wndproc { nullptr };
	uint32_t present_retn { 0 };
	uint32_t reset_retn { 0 };

	IDirect3DDevice9* m_d3d_device{ nullptr };
};