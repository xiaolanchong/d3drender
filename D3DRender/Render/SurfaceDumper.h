#pragma once

#include <atlstr.h>
#include "RenderTypes.h"

namespace d3drender
{
	class SurfaceDumper
	{
	public:
		void Dump(const IDirect3DSurfacePtr& surface, const CString& operation);

	private:
		size_t m_operationCounter = 0;
	};
}