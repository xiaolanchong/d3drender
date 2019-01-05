#pragma once

#include <cstdint>
#include "../include/ISurface.h"
#include "../include/ISurfaceBlt.h"
#include "RenderTypes.h"
#include "Blitter.h"

namespace d3drender
{
	class Surface : public ISurface
	{
	public:
		Surface(const IDirect3DDevicePtr& device,
			const SurfaceCreationParams& params);

		Surface& operator= (const Surface&) = delete;
		Surface(const Surface&) = delete;

		//IDirect3DSurfacePtr GetRenderTarget() const;
		IDirect3DTexturePtr GetTexture() const;
	private:
		virtual LockResult Lock(const RECT* rect) override;
		virtual void Unlock() override;

		virtual HDC GetDC() override;
		virtual void ReleaseDC(HDC hdc) override;

		virtual void SetChromaKey(COLORREF color) override;
		virtual COLORREF GetChromaKey() const override;

		virtual const SurfaceCreationParams& GetParams() const;
		virtual bool IsOK() const override;

		virtual ISurfaceBlt& GetBlitter() override;
		virtual void Flip() override;

	private:
		const SurfaceCreationParams m_creationParams;
		IDirect3DDevicePtr  m_device;

		IDirect3DTexturePtr m_texture;
		IDirect3DSurfacePtr m_textureLevel0; // Level 0 surface

		IDirect3DTexturePtr m_inputTexture;
		IDirect3DSurfacePtr m_inputTextureLevel0; // Level 0 surface

		D3DCOLOR m_chromaKeyColor;
		bool m_chromaKeyColorValid;

		std::unique_ptr<Blitter> m_blitter;
	};

	inline const SurfaceCreationParams& Surface::GetParams() const
	{
		return m_creationParams;
	}
	/*
	inline IDirect3DSurfacePtr Surface::GetRenderTarget() const
	{
		return m_textureLevel0;
	}*/

	inline IDirect3DTexturePtr Surface::GetTexture() const
	{
		return m_texture;
	}
}