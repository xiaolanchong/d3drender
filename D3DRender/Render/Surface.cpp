#include "stdafx.h"
#include "Surface.h"
#include "../include/Stencil.h"
#include "SurfaceUtils.h"

namespace d3drender
{
		D3DFORMAT ConvertFormat(PixelFormat fmt)
		{
			switch (fmt)
			{
			case PixelFormat::R5G5B5: return D3DFMT_X1R5G5B5;
			case PixelFormat::R5G6B5: return D3DFMT_R5G6B5;
			case PixelFormat::R8G8B8: return D3DFMT_R8G8B8;
			default:
				assert(!"Unknown pixel format");
				return D3DFMT_UNKNOWN;
			}
		}

	Surface::Surface(const IDirect3DDevicePtr& device,
		const SurfaceCreationParams& params, bool temporary)
		: m_creationParams(params)
		, m_device(device)
		, m_chromaKeyColor(RGB(0, 0, 0))
		, m_chromaKeyColorValid(false)
	{
		const unsigned int levels = 1;
		const long d3dRenderUsage = D3DUSAGE_RENDERTARGET;// | (temporary ? D3DUSAGE_DYNAMIC : 0);
		const D3DFORMAT d3dRenderTargetFmt = ConvertFormat(params.m_pixelFormat); //D3DFMT_A8R8G8B8;

		HRESULT hr = E_FAIL;
		hr = m_device->CreateTexture(params.m_width, params.m_height, levels, (DWORD) d3dRenderUsage, d3dRenderTargetFmt,
			D3DPOOL_DEFAULT, &m_targetTexture, nullptr);
		if (hr != S_OK)
		{
			throw std::runtime_error("Texture creation failed");
		}

		const D3DFORMAT d3dInputTextureFmt = ConvertFormat(params.m_pixelFormat);
		const auto d3dTextUsage = temporary ? D3DUSAGE_DYNAMIC : 0;
		hr = m_device->CreateTexture(params.m_width, params.m_height, levels, (DWORD)d3dTextUsage, d3dInputTextureFmt,
			D3DPOOL_SYSTEMMEM, &m_inputTexture, nullptr);
		if (hr != S_OK)
		{
			throw std::runtime_error("Offscreen plain surface creation failed");
		}

		hr = m_targetTexture->GetSurfaceLevel(0, &m_targetTextureLevel0);
		if (hr != S_OK)
		{
			throw std::runtime_error("Texture level 0 getting failed");
		}

		hr = m_inputTexture->GetSurfaceLevel(0, &m_inputTextureLevel0);
		if (hr != S_OK)
		{
			throw std::runtime_error("Texture level 0 getting failed");
		}

		m_blitter.reset(new Blitter(m_device, m_targetTextureLevel0, Blitter::SurfaceType::OffScreenPlain, SIZE{ LONG(params.m_width), LONG(params.m_height) }));
	}

	Surface::Surface(const IDirect3DDevicePtr& device,
		const SurfaceCreationParams& params, COLORREF chromaKeyColor)
	: Surface(device, params, true)  /// TODO: set RT TARGET off also
	{
		SetChromaKey(chromaKeyColor);
		m_blitter->ColorBlt(nullptr, chromaKeyColor);
		m_blitter->ResetBltDone();
	}

	LockResult Surface::Lock(const RECT* rect)
	{
		if (m_blitter->WasBltDone())
		{
			HRESULT hr = m_device->GetRenderTargetData(m_targetTextureLevel0, m_inputTextureLevel0);
			ATLVERIFY(hr == S_OK);

			m_blitter->ResetBltDone();
		}
		//ATLVERIFY(!m_blitter->WasBltDone());
		// D3DLOCK_DISCARD for temporary?
		return LockTexture(m_inputTexture, rect);
	}

	void Surface::Unlock()
	{
		HRESULT hr = m_inputTexture->UnlockRect(0);
		ATLVERIFY(hr == S_OK);

		hr = m_device->UpdateTexture(m_inputTexture, m_targetTexture);
		ATLVERIFY(hr == S_OK);

		m_blitter->ResetBltDone();
	}

	HDC Surface::GetDC()
	{
		ATLVERIFY(!m_blitter->WasBltDone());
		HDC hdc = nullptr;
		HRESULT hr = E_FAIL;
		hr = m_inputTextureLevel0->GetDC(&hdc);
		if (hr == S_OK)
		{
			return hdc;
		}
		return nullptr;
	}

	void Surface::ReleaseDC(HDC hdc)
	{
		HRESULT hr = m_inputTextureLevel0->ReleaseDC(hdc);
		ATLVERIFY(hr == S_OK);

		hr = m_device->UpdateTexture(m_inputTexture, m_targetTexture);
		ATLVERIFY(hr == S_OK);

		m_blitter->ResetBltDone();
	}

	void Surface::SetChromaKey(COLORREF color)
	{
		m_chromaKeyColorValid = true;
		m_chromaKeyColor = color;

		m_blitter->SetChromaKey(color);
	}

	COLORREF Surface::GetChromaKey() const
	{
		//assert(m_chromaKeyColorValid);
		return m_chromaKeyColorValid ? m_chromaKeyColor : RGB(0, 0, 0);
	}

	bool Surface::IsOK() const
	{
		return m_device && m_targetTexture;
	}

	ISurfaceBlt& Surface::GetBlitter()
	{
		return *m_blitter;
	}

	void Surface::Flip()
	{
		assert(!"Non-primary surface cannot be flipped");
	}

}