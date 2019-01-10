
#include "stdafx.h"
#include "SurfaceDumper.h"
#include <atlimage.h>
#include <gdiplusimaging.h>

namespace d3drender
{
	void SurfaceDumper::Dump(const IDirect3DSurfacePtr& surface, const CString& operation)
	{
		if (m_operationCounter == 94)
		{
			int a = 7;
		}
		CString fileName;
		fileName.AppendFormat(_T("dumps\\%03u_%s.bmp"), m_operationCounter, static_cast<LPCTSTR>(operation));

		D3DSURFACE_DESC desc{};
		surface->GetDesc(&desc);
		D3DLOCKED_RECT lockedRect{};
		HRESULT hr = surface->LockRect(&lockedRect, nullptr, 0);
		ATLASSERT(hr == S_OK);
		if (hr == S_OK)
		{
			/*
				if(g_is565Format)
	{

		biv4h.bV4RedMask       = 0x0000F800u;
		biv4h.bV4GreenMask     = 0x000007E0u;
		biv4h.bV4BlueMask      = 0x0000001Fu;
		biv4h.bV4V4Compression = BI_BITFIELDS;
	}
	else
	{
		biv4h.bV4RedMask       = 0x00007C00u;
		biv4h.bV4GreenMask     = 0x000003E0u;
		biv4h.bV4BlueMask      = 0x0000001Fu;

	}
			*/
			unsigned bytesPerPixel = 0;
			CImage image;
			if (desc.Format == D3DFMT_X8R8G8B8)
			{
				BOOL res = image.CreateEx(desc.Width, desc.Height, 32, BI_RGB);
				ATLVERIFY(res);
				bytesPerPixel = 4;
			}
			else if (desc.Format == D3DFMT_R5G6B5)
			{
				DWORD adwBitmasks[3] = { 0x0000001Fu, 0x000007E0u, 0x0000F800u };
				BOOL res = image.CreateEx(desc.Width, desc.Height, 16, BI_BITFIELDS, adwBitmasks);
				ATLVERIFY(res);
				bytesPerPixel = 2;
			}
			//
			// );

			const BYTE* src = (const BYTE*)lockedRect.pBits;
			BYTE* dest = (BYTE*)image.GetBits();
			for (size_t i = 0; i < desc.Height; ++i, src += lockedRect.Pitch, dest += image.GetPitch())
			{
				memcpy(dest, src, desc.Width * bytesPerPixel);
			}
			surface->UnlockRect();
			hr = image.Save(fileName);// , Gdiplus::ImageFormatBMP);
			//ATLVERIFY(hr == S_OK);
		}

		++m_operationCounter;
	}
}