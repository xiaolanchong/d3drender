#pragma once

#include <cstdint>
#include "../include/ISurface.h"
#include "../include/ISurfaceBlt.h"
#include "RenderTypes.h"

namespace d3drender
{
	/// Primary (front) pseudo-surface
	/// Only function - start, end the render loop
	class PrimarySurface : public ISurface, private ISurfaceBlt
	{
	public:
		PrimarySurface(const IDirect3DDevicePtr& device,
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

		PrimarySurface& operator= (const PrimarySurface&) = delete;
		PrimarySurface(const PrimarySurface&) = delete;

	private:
		virtual bool Blt(int x, int y, ISurface& srcSurf,
			const RECT* srcRect, const BltParams& params) override;
		virtual bool TileBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
			int anchorX, int anchorY, UseSrcChromaKey useSrcChromaKey) override;
		virtual bool BevelBlt(const RECT* dstRect, int thickness, BevelDir dir) override;
		virtual bool ColorBlt(const RECT* dstRect, COLORREF color) override;
		virtual bool StretchBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
			const BltParams& params) override;

		virtual bool StencilMixBlt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const Stencil& stencil) override;
	private:
		const SurfaceCreationParams m_creationParams;
		IDirect3DDevicePtr  m_device;
	};

	inline const SurfaceCreationParams& PrimarySurface::GetParams() const
	{
		return m_creationParams;
	}
}