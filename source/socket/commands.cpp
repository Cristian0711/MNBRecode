#include "core.h"

auto socket_commands::misc_command_recv_user_data(std::string_view message) const noexcept -> command_codes {
    json data = json::parse(message);

    for (auto& [key, value] : data.items()) {
        if (key == "DEV" && value.is_string()) {
            vars.user_data.user_status = std::stoi(value.get<std::string>());
        }
        else if (key == "NAME" && value.is_string()) {
            vars.user_data.licensename = value.get<std::string>();
        }
        else if (key == "HWID" && value.is_string()) {
            vars.user_data.hwid = value.get<std::string>();
        }
        else if (key == "DISCORD" && value.is_string()) {
            vars.user_data.discord_id = value.get<std::string>();
        }
        else {
            return command_codes::FAILED;
        }
    }

    return command_codes::SUCCESS;
}

auto socket_commands::misc_command_recv_hooks_data(std::string_view message) const noexcept -> command_codes {
    offsets.set_offsets(message);
    return command_codes::SUCCESS;
}

// USER COMMANDS
auto socket_commands::command_say(std::string_view message) const noexcept -> command_codes {
    modules::samp().local_player()->send(message);
    return command_codes::SUCCESS;
}

auto socket_commands::command_screenshot(std::string_view message) const noexcept -> command_codes {
    screenshot::get().take_screenshot();
    return command_codes::SUCCESS;
}

auto socket_commands::command_live(std::string_view message) const noexcept -> command_codes {
    screenshot::get().pause_or_start_live();
    return command_codes::SUCCESS;
}

auto socket_commands::command_add_public_configs_list(std::string_view message) const noexcept -> command_codes {
    public_configs::get().clear();
    json configs = json::parse(message);

    for (const auto& item : configs.items()) {
        public_configs::get().add_config(item.value());
    }

    return command_codes::SUCCESS;
}

auto socket_commands::command_add_public_config(std::string_view message) const noexcept -> command_codes {
    json configs = json::parse(message);
    public_configs::get().add_config(configs);
    return command_codes::SUCCESS;
}

auto socket_commands::command_remove_public_config(std::string_view message) const noexcept -> command_codes {
    public_configs::get().remove_config(message);
    return command_codes::SUCCESS;
}

auto socket_commands::command_set_public_config_verified(std::string_view message) const noexcept -> command_codes {
    public_configs::get().set_verified(message, true);
    return command_codes::SUCCESS;
}

auto socket_commands::command_set_public_config_unverified(std::string_view message) const noexcept -> command_codes {
    public_configs::get().set_verified(message, false);
    return command_codes::SUCCESS;
}

auto socket_commands::command_download_from_public_configs(std::string_view message) const noexcept -> command_codes {
    configs::get().download_route_config(message);
    return command_codes::SUCCESS;
}

auto socket_commands::command_enable(std::string_view message) const noexcept -> command_codes {
    global_utils::get().enable();
    return command_codes::SUCCESS;
}

auto socket_commands::command_disable(std::string_view message) const noexcept -> command_codes {
    global_utils::get().disable();
    return command_codes::SUCCESS;
}