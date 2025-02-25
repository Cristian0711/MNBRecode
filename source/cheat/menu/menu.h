
class menu {
private:
	menu() {};
public:
	[[nodiscard]] static auto get() -> menu& {
		static menu instance;
		return instance;
	}

	[[nodiscard]] inline auto open() const noexcept -> bool {
		return m_open;
	}

	[[nodiscard]] inline auto font() const noexcept -> const ImFont* {
		return m_font;
	}

	[[nodiscard]] inline auto enable_button_text() const noexcept -> std::string_view {
		return m_enable_button_text;
	}

	auto set_enable_button_text(bool enabled) noexcept -> void {
		m_enable_button_text = enabled ? "Disable auto clicker" : "Enable auto clicker";
	}

	auto setup_style() noexcept -> void;
	auto setup() noexcept -> void;
	auto draw() const noexcept -> void;

private:
	ImFont* m_font{ nullptr };
	bool m_open { false };
	std::string m_enable_button_text{ "Enable auto clicker" };
};