#include "core.h"

auto rendering::execute_frame_operations() noexcept -> void
{
	callbacks::get().call_callbacks(callbacks::callback_type::every_frame);

	if (FrontEndMenuManager.m_bMenuActive)
		return;

	renderer::get().begin_frame();
	callbacks::get().call_callbacks(callbacks::callback_type::render);

	if (menu::get().open())
		renderer::get().render(&menu::draw, &menu::get());

	renderer::get().end_frame();
}

auto __declspec(naked) rendering::present() noexcept -> void
{
	__asm pushad;
	static auto return_address = rendering::get().present_retn;

	// CALL execute_frame_operations, DO NOT DIRECTLY EXECUTE OTHERWISE STD::INVOKE WILL BREAK REGISTERS
	if (rendering::get().m_d3d_device != nullptr && renderer::get().is_initialized())
		execute_frame_operations();

	__asm popad;
	__asm jmp [return_address];
}

auto __declspec(naked) rendering::reset() noexcept -> void
{
	__asm pushad;
	static auto return_address = rendering::get().reset_retn;

	if (renderer::get().is_initialized())
		ImGui_ImplDX9_InvalidateDeviceObjects();
	__asm popad;

	__asm jmp [return_address];
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
auto __stdcall rendering::wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT 
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (menu::get().open()) {
		if (wParam == 'W' || wParam == 'w' ||
			wParam == 'A' || wParam == 'a' ||
			wParam == 'S' || wParam == 's' ||
			wParam == 'D' || wParam == 'd')
		{
			rendering::get().original_wndproc(hWnd, uMsg, wParam, lParam);
		}
		
		return true;
	}

	return rendering::get().original_wndproc(hWnd, uMsg, wParam, lParam);
}