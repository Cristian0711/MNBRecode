#pragma pack(push, 1)

struct samp_keys_t
{
	uint8_t keys_primaryFire : 1;
	uint8_t keys_horn__crouch : 1;
	uint8_t keys_secondaryFire__shoot : 1;
	uint8_t keys_accel__zoomOut : 1;
	uint8_t keys_enterExitCar : 1;
	uint8_t keys_decel__jump : 1;
	uint8_t keys_circleRight : 1;
	uint8_t keys_aim : 1;
	uint8_t keys_circleLeft : 1;
	uint8_t keys_landingGear__lookback : 1;
	uint8_t keys_unknown__walkSlow : 1;
	uint8_t keys_specialCtrlUp : 1;
	uint8_t keys_specialCtrlDown : 1;
	uint8_t keys_specialCtrlLeft : 1;
	uint8_t keys_specialCtrlRight : 1;
	uint8_t keys__unused : 1;
};

struct on_foot_data_t
{
	uint16_t left_right_keys;
	uint16_t up_down_keys;
	union
	{
		uint16_t samp_keys;
		struct samp_keys_t m_samp_keys;
	};
	CVector position;
	float quaternion[4];
	uint8_t health;
	uint8_t armuor;
	uint8_t current_weapon;
	uint8_t special_action;
	CVector move_speed;
	float surfing_offsets[3];
	uint16_t surfing_vehicle_id;
	short current_animation_id;
	short anim_flags;
};

struct in_car_data_t
{
	uint16_t vehicle_id;
	uint16_t left_right_keys;
	uint16_t up_down_keys;
	union
	{
		uint16_t samp_keys;
		struct samp_keys_t m_samp_keys;
	};
	float quaternion[4];
	CVector position;
	CVector move_speed;
	float vehicle_health;
	uint8_t player_health;
	uint8_t player_armour;
	uint8_t current_weapon;
	uint8_t siren_active;
	uint8_t landing_gear_state;
	uint16_t trailer_id;
	union
	{
		uint16_t hydra_thrust_angle[2];
		float train_speed;
	};
};

struct trailer_data_t
{
	uint16_t trailer_id;
	float position[3];
	float quaternion[4];
	float move_speed[3];
	float unknown[2];
	uint32_t pad;
};

#pragma pack(pop)

struct send_message_t
{
	uint8_t text_length;
	char text[256];
};

struct send_command_t
{
	uint32_t text_length;
	char text[50];
};

struct send_textdraw_t
{
	uint16_t id;
};

struct send_dialog_response_t
{
	uint16_t id;
	uint8_t response;
	uint16_t list_item;
	uint8_t text_length;
	char text[256];
};

enum PacketEnumeration : uint8_t
{
	ID_INTERNAL_PING = 6,
	ID_PING,
	ID_PING_OPEN_CONNECTIONS,
	ID_CONNECTED_PONG,
	ID_REQUEST_STATIC_DATA,
	ID_CONNECTION_REQUEST,
	ID_AUTH_KEY,
	ID_BROADCAST_PINGS = 14,
	ID_SECURED_CONNECTION_RESPONSE,
	ID_SECURED_CONNECTION_CONFIRMATION,
	ID_RPC_MAPPING,
	ID_SET_RANDOM_NUMBER_SEED = 19,
	ID_RPC,
	ID_RPC_REPLY,
	ID_DETECT_LOST_CONNECTIONS = 23,
	ID_OPEN_CONNECTION_REQUEST,
	ID_OPEN_CONNECTION_REPLY,
	ID_OPEN_CONNECTION_COOKIE,
	ID_RSA_PUBLIC_KEY_MISMATCH = 28,
	ID_CONNECTION_ATTEMPT_FAILED,
	ID_NEW_INCOMING_CONNECTION = 30,
	ID_NO_FREE_INCOMING_CONNECTIONS = 31,
	ID_DISCONNECTION_NOTIFICATION,
	ID_CONNECTION_LOST,
	ID_CONNECTION_REQUEST_ACCEPTED,
	ID_CONNECTION_BANNED = 36,
	ID_INVALID_PASSWORD,
	ID_MODIFIED_PACKET,
	ID_PONG,
	ID_TIMESTAMP,
	ID_RECEIVED_STATIC_DATA,
	ID_REMOTE_DISCONNECTION_NOTIFICATION,
	ID_REMOTE_CONNECTION_LOST,
	ID_REMOTE_NEW_INCOMING_CONNECTION,
	ID_REMOTE_EXISTING_CONNECTION,
	ID_REMOTE_STATIC_DATA,
	ID_ADVERTISE_SYSTEM = 55,

	ID_PLAYER_SYNC = 207,
	ID_MARKERS_SYNC = 208,
	ID_UNOCCUPIED_SYNC = 209,
	ID_TRAILER_SYNC = 210,
	ID_PASSENGER_SYNC = 211,
	ID_SPECTATOR_SYNC = 212,
	ID_AIM_SYNC = 203,
	ID_VEHICLE_SYNC = 200,
	ID_RCON_COMMAND = 201,
	ID_RCON_RESPONCE = 202,
	ID_WEAPONS_UPDATE = 204,
	ID_STATS_UPDATE = 205,
	ID_BULLET_SYNC = 206,
};

enum RPCEnumeration : uint8_t
{
	RPC_ClickPlayer = 23,
	RPC_ClientJoin = 25,
	RPC_EnterVehicle = 26,
	RPC_EnterEditObject = 27,
	RPC_ScriptCash = 31,
	RPC_ServerCommand = 50,
	RPC_Spawn = 52,
	RPC_Death = 53,
	RPC_NPCJoin = 54,
	RPC_DialogResponse = 62,
	RPC_ClickTextDraw = 83,
	RPC_SCMEvent = 96,
	RPC_Chat = 101,
	RPC_SrvNetStats = 102,
	RPC_ClientCheck = 103,
	RPC_DamageVehicle = 106,
	RPC_GiveTakeDamage = 115,
	RPC_EditAttachedObject = 116,
	RPC_EditObject = 117,
	RPC_SetInteriorId = 118,
	RPC_MapMarker = 119,
	RPC_RequestClass = 128,
	RPC_RequestSpawn = 129,
	RPC_PickedUpPickup = 131,
	RPC_MenuSelect = 132,
	RPC_VehicleDestroyed = 136,
	RPC_MenuQuit = 140,
	RPC_ExitVehicle = 154,
	RPC_UpdateScoresPingsIPs = 155,

	RPC_SetPlayerName = 11,
	RPC_SetPlayerPos = 12,
	RPC_SetPlayerPosFindZ = 13,
	RPC_SetPlayerHealth = 14,
	RPC_TogglePlayerControllable = 15,
	RPC_PlaySound = 16,
	RPC_SetPlayerWorldBounds = 17,
	RPC_GivePlayerMoney = 18,
	RPC_SetPlayerFacingAngle = 19,
	RPC_ResetPlayerMoney = 20,
	RPC_ResetPlayerWeapons = 21,
	RPC_GivePlayerWeapon = 22,
	RPC_SetVehicleParamsEx = 24,
	RPC_CancelEdit = 28,
	RPC_SetPlayerTime = 29,
	RPC_ToggleClock = 30,
	RPC_WorldPlayerAdd = 32,
	RPC_SetPlayerShopName = 33,
	RPC_SetPlayerSkillLevel = 34,
	RPC_SetPlayerDrunkLevel = 35,
	RPC_Create3DTextLabel = 36,
	RPC_DisableCheckpoint = 37,
	RPC_SetRaceCheckpoint = 38,
	RPC_DisableRaceCheckpoint = 39,
	RPC_GameModeRestart = 40,
	RPC_PlayAudioStream = 41,
	RPC_StopAudioStream = 42,
	RPC_RemoveBuildingForPlayer = 43,
	RPC_CreateObject = 44,
	RPC_SetObjectPos = 45,
	RPC_SetObjectRot = 46,
	RPC_DestroyObject = 47,
	RPC_DeathMessage = 55,
	RPC_SetPlayerMapIcon = 56,
	RPC_RemoveVehicleComponent = 57,
	RPC_Update3DTextLabel = 58,
	RPC_ChatBubble = 59,
	RPC_UpdateSystemTime = 60,
	RPC_ShowDialog = 61,
	RPC_DestroyPickup = 63,
	RPC_WeaponPickupDestroy = 64,
	RPC_LinkVehicleToInterior = 65,
	RPC_SetPlayerArmour = 66,
	RPC_SetPlayerArmedWeapon = 67,
	RPC_SetSpawnInfo = 68,
	RPC_SetPlayerTeam = 69,
	RPC_PutPlayerInVehicle = 70,
	RPC_RemovePlayerFromVehicle = 71,
	RPC_SetPlayerColor = 72,
	RPC_DisplayGameText = 73,
	RPC_ForceClassSelection = 74,
	RPC_AttachObjectToPlayer = 75,
	RPC_InitMenu = 76,
	RPC_ShowMenu = 77,
	RPC_HideMenu = 78,
	RPC_CreateExplosion = 79,
	RPC_ShowPlayerNameTagForPlayer = 80,
	RPC_AttachCameraToObject = 81,
	RPC_InterpolateCamera = 82,
	RPC_SetObjectMaterial = 84,
	RPC_GangZoneStopFlash = 85,
	RPC_ApplyAnimation = 86,
	RPC_ClearAnimations = 87,
	RPC_SetPlayerSpecialAction = 88,
	RPC_SetPlayerFightingStyle = 89,
	RPC_SetPlayerVelocity = 90,
	RPC_SetVehicleVelocity = 91,
	RPC_SetPlayerDrunkVisuals = 92,
	RPC_ClientMessage = 93,
	RPC_SetWorldTime = 94,
	RPC_CreatePickup = 95,
	RPC_SetVehicleTireStatus = 98,
	RPC_MoveObject = 99,
	RPC_EnableStuntBonusForPlayer = 104,
	RPC_TextDrawSetString = 105,
	RPC_SetCheckpoint = 107,
	RPC_GangZoneCreate = 108,
	RPC_PlayCrimeReport = 112,
	RPC_SetPlayerAttachedObject = 113,
	RPC_GangZoneDestroy = 120,
	RPC_GangZoneFlash = 121,
	RPC_StopObject = 122,
	RPC_SetNumberPlate = 123,
	RPC_TogglePlayerSpectating = 124,
	RPC_PlayerSpectatePlayer = 126,
	RPC_PlayerSpectateVehicle = 127,
	RPC_SetPlayerWantedLevel = 133,
	RPC_ShowTextDraw = 134,
	RPC_TextDrawHideForPlayer = 135,
	RPC_ServerJoin = 137,
	RPC_ServerQuit = 138,
	RPC_InitGame = 139,
	RPC_RemovePlayerMapIcon = 144,
	RPC_SetPlayerAmmo = 145,
	RPC_SetPlayerGravity = 146,
	RPC_SetVehicleHealth = 147,
	RPC_AttachTrailerToVehicle = 148,
	RPC_DetachTrailerFromVehicle = 149,
	RPC_SetPlayerDrunkHandling = 150,
	RPC_DestroyPickups = 151,
	RPC_SetWeather = 152,
	RPC_SetPlayerSkin = 153,
	RPC_SetPlayerInterior = 156,
	RPC_SetPlayerCameraPos = 157,
	RPC_SetPlayerCameraLookAt = 158,
	RPC_SetVehiclePos = 159,
	RPC_SetVehicleZAngle = 160,
	RPC_SetVehicleParamsForPlayer = 161,
	RPC_SetCameraBehindPlayer = 162,
	RPC_WorldPlayerRemove = 163,
	RPC_WorldVehicleAdd = 164,
	RPC_WorldVehicleRemove = 165,
	RPC_WorldPlayerDeath = 166,
};