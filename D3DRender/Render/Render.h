#pragma once

#include "../include/IRender.h"
#include "../include/ILogger.h"
#include "RenderTypes.h"

namespace d3drender
{
	class Render : public IRender
	{
	public:
		Render(HWND hWnd, bool windowMode, const ILoggerPtr& logger);

		virtual ISurfacePtr CreateSurface(const SurfaceCreationParams& params) override;
		virtual ISurfacePtr CreatePrimarySurface(const SurfaceCreationParams& params) override;
		virtual ISurfacePtr CreateSecondarySurface(const SurfaceCreationParams& params) override;

	private:
		ILoggerPtr m_logger;

		IDirect3DPtr m_d3d;
		IDirect3DDevicePtr m_device;
		const bool m_windowMode;
	};
}