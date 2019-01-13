#pragma once

#include <cstdint>
#include <mutex>
#include <thread>
#include "../include/ISurface.h"
#include "../include/ISurfaceBlt.h"
#include "RenderTypes.h"
#include "Blitter.h"

namespace d3drender
{
	using SharedMutexPtr = std::shared_ptr<std::mutex>;

	class ISourceSurface : public ISurface
	{
	public:
		virtual IDirect3DTexturePtr GetOutputTexture() = 0;
	};

	class Surface : public ISourceSurface,
		public std::enable_shared_from_this<Surface>
	{
	public:
		Surface(const IDirect3DDevicePtr& device,
			const ShaderContext& shaders,
			const SurfaceCreationParams& params, 
			bool temporary);

		Surface(const IDirect3DDevicePtr& device,
			const ShaderContext& shaders,
			const SurfaceCreationParams& params,
			COLORREF chromaKeyColor);

		Surface& operator= (const Surface&) = delete;
		Surface(const Surface&) = delete;

		virtual IDirect3DTexturePtr GetOutputTexture() override;
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

		IDirect3DTexturePtr m_targetTexture;
		IDirect3DSurfacePtr m_targetTextureLevel0; // Level 0 surface

		IDirect3DTexturePtr m_inputTexture;
		IDirect3DSurfacePtr m_inputTextureLevel0; // Level 0 surface

		D3DCOLOR m_chromaKeyColor;
		bool m_chromaKeyColorValid;

		std::unique_ptr<Blitter> m_blitter;
	};

	D3DFORMAT ConvertFormat(PixelFormat fmt);

	inline const SurfaceCreationParams& Surface::GetParams() const
	{
		return m_creationParams;
	}
	/*
	inline IDirect3DSurfacePtr Surface::GetRenderTarget() const
	{
		return m_textureLevel0;
	}*/

	inline IDirect3DTexturePtr Surface::GetOutputTexture()
	{
		return m_targetTexture;
	}
}