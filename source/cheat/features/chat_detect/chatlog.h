class chatlog {
public:
	enum class chat_log_type : uint8_t
	{
		TYPE_TEXT = 1,
		TYPE_COLOR,
		DEFAULT // used only for constructing
	};
private:
	chatlog() {};

	struct chat_log_line_t {
		chat_log_line_t(chat_log_type type, ImVec4 color, const std::string_view data) noexcept
			: type(type), color(color), data(data) {};

		// USED FOR TEXT
		chat_log_line_t(chat_log_type type, const std::string_view data) noexcept
			: type(type), data(data) {};

		chatlog::chat_log_type type;
		ImVec4	color;
		std::string data;
	};

	typedef std::vector<chat_log_line_t> line_data_t;
public:

	[[nodiscard]] static auto get() noexcept -> chatlog& {
		static chatlog instance;
		return instance;
	}

	[[nodiscard]] auto get_last_entry() const noexcept -> std::optional<std::reference_wrapper<const line_data_t>>
	{
		if (chatlog_data.empty()) {
			return std::nullopt;
		}

		return std::cref(chatlog_data.back());
	}

	auto do_chatlog_menu() const noexcept -> void;
	auto add_to_chat_log(int32_t type, const std::string_view text, const std::string_view prefix, uint32_t text_color, uint32_t prefix_color) noexcept -> void;

private:
	auto draw_color_tree_list() const noexcept -> void;
	auto draw_chat_log_child() const noexcept -> void;

	auto add_color_to_line_data(line_data_t& data, uint32_t color) noexcept -> void;
	auto add_color_to_line_data(line_data_t& data, const std::string_view color) noexcept -> void;
	auto add_text_to_line_data(line_data_t& data, const std::string_view text) noexcept -> void;

private:
	std::vector<line_data_t> chatlog_data;
};