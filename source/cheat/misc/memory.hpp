#pragma once

#include <map>

class memory {
	memory() : oldvp(0) {}
	DWORD oldvp;

	std::map<uint32_t, uint8_t> original;

public:
	[[nodiscard]] static auto get() noexcept -> memory& {
		static memory instance;
		return instance;
	}

	auto write_multiple(const char* bytes, uint32_t address, size_t size) noexcept -> void {
		for (auto byte_index = 0u; byte_index != size; ++byte_index) {
			write(bytes[byte_index], address + byte_index);
		}
	}

	template<typename A>
	auto write(A value, uint32_t address, bool vp = true) noexcept -> void {

		if (vp)
			VirtualProtect(reinterpret_cast<void*>(address), sizeof(A), PAGE_EXECUTE_READWRITE, &oldvp);

		save(address, sizeof(A));
		*reinterpret_cast<A*>(address) = value;

		if (vp)
			VirtualProtect(reinterpret_cast<void*>(address), sizeof(A), oldvp, &oldvp);
	}

	auto save(uint32_t address, size_t size) noexcept -> void {
		for (auto i = 0u; i != size; ++i)
			if (!original.contains(address + i))
				original.insert({ address + i, *reinterpret_cast<byte*>(address + i) });
	}

	template<typename A>
	auto get_original(uint32_t address) const noexcept -> void {

		auto val = *reinterpret_cast<A*>(address);
		auto val_addr = std::addressof(val);

		for (auto i = 0u; i != sizeof(A); ++i)
			if (auto saved = original.find(address + i); saved != original.end())
				*reinterpret_cast<byte*>(val_addr + i) = saved->second;

		return val;
	}

	[[nodiscard]] auto scan_dword(uint32_t value) noexcept -> std::vector<uint32_t*>
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);

		MEMORY_BASIC_INFORMATION memInfo;
		uint8_t* addr = 0; 
		uint8_t* maxAddr = (uint8_t*)systemInfo.lpMaximumApplicationAddress;

		std::vector<uint32_t*> addresses;

		while (addr < maxAddr) {
			if (VirtualQuery(addr, &memInfo, sizeof(memInfo)) == sizeof(memInfo)) {
				if (memInfo.State == MEM_COMMIT && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY)) {
					uint32_t* memRegionStart = (uint32_t*)memInfo.BaseAddress;
					uint32_t* memRegionEnd = (uint32_t*)((char*)memInfo.BaseAddress + memInfo.RegionSize);

					for (uint32_t* pCurrent = memRegionStart; pCurrent < memRegionEnd; ++pCurrent)
						if (*pCurrent == value)
							addresses.push_back(pCurrent);
				}
			}
			addr += memInfo.RegionSize;
		}

		return addresses;
	}
	
};