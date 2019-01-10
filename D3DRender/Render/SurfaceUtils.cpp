
#include "stdafx.h"
#include "SurfaceUtils.h"

namespace d3drender
{

	LockResult LockSurface(const IDirect3DSurfacePtr& surface, const RECT* rect)
	{
		D3DLOCKED_RECT lockedRect{};
		HRESULT hr = surface->LockRect(&lockedRect, rect, D3DLOCK_NOSYSLOCK);
		ATLASSERT(hr == S_OK);
		if (hr == S_OK)
		{
			return LockResult{ lockedRect.pBits, lockedRect.Pitch };
		}
		else
		{
			return LockResult{ nullptr, 0 };
		}
	}

	LockResult LockTexture(const IDirect3DTexturePtr& texture, const RECT* rect)
	{
		D3DLOCKED_RECT lockedRect{};
		HRESULT hr = texture->LockRect(0, &lockedRect, rect, 0);// D3DLOCK_NOSYSLOCK);
		if (hr == S_OK)
		{
			return LockResult{ lockedRect.pBits, lockedRect.Pitch };
		}
		else
		{
			return LockResult{ nullptr, 0 };
		}
	}

	CPointF TransformToXYZ(const CPoint& pt, const CSize& viewport)
	{
		// +--------------------------- +       +-------------------+
		// |(0, 0)                      |  ---> |(-1, 1)      (1, 1)|
		// |                            |       |                   |
		// +         (width-1, height-1)+       |(-1,-1)-----(-1, 1)+
		//

		float x = viewport.cx ? ( 2 * float(pt.x) / viewport.cx - 1) : 0;
		float y = viewport.cy ? (-2 * float(pt.y) / viewport.cy + 1) : 0;
		return CPointF{ x, y };
	}

	CRectF TransformToXYZ(const CRect& rect, const CSize& viewport)
	{
		CPointF topLeft = TransformToXYZ(rect.TopLeft(), viewport);
		CPointF bottomRight = TransformToXYZ(rect.BottomRight(), viewport);
		return CRectF{ topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
	}

	namespace internal
	{

		Pixel16 ConvertToPixel16(PixelFormat fmt, COLORREF color)
		{
			const Pixel16 r = GetRValue(color);
			const Pixel16 g = GetGValue(color);
			const Pixel16 b = GetBValue(color);
			switch (fmt)
			{
			case PixelFormat::R5G6B5:
				return (Pixel16)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3));
			case PixelFormat::R5G5B5:
				return (Pixel16)(((r & 0xF8) << 7) | ((g & 0xF8) << 2) | ((b & 0xF8) >> 3));
			case PixelFormat::R8G8B8:
			default:
				assert(false);
				return Pixel16(0);
			}
		}
	}
}

