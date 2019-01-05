#include "stdafx.h"
#include "Surface.h"
#include "../include/Stencil.h"
#include "SurfaceUtils.h"

namespace d3drender
{
	namespace
	{
		D3DFORMAT convert(PixelFormat fmt)
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
	}

	Surface::Surface(const IDirect3DDevicePtr& device,
		const SurfaceCreationParams& params)
		: m_creationParams(params)
		, m_device(device)
		, m_chromaKeyColor(RGB(0, 0, 0))
		, m_chromaKeyColorValid(false)
	{
		const unsigned int levels = 1;
		const unsigned int d3dUsage = D3DUSAGE_RENDERTARGET;
		const auto d3dFmt = convert(params.m_pixelFormat);

		HRESULT hr = E_FAIL;
		hr = m_device->CreateTexture(params.m_width, params.m_height, levels, d3dUsage, d3dFmt,
			D3DPOOL_DEFAULT, &m_texture, nullptr);
		if (hr != S_OK)
		{
			throw std::runtime_error("Texture creation failed");
		}

		hr = m_device->CreateTexture(params.m_width, params.m_height, levels, 0, d3dFmt,
			D3DPOOL_SYSTEMMEM, &m_inputTexture, nullptr);
		if (hr != S_OK)
		{
			throw std::runtime_error("Offscreen plain surface creation failed");
		}
#if 1
		hr = m_texture->GetSurfaceLevel(0, &m_textureLevel0);
		if (hr != S_OK)
		{
			throw std::runtime_error("Texture level 0 getting failed");
		}

		hr = m_inputTexture->GetSurfaceLevel(0, &m_inputTextureLevel0);
		if (hr != S_OK)
		{
			throw std::runtime_error("Texture level 0 getting failed");
		}
#endif
		m_blitter.reset(new Blitter(m_device, m_textureLevel0, Blitter::SurfaceType::OffScreenPlain, SIZE{ LONG(params.m_width), LONG(params.m_height) }));
	}

	LockResult Surface::Lock(const RECT* rect)
	{
		ATLVERIFY(!m_blitter->WasBltDone());
		return LockTexture(m_inputTexture, rect);
	}

	void Surface::Unlock()
	{
		HRESULT hr = m_inputTexture->UnlockRect(0);
		ATLVERIFY(hr == S_OK);

		hr = m_device->UpdateTexture(m_inputTexture, m_texture);
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

		hr = m_device->UpdateTexture(m_inputTexture, m_texture);
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
		assert(m_chromaKeyColorValid);
		return m_chromaKeyColorValid ? m_chromaKeyColor : RGB(0, 0, 0);
	}

	bool Surface::IsOK() const
	{
		return m_device && m_texture;
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