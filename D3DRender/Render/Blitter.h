
#pragma once

#include "../include/ISurfaceBlt.h"
#include "RenderTypes.h"
#include "SurfaceUtils.h"
#include "SurfaceDumper.h"

namespace d3drender
{
	class Blitter : public ISurfaceBlt
	{
	public:

		enum class SurfaceType
		{
			OffScreenPlain,
			BackBuffer
		};

		Blitter(const IDirect3DDevicePtr& device, const IDirect3DSurfacePtr& dstSurface, SurfaceType surfaceType, const CSize& extents);

		Blitter& operator= (const Blitter&) = delete;
		Blitter(const Blitter&) = delete;

		virtual bool Blt(int x, int y, ISurface& srcSurf,
			const RECT* srcRect, const BltParams& params) override;
		virtual bool TileBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
			int anchorX, int anchorY, UseSrcChromaKey useSrcChromaKey) override;
		virtual bool BevelBlt(const RECT* dstRect, int thickness, BevelDir dir) override;
		virtual bool ColorBlt(const RECT* dstRect, COLORREF color) override;
		virtual bool StretchBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
			const BltParams& params) override;

		virtual bool StencilMixBlt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const Stencil& stencil) override;

		void SetChromaKey(COLORREF color)
		{
			m_chromaKeyColor = color;
		}

		bool WasBltDone() const
		{
			return m_wasBltDone;
		}

		void ResetBltDone()
		{
			m_wasBltDone = false;
		}

		bool BltWithChromaKey(int x, int y, const IDirect3DTexturePtr& texture, const CRect& rect, COLORREF chromaKeyColor);

	private:

		void setupMatrices();
		void drawSquare(const RECT* dstRect, D3DCOLOR color);
		void drawSquare(const RECT& dstRect, D3DCOLOR color);
		IDirect3DVBPtr createSquareBuffer(const RECT& rect, D3DCOLOR color);

		void drawTexturedSquare(const CRect& dstRect, const IDirect3DTexturePtr& texture, const RECT* srcRect,
			const BltParams& params, COLORREF srcChromaKey, const CRectF* calculatedTexturePatch);
		IDirect3DVBPtr createTexturedSquareBuffer(const RECT& rect, const CRectF& texturePatc);

		IDirect3DSurfacePtr setRenderTarget();
		void restoreRenderTarget(const IDirect3DSurfacePtr& prevRenderTarget);

		void dumpSurface(const CString& operation);

	private:
		const IDirect3DDevicePtr m_device;
		const IDirect3DSurfacePtr m_renderTarget;
		IDirect3DPixelShaderPtr m_textureShader;
		IDirect3DPixelShaderPtr m_chromaKeyShader;
		const SurfaceType m_surfaceType;
		const CSize m_extents;

		COLORREF m_chromaKeyColor;

		bool m_wasBltDone;

		IDirect3DSurfacePtr m_dumpTarget;
		SurfaceDumper m_dumper;
	};
}