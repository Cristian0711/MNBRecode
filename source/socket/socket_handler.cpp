#include "core.h"
#include "vendor/obfy/instr.h"
#include "hwid.hpp"

#define CLIENT_CONNECTION           "\x70"

auto socket_handler::start_connection_with_server() noexcept -> bool
{
    //WSADATA info;
    //if (WSAStartup(MAKEWORD(2, 2), &info)) {
    //    return false;
    //}

    //std::string error;
    //struct addrinfo* result = NULL, * ptr = NULL, hints;

    //ZeroMemory(&hints, sizeof(hints));
    //hints.ai_family = AF_UNSPEC;
    //hints.ai_socktype = SOCK_STREAM;
    //hints.ai_protocol = IPPROTO_TCP;

    //int iResult = getaddrinfo("45.83.244.168", "55566", &hints, &result);
    //if (iResult != 0) {
    //    return false;
    //}

    //for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    //    server_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    //    if (server_socket == INVALID_SOCKET) {
    //        return false;
    //    }

    //    iResult = connect(server_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
    //    if (iResult == SOCKET_ERROR) {
    //        closesocket(server_socket);
    //        server_socket = INVALID_SOCKET;
    //        continue;
    //    }
    //    break;
    //}

    //freeaddrinfo(result);

    // Return false if no connection was established
    //if (server_socket == INVALID_SOCKET) {
    //    return false;
    //}

     WSADATA info;
    if (WSAStartup(MAKEWORD(2, 2), &info)) {
        //throw std::exception(xorstr_("Could not start WSA"));
        return false;
    }

    hostent* he;
    if ((he = gethostbyname("localhost")) == 0) {
        LOG("Unable to get host by name!");
        //throw std::exception(xorstr_("Unable to get host by name!"));
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(55566);
    addr.sin_addr = *((in_addr*)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (::connect(server_socket, (sockaddr*)&addr, sizeof(sockaddr))) {
        LOG("Unable to connect to the socket!");
        //throw std::exception(xorstr_("Unable to get host by name!"));
        return false;
    }

    if (server_socket == INVALID_SOCKET) {
        LOG("Unable to connect to server!");
        //throw std::exception(xorstr_("Unable to connect to server!"));
        return false;
    }


    socket_utils::get().set_server_socket(server_socket);
    return true;
}

auto socket_handler::handshake_with_server_and_get_aes_key() noexcept -> return_status
{
    CryptoPP::RSA::PrivateKey   private_key;
    CryptoPP::RSA::PublicKey    public_key;

    const auto& crypto = crypto::get();
    crypto.gen_rsa_key_pair(private_key, public_key);

    auto public_key_in_der_format = crypto::get().get_public_key_in_der_format(public_key);
    socket_utils::get().send_message(public_key_in_der_format);

    auto rsa_encrypted_aes_key = socket_utils::get().receive_message();

    if (std::holds_alternative<socket_codes>(rsa_encrypted_aes_key)) {
        return return_status::FAILED_TO_INIT;
    }

    crypto.decrypt_rsa_message(private_key, std::get<std::string>(rsa_encrypted_aes_key), aes_key);
    socket_utils::get().set_aes_key(aes_key);

    return return_status::SUCCESS;
}

auto socket_handler::receive_and_process_command(CommandRelevance relevance, user_commands_t command_byte) noexcept -> socket_codes {
    auto command_data = socket_utils::get().receive_aes_message();

    if (std::holds_alternative<std::string>(command_data)) {
        std::string& command = std::get<std::string>(command_data);

        if (command == "HEARTBEAT")
            return socket_codes::SUCCESS;

        if (relevance == CommandRelevance::HIGH && command.back() != static_cast<uint8_t>(command_byte)) {
            return socket_codes::TERMINATE;
        }

        return socket_commands::get().handle_command(command) == command_codes::SUCCESS ? socket_codes::SUCCESS : socket_codes::FAILED;
    }
    else {
        return relevance == CommandRelevance::HIGH ? socket_codes::TERMINATE : std::get<socket_codes>(command_data);
    }
}

auto socket_handler::heartbeat_thread() noexcept -> socket_codes
{
    static auto send_tick = GetTickCount64();

    socket_codes status;
    if (socket_utils::get().is_data_available_to_read() == socket_codes::SUCCESS) {
        status = receive_and_process_command(CommandRelevance::NORMAL, user_commands_t::DEFAULT);

        if (status != socket_codes::SUCCESS)
            return status;
    }

    if (GetTickCount64() > send_tick + 10000) {
        status = socket_utils::get().send_aes_message("HEARTBEATCLIENT");

        if (status != socket_codes::SUCCESS)
            return status;

        send_tick = GetTickCount64();
    }

    if (socket_commands::get().get_commands().empty() == false) {
        status = socket_utils::get().send_aes_message(socket_commands::get().get_commands().at(0));

        if (status != socket_codes::SUCCESS)
            return status;

        socket_commands::get().clear_last_command();
    }

    return socket_codes::SUCCESS;
}

auto socket_handler::handle_connection() noexcept -> return_status
{
    if (start_connection_with_server() == false) {
        return return_status::FAILED_TO_CONNECT;
    }

    if (handshake_with_server_and_get_aes_key() != return_status::SUCCESS) {
        return return_status::FAILED_TO_INIT;
    }

    auto status = socket_utils::get().send_aes_message(CLIENT_CONNECTION);
    if (status != socket_codes::SUCCESS)
        return return_status::FAILED_TO_INIT;

    status = socket_utils::get().send_aes_message(hwid::get().get_hwid());
    if (status != socket_codes::SUCCESS)
        return return_status::FAILED_TO_INIT;

    status = receive_and_process_command(CommandRelevance::HIGH, user_commands_t::RECV_USER_DATA);
    if (status != socket_codes::SUCCESS) {
        return return_status::INVALID_HWID;
    }

    status = receive_and_process_command(CommandRelevance::HIGH, user_commands_t::RECV_HOOKS_DATA);
    if (status != socket_codes::SUCCESS)
        return return_status::FAILED_TO_INIT;

    m_inited = true;
    while(modules::inited() == false)
        std::this_thread::sleep_for(20ms);

    while (modules::samp().inited() == false)
        std::this_thread::sleep_for(20ms);

    status = socket_utils::get().send_aes_message(modules::samp().local_player_name().data());
    if (status != socket_codes::SUCCESS)
        return return_status::FAILED_TO_INIT;

    status = receive_and_process_command(CommandRelevance::HIGH, user_commands_t::ADD_PUBLIC_CONFIGS_LIST);
    if (status != socket_codes::SUCCESS)
        return return_status::FAILED_TO_INIT;

    while (true) {
        if (heartbeat_thread() != socket_codes::SUCCESS)
            return return_status::DISCONNECTED;

        std::this_thread::sleep_for(100ms);
    }

    return return_status::SUCCESS;
}

auto socket_handler::tick() noexcept -> void
{
    while (true)
    {
        auto status = handle_connection();
        if (status == return_status::INVALID_HWID) {
            TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, GetCurrentProcessId()), 0);
        }
        closesocket(server_socket);
        aes_key.clear();

        std::this_thread::sleep_for(5s);
    }
}