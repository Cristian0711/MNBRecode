#define CHEAT_VERSION "1.4"

struct chat_keywords_t {
	std::string keyword;
	std::string response;

	chat_keywords_t() noexcept = default;
	chat_keywords_t(const std::string_view _keyword, const std::string_view _response) noexcept 
		: keyword(_keyword), response(_response) {}
};

struct color_keywords_t {
	uint8_t ignore_mode;
	std::string color;

	color_keywords_t() = default;
	color_keywords_t(uint8_t _ignore, const std::string_view _color) noexcept 
		: ignore_mode(_ignore), color(_color) {}
};

struct waypoint_t
{
	float speed;
	CVector position;

	waypoint_t() = default;
	waypoint_t(const CVector& pos, float S) noexcept
		: position(pos), speed(S) {}
};

struct packet_sync_data_t
{
	struct packet_t
	{
		uint16_t model_id;
		struct on_foot_data_t on_foot_data;
		struct in_car_data_t in_car_data;
		struct trailer_data_t trailer_data;
		uint8_t id;
	} packet;

	struct rpc_t
	{
		struct send_message_t send_message;
		struct send_command_t send_command;
		struct send_dialog_response_t send_dialog_response;
		struct send_textdraw_t send_text_draw;
		CVector position;

		uint8_t id;
	} rpc;

	uint8_t type;
} s;


struct vars_t {
	struct alarm
	{
		bool enable{ false };
		bool use_custom_alarm{ false };
		char custom_alarm_path[MAX_PATH]{ "" };
	} alarm;

	struct chat_detect
	{
		bool enable{ false };

		std::vector< chat_keywords_t > chat_keywords;
		std::vector< color_keywords_t > color_keywords;

	} chat_detect;

	struct waypoints
	{
		bool enable{ false };
		bool record_waypoints{ false };
		bool record_fill_waypoints{ false };
		bool use_default_waypoint_range{ true };
		bool use_fill_route{ false };

		uint16_t record_delay{ 400 };
		uint8_t walk_mode{ 4 };
		uint8_t fill_routes_loops_count{ 1 };
		int32_t fill_waypoint_index{ 10 };
		int32_t fill_route_sleep{ 10000 };

		float waypoint_range{ 5.f };

		std::vector < waypoint_t > waypoints_vector;
		std::vector < waypoint_t > fill_waypoints_vector;
	} waypoints;
	
	struct packets
	{
		bool enable{ false };
		bool record_packets{ false };
		bool start_from_closest_packet{ false };

		std::vector < packet_sync_data_t > packets_vector;

	} packets;

	struct smart_waypoints
	{
		bool enable{ false };
		bool record_waypoints{ false };

		std::vector < waypoint_t > waypoints_vector;

	} smart_waypoints;

	struct conditional_routes
	{
		bool enable{ false };
		bool use_random_routes{ false };
		bool do_not_use_last_route{ false };
	} conditional_routes;

	struct collisions
	{
		bool enable{ false };
		bool use_when_recording{ false };
	} collisions;

	struct captcha
	{
		bool enable{ false };
		uint8_t min_seconds_solve{ 4 };
		uint8_t max_seconds_solve{ 10 };
	} captcha;

	struct misc
	{
		bool use_global_keywords_and_colors{ false };
		bool use_discord_alerts{ false };
		uint16_t show_count{ 1u };
	} misc;

	struct style
	{
		ImColor waypoint_text_color{ 0.05f, 0.98f, 0.3f, 1.0f };
		ImColor fill_waypoint_text_color{ 0.69f, 0.20f, 0.20f, 1.00f };

		ImVec4 waypoint_dot_color{ 1.00f, 1.00f, 0.00f, 1.00f };
		ImVec4 tab_menu_color{ 0.80f, 0.50f, 0.00f, 1.00f };;
	} style;

	struct user_data {
		uint32_t user_status = 0; // 0 - User, 1 - Suport, 2 - Admin
		std::string licensename;
		std::string hwid;
		std::string discord_id;
	} user_data;

	CPlayerPed* local_ped{ nullptr };
	CVehicle* local_vehicle{ nullptr };
};

inline vars_t vars;