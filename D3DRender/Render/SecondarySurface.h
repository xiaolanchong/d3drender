#pragma once

#include <cstdint>
#include "../include/ISurface.h"
#include "../include/ISurfaceBlt.h"
#include "RenderTypes.h"
#include "Blitter.h"

namespace d3drender
{
	/// Secondary (back buffer) surface
	class SecondarySurface : public ISurface
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

		SecondarySurface& operator= (const SecondarySurface&) = delete;
		SecondarySurface(const SecondarySurface&) = delete;

	private:
		const SurfaceCreationParams m_creationParams;
		IDirect3DDevicePtr  m_device;
		IDirect3DSurfacePtr m_surface;
		std::unique_ptr<Blitter> m_blitter;
	};

	inline const SurfaceCreationParams& SecondarySurface::GetParams() const
	{
		return m_creationParams;
	}
}