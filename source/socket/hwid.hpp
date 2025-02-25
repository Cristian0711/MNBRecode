class hwid {
private:
	hwid() {};
public:
	[[nodiscard]] static auto get() -> hwid& {
		static hwid instance;
		return instance;
	}

	[[nodiscard]] __forceinline auto get_hwid() const noexcept -> std::string
	{
		std::string HWID;
		OBF_BEGIN
		{
			char buffer[256];

			SYSTEM_INFO siSysInfo;
			GetSystemInfo(&siSysInfo);

			int HWID_Calculator[6];
			V(HWID_Calculator[0]) = siSysInfo.dwOemId;
			V(HWID_Calculator[1]) = siSysInfo.dwNumberOfProcessors;
			V(HWID_Calculator[2]) = siSysInfo.dwProcessorType;
			V(HWID_Calculator[3]) = siSysInfo.dwActiveProcessorMask;
			V(HWID_Calculator[4]) = siSysInfo.wProcessorLevel;
			V(HWID_Calculator[5]) = siSysInfo.wProcessorRevision;

			int HWID_Calculated;
			V(HWID_Calculated) = HWID_Calculator[0, 1, 2, 3, 4, 5] * 2 * 4 * 8 * 16 * 32 * 64 * 120;

			HWID += std::to_string(HWID_Calculated);

			int cpuid[4];
			V(cpuid[0]) = N(0);
			V(cpuid[1]) = N(0);
			V(cpuid[2]) = N(0);
			V(cpuid[3]) = N(0);
			__cpuid(cpuid, 0);

			char16_t hold = 0;
			char16_t* pointer;
			pointer = reinterpret_cast<char16_t*>(cpuid);

			int i = N(0);
			FOR(V(i) = N(0), V(i) < N(8), V(i)++)
				V(hold) += pointer[i];
			ENDFOR

			HWID += std::to_string(hold);

			std::string firstHddSerialNumber = get_firrst_hdd_serial_number();
			while (firstHddSerialNumber.empty())
				firstHddSerialNumber = get_firrst_hdd_serial_number();

			HWID += firstHddSerialNumber;

			FOR(V(i) = N(0), V(i) < N(28), V(i)++)
				std::string shastring = crypto::get().sha256_encode(HWID);
				std::transform(shastring.begin(), shastring.end(), shastring.begin(), ::tolower);
				HWID = shastring;
			ENDFOR
		}
		OBF_END

		return HWID;
	}

private:
	[[nodiscard]] __forceinline auto get_firrst_hdd_serial_number() const noexcept -> std::string {
		//get a handle to the first physical drive
		HANDLE h = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (h == INVALID_HANDLE_VALUE) return {};
		//an std::unique_ptr is used to perform cleanup automatically when returning (i.e. to avoid code duplication)
		std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> hDevice{ h, [](HANDLE handle) {CloseHandle(handle); } };
		//initialize a STORAGE_PROPERTY_QUERY data structure (to be used as input to DeviceIoControl)
		STORAGE_PROPERTY_QUERY storagePropertyQuery{};
		storagePropertyQuery.PropertyId = StorageDeviceProperty;
		storagePropertyQuery.QueryType = PropertyStandardQuery;
		//initialize a STORAGE_DESCRIPTOR_HEADER data structure (to be used as output from DeviceIoControl)
		STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader{};
		//the next call to DeviceIoControl retrieves necessary size (in order to allocate a suitable buffer)
		//call DeviceIoControl and return an empty std::string on failure
		DWORD dwBytesReturned = 0;
		if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL))
			return {};
		//allocate a suitable buffer
		const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
		std::unique_ptr<BYTE[]> pOutBuffer{ new BYTE[dwOutBufferSize]{} };
		//call DeviceIoControl with the allocated buffer
		if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			pOutBuffer.get(), dwOutBufferSize, &dwBytesReturned, NULL))
			return {};
		//read and return the serial number out of the output buffer
		STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(pOutBuffer.get());
		const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
		if (dwSerialNumberOffset == 0) return {};
		const char* serialNumber = reinterpret_cast<const char*>(pOutBuffer.get() + dwSerialNumberOffset);
		return serialNumber;
	}
};