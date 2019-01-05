
#pragma once

#include "include/IRender.h"
#include "include/ILogger.h"

#if D3DRENDER_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace d3drender
{
	DLLEXPORT IRenderPtr CreateRender(bool windowMode, HWND hWnd, const ILoggerPtr& logger);

	DLLEXPORT Pixel16 ConvertToPixel16(PixelFormat fmt, COLORREF color);
}