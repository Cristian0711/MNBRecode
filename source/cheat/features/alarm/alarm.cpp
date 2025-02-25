#include "core.h"

// BEEP IS NOT ASYNC
auto alarm::beep_thread() noexcept -> void
{
	Beep(1568, 200);
	Beep(1568, 200);
	Beep(1568, 200);
	Beep(1568, 200);
	Beep(1245, 1000);
	Beep(1397, 200);
	Beep(1397, 200);
	Beep(1397, 200);
	Beep(1175, 1000);
	m_alarm_active = false;
}

auto alarm::custom_alarm_thread() noexcept -> void
{
	PlaySoundA(vars.alarm.custom_alarm_path, NULL, SND_FILENAME | SND_SYNC);
	m_alarm_active = false;
}

auto alarm::start_alarm() noexcept -> void
{
	if (vars.alarm.enable == false)
		return;

	if (global_utils::get().any_route_enabled() == false)
		return;

	if (m_alarm_active == true)
		return;

	m_alarm_active = true;
	if (vars.alarm.use_custom_alarm == false)
		std::thread(&alarm::beep_thread, this).detach();
	else
		std::thread(&alarm::custom_alarm_thread, this).detach();
}