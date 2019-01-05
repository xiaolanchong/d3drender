#include "stdafx.h"
#include <cassert>
#include "PrimarySurface.h"

namespace d3drender
{
	PrimarySurface::PrimarySurface(const IDirect3DDevicePtr& device,
		const SurfaceCreationParams& params)
		: m_creationParams(params)
		, m_device(device)
	{

	}

	LockResult PrimarySurface::Lock(const RECT* /*rect*/)
	{
		assert(!"Lock not supported for primary surface");
		return LockResult{};
	}

	void PrimarySurface::Unlock()
	{
		assert(!"Unlock not supported for primary surface");
	}

	HDC PrimarySurface::GetDC()
	{
		assert(!"GetDC not supported for primary surface");
		return nullptr;
	}

	void PrimarySurface::ReleaseDC(HDC /*hdc*/)
	{
		assert(!"ReleaseDC not supported for primary surface");
	}

	void PrimarySurface::SetChromaKey(COLORREF /*color*/)
	{
		assert(!"SetChromaKey not supported for primary surface");
	}

	COLORREF PrimarySurface::GetChromaKey() const
	{
		assert(!"GetChromaKey not supported for primary surface");
		return RGB(0, 0, 0);
	}

	bool PrimarySurface::IsOK() const
	{
		return true;
	}

	ISurfaceBlt& PrimarySurface::GetBlitter()
	{
		return *this;
	}

	void PrimarySurface::Flip()
	{
		HRESULT hr = m_device->Present(nullptr, nullptr, nullptr, nullptr);
		ATLVERIFY(SUCCEEDED(hr));
	}

	/////////////////////////////////////////////////////////////////////

	bool PrimarySurface::Blt(int /*x*/, int /*y*/, ISurface& /*srcSurf*/,
		const RECT* /*srcRect*/, const BltParams& /*params*/)
	{
		// done by Present
		return true;
	}

	bool PrimarySurface::TileBlt(const RECT* /*dstRect*/, ISurface& /*srcSurf*/, const RECT* /*srcRect*/,
		int /*anchorX*/, int /*anchorY*/, UseSrcChromaKey /*useSrcChromaKey*/)
	{
		assert(!"TileBlt not supported for primary surface");
		return false;
	}

	bool PrimarySurface::BevelBlt(const RECT* /*dstRect*/, ISurface& /*srcSurf*/, const RECT* /*srcRect*/,
		int /*lightX*/, int /*ligthY*/, BevelDir /*dir*/)
	{
		assert(!"BevelBlt not supported for primary surface");
		return false;
	}

	bool PrimarySurface::ColorBlt(const RECT* /*dstRect*/, COLORREF /*color*/)
	{
		assert(!"ColorBlt not supported for primary surface");
		return false;
	}

	bool PrimarySurface::StretchBlt(const RECT* /*dstRect*/, ISurface& /*srcSurf*/, const RECT* /*srcRect*/,
		const BltParams& /*params*/)
	{
		assert(!"StretchBlt not supported for primary surface");
		return false;
	}

	bool PrimarySurface::StencilMixBlt(int /*x*/, int /*y*/, ISurface& /*srcSurf*/, const RECT* /*srcRect*/, const Stencil& /*stencil*/)
	{
		assert(!"StencilMixBlt not supported for primary surface");
		return false;
	}
}
