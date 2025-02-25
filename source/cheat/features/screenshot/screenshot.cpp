#include "core.h"

#define RELEASE(__p) {if(__p!=nullptr){__p->Release();__p=nullptr;}}

auto surface_deleter = [](IDirect3DSurface9* ptr) {
	if (ptr) {
		ptr->Release();
	}
};

using release_ptr = std::unique_ptr<IDirect3DSurface9, decltype(surface_deleter)>;

static const struct {
	const GUID* wic_guid;
	D3DFORMAT d3dformat;
} wic_pixel_formats[] = {
  {&GUID_WICPixelFormat8bppGray,			D3DFMT_L8},
  {&GUID_WICPixelFormat8bppIndexed,		D3DFMT_P8},
  {&GUID_WICPixelFormat8bppAlpha,			D3DFMT_A8},

  {&GUID_WICPixelFormat16bppBGR565,		D3DFMT_R5G6B5},
  {&GUID_WICPixelFormat16bppBGR555,		D3DFMT_X1R5G5B5},
  {&GUID_WICPixelFormat16bppBGR555,		D3DFMT_A1R5G5B5},

  {&GUID_WICPixelFormat24bppBGR,			D3DFMT_R8G8B8},
  {&GUID_WICPixelFormat24bppBGR,			D3DFMT_A4R4G4B4},
  {&GUID_WICPixelFormat24bppBGR,			D3DFMT_X4R4G4B4},

  {&GUID_WICPixelFormat32bppBGR,			D3DFMT_X8R8G8B8},
  {&GUID_WICPixelFormat32bppBGR,			D3DFMT_A8R8G8B8},

  {&GUID_WICPixelFormat32bppBGR101010,	D3DFMT_A2R10G10B10},
  {&GUID_WICPixelFormat32bppBGR101010,	D3DFMT_A2B10G10R10},

  {&GUID_WICPixelFormat16bppGrayHalf,		D3DFMT_R16F},
  {&GUID_WICPixelFormat32bppGrayFloat,	D3DFMT_R32F},

  {&GUID_WICPixelFormat64bppRGBAHalf,		D3DFMT_A16B16G16R16F},
  {&GUID_WICPixelFormat128bppRGBFloat,	D3DFMT_A32B32G32R32F}
};

auto screenshot_t::ansi_to_unicode(std::string_view ansiStr, UINT codePage) const noexcept -> std::wstring {
	int wideCharLength = MultiByteToWideChar(codePage, 0, ansiStr.data(), static_cast<int>(ansiStr.size()), nullptr, 0);
	if (wideCharLength == 0) {
		return L"";
	}

	std::wstring unicodeStr(wideCharLength, 0);
	MultiByteToWideChar(codePage, 0, ansiStr.data(), static_cast<int>(ansiStr.size()), &unicodeStr[0], wideCharLength);

	return unicodeStr;
}

auto screenshot_t::d3dformat_to_wic_guid(D3DFORMAT format) const noexcept -> const GUID*
{
	for (const auto& fmt : wic_pixel_formats)
		if (fmt.d3dformat == format)
			return fmt.wic_guid;

	return nullptr;
};

auto screenshot_t::copy_surface_pixels(LPDIRECT3DSURFACE9 surface) noexcept -> HRESULT
{
	D3DSURFACE_DESC surface_desc;
	if (FAILED(surface->GetDesc(&surface_desc)))
		return E_FAIL;

	D3DLOCKED_RECT locked_rect;
	if (FAILED(surface->LockRect(&locked_rect, nullptr, D3DLOCK_READONLY)))
		return E_FAIL;

	m_width = surface_desc.Width; m_height = surface_desc.Height;
	m_format = surface_desc.Format; m_pitch = locked_rect.Pitch;

	if (m_pixels = new std::uint8_t[locked_rect.Pitch * surface_desc.Height])
		std::memcpy(m_pixels, locked_rect.pBits, m_pitch * m_height);

	surface->UnlockRect();

	if (m_pixels)
		return D3D_OK;

	return E_OUTOFMEMORY;
};

auto screenshot_t::save_pixels_to_disk(const std::string_view filename, D3DXIMAGE_FILEFORMAT file_format) noexcept -> HRESULT
{
	const GUID* format = nullptr;
	switch (file_format) {
	case D3DXIFF_BMP:
	case D3DXIFF_DIB:
		format = &GUID_ContainerFormatBmp;
		break;
	case D3DXIFF_PNG:
		format = &GUID_ContainerFormatPng;
		break;
	case D3DXIFF_JPG:
		format = &GUID_ContainerFormatJpeg;
		break;
	case D3DXIFF_DDS:
	case D3DXIFF_HDR:
	case D3DXIFF_PFM:
	case D3DXIFF_TGA:
	case D3DXIFF_PPM:
		LOG("Unsupported file format");
		return E_NOTIMPL;
	default:
		LOG("Invalid file format");
		return D3DERR_INVALIDCALL;
	}

	const GUID* src_format = d3dformat_to_wic_guid(m_format);
	if (src_format == nullptr) {
		LOG("Unsupported source format");
		return E_NOTIMPL;
	}

	HRESULT hr = S_OK;
	IWICImagingFactory* factory = nullptr;
	IWICStream* stream = nullptr;
	IWICBitmapEncoder* encoder = nullptr;
	IWICBitmapFrameEncode* frame = nullptr;
	WICPixelFormatGUID wic_pixel_format;
	IWICBitmap* bitmap = nullptr;
	IWICFormatConverter* converter = nullptr;
	WICRect screen_rect = { 0u, 0u, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	std::wstring filepath = ansi_to_unicode(filename);
	const GUID* dst_format = &GUID_WICPixelFormat24bppRGB;
	BOOL can_convert = FALSE;

	if (FAILED(hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
		LOG("Failed to create WIC imaging factory");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateStream(&stream))) {
		LOG("Failed to create stream");
		goto cleanup;
	}

	if (FAILED(hr = stream->InitializeFromFilename(filepath.c_str(), GENERIC_WRITE))) {
		LOG("Failed to initialize stream from filename");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateEncoder(*format, nullptr, &encoder))) {
		LOG("Failed to create encoder");
		goto cleanup;
	}

	if (FAILED(hr = encoder->Initialize(stream, WICBitmapEncoderNoCache))) {
		LOG("Failed to initialize encoder");
		goto cleanup;
	}

	if (FAILED(hr = encoder->CreateNewFrame(&frame, nullptr))) {
		LOG("Failed to create new frame");
		goto cleanup;
	}

	if (FAILED(hr = frame->Initialize(nullptr))) {
		LOG("Failed to initialize frame");
		goto cleanup;
	}

	if (FAILED(hr = frame->SetSize(m_width, m_height))) {
		LOG("Failed to set frame size");
		goto cleanup;
	}

	std::memcpy(&wic_pixel_format, src_format, sizeof(GUID));
	if (FAILED(hr = frame->SetPixelFormat(&wic_pixel_format))) {
		LOG("Failed to set pixel format");
		goto cleanup;
	}

	if (!dst_format) {
		hr = E_NOTIMPL;
		LOG("Unsupported destination format");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateBitmapFromMemory(m_width, m_height, *src_format, m_pitch, m_pitch * m_height, m_pixels, &bitmap))) {
		LOG("Failed to create bitmap from memory");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateFormatConverter(&converter))) {
		LOG("Failed to create format converter");
		goto cleanup;
	}

	if (FAILED(hr = converter->CanConvert(*src_format, *dst_format, &can_convert)) || !can_convert) {
		LOG("Format conversion not supported");
		goto cleanup;
	}

	if (FAILED(hr = converter->Initialize(bitmap, *dst_format, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut))) {
		LOG("Failed to initialize converter");
		goto cleanup;
	}

	if (FAILED(hr = frame->WriteSource(converter, &screen_rect))) {
		LOG("Failed to write source to frame");
		goto cleanup;
	}

	if (FAILED(hr = frame->Commit())) {
		LOG("Failed to commit frame");
		goto cleanup;
	}

	if (FAILED(hr = encoder->Commit())) {
		LOG("Failed to commit encoder");
		goto cleanup;
	}

cleanup:
	RELEASE(factory);
	RELEASE(stream);
	RELEASE(encoder);
	RELEASE(frame);
	RELEASE(bitmap);
	RELEASE(converter);

	return hr;
}


auto screenshot_t::save_pixels_to_memory(std::string& buffer, D3DXIMAGE_FILEFORMAT file_format, UINT new_width, UINT new_height) noexcept -> HRESULT
{
	const GUID* format = nullptr;
	switch (file_format) {
	case D3DXIFF_BMP:
	case D3DXIFF_DIB:
		format = &GUID_ContainerFormatBmp;
		break;
	case D3DXIFF_PNG:
		format = &GUID_ContainerFormatPng;
		break;
	case D3DXIFF_JPG:
		format = &GUID_ContainerFormatJpeg;
		break;
	case D3DXIFF_DDS:
	case D3DXIFF_HDR:
	case D3DXIFF_PFM:
	case D3DXIFF_TGA:
	case D3DXIFF_PPM:
		LOG("Unsupported file format");
		return E_NOTIMPL;
	default:
		LOG("Invalid file format");
		return D3DERR_INVALIDCALL;
	}

	const GUID* src_format = d3dformat_to_wic_guid(m_format);
	if (src_format == nullptr) {
		LOG("Unsupported source format");
		return E_NOTIMPL;
	}

	HRESULT hr = S_OK;
	IWICImagingFactory* factory = nullptr;
	IWICStream* stream = nullptr;
	IWICBitmapEncoder* encoder = nullptr;
	IWICBitmapFrameEncode* frame = nullptr;
	WICPixelFormatGUID wic_pixel_format;
	IWICBitmap* bitmap = nullptr;
	IWICFormatConverter* converter = nullptr;
	IWICBitmapScaler* scaler = nullptr;
	WICRect screen_rect = { 0u, 0u, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	const GUID* dst_format = &GUID_WICPixelFormat24bppRGB;
	BOOL can_convert = FALSE;
	IStream* memory_stream = nullptr;
	STATSTG stat = {};
	ULONG buffer_size = 0;
	LARGE_INTEGER liZero = {};

	if (FAILED(hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
		LOG("Failed to create WIC imaging factory");
		goto cleanup;
	}

	if (FAILED(hr = CreateStreamOnHGlobal(NULL, TRUE, &memory_stream))) {
		LOG("Failed to create stream on HGlobal");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateStream(&stream))) {
		LOG("Failed to create stream");
		goto cleanup;
	}

	if (FAILED(hr = stream->InitializeFromIStream(memory_stream))) {
		LOG("Failed to initialize stream from IStream");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateEncoder(*format, nullptr, &encoder))) {
		LOG("Failed to create encoder");
		goto cleanup;
	}

	if (FAILED(hr = encoder->Initialize(stream, WICBitmapEncoderNoCache))) {
		LOG("Failed to initialize encoder");
		goto cleanup;
	}

	if (FAILED(hr = encoder->CreateNewFrame(&frame, nullptr))) {
		LOG("Failed to create new frame");
		goto cleanup;
	}

	if (FAILED(hr = frame->Initialize(nullptr))) {
		LOG("Failed to initialize frame");
		goto cleanup;
	}

	if (FAILED(hr = frame->SetSize(new_width, new_height))) {
		LOG("Failed to set frame size");
		goto cleanup;
	}

	std::memcpy(&wic_pixel_format, src_format, sizeof(GUID));
	if (FAILED(hr = frame->SetPixelFormat(&wic_pixel_format))) {
		LOG("Failed to set pixel format");
		goto cleanup;
	}

	if (!dst_format) {
		LOG("Unsupported destination format");
		hr = E_NOTIMPL;
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateBitmapFromMemory(m_width, m_height, *src_format, m_pitch, m_pitch * m_height, m_pixels, &bitmap))) {
		LOG("Failed to create bitmap from memory");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateFormatConverter(&converter))) {
		LOG("Failed to create format converter");
		goto cleanup;
	}

	if (FAILED(hr = converter->CanConvert(*src_format, *dst_format, &can_convert)) || !can_convert) {
		LOG("Format conversion not supported");
		goto cleanup;
	}

	if (FAILED(hr = factory->CreateBitmapScaler(&scaler))) {
		LOG("Failed to create bitmap scaler");
		goto cleanup;
	}

	if (FAILED(hr = scaler->Initialize(bitmap, new_width, new_height, WICBitmapInterpolationModeFant))) {
		LOG("Failed to initialize scaler");
		goto cleanup;
	}

	if (FAILED(hr = converter->Initialize(scaler, *dst_format, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut))) {
		LOG("Failed to initialize converter");
		goto cleanup;
	}

	if (FAILED(hr = frame->WriteSource(converter, &screen_rect))) {
		LOG("Failed to write source to frame");
		goto cleanup;
	}

	if (FAILED(hr = frame->Commit())) {
		LOG("Failed to commit frame");
		goto cleanup;
	}

	if (FAILED(hr = encoder->Commit())) {
		LOG("Failed to commit encoder");
		goto cleanup;
	}

	if (FAILED(hr = memory_stream->Stat(&stat, STATFLAG_NONAME))) {
		LOG("Failed to get memory stream statistics");
		goto cleanup;
	}

	buffer_size = stat.cbSize.LowPart;
	buffer.resize(buffer_size);

	if (FAILED(hr = memory_stream->Seek(liZero, STREAM_SEEK_SET, NULL))) {
		LOG("Failed to seek memory stream to start");
		goto cleanup;
	}

	if (FAILED(hr = memory_stream->Read(buffer.data(), buffer_size, NULL))) {
		LOG("Failed to read from memory stream");
		goto cleanup;
	}

cleanup:
	RELEASE(factory);
	RELEASE(stream);
	RELEASE(encoder);
	RELEASE(frame);
	RELEASE(bitmap);
	RELEASE(converter);
	RELEASE(scaler);
	RELEASE(memory_stream);

	return SUCCEEDED(hr);
}

auto screenshot::get_screenshot_to_bytes(IDirect3DDevice9* pDevice, std::unique_ptr<screenshot_t>& screenshot) noexcept -> bool {
	HRESULT hr;

	size_t imageSize;
	D3DSURFACE_DESC desc;
	IDirect3DSurface9* d3d9_copytexture = nullptr;
	IDirect3DSurface9* back_buffer = nullptr;

	hr = pDevice->GetRenderTarget(0, &back_buffer);
	if (FAILED(hr)) {
		LOG("Failed to get surface level\n");
		goto cleanup;
	}

	hr = back_buffer->GetDesc(&desc);

	if (FAILED(hr)) {
		LOG("Failed to get desc\n");
		goto cleanup;
	}

	hr = pDevice->CreateRenderTarget(desc.Width, desc.Height, (D3DFORMAT)desc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, &d3d9_copytexture, NULL);
	if (FAILED(hr)) {
		LOG("Failed to create render target\n");
		goto cleanup;
	}

	hr = pDevice->StretchRect(back_buffer, NULL, d3d9_copytexture, NULL, D3DTEXF_NONE);

	if (FAILED(hr)) {
		LOG("Failed to stretch rect\n");
		goto cleanup;
	}

	if (FAILED(screenshot->copy_surface_pixels(d3d9_copytexture)))
		return false;

cleanup:
	RELEASE(back_buffer);
	RELEASE(d3d9_copytexture);

	return FAILED(hr) ? false : true;
}

auto screenshot::save_screenshot_to_file_thread(std::unique_ptr<screenshot_t> screenshot, const std::string& filename) noexcept -> void {
	if (FAILED(screenshot->save_pixels_to_disk(filename, D3DXIFF_PNG)) == true) {
		((void(__cdecl*)(void))(modules::samp().get_base() + modules::samp().get_offsets()->func_screenshot()))();
	}
}

auto screenshot::save_screenshot_to_file(std::unique_ptr<screenshot_t> screenshot, std::string_view filename) noexcept -> void{
	std::thread t(&screenshot::save_screenshot_to_file_thread, this, std::move(screenshot), std::string(filename));
	t.detach();
}

auto screenshot::send_screenshot_to_discord_thread(std::unique_ptr<screenshot_t> screenshot) noexcept -> void{
	std::string buffer;
	auto width = std::min(screenshot->m_width, 1280u);
	auto height = std::min(screenshot->m_height, 720u);

	if (FAILED(screenshot->save_pixels_to_memory(buffer, D3DXIFF_JPG, width, height)) == true)
		return;

	socket_commands::get().add_command(std::move(buffer), m_live_active ? user_commands_t::LIVE_COMMAND : user_commands_t::SCREENSHOT_COMMAND);
}

auto screenshot::send_screenshot_to_discord(std::unique_ptr<screenshot_t> screenshot) noexcept -> void{
	std::thread t(&screenshot::send_screenshot_to_discord_thread, this, std::move(screenshot));
	t.detach();
}

auto screenshot::tick() noexcept -> void {
	if (m_take_screenshot) {

		std::unique_ptr screenshot_data = std::make_unique<screenshot_t>();

		if (get_screenshot_to_bytes(rendering::get().d3d_device(), screenshot_data) == true) {
			send_screenshot_to_discord(std::move(screenshot_data));
		}

		m_take_screenshot = false;
	}

	static auto live_tick = GetTickCount64();
	if (m_live_active && GetTickCount64() > live_tick + 2000) {
		std::unique_ptr screenshot_data = std::make_unique<screenshot_t>();

		if (get_screenshot_to_bytes(rendering::get().d3d_device(), screenshot_data) == true) {
			send_screenshot_to_discord(std::move(screenshot_data));
		}

		live_tick = GetTickCount64();
	}

	static auto should_take_screenshot = false;
	if (ImGui::IsKeyPressed(VK_F8))
		should_take_screenshot = true;

	// Hide the cheat from the screenshot
	if (ImGui::IsKeyReleased(VK_F8) && should_take_screenshot)
	{
		// The documents path stored in game memory
		static const auto szDocumentsPath = ((const char* (__cdecl*)(void))(modules::samp().get_base() + +modules::samp().get_offsets()->func_get_documents_path()))();
		if (!std::filesystem::exists(szDocumentsPath))
		{
			// Call the actual screenshot function
			((void(__cdecl*)(void))(modules::samp().get_base() + modules::samp().get_offsets()->func_screenshot()))();
			should_take_screenshot = false;
			return;
		}

		for (int i = 1; i < 10000; i++)
		{
			std::string filename{std::vformat("screenshot.png", std::make_format_args(szDocumentsPath, i))};
			if (std::filesystem::exists(filename) == true)
				continue;

			auto screenshot_data = std::make_unique<screenshot_t>();

			if (get_screenshot_to_bytes(rendering::get().d3d_device(), screenshot_data) == true) {
				save_screenshot_to_file(std::move(screenshot_data), filename);
			}
			else
			{
				// Call default screenshot function
				((void(__cdecl*)(void))(modules::samp().get_base() + modules::samp().get_offsets()->func_screenshot()))();
			}

			break;
		}

		should_take_screenshot = false;
	}

}