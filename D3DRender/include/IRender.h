#pragma once

#include <cstdint>
#include <memory>
#include "ISurface.h"

namespace d3drender
{
	class IRender
	{
	public:
		virtual ~IRender() = default;

		virtual ISurfacePtr CreateSurface(const SurfaceCreationParams& params) = 0;
		virtual ISurfacePtr CreateTemporarySurface(const SurfaceCreationParams& params) = 0;
		virtual ISurfacePtr CreatePrimarySurface(const SurfaceCreationParams& params) = 0;
		virtual ISurfacePtr CreateSecondarySurface(const SurfaceCreationParams& params) = 0;
	};

	using IRenderPtr = std::shared_ptr<IRender>;
}