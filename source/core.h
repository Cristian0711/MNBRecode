#pragma once

#ifdef DEBUG
#define LOG(X) std::cout << X << '\n'
#else
#define LOG(X)
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <stdint.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include <iostream>
#include <format>
#include <regex>
#include <unordered_set>
#include <d3d9.h>
#include <filesystem>
#include <random>
#include <variant>
#include <wincodec.h>
#include <d3dx9tex.h>
#include <winioctl.h>
#include <playsoundapi.h>

#include <ws2tcpip.h>
#pragma comment(lib,"WS2_32")
#pragma comment(lib, "winmm.lib")

using namespace std::chrono_literals;

#include <plugin.h>
#include <CWorld.h>
#include <CMenuManager.h>
#include <CTimer.h>
#include <CCarCtrl.h>
#include <CMenuManager.h>
#include <extensions/ScriptCommands.h>
#include <extensions/scripting/ScriptCommandNames.h>

#include "vendor/detours/detours.h"
#include "vendor/detours/wrapper.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_internal.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"
#include "vendor/imgui/FontAwesome.h"

#include "vendor/nlohmann/json.hpp"
using json = nlohmann::json;

#include "vendor/cryptopp/rsa.h"
#include "vendor/cryptopp/base64.h"
#include "vendor/cryptopp/aes.h"
#include "vendor/cryptopp/modes.h"
#include "vendor/cryptopp/osrng.h"
#include "vendor/cryptopp/hex.h"
#pragma comment(lib, "vendor/cryptopp/lib/cryptopp-static.lib")

#include "cheat/misc/callbacks.hpp"

#include "game/offsets.hpp"
#include "game/modules.hpp"
#include "game/raknet/bitstream.h"
#include "game/raknet/NetworkTypes.h"
#include "game/raknet/rakclient.h"

#include "cheat/variables.hpp"

#include "cheat/menu/menu.h"

#include "cheat/utils/crypto.hpp"
#include "cheat/utils/utils.hpp"

#include "socket/socket_utils.hpp"
#include "socket/commands.h"

#include "cheat/rendering/rendering.h"
#include "cheat/rendering/renderer.hpp"
#include "cheat/rendering/render_utils.hpp"

#include "cheat/features/alarm/alarm.h"

#include "cheat/features/chat_detect/chat_block.h"
#include "cheat/features/chat_detect/chatlog.h"
#include "cheat/features/chat_detect/chat_detect.h"

#include "cheat/features/waypoints/waypoints.h"
#include "cheat/features/packets/packets.h"

#include "cheat/features/collisions/collisions.h"
#include "cheat/features/captcha/captcha.h"

#include "cheat/features/screenshot/screenshot.h"

#include "cheat/utils/waypoints_utils.hpp"
#include "cheat/utils/packets_utils.hpp"

#include "cheat/misc/hooks.hpp"

#include "cheat/config/configs.h"
#include "cheat/config/waypoints_config.hpp"
#include "cheat/config/packets_config.hpp"

#include "cheat/menu/imgui_addon.h"

#include "cheat/config/public_configs.hpp"

#include "socket/socket_handler.h"

#include "cheat/features/route_randomizer/route_randomizer.h"
#include "cheat/cheat.hpp"

#include "cheat/utils/global_utils.hpp"