struct screenshot_t {
	std::size_t m_width, m_height, m_pitch;
	D3DFORMAT m_format;
	std::uint8_t* m_pixels = nullptr;

public:
	~screenshot_t(void) { delete[] m_pixels; };

	[[nodiscard]] auto copy_surface_pixels(LPDIRECT3DSURFACE9 surface) noexcept -> HRESULT;
	[[nodiscard]] auto save_pixels_to_disk(const std::string_view filename, D3DXIMAGE_FILEFORMAT file_format) noexcept -> HRESULT;
	[[nodiscard]] auto save_pixels_to_memory(std::string& buffer, D3DXIMAGE_FILEFORMAT file_format, UINT new_width, UINT new_height) noexcept -> HRESULT;

private:
	[[nodiscard]] auto ansi_to_unicode(std::string_view ansiStr, UINT codePage = CP_ACP) const noexcept -> std::wstring;
	[[nodiscard]] auto d3dformat_to_wic_guid(D3DFORMAT format) const noexcept -> const GUID*;
};

class screenshot {
private:
	screenshot() {};
public:
	[[nodiscard]] static auto get() noexcept -> screenshot& {
		static screenshot instance;
		return instance;
	}

	inline auto setup() noexcept -> void
	{
		callbacks::get().add_callback(callbacks::callback_type::every_frame, [this]() {
			tick();
			});
	}

	inline auto take_screenshot() noexcept -> void {
		m_take_screenshot = true;
	}

	inline auto pause_or_start_live() noexcept -> void {
		m_live_active = !m_live_active;
	}

private:
	auto tick() noexcept -> void;

	auto get_screenshot_to_bytes(IDirect3DDevice9* pDevice, std::unique_ptr<screenshot_t>& screenshot) noexcept -> bool;
	void save_screenshot_to_file_thread(std::unique_ptr<screenshot_t> screenshot, const std::string& filename) noexcept;
	void save_screenshot_to_file(std::unique_ptr<screenshot_t> screenshot, std::string_view filename) noexcept;
	void send_screenshot_to_discord_thread(std::unique_ptr<screenshot_t> screenshot) noexcept;
	void send_screenshot_to_discord(std::unique_ptr<screenshot_t> screenshot) noexcept;

private:
	bool m_take_screenshot{ false };
	bool m_live_active{ false };
};