#include "stdafx.h"
#include <stdexcept>
#include "SecondarySurface.h"
#include "SurfaceUtils.h"
#include "Surface.h"

namespace d3drender
{
	namespace
	{
		const COLORREF c_chromaKeyColor = RGB(47, 223, 173);
	}

	SecondarySurface::SecondarySurface(const IDirect3DDevicePtr& device,
		const ShaderContext& shaders,
		const SurfaceCreationParams& params)
		: m_creationParams(params)
		, m_device(device)
		, m_shaderContext(shaders)
	{
		HRESULT hr = m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_surface);
		if (hr != S_OK)
		{
			throw std::runtime_error("Surface creation failed");
		}

		CSize size{ LONG(params.m_width), LONG(params.m_height) };
		BlitterContext context(shaders);
		context.m_device = m_device;
		context.m_renderTarget = m_surface;
		context.m_surfaceType = SurfaceType::BackBuffer;
		context.m_extents = size;

		m_blitter = std::make_unique<Blitter>(context);
		m_blitter->ColorBlt(nullptr, RGB(64, 64, 64));
		m_blitter->ResetBltDone();
	}

	LockResult SecondarySurface::Lock(const RECT* /*rect*/)
	{
#if 0
		const long d3dRenderUsage = 0;// | (temporary ? D3DUSAGE_DYNAMIC : 0);
		const D3DFORMAT d3dFmt = ConvertFormat(m_creationParams.m_pixelFormat); // ConvertFormat(params.m_pixelFormat); //D3DFMT_X8R8G8B8;

		HRESULT hr = E_FAIL;
		hr = m_device->CreateOffscreenPlainSurface(m_creationParams.m_height, m_creationParams.m_width, d3dFmt, D3DPOOL_SYSTEMMEM, &m_lockedSurface, nullptr);
		ATLVERIFY(hr == S_OK);

		hr = m_device->GetRenderTargetData(m_lockedSurface, m_surface);
		ATLVERIFY(hr == S_OK);

		if (rect)
		{
			m_lockedRect = std::make_unique<CRect>(*rect);
		}

		return LockSurface(m_lockedSurface, rect);
#else
		ATLASSERT(!"SecondarySurface is not lockable");
		return LockResult{};
#endif
	}

	void SecondarySurface::Unlock()
	{
#if 0
		HRESULT hr = m_surface->UnlockRect();
		ATLVERIFY(hr == S_OK);
#else
		if (m_lockedSurface)
		{
			HRESULT hr = E_FAIL;
			hr = m_lockedSurface->UnlockRect();
			ATLVERIFY(hr == S_OK);

			hr = m_device->UpdateSurface(m_lockedSurface, m_lockedRect.get(), m_surface, nullptr);
			ATLVERIFY(hr == S_OK);

			m_lockedSurface = nullptr;
			m_lockedRect = nullptr;
		}
#endif
	}

	HDC SecondarySurface::GetDC()
	{
		ISurface& tempSurface = createTempSurface();
		HDC hdc = tempSurface.GetDC();
		if (hdc)
		{
			HBRUSH brush = ::CreateSolidBrush(c_chromaKeyColor);
			ATLASSERT(brush);
			CRect wholeArea(0, 0, m_creationParams.m_width, m_creationParams.m_height);
			BOOL res = ::FillRect(hdc, &wholeArea, brush);
			ATLASSERT(res);
			res = ::DeleteObject(brush);
			ATLASSERT(res);
		}
		return hdc;
	}

	void SecondarySurface::ReleaseDC(HDC hdc)
	{
		if (m_tempSurface)
		{
			m_tempSurface->ReleaseDC(hdc);
			m_blitter->Blt(0, 0, *m_tempSurface, nullptr, BltParams{ UseSrcChromaKey::Yes });
			m_tempSurface = nullptr;
		}
	//	ATLVERIFY(hr == S_OK);
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


	ISurface& SecondarySurface::createTempSurface()
	{
		if (!m_tempSurface)
		{
			m_tempSurface = std::make_unique<Surface>(m_device, m_shaderContext, GetParams(), c_chromaKeyColor);
		}
		return *m_tempSurface;
	}

	IDirect3DTexturePtr SecondarySurface::GetOutputTexture()
	{
		HRESULT hr = E_FAIL;
		if (!m_tempCopyTexture)
		{
			
			const int levels = 1;
			const D3DFORMAT d3dInputTextureFmt = D3DFMT_X8R8G8B8;// ConvertFormat(GetParams().m_pixelFormat);
			const auto d3dTextUsage = 0;// D3DUSAGE_DYNAMIC;
			hr = m_device->CreateTexture(GetParams().m_width, GetParams().m_height, levels, (DWORD)d3dTextUsage, d3dInputTextureFmt,
				D3DPOOL_SYSTEMMEM, &m_tempCopyTexture, nullptr);
			ATLVERIFY(hr == S_OK);
		}

		IDirect3DSurfacePtr tempCopyTextureLevel0;
		hr = m_tempCopyTexture->GetSurfaceLevel(0, &tempCopyTextureLevel0);
		ATLVERIFY(hr == S_OK);

		IDirect3DSurfacePtr backBufferSurface;
		hr = m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBufferSurface);
		ATLVERIFY(hr == S_OK);

		hr = m_device->GetRenderTargetData(backBufferSurface, tempCopyTextureLevel0);
		ATLVERIFY(hr == S_OK);

		return m_tempCopyTexture;
	}

}