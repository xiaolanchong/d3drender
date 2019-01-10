#pragma once

#include <cstdint>
#include "../include/ISurface.h"
#include "../include/ISurfaceBlt.h"
#include "Surface.h"
#include "RenderTypes.h"
#include "Blitter.h"

namespace d3drender
{
	/// Secondary (back buffer) surface
	class SecondarySurface : public ISourceSurface
	{
	public:
		SecondarySurface(const IDirect3DDevicePtr& device,
			const SurfaceCreationParams& params);

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

		virtual IDirect3DTexturePtr GetOutputTexture() override;

		SecondarySurface& operator= (const SecondarySurface&) = delete;
		SecondarySurface(const SecondarySurface&) = delete;
	private:
		ISurface& createTempSurface();
		void copyToBackBuffer();

		void createTempCopyTexture();

	private:
		const SurfaceCreationParams m_creationParams;
		IDirect3DDevicePtr  m_device;
		IDirect3DSurfacePtr m_surface;

		IDirect3DTexturePtr m_tempCopyTexture;

		std::unique_ptr<ISurface> m_tempSurface;
		std::unique_ptr<Blitter> m_blitter;

		IDirect3DSurfacePtr m_lockedSurface;
		std::unique_ptr<CRect> m_lockedRect;
	};

	inline const SurfaceCreationParams& SecondarySurface::GetParams() const
	{
		return m_creationParams;
	}
}