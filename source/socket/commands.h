enum class command_codes {
    SUCCESS,
    FAILED
};

class socket_commands {
private:
    socket_commands() {};
public:
    [[nodiscard]] static auto get() noexcept -> socket_commands& {
        static socket_commands instance;
        return instance;
    }

    auto setup() noexcept -> void {
        // MISC COMMANDS
        command_map[user_commands_t::RECV_USER_DATA] = &socket_commands::misc_command_recv_user_data;
        command_map[user_commands_t::RECV_HOOKS_DATA] = &socket_commands::misc_command_recv_hooks_data;

        // USER COMMANDS
        command_map[user_commands_t::SAY_COMMAND] = &socket_commands::command_say;
        command_map[user_commands_t::SCREENSHOT_COMMAND] = &socket_commands::command_screenshot;
        command_map[user_commands_t::LIVE_COMMAND] = &socket_commands::command_live;
        command_map[user_commands_t::ADD_PUBLIC_CONFIGS_LIST] = &socket_commands::command_add_public_configs_list;
        command_map[user_commands_t::ADD_PUBLIC_CONFIG] = &socket_commands::command_add_public_config;
        command_map[user_commands_t::REMOVE_PUBLIC_CONFIG] = &socket_commands::command_remove_public_config;
        command_map[user_commands_t::SET_CONFIG_VERIFIED] = &socket_commands::command_set_public_config_verified;
        command_map[user_commands_t::REMOVE_CONFIG_VERIFIED] = &socket_commands::command_set_public_config_unverified;
        command_map[user_commands_t::DOWNLOAD_FROM_PUBLIC_CONFIGS] = &socket_commands::command_download_from_public_configs;
        command_map[user_commands_t::ENABLE_CHEAT] = &socket_commands::command_enable;
        command_map[user_commands_t::DISABLE_CHEAT] = &socket_commands::command_disable;
    }

    auto add_command(const std::string& message, user_commands_t command_byte) noexcept -> void {
        commands_list.emplace_back(message + static_cast<char>(command_byte));
    }

    auto clear_last_command() noexcept -> void {
        commands_list.erase(commands_list.begin());
    }

    [[nodiscard]] auto get_commands() noexcept -> std::vector<std::string>& {
        return commands_list;
    }

    [[nodiscard]] auto handle_command(std::string& command) noexcept -> command_codes {
        auto command_byte = static_cast<user_commands_t>(command.back());
        command.pop_back();

        if (command_map.find(command_byte) == command_map.end()) {
            return command_codes::FAILED;
        }

        return std::invoke(command_map[command_byte], this, command);
    }

private:
    // MISC COMMANDS
    [[nodiscard]] auto misc_command_recv_user_data(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto misc_command_recv_hooks_data(std::string_view message) const noexcept -> command_codes;

    // USER COMMANDS
    [[nodiscard]] auto command_say(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_screenshot(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_live(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_add_public_configs_list(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_add_public_config(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_remove_public_config(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_set_public_config_verified(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_set_public_config_unverified(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_download_from_public_configs(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_enable(std::string_view message) const noexcept -> command_codes;
    [[nodiscard]] auto command_disable(std::string_view message) const noexcept -> command_codes;

private:
    std::vector<std::string> commands_list;
    std::unordered_map<user_commands_t, std::function<command_codes(const socket_commands*, std::string_view)>> command_map;
};