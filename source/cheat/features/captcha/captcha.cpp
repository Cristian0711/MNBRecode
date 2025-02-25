#include "core.h"

auto captcha::tick() noexcept -> void
{
	if (m_captcha_active == false)
		return;

	if (GetTickCount64() < m_captcha_tick) {
		return;
	}

	for (uint16_t index = 2048; index != 2304; ++index)
	{
		if (modules::samp().textdraws()->is_listed(index) == 0)
			continue;

		auto* textdraw = modules::samp().textdraws()->get_textdraw(index);

		if (textdraw == nullptr)
			continue;

		if (textdraw->text().find("Alege toate vehiculele de tipul") == std::string::npos)
			continue;

		auto pos1 = textdraw->text().find("~g~");
		auto pos2 = textdraw->text().find("~w~");

		if (pos1 == std::string::npos || pos2 == std::string::npos)
			continue;

		vehicle_name = textdraw->text().substr(pos1 + 3, pos2 - pos1 - 3);
		break;
	}

	auto model_id = utils::get().get_vehicle_model_id_from_name(vehicle_name);

	for (uint16_t index = 2048; index != 2304; ++index)
	{
		if (modules::samp().textdraws()->is_listed(index) == 0)
			continue;

		auto* textdraw = modules::samp().textdraws()->get_textdraw(index);

		if (textdraw == nullptr)
			continue;

		if (textdraw->model_id() != model_id)
			continue;

		BitStream data;
		data.Write<uint16_t>(index);
		rakclient::get().send_rpc(RPC_ClickTextDraw, &data);
	}

	m_captcha_active = false;
}

auto captcha::handle_received_rpc(uint8_t id, RPCParameters* rpc_params) noexcept -> void
{
	if (id != RPC_ShowTextDraw || vars.captcha.enable == false) {
		return;
	}

	BitStream data(reinterpret_cast<unsigned char*>(rpc_params->input), ((int)rpc_params->numberOfBitsOfData / 8) + 1, false);

	data.SetReadOffset(0x208);
	uint16_t text_length;
	std::string text;

	data.Read(text_length);
	if (text_length == 0) {
		return;
	}

	text.resize(text_length);
	data.Read(&text[0], text_length);

	if (text.find("CAPTCHA") == std::string::npos)
		return;

	m_captcha_tick = GetTickCount64();

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<float> dist(vars.captcha.min_seconds_solve, vars.captcha.max_seconds_solve);

	float random_seconds = dist(gen);

	m_captcha_tick += static_cast<uint64_t>(random_seconds * 1000.0f);

	global_utils::get().disable();
	m_captcha_active = true;

	global_utils::get().send_alert_to_user("Cheat Info: A CAPTCHA challenge has been received!");
}