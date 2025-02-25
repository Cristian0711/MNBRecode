class public_configs {
private:
	public_configs() {};
public:
	class public_config_t
	{
	public:
		public_config_t(std::string_view _name, std::string_view _uuid, std::string_view _hwid, std::string_view _licensename, int _userStatus, int _verified, int _routeType)
			: name(_name), uuid(_uuid), hwid(_hwid), licensename(_licensename), userStatus(_userStatus), verified(_verified), routeType(_routeType)
		{

		}

		std::string name;
		std::string uuid;
		std::string hwid;
		std::string licensename;

		int32_t verified;
		int32_t userStatus;
		int32_t routeType;

		[[nodiscard]] auto text() const noexcept -> std::string {
			json config;
			config["configname"] = name;
			config["uuid"] = uuid;
			return config.dump();
		}
	};

	[[nodiscard]] static auto get() noexcept -> public_configs& {
		static public_configs instance;
		return instance;
	}

	inline auto clear() noexcept -> void {
		vec_public_configs.clear();
	}

	inline auto configs() const noexcept -> const std::vector<public_config_t>& {
		return vec_public_configs;
	}
	
	inline auto add_config(json& config) noexcept -> void {
		if (!(config.contains("configname")) && config["configname"].is_string() &&
			config.contains("uuid") && config["uuid"].is_string() &&
			config.contains("hwid") && config["hwid"].is_string() &&
			config.contains("licensename") && config["licensename"].is_string() &&
			config.contains("number") && config["number"].is_number_integer() &&
			config.contains("verified") && config["verified"].is_number_integer() &&
			config.contains("routetype") && config["routetype"].is_number_integer()) {
			return;
		}

		vec_public_configs.emplace_back(
			config["configname"].get<std::string>(),
			config["uuid"].get<std::string>(),
			config["hwid"].get<std::string>(),
			config["licensename"].get<std::string>(),
			config["number"].get<int32_t>(),
			config["verified"].get<int32_t>(),
			config["routetype"].get<int32_t>()
		);
	}

	inline auto remove_config(std::string_view uuid) noexcept -> void {
		std::erase_if(vec_public_configs, [&uuid](const auto& config) {
			return config.uuid == uuid;
			});
	}

	inline auto set_verified(std::string_view uuid, bool verified) noexcept -> void {
		for (auto& config : vec_public_configs) {
			if (config.uuid == uuid) {
				config.verified = verified;
				break;
			}
		}
	}

	auto do_settings_menu() noexcept -> void {
		if (ImGui::BeginChild("##Configs", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - ImGui::GetContentRegionAvail().y / 4.f}, true))
		{
			for (auto& current_config : vec_public_configs)
			{
				float hoverSize = ImGui::GetContentRegionAvail().x;
				ImGui::PushID(current_config.uuid.c_str());

				if (ImGui::BeginChild("##Config", { ImGui::GetContentRegionAvail().x, 30 }, true))
				{
					if (current_config.routeType == 1)
						ImGui::PushStyleColor(ImGuiCol_Text, vars.style.tab_menu_color);

					ImGui::Text(current_config.name.c_str());
					ImGui::SameLine(120);
					ImGui::Text(current_config.licensename.c_str());
					ImGui::SameLine(240);
					if (current_config.userStatus == 2)
						ImGui::Text("Admin");
					else if (current_config.userStatus == 1)
						ImGui::Text("Support");
					else
						ImGui::Text("User");

					ImGui::SameLine(360);
					ImGui::Text(current_config.verified ? "Verified" : "Not verified");

					if (current_config.routeType == 1)
						ImGui::PopStyleColor();
				}
				ImGui::EndChild();

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
				{
					ImGui::OpenPopup("configpopup");
				}

				if (ImGui::BeginPopup("configpopup"))
				{
					if (current_config.hwid == vars.user_data.hwid || vars.user_data.user_status == 2) {
						if (ImGui::Button("Remove", { 90, 0 }))
							socket_commands::get().add_command(current_config.text(), user_commands_t::REMOVE_PUBLIC_CONFIG);
					}

					if (vars.user_data.user_status > 0 && ImGui::Button(current_config.verified ? "Unverify" : "Verify", { 90, 0 })) {
						socket_commands::get().add_command(current_config.text(), current_config.verified ? user_commands_t::REMOVE_CONFIG_VERIFIED : user_commands_t::SET_CONFIG_VERIFIED);
					}

					if (ImGui::Button("Download", { 90, 0 })) {
						socket_commands::get().add_command(current_config.text(), user_commands_t::DOWNLOAD_FROM_PUBLIC_CONFIGS);
					}
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}
			ImGui::EndChild();
		}
	}

private:
	std::vector<public_config_t> vec_public_configs;
};