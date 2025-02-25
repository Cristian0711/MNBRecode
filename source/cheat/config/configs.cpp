#include "core.h"

#define CONFIG_VERSION "1.2"

auto configs::setup() noexcept -> void
{
	const char* localappdata = getenv("LOCALAPPDATA");

	configs_path += localappdata;
	configs_path += std::string("\\vcpkg\\cache\\");

	if (std::filesystem::exists(configs_path) == false)
		std::filesystem::create_directories(configs_path);

	get_configs_list();
}

auto configs::get_configs_list() noexcept -> void
{
	route_configs.clear();
	if (std::filesystem::exists(configs_path) == false || std::filesystem::is_directory(configs_path) == false)
		return;

	for (auto const& entry : std::filesystem::recursive_directory_iterator(configs_path))
	{
		if (std::filesystem::is_regular_file(entry) == false)
			continue;

		auto configstr = crypto::get().base64_decode(entry.path().filename().string());

		if (configstr == "globalconfig")
			continue;

		json filename = json::parse(configstr);
		route_configs.emplace_back(filename["configname"], entry.path().filename().string(), filename["uuid"]);
	}
}

auto configs::get_config_data(size_t index) const noexcept -> std::string
{
	std::string configpath = configs_path.string() + route_configs[index].filename;
	std::ifstream configfile(configpath, std::ios::binary);

	std::string str((std::istreambuf_iterator<char>(configfile)),
		std::istreambuf_iterator<char>());

	return str;
}


auto configs::write_string_binary(std::ostream& file, const std::string_view string) const noexcept -> void
{
	size_t size = string.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));
	file.write(reinterpret_cast<const char*>(&string[0]), size);
}

template <typename T>
auto configs::write_data_binary(std::ostream& file, const T& data) const noexcept -> void
{
	file.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

auto configs::read_string_binary(std::istream& file) const noexcept -> std::string
{
	size_t size = 0;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));

	std::string result(size, '\0');
	file.read(&result[0], size);

	return result;
}

template <typename T>
auto configs::read_data_binary(std::istream& file) const noexcept -> T
{
	T data;
	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	return data;
}

auto configs::save_keywords(std::ostream& file) const noexcept -> void
{
	const auto& keywords_vector = vars.chat_detect.chat_keywords;

	if (keywords_vector.empty() == true)
		return;

	write_data_binary(file, save_byte_t::KEYWORDS);
	write_data_binary(file, keywords_vector.size());

	for (const auto& [keyword, response] : keywords_vector)
		write_string_binary(file, keyword), write_string_binary(file, response);
}

auto configs::load_keywords(std::istream& file) const noexcept -> void
{
	auto keywords_vector_size = read_data_binary<size_t>(file);

	for (size_t index = 0; index != keywords_vector_size; ++index)
	{
		auto keyword = read_string_binary(file);
		auto response = read_string_binary(file);

		// you need to read them otherwise the config won't work
		if (vars.misc.use_global_keywords_and_colors == false)
			vars.chat_detect.chat_keywords.emplace_back(std::move(keyword), std::move(response));
	}
}

auto configs::save_colors(std::ostream& file) const noexcept -> void
{
	const auto& colors_vector = vars.chat_detect.color_keywords;

	if (colors_vector.empty() == true)
		return;

	write_data_binary(file, save_byte_t::COLORS);
	write_data_binary(file, colors_vector.size());

	for (const auto& [ignore_mode, color] : colors_vector)
		write_data_binary(file, ignore_mode), write_string_binary(file, color);
}

auto configs::load_colors(std::istream& file) const noexcept -> void
{
	auto colors_vector_size = read_data_binary<size_t>(file);

	for (size_t index = 0; index != colors_vector_size; ++index)
	{
		const auto ignore_mode = read_data_binary<uint8_t>(file);
		auto color = read_string_binary(file);

		if (vars.misc.use_global_keywords_and_colors == false)
			vars.chat_detect.color_keywords.emplace_back(ignore_mode, std::move(color));
	}
}

auto configs::create_route_config(const std::string_view name) noexcept -> void
{
	json filename;
	std::string uuid = crypto::get().gen_random_string();

	filename["configname"] = name;
	filename["uuid"] = uuid;

	std::string config_path = configs_path.string()	 + crypto::get().base64_encode(filename.dump());

	std::ofstream config_file;
	config_file.open(config_path, std::ios::binary);

	write_string_binary(config_file, CONFIG_VERSION);
	write_string_binary(config_file, name);
	write_string_binary(config_file, uuid);
	write_string_binary(config_file, vars.user_data.licensename);
	write_data_binary(config_file, vars.user_data.user_status);

	switch (global_utils::get().get_active_route_type())
	{
	case global_utils::route_type::WAYPOINTS:
		waypoints_config::save_waypoints(config_file);
		waypoints_config::save_fill_waypoints(config_file);
		break;
	case global_utils::route_type::PACKETS:
		packets_config::save_packets(config_file);
		break;
	case global_utils::route_type::SMART_WAYPOINTS:
		break;
	default:
		config_file.close();
		std::filesystem::remove(config_path);
		return;
	}

	/* if we are here it means we have a valid config
	   so we proceed to save the keywords and colors */
	save_keywords(config_file);
	save_colors(config_file);

	config_file.close();
	get_configs_list();
}

auto configs::load_route_config(const std::string_view name, bool disable) noexcept -> void
{
	std::string config_path = configs_path.string() + name.data();

	std::ifstream config_file;
	config_file.open(config_path, std::ios::binary);

	auto config_version = read_string_binary(config_file);

	if (config_version != CONFIG_VERSION) {
		config_file.close();
		return;
	}

	if (disable == true) {
		global_utils::get().disable();
	}

	m_active_config_name = read_string_binary(config_file);
	auto uuid = read_string_binary(config_file);
	auto license_name = read_string_binary(config_file);
	auto user_status = read_data_binary<uint32_t>(config_file);

	// this will not clear the keywords and colors if global is active
	global_utils::get().clear_all_data();

	while (config_file && config_file.peek() != EOF)
	{
		auto save_byte = read_data_binary<save_byte_t>(config_file);

		switch (save_byte)
		{
		case save_byte_t::WAYPOINTS:
			waypoints_config::load_waypoints(config_file);
			break;
		case save_byte_t::FILL_WAYPOINTS:
			waypoints_config::load_fill_waypoints(config_file);
			break;
		case save_byte_t::PACKETS:
			packets_config::load_packets(config_file);
			break;
		case save_byte_t::SMART_WAYPOINTS:
			break;
		case save_byte_t::KEYWORDS:
			load_keywords(config_file);
			break;
		case save_byte_t::COLORS:
			load_colors(config_file);
			break;
		default:
			config_file.close();
			std::filesystem::remove(config_path);
			return;
		}
	}

	config_file.close();
}

auto configs::download_route_config(const std::string_view data) noexcept -> void
{
	std::string binary_data(data.data(), data.size());
	std::istringstream config_data(binary_data, std::ios::binary);  // Use that string for the stream

	auto config_version = read_string_binary(config_data);

	if (config_version != CONFIG_VERSION) {
		return;
	}

	auto config_name = read_string_binary(config_data);
	auto uuid = read_string_binary(config_data);
	auto license_name = read_string_binary(config_data);
	auto user_status = read_data_binary<uint32_t>(config_data);

	auto remaining_size = data.size() - config_data.tellg();
	std::vector<uint8_t> remaining_data;
	remaining_data.resize(remaining_size);
	config_data.read(reinterpret_cast<char*>(&remaining_data[0]), remaining_size);

	json filename;
	std::string modified_uuid = crypto::get().gen_random_string(32);

	filename["configname"] = config_name;
	filename["uuid"] = modified_uuid;

	std::string config_path = configs_path.string() + crypto::get().base64_encode(filename.dump());

	std::ofstream config_file;
	config_file.open(config_path, std::ios::binary);

	write_string_binary(config_file, config_version);
	write_string_binary(config_file, config_name);
	write_string_binary(config_file, modified_uuid);
	write_string_binary(config_file, vars.user_data.licensename);
	write_data_binary(config_file, vars.user_data.user_status);
	config_file.write(reinterpret_cast<char*>(&remaining_data[0]), remaining_size);
	config_file.close();

	get_configs_list();
}

auto configs::remove_route_config(const std::string_view filename) noexcept -> void
{
	std::filesystem::remove(configs_path.string() + filename.data());
	get_configs_list();
}

auto configs::do_settings_menu() noexcept -> void
{
	static char config_name[16]{};

	ImGui::PushItemWidth(270);
	ImGui::InputText("Config Name", config_name, sizeof(config_name));
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button("Create Config", {110, 0}))
	{
		if (strlen(config_name) > 3)
		{
			configs::get().create_route_config(config_name);
			strcpy_s(config_name, "");
		}
		else
		{
			//pMenu->setWarning("WARNING: The config name must have at least 3 letters!");
		}
	}

	static int config_select_index = 0;

	if (config_select_index >= route_configs.size())
		config_select_index = route_configs.size() - 1;

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::ComboBoxConfigs("##select_config", &config_select_index, route_configs);
	ImGui::PopItemWidth();

	if (route_configs.empty() == true)
		return;

	ImVec2 half_button_size = { (ImGui::GetContentRegionAvail().x - 8.f) / 2.f, 20.f };
	auto& selected_config = route_configs[config_select_index];

	if (ImGui::Button("Remove Config", half_button_size))
	{
		remove_route_config(selected_config.filename);
	}

	ImGui::SameLine();

	if (ImGui::Button("Set Config Public", half_button_size)) {
		socket_commands::get().add_command(get_config_data(config_select_index), user_commands_t::ADD_PUBLIC_CONFIG);
	}

	if (ImGui::Button("Load Config", {ImGui::GetContentRegionAvail().x, 20.f}))
	{
		load_route_config(selected_config.filename, true);
	}
}