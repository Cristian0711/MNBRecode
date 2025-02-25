enum class socket_codes {
    SUCCESS,
    CONNECTION_CLOSED,
    RECEIVE_ERROR,
    INVALID_START_DELIMITER,
    INVALID_END_DELIMITER,
    ENCRYPTION_ERROR,
    DECRYPTION_ERROR,
    NO_DATA_TO_READ,
    TERMINATE,
    FAILED
};

enum class CommandRelevance {
    NORMAL,
    HIGH
};

enum class user_commands_t : uint8_t {
    // user based commands
    DEFAULT = '\x0',
    SCREENSHOT_COMMAND = '\x1',
    SAY_COMMAND = '\x2',
    LIVE_COMMAND = '\x3',
    ADD_PUBLIC_CONFIGS_LIST = '\x4',
    ADD_PUBLIC_CONFIG = '\x5',
    REMOVE_PUBLIC_CONFIG = '\x6',
    SET_CONFIG_VERIFIED = '\x7',
    REMOVE_CONFIG_VERIFIED = '\x8',
    DOWNLOAD_FROM_PUBLIC_CONFIGS = '\x9',
    CHAT_DISCORD_ALERT = '\x10',
    SEND_GLOBAL_CHAT = '\x11',
    ENABLE_CHEAT = '\x12',
    DISABLE_CHEAT = '\x13',
    ADMIN_DISCORD_ALERT = '\x14',

    // misc commands
    RECV_HOOKS_DATA = '\x50',
    INVALID_CREDENTIALS = '\x51',
    RECV_USER_DATA = '\x52',

    // used to send what kind of connection is this
    CLIENT_CONNECTION = '\x70'

};

enum return_status : uint8_t {
    SUCCESS = 0,
    DISCONNECTED,
    NO_DATA_AVAILABLE,
    FAILED_TO_CONNECT,
    INVALID_HWID,
    FAILED_TO_INIT,
    MORE_STEPS_UNTIL_FINISHED,
    EXIT
};


using MessageResult = std::variant<std::string, socket_codes>;
using SendResult = std::optional<socket_codes>;

class socket_utils {
private:
    socket_utils() {};
public:
    [[nodiscard]] static auto get() -> socket_utils& {
        static socket_utils instance;
        return instance;
    }

    constexpr static uint32_t start_frame_delimiter = 0xABABABAB;
    constexpr static uint32_t end_frame_delimiter = 0xBABABABA;

    auto set_aes_key(const std::string& aes_key) noexcept {
        m_aes_key = aes_key;
    }

    auto set_server_socket(const SOCKET server_socket) noexcept {
        m_server_socket = server_socket;
    }

    auto send_message(const std::string& message) const noexcept -> socket_codes {
        uint32_t size_network_order = message.size(); // little-endian
        std::cout << "SENT SIZE: " << size_network_order << '\n';

        if (!send_all(&start_frame_delimiter, sizeof(start_frame_delimiter)) ||
            !send_all(&size_network_order, sizeof(size_network_order)) ||
            !send_all(message.data(), size_network_order) ||
            !send_all(&end_frame_delimiter, sizeof(end_frame_delimiter))) {
            return socket_codes::CONNECTION_CLOSED;
        }

        std::cout << "SENT SUCCESS: " << size_network_order << '\n';
        return socket_codes::SUCCESS; // Success, no error
    }

    [[nodiscard]] auto send_aes_message(const std::string& message) const noexcept -> socket_codes {
        CryptoPP::SecByteBlock   iv(CryptoPP::AES::BLOCKSIZE);
        auto striv = crypto::get().generate_iv(iv);

        auto encrypted_message =  crypto::get().aes_encrypt(message, m_aes_key, iv);
        auto final_message = striv + encrypted_message;

        return send_message(final_message);
    }

    [[nodiscard]] auto receive_message() const noexcept -> MessageResult {
        uint32_t m_start_frame_delimiter;
        if (!receive_all(&m_start_frame_delimiter, sizeof(m_start_frame_delimiter))) {
            return socket_codes::CONNECTION_CLOSED;
        }
        if (m_start_frame_delimiter != start_frame_delimiter) {
            return socket_codes::INVALID_START_DELIMITER;
        }

        uint32_t size_network_order;
        if (!receive_all(&size_network_order, sizeof(size_network_order))) {
            return socket_codes::CONNECTION_CLOSED;
        }

        uint32_t message_size = size_network_order;
        std::string message(message_size, '\0');
        if (!receive_all(&message[0], message_size)) {
            return socket_codes::RECEIVE_ERROR;
        }

        uint32_t m_end_frame_delimiter;
        if (!receive_all(&m_end_frame_delimiter, sizeof(m_end_frame_delimiter))) {
            return socket_codes::CONNECTION_CLOSED;
        }
        if (m_end_frame_delimiter != end_frame_delimiter) {
            return socket_codes::INVALID_END_DELIMITER;
        }

        return message;
    }

    [[nodiscard]] auto receive_aes_message() const noexcept -> MessageResult {
        auto result = receive_message();

        if (std::holds_alternative<socket_codes>(result)) {
            return result;
        }

        std::string encrypted_message = std::get<std::string>(result);
        return crypto::get().aes_decrypt_with_iv(encrypted_message, m_aes_key);
    }

    [[nodiscard]] auto is_data_available_to_read() const noexcept -> socket_codes {
        u_long buffer_size = 0;
        if (ioctlsocket(m_server_socket, FIONREAD, &buffer_size) == SOCKET_ERROR)
            return socket_codes::CONNECTION_CLOSED;

        if (buffer_size == 0)
            return socket_codes::NO_DATA_TO_READ;

        return socket_codes::SUCCESS;
    }

    auto wait_for_aes_message() const noexcept -> MessageResult {
        auto status = is_data_available_to_read();
        while (status != socket_codes::SUCCESS) {
            if (status == socket_codes::CONNECTION_CLOSED) {
                return status;
            }
            status = is_data_available_to_read();
        }

        return receive_aes_message();
    }

private:
    SOCKET m_server_socket;
    std::string m_aes_key;

    [[nodiscard]] auto send_all(const void* data, size_t len) const noexcept -> bool {
        const char* ptr = static_cast<const char*>(data);
        size_t sent = 0;
        while (sent < len) {
            int32_t result = send(m_server_socket, ptr + sent, len - sent, 0);
            if (result <= 0) {
                return false;
            }
            sent += result;
        }
        return true;
    }

    [[nodiscard]] auto receive_all(void* data, size_t len) const noexcept -> bool {
        char* ptr = static_cast<char*>(data);
        size_t received = 0;
        while (received < len) {
            int32_t result = recv(m_server_socket, ptr + received, len - received, 0);
            if (result <= 0) {
                return false;
            }
            received += result;
        }
        return true;
    }
};
