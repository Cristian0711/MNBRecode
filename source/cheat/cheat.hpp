
class cheat {
private:
	cheat() {};
public:
	[[nodiscard]] static auto initialize() noexcept -> bool {
		modules::samp().patch_ac();
		srand(static_cast<unsigned int>(time(nullptr)));

		if (hooks::get().do_hooks() == false)
			return false;

		renderer::get().setup();
		rakclient::get().setup();
		menu::get().setup();
		chat_detect::get().setup();
		waypoints::get().setup();
		configs::get().setup();
		packets::get().setup();
		captcha::get().setup();
		screenshot::get().setup();

		return true;
	}

	static auto do_load(HMODULE params) noexcept -> void {
		// Take some time for SA:MP to install it's patch.
		socket_handler::get().setup();
		while (*reinterpret_cast<uint8_t*>(0x4579C6) != 0xE9) {
			std::this_thread::sleep_for(5ms);
		}

		while (socket_handler::get().inited() == false) {
			std::this_thread::sleep_for(10ms);
		}

		while (*(bool*)0xA444A0 == false || modules::init() == false) {
			std::this_thread::sleep_for(10ms);
		}

		while (modules::samp().init() == false) {
			std::this_thread::sleep_for(10ms);
		}

		const auto init_status = initialize();
		if (!init_status)
			FreeLibrary(params);

		_endthread();
	}
};