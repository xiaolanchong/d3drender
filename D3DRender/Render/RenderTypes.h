#pragma once

#include <d3d9.h>
#include <atlbase.h>

namespace d3drender
{
	using IDirect3DPtr = CComPtr<::IDirect3D9>;
	using IDirect3DDevicePtr = CComPtr<::IDirect3DDevice9>;
	using IDirect3DSurfacePtr = CComPtr<::IDirect3DSurface9>;
	using IDirect3DTexturePtr = CComPtr<::IDirect3DTexture9>;
	using IDirect3DVBPtr = CComPtr<::IDirect3DVertexBuffer9>;

	using IDirect3DPixelShaderPtr = CComPtr<::IDirect3DPixelShader9>;

	struct ShaderContext
	{
		IDirect3DPixelShaderPtr m_textureShader;
		IDirect3DPixelShaderPtr m_chromaKeyShader;
	};

	inline D3DCOLOR FromColorRef(COLORREF color)
	{
		return D3DCOLOR_XRGB(GetRValue(color), GetGValue(color), GetBValue(color));
	}
}