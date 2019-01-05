#include "stdafx.h"
#include "Blitter.h"
#include "Surface.h"
#include "SurfaceUtils.h"

namespace shader_texture
{
#include "shaders/Texture.h"
}

namespace shader_chroma_key
{
#include "shaders/ChromaKey.h"
}


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
			return CRect{ 0, 0, int(srcSurf.GetParams().m_width) - 1, int(srcSurf.GetParams().m_height) - 1 };
		}

		CRect getMyRenderRect(const CSize& extents, const RECT* srcRect)
		{
			if (srcRect)
			{
				return *srcRect;
			}
			return CRect{ 0, 0, extents.cx - 1, extents.cy - 1 };
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

	Blitter::Blitter(const IDirect3DDevicePtr& device, const IDirect3DSurfacePtr& renderTarget, SurfaceType surfaceType, const CSize& extents)
		: m_device(device)
		, m_renderTarget(renderTarget)
		, m_surfaceType(surfaceType)
		, m_extents(extents)
		, m_chromaKeyColor(0)
		, m_wasBltDone(false)
	{
		HRESULT hr = E_FAIL;

		static_assert(sizeof(shader_texture::g_ps20_main) % sizeof(DWORD) == 0, "Shader obj code must be DWORD aligned");
		hr = m_device->CreatePixelShader((const DWORD*)shader_texture::g_ps20_main, &m_textureShader);
		ATLVERIFY(hr == S_OK);

		static_assert(sizeof(shader_chroma_key::g_ps20_main) % sizeof(DWORD) == 0, "Shader obj code must be DWORD aligned");
		hr = m_device->CreatePixelShader((const DWORD*)shader_chroma_key::g_ps20_main, &m_chromaKeyShader);
	}

	bool Blitter::Blt(int x, int y, ISurface& srcSurf, const RECT* srcRect, const BltParams& params)
	{
		HRESULT hr = S_OK;
		const Surface* srcSurfImpl = dynamic_cast<const Surface*>(&srcSurf);
		if (!srcSurfImpl)
		{
			return false;
		}

		const CRect srcCalcRect = getRenderRect(srcSurf, srcRect);
		const CRect dstRect = CRect(CPoint(x, y), CSize(srcCalcRect.Width(), srcCalcRect.Height()));
		const auto chromaKey = params.m_useSrcChromaKey == UseSrcChromaKey::Yes ? srcSurf.GetChromaKey() : RGB(0, 0, 0);
		drawTexturedSquare(dstRect, srcSurfImpl->GetTexture(), srcRect, params, chromaKey, nullptr);

		m_wasBltDone = true;
		return hr;
	}

	bool Blitter::TileBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
		int anchorX, int anchorY, UseSrcChromaKey useSrcChromaKey)
	{
		HRESULT hr = S_OK;
		const Surface* srcSurfImpl = dynamic_cast<const Surface*>(&srcSurf);
		if (!srcSurfImpl)
		{
			return false;
		}
		ATLVERIFY(anchorX == 0);
		ATLVERIFY(anchorY == 0);

		const CRect srcCalcRect = getRenderRect(srcSurf, srcRect);
		const CRect dstCalcRect = getMyRenderRect(m_extents, dstRect);
		const auto chromaKey = useSrcChromaKey == UseSrcChromaKey::Yes ? srcSurf.GetChromaKey() : RGB(0, 0, 0);
		BltParams params{ useSrcChromaKey };
		CRectF texturePatch
		{ 
			0, 0, 
			float(dstCalcRect.Width()) / srcCalcRect.Width(),
			float(dstCalcRect.Height()) / srcCalcRect.Height()
		};
		drawTexturedSquare(dstCalcRect, srcSurfImpl->GetTexture(), srcRect, params, chromaKey, &texturePatch);

		m_wasBltDone = true;
		ATLVERIFY(hr == S_OK);
		return hr == S_OK;
	}

	bool Blitter::BevelBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
		int lightX, int ligthY, BevelDir dir)
	{
		UNREFERENCED_PARAMETER(lightX);
		UNREFERENCED_PARAMETER(ligthY);
		UNREFERENCED_PARAMETER(dir);
		return StretchBlt(dstRect, srcSurf, srcRect, BltParams{});
	}

	bool Blitter::ColorBlt(const RECT* dstRect, COLORREF color)
	{
		m_wasBltDone = true;
		HRESULT hr = E_FAIL;
		if (m_surfaceType == Blitter::SurfaceType::BackBuffer && true)
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

			return true;
		}

		else if (m_surfaceType == Blitter::SurfaceType::OffScreenPlain && true)
		{
			hr = m_device->ColorFill(m_renderTarget, dstRect, FromColorRef(color));
			ATLVERIFY(hr == S_OK);
			return hr == S_OK;
		}
		
		{
	//		hr = m_device->SetRenderTarget(0, m_renderTarget);
	//		ATLVERIFY(hr == S_OK);
		}
		setupMatrices();
		drawSquare(dstRect, color);
		return false;
	}

	bool Blitter::StretchBlt(const RECT* dstRect, ISurface& srcSurf, const RECT* srcRect,
		const BltParams& params)
	{
		const Surface* srcSurfImpl = dynamic_cast<const Surface*>(&srcSurf);
		if (!srcSurfImpl)
		{
			return false;
		}

		const CRect srcCalcRect = getRenderRect(srcSurf, srcRect);
		const CRect dstCalcRect = getMyRenderRect(m_extents, dstRect);
		const auto chromaKey = params.m_useSrcChromaKey == UseSrcChromaKey::Yes ? srcSurf.GetChromaKey() : RGB(0, 0, 0);
		drawTexturedSquare(dstCalcRect, srcSurfImpl->GetTexture(), srcRect, params, chromaKey, nullptr);

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
}