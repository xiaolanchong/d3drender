#include "stdafx.h"
#include "Blitter.h"
#include "Surface.h"
#include "SurfaceUtils.h"




namespace d3drender
{
	namespace
	{
		CRect getRenderRect(const ISurface& srcSurf, const RECT* srcRect)
		{
			if (srcRect)
			{
				return *srcRect;
			}
			return CRect{ 0, 0, int(srcSurf.GetParams().m_width), int(srcSurf.GetParams().m_height)};
		}

		CRect getMyRenderRect(const CSize& extents, const RECT* srcRect)
		{
			if (srcRect)
			{
				return *srcRect;
			}
			return CRect{ 0, 0, extents.cx, extents.cy };
		}

		struct ColoredVertex
		{
			D3DVECTOR position; // The position
			D3DCOLOR color;    // The color

			static const DWORD FVF = (D3DFVF_XYZ | D3DFVF_DIFFUSE);
		};

		struct Vector4D {
			float x;
			float y;
			float z;
			float rhw;
		};

		struct TexturedVertex
		{
			Vector4D position; // The position
			FLOAT tu, tv;   // The texture coordinates

			static const DWORD FVF = (D3DFVF_XYZRHW | D3DFVF_TEX1);
		};
	}

	Blitter::Blitter(const BlitterContext& context)
		: m_chromaKeyColor(RGB(0, 0, 0))
		, m_wasBltDone(false)
		, BlitterContext(context)
	{
	}

	IDirect3DSurfacePtr Blitter::setRenderTarget()
	{
		IDirect3DSurfacePtr prevRenderTarget;
		if (m_surfaceType == SurfaceType::OffScreenPlain)
		{
			HRESULT hr = m_device->GetRenderTarget(0, &prevRenderTarget);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetRenderTarget(0, m_renderTarget);
			ATLVERIFY(hr == S_OK);
		}
		return prevRenderTarget;
	}

	void Blitter::restoreRenderTarget(const IDirect3DSurfacePtr& prevRenderTarget)
	{
		if (prevRenderTarget)
		{
			HRESULT hr = m_device->SetRenderTarget(0, prevRenderTarget);
			ATLVERIFY(hr == S_OK);
		}
	}

	bool Blitter::Blt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const BltParams& params)
	{
		if (srcRect && IsRectEmpty(srcRect))
		{
			return true;
		}

		HRESULT hr = S_OK;
		ISourceSurface* const srcSurfImpl = dynamic_cast<ISourceSurface*>(&srcSurf);
		if (!srcSurfImpl)
		{
			return false;
		}

		const CRect srcCalcRect = getRenderRect(srcSurf, srcRect);
		const CRect dstRect = CRect(CPoint(x, y), CSize(srcCalcRect.Width(), srcCalcRect.Height()));
		const auto chromaKey = params.m_useSrcChromaKey == UseSrcChromaKey::Yes ? srcSurf.GetChromaKey() : RGB(0, 0, 0);

		if (params.m_useSrcChromaKey == UseSrcChromaKey::Yes)
		{
			int a = 7;
		}

		if (srcCalcRect.Width() == 800 && srcCalcRect.Height() == 254)
		{
			int a = 7;
		}

		const auto& srcSurfParams = srcSurf.GetParams();
		CRectF texturePath = CRectF
		{ float(srcCalcRect.left) / srcSurfParams.m_width,  float(srcCalcRect.top) / srcSurfParams.m_height,
		  float(srcCalcRect.right) / srcSurfParams.m_width, float(srcCalcRect.bottom) / srcSurfParams.m_height };

		IDirect3DSurfacePtr prevRenderTarget = setRenderTarget();
		drawTexturedSquare(dstRect, srcSurfImpl->GetOutputTexture(), srcRect, params, chromaKey, &texturePath);
		restoreRenderTarget(prevRenderTarget);

		m_wasBltDone = true;

		CString op;
		op.Format(_T("Blt-to_%d_%d-from_%d_%d_%d_%d"), x, y, dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);
		dumpSurface(op);
		return hr == S_OK;
	}

	bool Blitter::TileBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
		int anchorX, int anchorY, UseSrcChromaKey useSrcChromaKey)
	{
		HRESULT hr = S_OK;
		ISourceSurface* const srcSurfImpl = dynamic_cast<ISourceSurface*>(&srcSurf);
		if (!srcSurfImpl)
		{
			return false;
		}
		ATLVERIFY(anchorX == 0);
		ATLVERIFY(anchorY == 0);

		const CRect srcCalcRect = getRenderRect(srcSurf, srcRect);
		ATLVERIFY(srcCalcRect.left == 0);
		ATLVERIFY(srcCalcRect.top == 0);
		const CRect dstCalcRect = getMyRenderRect(m_extents, dstRect);
		ATLVERIFY(srcCalcRect.Width() == srcSurf.GetParams().m_width);
		ATLVERIFY(srcCalcRect.Height() == srcSurf.GetParams().m_height);
		const auto chromaKey = useSrcChromaKey == UseSrcChromaKey::Yes ? srcSurf.GetChromaKey() : RGB(0, 0, 0);
		BltParams params{ useSrcChromaKey };
		CRectF texturePatch
		{ 
			0, 0, 
			float(dstCalcRect.Width()) / srcCalcRect.Width(),
			float(dstCalcRect.Height()) / srcCalcRect.Height()
		};

		IDirect3DSurfacePtr prevRenderTarget = setRenderTarget();
		drawTexturedSquare(dstCalcRect, srcSurfImpl->GetOutputTexture(), /*srcRect*/dstCalcRect, params, chromaKey, &texturePatch);
		restoreRenderTarget(prevRenderTarget);

		m_wasBltDone = true;
		dumpSurface("TileBlt");
		ATLVERIFY(hr == S_OK);
		return hr == S_OK;
	}

	bool Blitter::BevelBlt(const RECT* dstRect, int thickness, BevelDir dir)
	{
		UNREFERENCED_PARAMETER(dstRect);
		UNREFERENCED_PARAMETER(thickness);
		UNREFERENCED_PARAMETER(dir);

		if (dstRect && IsRectEmpty(dstRect))
		{
			return true;
		}
		return true;
	}

	bool Blitter::ColorBlt(const RECT* dstRect, COLORREF color)
	{
		if (dstRect && IsRectEmpty(dstRect))
		{
			return true;
		}

		m_wasBltDone = true;
		HRESULT hr = E_FAIL;
		if (m_surfaceType == SurfaceType::BackBuffer)
		{
			// Clear the backbuffer and the zbuffer
			const DWORD rectCount = dstRect ? 1U : 0U;
			const D3DRECT rect = dstRect ? D3DRECT{ dstRect->left, dstRect->top, dstRect->right, dstRect->bottom } : D3DRECT{};
			hr = m_device->Clear(rectCount, (rectCount ? &rect : nullptr), D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				FromColorRef(color), 1.0f, 0);
			ATLVERIFY(hr == S_OK);
			if (FAILED(hr))
			{
				//m_logger->Log(Severity::Error, "Clear failed", __FILE__, __LINE__);
				return false;
			}

			dumpSurface("ColorBlt");
			return true;
		}

		else if (m_surfaceType == SurfaceType::OffScreenPlain)
		{
			hr = m_device->ColorFill(m_renderTarget, dstRect, FromColorRef(color));
			ATLVERIFY(hr == S_OK);
			dumpSurface("ColorBlt");
#if 0
			if (m_inputSurface)
			{
				hr = m_device->ColorFill(m_inputSurface, dstRect, FromColorRef(color));
				ATLVERIFY(hr == S_OK);
			}
#else
			m_wasBltDone = true;
#endif
			return hr == S_OK;
		}
		
		ATLASSERT(!"Not impelemented");
		return false;
	}

	bool Blitter::StretchBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
		const BltParams& params)
	{
		ISourceSurface* const srcSurfImpl = dynamic_cast<ISourceSurface*>(&srcSurf);
		if (!srcSurfImpl)
		{
			return false;
		}

		const CRect srcCalcRect = getRenderRect(srcSurf, srcRect);
		const CRect dstCalcRect = getMyRenderRect(m_extents, dstRect);
		const auto chromaKey = params.m_useSrcChromaKey == UseSrcChromaKey::Yes ? srcSurf.GetChromaKey() : RGB(0, 0, 0);

		IDirect3DSurfacePtr prevRenderTarget = setRenderTarget();
		drawTexturedSquare(dstCalcRect, srcSurfImpl->GetOutputTexture(), srcRect, params, chromaKey, nullptr);
		restoreRenderTarget(prevRenderTarget);

		dumpSurface("StretchBlt");
		m_wasBltDone = true;
		HRESULT hr = S_OK;
		ATLVERIFY(hr == S_OK);
		return hr == S_OK;
	}

	bool Blitter::StencilMixBlt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const Stencil& stencil)
	{
		UNREFERENCED_PARAMETER(stencil);
		return Blt(x, y, srcSurf, srcRect, BltParams{});
	}

	void Blitter::setupMatrices()
	{
		D3DMATRIX matIdentity{};
		for (auto i : { 0, 1, 2, 3 })
		{
			matIdentity.m[i][i] = 1;
		}
		
		HRESULT hr = E_FAIL;
		hr = m_device->SetTransform(D3DTS_WORLD, &matIdentity);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetTransform(D3DTS_VIEW, &matIdentity);
		ATLVERIFY(hr == S_OK);

		D3DMATRIX matProjection{};

		const float cotFovW = 1; /// no x-fov
		const float cotFovH = 1; /// no y-fov
		const float zNear = 0.1f; /// z-coord of the near clipping plane
		const float zFar = 1000;  /// z-coord of the far clipping plane
		const float Q = zFar / float(zFar - zNear);
		const float qZ = -Q * zNear;

		matProjection.m[0][0] = cotFovW;
		matProjection.m[1][1] = cotFovH;
		matProjection.m[2][2] = Q;
		matProjection.m[2][3] = 1;
		matProjection.m[3][2] = qZ; // qZ

		hr = m_device->SetTransform(D3DTS_PROJECTION, &matProjection);
		ATLVERIFY(hr == S_OK);
	}


	void Blitter::drawSquare(const RECT* dstRect, D3DCOLOR color)
	{
		auto renderRect = getMyRenderRect(m_extents, dstRect);
		drawSquare(renderRect, color);
	}

	void Blitter::drawSquare(const RECT& rect, D3DCOLOR color)
	{
		setupMatrices();

		auto vertexBuffer = createSquareBuffer(rect, color);

		HRESULT hr = E_FAIL;
		// Render the vertex buffer contents
		hr = m_device->SetStreamSource(0, vertexBuffer, 0, sizeof(ColoredVertex));
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetFVF(ColoredVertex::FVF);
		ATLVERIFY(hr == S_OK);

		hr = m_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		//hr = m_device->DrawPrimitive(D3DPT_LINELIST, 0, 2);
		ATLVERIFY(hr == S_OK);
	}

	IDirect3DVBPtr Blitter::createSquareBuffer(const RECT& rect, D3DCOLOR color)
	{
		HRESULT hr = E_FAIL;
		IDirect3DVBPtr vertexBuffer;
		hr = m_device->CreateVertexBuffer(4 * sizeof(ColoredVertex),
			D3DUSAGE_DYNAMIC, ColoredVertex::FVF,
			D3DPOOL_DEFAULT, &vertexBuffer, NULL);
		ATLVERIFY(hr == S_OK);

		ColoredVertex* vertices = nullptr;
		hr = vertexBuffer->Lock(0, 0, (void**)&vertices, 0);
		ATLVERIFY(hr == S_OK);
		
		const float zCoord = 0.99f;
		const CRectF rectNormalized = TransformToXYZ(CRect(rect), m_extents);

		vertices[0].position = D3DVECTOR{ rectNormalized.left, rectNormalized.top, zCoord };
		vertices[0].color = color; // D3DCOLOR_XRGB(255, 0, 0);

		vertices[1].position = D3DVECTOR{ rectNormalized.left, rectNormalized.bottom, zCoord };
		vertices[1].color = color; //D3DCOLOR_XRGB(0, 255, 0);

		vertices[2].position = D3DVECTOR{ rectNormalized.right, rectNormalized.top, zCoord };
		vertices[2].color = color; //D3DCOLOR_XRGB(0, 0, 255);

		vertices[3].position = D3DVECTOR{ rectNormalized.right, rectNormalized.bottom, zCoord };
		vertices[3].color = color; //D3DCOLOR_XRGB(0, 128, 128);

		hr = vertexBuffer->Unlock();
		ATLVERIFY(hr == S_OK);

		return vertexBuffer;
	}

	void Blitter::drawTexturedSquare(const CRect& dstRect, const IDirect3DTexturePtr& texture, const RECT* srcRect,
		const BltParams& params, COLORREF srcChromaKey, const CRectF* calculatedTexturePatch)
	{
		HRESULT hr = E_FAIL;

		CRectF texturePatch{ 0.0, 0.0, 1.0, 1.0 };
		if (calculatedTexturePatch)
		{
			texturePatch = *calculatedTexturePatch;
		}
		else if (srcRect)
		{
			CRect srcCRect(*srcRect);
			texturePatch = CRectF{ float(srcRect->left) / srcCRect.Width(), float(srcRect->top) / srcCRect.Height(),
								   float(srcRect->right) / srcCRect.Width(), float(srcRect->bottom) / srcCRect.Height() };
		}

		hr = m_device->BeginScene();
		ATLVERIFY(hr == S_OK);
		
		auto vertexBuffer = createTexturedSquareBuffer(dstRect, texturePatch);

		//setupMatrices();


		hr = m_device->SetStreamSource(0, vertexBuffer, 0, sizeof(TexturedVertex));
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetTexture(0, texture);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		ATLVERIFY(hr == S_OK);

		if (params.m_useSrcChromaKey == UseSrcChromaKey::Yes)
		{
			const float chromaKeyFlt[4] = { GetRValue(srcChromaKey) / 255.f, GetGValue(srcChromaKey) / 255.f, GetBValue(srcChromaKey) / 255.f, 0.f };
			hr = m_device->SetPixelShaderConstantF(0, chromaKeyFlt, 1);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetPixelShader(m_chromaKeyShader);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetRenderState(D3DRS_ALPHAREF, 0x0);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			ATLVERIFY(hr == S_OK);
		}
		else
		{
			hr = m_device->SetPixelShader(m_textureShader);
			ATLVERIFY(hr == S_OK);
		}

		if (params.m_blendSurfaces == BlendSurfaces::Yes)
		{
			hr = m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			ATLVERIFY(hr == S_OK);

			hr = m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
			ATLVERIFY(hr == S_OK);
		}

		hr = m_device->SetFVF(TexturedVertex::FVF);
		ATLVERIFY(hr == S_OK);

		hr = m_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		ATLVERIFY(hr == S_OK);

		hr = m_device->EndScene();
		ATLVERIFY(hr == S_OK);

		/// restore 
		hr = m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		ATLVERIFY(hr == S_OK);

		hr = m_device->SetTexture(0, nullptr);
		ATLVERIFY(hr == S_OK);
	}

	IDirect3DVBPtr Blitter::createTexturedSquareBuffer(const RECT& rect, const CRectF& texturePatch)
	{
		HRESULT hr = E_FAIL;
		IDirect3DVBPtr vertexBuffer;
		hr = m_device->CreateVertexBuffer(4 * sizeof(TexturedVertex),
			D3DUSAGE_DYNAMIC, TexturedVertex::FVF,
			D3DPOOL_DEFAULT, &vertexBuffer, NULL);
		ATLVERIFY(hr == S_OK);

		TexturedVertex* vertices = nullptr;
		hr = vertexBuffer->Lock(0, 0, (void**)&vertices, 0);
		ATLVERIFY(hr == S_OK);

		const float zCoord = 0.5f;
		const CRectF rectNormalized = CRectF{ float(rect.left), float(rect.top), float(rect.right), float(rect.bottom) };
		// TransformToXYZ(CRect(rect), m_extents);

		vertices[0].position = Vector4D{ rectNormalized.left, rectNormalized.top, zCoord, 0.5f };
		vertices[0].tu = texturePatch.left;
		vertices[0].tv = texturePatch.top;

		vertices[1].position = Vector4D{ rectNormalized.left, rectNormalized.bottom, zCoord, 0.5f };
		vertices[1].tu = texturePatch.left;
		vertices[1].tv = texturePatch.bottom;

		vertices[2].position = Vector4D{ rectNormalized.right, rectNormalized.top, zCoord, 0.5f };
		vertices[2].tu = texturePatch.right;
		vertices[2].tv = texturePatch.top;

		vertices[3].position = Vector4D{ rectNormalized.right, rectNormalized.bottom, zCoord, 0.5f };
		vertices[3].tu = texturePatch.right;
		vertices[3].tv = texturePatch.bottom;

		hr = vertexBuffer->Unlock();
		ATLVERIFY(hr == S_OK);

		return vertexBuffer;
	}

	bool Blitter::BltWithChromaKey(int x, int y, const IDirect3DTexturePtr& texture, const CRect& srcRect, COLORREF chromaKeyColor)
	{
		CRect dstRect(CPoint(x, y), CSize(srcRect.Width(), srcRect.Height()));
		BltParams params{ UseSrcChromaKey::Yes };
		drawTexturedSquare(dstRect, texture, nullptr, params, chromaKeyColor, nullptr);
		return true;
	}

	void Blitter::dumpSurface(const CString& operation)
	{
		return;
		if (m_surfaceType == SurfaceType::BackBuffer)
		{
			if (!m_dumpTarget)
			{
				const long d3dRenderUsage = 0;// | (temporary ? D3DUSAGE_DYNAMIC : 0);
				const D3DFORMAT d3dFmt = D3DFMT_X8R8G8B8; // ConvertFormat(params.m_pixelFormat); //D3DFMT_X8R8G8B8;

				HRESULT hr = E_FAIL;
				hr = m_device->CreateOffscreenPlainSurface(m_extents.cx, m_extents.cy, d3dFmt, D3DPOOL_SYSTEMMEM, &m_dumpTarget, nullptr);
				ATLVERIFY(hr == S_OK);
			}
			{
				IDirect3DSurfacePtr surface;
				HRESULT hr = E_FAIL;

				hr = m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
				ATLVERIFY(hr == S_OK);

				hr = m_device->GetRenderTargetData(surface, m_dumpTarget);
				ATLVERIFY(hr == S_OK);
			}

			m_dumper.Dump(m_dumpTarget, operation);
		}
		else
		{
			return;

			const long d3dRenderUsage = 0;// | (temporary ? D3DUSAGE_DYNAMIC : 0);
			const D3DFORMAT d3dFmt = D3DFMT_R5G6B5; // ConvertFormat(params.m_pixelFormat); //D3DFMT_X8R8G8B8;

			m_dumpTarget = nullptr;
			HRESULT hr = E_FAIL;

			D3DSURFACE_DESC desc;
			m_renderTarget->GetDesc(&desc);

			ATLVERIFY(desc.Width == m_extents.cx);
			ATLVERIFY(desc.Height == m_extents.cy);
			hr = m_device->CreateOffscreenPlainSurface(desc.Width, desc.Height, d3dFmt, D3DPOOL_SYSTEMMEM, &m_dumpTarget, nullptr);
			ATLVERIFY(hr == S_OK);

			hr = m_device->GetRenderTargetData(m_renderTarget, m_dumpTarget);
			ATLVERIFY(hr == S_OK);

			m_dumper.Dump(m_dumpTarget, operation);
		}
	}
}