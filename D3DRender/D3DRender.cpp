// D3DRender.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "D3DRender.h"
#include "Render/Render.h"
#include "Render/SurfaceUtils.h"

namespace d3drender
{

	IRenderPtr CreateRender(bool windowMode, HWND hWnd, const ILoggerPtr& logger)
	{
		return std::make_shared<Render>(hWnd, windowMode, logger);
	}

	Pixel16 ConvertToPixel16(PixelFormat fmt, COLORREF color)
	{
		return internal::ConvertToPixel16(fmt, color);
	}
}