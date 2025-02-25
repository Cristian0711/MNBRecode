#pragma once

#include "core.h"
#include "cheat/misc/memory.hpp"

class detours_wrapper {
private:
	detours_wrapper() {};

public:
	[[nodiscard]] static auto get() noexcept -> detours_wrapper& {
		static detours_wrapper instance;
		return instance;
	}

	template<typename A, typename T>
	[[nodiscard]] inline auto hook(A& original, T func) const noexcept -> bool
	{
		auto result = DetourAttach(reinterpret_cast<PVOID*>(&original), func);

		if (result != NO_ERROR)
			return false;

		return true;
	}

	[[nodiscard]] inline auto start_transaction() const noexcept -> bool
	{
		auto result = DetourTransactionBegin();
		if (result != NO_ERROR)
			return false;

		result = DetourUpdateThread(GetCurrentThread());
		if (result != NO_ERROR)
			return false;

		return true;
	}

	[[nodiscard]] inline auto commit_transaction() const noexcept -> bool
	{
		auto result = DetourTransactionCommit();
		if (result != NO_ERROR)
			return false;

		return true;
	}

	[[nodiscard]] inline auto release_all_hooks() const noexcept -> void
	{
		if (start_transaction() == false)
			return;

		
	}
private:
	std::unordered_map<uint32_t, void*> m_hooks;
};