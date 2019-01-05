#pragma once

#include <memory>
#include <cstdint>
#include <windows.h>

namespace d3drender
{
	class Stencil;
	class ISurface;

	enum class UseSrcChromaKey
	{
		Yes,
		No
	};

	enum class BlendSurfaces
	{
		Yes,
		No
	};

	struct BltParams
	{
		UseSrcChromaKey m_useSrcChromaKey = UseSrcChromaKey::No;
		BlendSurfaces m_blendSurfaces     = BlendSurfaces::No;
	};

	enum class BevelDir
	{
		In,
		Out
	};

	class ISurfaceBlt
	{
	public:
		virtual ~ISurfaceBlt() = default;

		virtual bool Blt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const BltParams& params) = 0;
		virtual bool TileBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
			int anchorX, int anchorY, UseSrcChromaKey useSrcChromaKey) = 0;
		virtual bool BevelBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
			int lightX, int ligthY, BevelDir dir) = 0;
		virtual bool ColorBlt(const RECT* dstRect, COLORREF color) = 0;
		virtual bool StretchBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect, const BltParams& params) = 0;
		virtual bool StencilMixBlt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const Stencil& stencil) = 0;
	};
}