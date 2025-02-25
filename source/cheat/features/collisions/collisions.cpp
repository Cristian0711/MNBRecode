#include "core.h"

auto _declspec (naked) collisions::hook_process_car_collision() noexcept -> void
{
	__asm pushad
	static uint32_t RETURN_PlayerCollision_ovrwr = 0x54CEFC;
	static uint32_t RETURN_PlayerCollision_process = 0x54BCDC;
	static uint32_t RETURN_PlayerCollision_noProcessing = 0x54CF8D;
	static uint32_t PlayerCollision_eax_back, PlayerCollision_ebx_back, PlayerCollision_ecx_back;
	static uint32_t PlayerCollision_edi_back, PlayerCollision_esi_back, PlayerCollision_edx_back;
	static uint32_t PlayerCollision_esp_back;
	static uint32_t PlayerCollision_tmp;

	static CVehicle* localVehicle = nullptr;
	static CEntity* vehicle1 = nullptr;
	static CEntity* vehicle2 = nullptr;
	__asm popad

	__asm test byte ptr[esi + 0x1c], 1
	__asm jne hk_PlCol_process
	__asm jmp RETURN_PlayerCollision_ovrwr

	hk_PlCol_process :
	__asm mov PlayerCollision_esi_back, esi
	__asm mov PlayerCollision_edi_back, edi

	__asm mov PlayerCollision_eax_back, eax
	__asm mov PlayerCollision_ebx_back, ebx
	__asm mov PlayerCollision_ecx_back, ecx
	__asm mov PlayerCollision_edx_back, edx
	__asm mov PlayerCollision_esp_back, esp

	__asm pushad

	// already crashed, if true
	if (PlayerCollision_edi_back == NULL || PlayerCollision_esi_back == NULL)
		goto hk_PlCol_noCol;

	// No vehicle collisions
	if (vars.local_vehicle && (global_utils::get().any_record_enabled() && vars.collisions.use_when_recording) ||
		(global_utils::get().any_route_enabled() && vars.collisions.enable))
	{
		vehicle1 = (CEntity*)PlayerCollision_edi_back;
		vehicle2 = (CEntity*)PlayerCollision_esi_back;

		// If our car doesn't collide then process
		if (vars.local_vehicle != vehicle1 && vars.local_vehicle != vehicle2)
			goto hk_PlCol_processCol;

		// Only no collision with other cars if not the process it
		if (vars.local_vehicle->m_nType == ENTITY_TYPE_VEHICLE && vehicle1->m_nType == ENTITY_TYPE_VEHICLE && vehicle2->m_nType == ENTITY_TYPE_VEHICLE)
			goto hk_PlCol_noCol;

		goto hk_PlCol_processCol;
	}

hk_PlCol_processCol:
	__asm popad
	__asm mov eax, PlayerCollision_eax_back
	__asm mov ebx, PlayerCollision_ebx_back
	__asm mov ecx, PlayerCollision_ecx_back
	__asm mov edx, PlayerCollision_edx_back
	__asm mov edi, PlayerCollision_edi_back
	__asm mov esi, PlayerCollision_esi_back
	__asm mov esp, PlayerCollision_esp_back
	__asm jmp RETURN_PlayerCollision_process

	hk_PlCol_noCol :
	__asm popad
	__asm mov esi, PlayerCollision_esi_back
	__asm mov edi, PlayerCollision_edi_back
	__asm mov ebx, PlayerCollision_ebx_back
	__asm mov eax, PlayerCollision_eax_back
	__asm mov esp, PlayerCollision_esp_back
	__asm jmp RETURN_PlayerCollision_noProcessing
}

auto __fastcall collisions::hook_process_surfing(void* _this, void* edx) -> int32_t
{
	if (vars.packets.enable)
		return 0;

	return collisions::get().o_process_surfing(_this);
}
