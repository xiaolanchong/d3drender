#pragma once

#include <atltypes.h>
#include "../include/ISurface.h"
#include "RenderTypes.h"

namespace d3drender
{
	struct CPointF
	{
		float x, y;
	};

	struct CRectF
	{
		float left, top, right, bottom;
	};

	LockResult LockSurface(const IDirect3DSurfacePtr& surface, const RECT* rect);
	LockResult LockTexture(const IDirect3DTexturePtr& texture, const RECT* rect);

	CPointF TransformToXYZ(const CPoint& pt, const CSize& viewport);
	CRectF TransformToXYZ(const CRect& rect, const CSize& viewport);

	namespace internal
	{
		Pixel16 ConvertToPixel16(PixelFormat fmt, COLORREF color);
	}
}