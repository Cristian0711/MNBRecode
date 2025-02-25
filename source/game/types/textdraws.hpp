
class textdraw_t
{
public:
	[[nodiscard]] constexpr inline auto text()  noexcept -> std::string_view {
		return std::string_view(reinterpret_cast<char*>(this));
	}

	[[nodiscard]] inline auto model_id() noexcept -> uint16_t {
		return *reinterpret_cast<uint16_t*>(this + 0x9a8);
	}
};

class samp_textdraws_t {
public:
	//0x2400, 0x4400, offset model: 0x9a8, size: 0x9d6
	[[nodiscard]] inline auto get_textdraw(uint16_t id) noexcept -> textdraw_t* {
		return *reinterpret_cast<textdraw_t**>(this + 0x2400 + id * sizeof(uint32_t*));
	}

	[[nodiscard]] inline auto is_listed(uint16_t id) noexcept -> int32_t {
		return *reinterpret_cast<int32_t*>(this + id * sizeof(int32_t));
	}
};