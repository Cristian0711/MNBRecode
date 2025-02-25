class chat_block {
private:
	chat_block() {};
public:
	enum chat_ignore_mode : uint8_t
	{
		DELETE_CHAT = 0,
		IGNORE_CHAT,
		DEFAULT
	};

	[[nodiscard]] static auto get() noexcept -> chat_block& {
		static chat_block instance;
		return instance;
	}

	[[nodiscard]] inline auto should_ignore_last_chat() const noexcept -> bool {
		return m_ignore_last_chat;
	}

	auto add_color(uint8_t ignore_mode, const std::string_view color) noexcept -> void;
	auto remove_color(const std::string_view color) noexcept -> void;

	[[nodiscard]] auto is_color_in_vector(const std::string_view color) const noexcept -> std::optional<uint8_t>;
	[[nodiscard]] auto should_delete_last_chat() noexcept -> bool;
private:
	bool m_ignore_last_chat = false;
};