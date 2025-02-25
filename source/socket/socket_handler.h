struct user_data_t
{
	std::string_view name;
	std::string_view dev;
	std::string_view hwid;
	std::string_view discord;
	uint8_t panicByte = 0;
};

class socket_handler {
private:
	socket_handler() {};
public:
	enum class return_status : uint8_t {
		SUCCESS = 0,
		DISCONNECTED,
		FAILED_TO_CONNECT,
		INVALID_HWID,
		FAILED_TO_INIT
	};


	[[nodiscard]] static auto get() noexcept -> socket_handler& {
		static socket_handler instance;
		return instance;
	}

	inline auto setup() noexcept -> void {
		socket_commands::get().setup();
		std::thread(&socket_handler::tick, this).detach();
	}

	inline auto inited() const noexcept -> bool {
		return m_inited;
	}

private:
	auto tick() noexcept -> void;

	[[nodiscard]] auto start_connection_with_server() noexcept -> bool;
	[[nodiscard]] auto handle_connection() noexcept -> return_status;
	[[nodiscard]] auto handshake_with_server_and_get_aes_key() noexcept -> return_status;

	[[nodiscard]] auto heartbeat_thread() noexcept -> socket_codes;
	[[nodiscard]] auto receive_and_process_command(CommandRelevance relevance, user_commands_t command_byte) noexcept -> socket_codes;

private:
	SOCKET server_socket;

	std::string aes_key;
	std::shared_ptr<user_data_t> user_data{ nullptr };

	bool m_inited{ false };

};