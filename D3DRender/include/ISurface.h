#pragma once

#include <memory>
#include <cstdint>
#include <windows.h>

namespace d3drender
{
	class ISurfaceBlt;

	using BufferPtr = void*;
	using Pixel16 = std::uint16_t;

	enum class PixelFormat
	{
		R5G5B5,
		R5G6B5,
		R8G8B8,
	};
	struct SurfaceCreationParams
	{
		PixelFormat m_pixelFormat;
		unsigned int m_width;
		unsigned int m_height;
	};

	struct LockResult
	{
		BufferPtr m_buffer = nullptr;
		int m_pitch = 0;
	};

	class ISurface
	{
	public:
		virtual ~ISurface() = default;

		virtual LockResult Lock(const RECT* rect) = 0;
		virtual void Unlock() = 0;

		virtual HDC GetDC() = 0;
		virtual void ReleaseDC(HDC hdc) = 0;

		virtual void SetChromaKey(COLORREF color) = 0;
		virtual COLORREF GetChromaKey() const = 0;

		virtual const SurfaceCreationParams& GetParams() const = 0;
		virtual bool IsOK() const = 0;

		virtual ISurfaceBlt& GetBlitter() = 0;
		virtual void Flip() = 0;
	};

	using ISurfacePtr = std::shared_ptr<ISurface>;
}