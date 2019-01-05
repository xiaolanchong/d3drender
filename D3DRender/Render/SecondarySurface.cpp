#include "stdafx.h"
#include <stdexcept>
#include "SecondarySurface.h"
#include "SurfaceUtils.h"
#include "Surface.h"

namespace d3drender
{
	SecondarySurface::SecondarySurface(const IDirect3DDevicePtr& device,
		const SurfaceCreationParams& params)
		: m_creationParams(params)
		, m_device(device)
	{
		HRESULT hr = m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_surface);
		if (hr != S_OK)
		{
			throw std::runtime_error("Surface creation failed");
		}

		m_blitter.reset(new Blitter(m_device, m_surface, Blitter::SurfaceType::BackBuffer,
			SIZE{ LONG(params.m_width), LONG(params.m_height) }));
	}

	LockResult SecondarySurface::Lock(const RECT* rect)
	{
		return LockSurface(m_surface, rect);
	}

	void SecondarySurface::Unlock()
	{
		HRESULT hr = m_surface->UnlockRect();
		ATLVERIFY(hr == S_OK);
	}

	HDC SecondarySurface::GetDC()
	{
		HDC hdc = nullptr;
		HRESULT hr = m_surface->GetDC(&hdc);
		if (hr == S_OK)
		{
			return hdc;
		}
		return nullptr;
	}

	void SecondarySurface::ReleaseDC(HDC hdc)
	{
		HRESULT hr = m_surface->ReleaseDC(hdc);
		ATLVERIFY(hr == S_OK);
	}

	void SecondarySurface::SetChromaKey(COLORREF /*color*/)
	{
		assert(!"SetChromaKey not supported for secondary surface");
	}

	COLORREF SecondarySurface::GetChromaKey() const
	{
		assert(!"GetChromaKey not supported for secondary surface");
		return RGB(0, 0, 0);
	}

	bool SecondarySurface::IsOK() const
	{
		return true;
	}

	ISurfaceBlt& SecondarySurface::GetBlitter()
	{
		return *m_blitter;
	}

	void SecondarySurface::Flip()
	{
		assert(!"Flip not supported for secondary surface");
	}

	//////////////////////////////////////////


}