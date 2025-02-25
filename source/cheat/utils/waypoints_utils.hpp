class waypoints_utils {
private:
	enum vehicle_class_t
	{
		HELICOPTER = 3,
		AIRPLANE = 4,
		BOAT = 6
	};

	waypoints_utils() {};
public:
	[[nodiscard]] static auto get() -> waypoints_utils& {
		static waypoints_utils instance;
		return instance;
	}

	[[nodiscard]] auto is_ped_in_waypoints_proximity(CVector waypoint_position) const noexcept -> bool
	{
		float waypoint_range{ 5.f };
		if (vars.waypoints.use_default_waypoint_range == false)
			waypoint_range = vars.waypoints.waypoint_range;
		else
		{
			waypoint_range = vars.local_vehicle != nullptr ? 5.f : 1.f;
		}

		if (utils::get().get_distance_to_local_ped(waypoint_position) < waypoint_range)
			return true;

		return false;
	}

	auto walk_to_coords(const CVector& waypoint_position) noexcept -> void
	{
		// this task will lock the ped
		utils::get().set_ped_locked();
		sync_ped_controls();

		const auto& local_position = utils::get().get_local_ped_position();
		CVector fixed_position = waypoint_position - 5 * (local_position - waypoint_position) * (1 / (local_position - waypoint_position).Magnitude());
		plugin::Command<plugin::Commands::TASK_GO_STRAIGHT_TO_COORD>(vars.local_ped, fixed_position.x, fixed_position.y, fixed_position.z, vars.waypoints.walk_mode, -1);
	}

	auto drive_to_coords(const CVector& waypoint_position, float speed) noexcept -> void
	{
		// this task will lock the ped
		utils::get().set_ped_locked();
		sync_driving_controls(speed);

		const auto& local_position = utils::get().get_local_ped_position();
		speed = speed < 1.f ? 1.f : speed;

		CVector fixed_position;
		if (vars.local_vehicle->m_nVehicleSubClass == HELICOPTER)
		{
			bool has_found_ground{ false };
			float ground_z_height = CWorld::FindGroundZFor3DCoord(local_position.x, local_position.y, local_position.z + 0.5f, &has_found_ground, nullptr);
			reinterpret_cast<CAutomobile*>(vars.local_vehicle)->TellHeliToGoToCoors(waypoint_position.x, waypoint_position.y, waypoint_position.z, ground_z_height + 10, waypoint_position.z + 5);

			return;
		}
		else if (vars.local_vehicle->m_nVehicleSubClass == AIRPLANE)
		{
			bool has_found_ground;
			float ground_z_height = CWorld::FindGroundZFor3DCoord(local_position.x, local_position.y, local_position.z + 0.5f, &has_found_ground, nullptr);
			reinterpret_cast<CAutomobile*>(vars.local_vehicle)->TellPlaneToGoToCoors(waypoint_position.x, waypoint_position.y, waypoint_position.z, ground_z_height + 10, waypoint_position.z + 5);

			return;
		}
		else if (vars.local_vehicle->m_nVehicleSubClass == BOAT)
		{
			fixed_position = waypoint_position;
		}
		else
		{
			if ((local_position - waypoint_position).Magnitude() > 25.f)
				fixed_position = waypoint_position + ((local_position - waypoint_position).Magnitude() - 20.00f) * (local_position - waypoint_position) * (1 / (local_position - waypoint_position).Magnitude());
		}

		CCarCtrl::JoinCarWithRoadSystemGotoCoors(vars.local_vehicle, waypoint_position, false, false);

		vars.local_vehicle->m_autoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
		vars.local_vehicle->m_autoPilot.m_nCarDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
		vars.local_vehicle->m_nStatus = STATUS_PHYSICS;

		vars.local_vehicle->m_nVehicleFlags.bEngineOn = true;
		vars.local_vehicle->m_nType &= 7;

		vars.local_vehicle->m_autoPilot.m_nCruiseSpeed = static_cast<char>(speed);
		vars.local_vehicle->m_autoPilot.m_nTimeToStartMission = CTimer::m_snTimeInMilliseconds;
	}

private:
	[[nodiscard]] auto get_steer_value() noexcept -> int16_t
	{
		if (vars.local_vehicle->m_fSteerAngle <= -0.52)
			return 128;
		else if (vars.local_vehicle->m_fSteerAngle >= 0.52)
			return -128;

		auto steer = static_cast<int16_t>(vars.local_vehicle->m_fSteerAngle * 100);
		for (auto data : steer_vector)
			if (steer == static_cast<int16_t>(data.first * 100))
				return data.second;

		return 0;
	}

	auto sync_driving_controls(float speed) noexcept -> void
	{
		double speed_magnitude = std::abs(vars.local_vehicle->m_vecMoveSpeed.Magnitude() * 170.0f / 2.8f - speed);
		auto local_pad = vars.local_ped->GetPadFromPlayer();

		if (speed_magnitude > -1 && speed_magnitude < 1 || vars.local_vehicle->m_fGasPedal == 1.f)
			local_pad->PCTempKeyState.ButtonCross = 255;
		else if (vars.local_vehicle->m_fBreakPedal == 1.f)
			local_pad->PCTempKeyState.ButtonSquare = 255;

		if (vars.local_vehicle->m_fSteerAngle <= -0.03 || vars.local_vehicle->m_fSteerAngle >= 0.03)
			local_pad->PCTempKeyState.LeftStickX = get_steer_value();
	}

	auto sync_ped_controls() noexcept -> void
	{
		auto local_pad = vars.local_ped->GetPadFromPlayer();

		if (vars.waypoints.walk_mode == 4)
		{
			*(BYTE*)(0xB73500 + 0x2A) = 255;
		}
		else if (vars.waypoints.walk_mode == 7)
		{
			vars.local_ped->GetPadFromPlayer()->PCTempKeyState.ButtonCross = 255;
			vars.local_ped->ResetSprintEnergy();
		}

	}

	std::vector<std::pair<float, int>> steer_vector
	{
		{0.52, -128}, { 0.52, -127 }, { 0.51, -126 }, { 0.50, -125 }, { 0.49, -124 }, { 0.48, -123 }, { 0.48, -122 }, { 0.47, -121 }, { 0.46, -120 }, { 0.45, -119 },
		{ 0.44, -118 }, { 0.44, -117 }, { 0.43, -116 }, { 0.42, -115 }, { 0.42, -114 }, { 0.41, -113 }, { 0.40, -112 }, { 0.39, -111 }, { 0.39, -110 }, { 0.38, -109 },
		{ 0.37, -108 }, { 0.37, -107 }, { 0.36, -106 }, { 0.35, -105 }, { 0.35, -104 }, { 0.34, -103 }, { 0.33, -102 }, { 0.33, -101 }, { 0.32, -100 }, { 0.31, -99 },
		{ 0.31, -98 }, { 0.30, -97 }, { 0.29, -96 }, { 0.29, -95 }, { 0.28, -94 }, { 0.28, -93 }, { 0.27, -92 }, { 0.26, -91 }, { 0.26, -90 }, { 0.25, -89 }, { 0.25, -88 },
		{ 0.24, -87 }, { 0.24, -86 }, { 0.23, -85 }, { 0.23, -84 }, { 0.22, -83 }, { 0.21, -82 }, { 0.21, -81 }, { 0.20, -80 }, { 0.20, -79 }, { 0.19, -78 }, { 0.19, -77 },
		{ 0.18, -76 }, { 0.18, -75 }, { 0.18, -74 }, { 0.17, -73 }, { 0.17, -72 }, { 0.16, -71 }, { 0.16, -70 }, { 0.15, -69 }, { 0.15, -68 }, { 0.14, -67 }, { 0.14, -66 },
		{ 0.14, -65 }, { 0.13, -64 }, { 0.13, -63 }, { 0.12, -62 }, { 0.12, -61 }, { 0.12, -60 }, { 0.11, -59 }, { 0.11, -58 }, { 0.10, -57 }, { 0.10, -56 }, { 0.10, -55 },
		{ 0.09, -54 }, { 0.09, -53 }, { 0.09, -52 }, { 0.08, -51 }, { 0.08, -50 }, { 0.08, -49 }, { 0.07, -48 }, { 0.07, -47 }, { 0.07, -46 }, { 0.06, -45 }, { 0.06, -44 },
		{ 0.06, -43 }, { 0.06, -42 }, { 0.05, -41 }, { 0.05, -40 }, { 0.05, -39 }, { 0.05, -38 }, { 0.04, -37 }, { 0.04, -36 }, { 0.04, -35 }, { 0.04, -34 }, { 0.03, -33 },
		{ 0.03, -32 }, { 0.03, -31 }, { 0.03, -30 }, { 0.03, -29 }, { 0.03, -28 }, { 0.02, -27 }, { 0.02, -26 }, { 0.02, -25 }, { 0.02, -24 }, { 0.02, -23 }, { 0.02, -22 },
		{ 0.01, -21 }, { 0.01, -20 }, { 0.01, -19 }, { 0.01, -18 }, { 0.01, -17 }, { 0.01, -16 }, { 0.01, -15 }, { 0.01, -14 }, { 0.01, -13 }, { 0.00, -12 }, { 0.00, -11 },
		{ 0.00, -10 }, { 0.00, -9 }, { 0.00, -8 }, { 0.00, -7 }, { 0.00, -6 }, { 0.00, -5 }, { 0.00, -4 }, { 0.00, -3 }, { 0.00, -2 }, { 0.00, -1 }, { 0.00, 0 }, { -0.00, 1 },
		{ -0.00, 2 }, { -0.00, 3 }, { -0.00, 4 }, { -0.00, 5 }, { -0.00, 6 }, { -0.00, 7 }, { -0.00, 8 }, { -0.00, 9 }, { -0.00, 10 }, { -0.00, 11 }, { -0.00, 12 }, { -0.01, 13 },
		{ -0.01, 14 }, { -0.01, 15 }, { -0.01, 16 }, { -0.01, 17 }, { -0.01, 18 }, { -0.01, 19 }, { -0.01, 20 }, { -0.01, 21 }, { -0.02, 22 }, { -0.02, 23 }, { -0.02, 24 }, { -0.02, 25 },
		{ -0.02, 26 }, { -0.02, 27 }, { -0.03, 28 }, { -0.03, 29 }, { -0.03, 30 }, { -0.03, 31 }, { -0.03, 32 }, { -0.03, 33 }, { -0.04, 34 }, { -0.04, 35 }, { -0.04, 36 },
		{ -0.04, 37 }, { -0.05, 38 }, { -0.05, 39 }, { -0.05, 40 }, { -0.05, 41 }, { -0.06, 42 }, { -0.06, 43 }, { -0.06, 44 }, { -0.06, 45 }, { -0.07, 46 }, { -0.07, 47 },
		{ -0.07, 48 }, { -0.08, 49 }, { -0.08, 50 }, { -0.08, 51 }, { -0.09, 52 }, { -0.09, 53 }, { -0.09, 54 }, { -0.10, 55 }, { -0.10, 56 }, { -0.10, 57 }, { -0.11, 58 },
		{ -0.11, 59 }, { -0.12, 60 }, { -0.12, 61 }, { -0.12, 62 }, { -0.13, 63 }, { -0.13, 64 }, { -0.14, 65 }, { -0.14, 66 }, { -0.14, 67 }, { -0.15, 68 }, { -0.15, 69 },
		{ -0.16, 70 }, { -0.16, 71 }, { -0.17, 72 }, { -0.17, 73 }, { -0.18, 74 }, { -0.18, 75 }, { -0.18, 76 }, { -0.19, 77 }, { -0.19, 78 }, { -0.20, 79 }, { -0.20, 80 },
		{ -0.21, 81 }, { -0.21, 82 }, { -0.22, 83 }, { -0.23, 84 }, { -0.23, 85 }, { -0.24, 86 }, { -0.24, 87 }, { -0.25, 88 }, { -0.25, 89 }, { -0.26, 90 }, { -0.26, 91 },
		{ -0.27, 92 }, { -0.28, 93 }, { -0.28, 94 }, { -0.29, 95 }, { -0.29, 96 }, { -0.30, 97 }, { -0.31, 98 }, { -0.31, 99 }, { -0.32, 100 }, { -0.33, 101 }, { -0.33, 102 },
		{ -0.34, 103 }, { -0.35, 104 }, { -0.35, 105 }, { -0.36, 106 }, { -0.37, 107 }, { -0.37, 108 }, { -0.38, 109 }, { -0.39, 110 }, { -0.39, 111 }, { -0.40, 112 }, { -0.41, 113 },
		{ -0.42, 114 }, { -0.42, 115 }, { -0.43, 116 }, { -0.44, 117 }, { -0.44, 118 }, { -0.45, 119 }, { -0.46, 120 }, { -0.47, 121 }, { -0.48, 122 }, { -0.48, 123 }, { -0.49, 124 },
		{ -0.50, 125 }, { -0.51, 126 }, { -0.52, 127 }, { -0.52, 128 }
	};
};