#include "stdafx.h"
#include "Scene.h"
#include "../D3DRender/include/ISurfaceBlt.h"
#include "../D3DRender/Render/SurfaceUtils.h"

using namespace d3drender;

Scene::Scene(HWND hWnd)
	: m_hWnd(hWnd)
{
	m_render = CreateRender(true, hWnd, nullptr);
	auto extent = getExtent();
	
	SurfaceCreationParams mainSurfParams{ PixelFormat::R5G5B5, extent.cx, extent.cy };
	m_primary = m_render->CreatePrimarySurface(mainSurfParams);
	m_secondary = m_render->CreateSecondarySurface(mainSurfParams);

	SurfaceCreationParams surfParams{ PixelFormat::R5G5B5, extent.cx / 4, extent.cy / 4 };
	m_bltSurface = m_render->CreateSurface(surfParams);
	m_stretchSurface = m_render->CreateSurface(surfParams);
	m_bltSurfaceWithMask = m_render->CreateSurface(surfParams);

	LockResult res = m_bltSurfaceWithMask->Lock(nullptr);
	const auto width = m_bltSurfaceWithMask->GetParams().m_width;
	const auto height = m_bltSurfaceWithMask->GetParams().m_height;
	std::uint8_t* buffer = reinterpret_cast<std::uint8_t*>(res.m_buffer);

	const Pixel16 maskColor = ConvertToPixel16(PixelFormat::R5G5B5, RGB(32, 32, 32));
	for (size_t j = 0; j < height; ++j, buffer += res.m_pitch)
	{
		Pixel16* line = reinterpret_cast<Pixel16*>(buffer);
		for (size_t i = 0; i < width; ++i)
		{
			if (i > width / 4 && i < 3 * width / 4 &&
				j > height / 4 && j < 3 * height / 4)
			{
				line[i] = maskColor;
			}
			else
			{
				line[i] = ConvertToPixel16(PixelFormat::R5G5B5, RGB(0, 0, 255 * i / width));
			}
		}
	}

	m_bltSurfaceWithMask->Unlock();
	m_bltSurfaceWithMask->SetChromaKey(RGB(32, 32, 32));

	SurfaceCreationParams tileParams{ PixelFormat::R5G5B5, 10, 10 };
	m_tileSurface = m_render->CreateSurface(tileParams);
}

CSize Scene::getExtent() const
{
	RECT rect{};
	::GetClientRect(m_hWnd, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	return CSize(width, height);
}

void Scene::testColorBlt()
{
	m_secondary->GetBlitter().ColorBlt(nullptr, RGB(216, 216, 216));
	m_secondary->GetBlitter().ColorBlt(CRect(5, 5, 200, 250), RGB(100, 100, 100));

	m_bltSurface->GetBlitter().ColorBlt(nullptr, RGB(0, 128, 255));
	m_stretchSurface->GetBlitter().ColorBlt(nullptr, RGB(0, 255, 0));
}

void Scene::testBitBlt()
{
	auto extent = getExtent();

	m_secondary->GetBlitter().Blt(10, 10, *m_bltSurface, nullptr, BltParams{});
	m_secondary->GetBlitter().Blt(extent.cx/4 + 15, 10, 
		*m_bltSurface, &CRect(10, 10, 50, 60), BltParams{});

	m_secondary->GetBlitter().Blt(extent.cx / 2 + 15, 10,
		*m_bltSurfaceWithMask, nullptr, BltParams{ UseSrcChromaKey::Yes});

	m_secondary->GetBlitter().Blt(extent.cx / 2 + 15, 10 + extent.cy / 4,
		*m_bltSurfaceWithMask, nullptr, BltParams{ UseSrcChromaKey::No, BlendSurfaces::Yes });

	m_secondary->GetBlitter().Blt(extent.cx / 2 + 15, 10 + 2 * extent.cy / 4,
		*m_bltSurfaceWithMask, nullptr, BltParams{ UseSrcChromaKey::Yes, BlendSurfaces::Yes });

	m_secondary->GetBlitter().Blt(extent.cx / 2 + 15, 10 + 3 * extent.cy / 4,
		*m_bltSurfaceWithMask, nullptr, BltParams{ UseSrcChromaKey::No, BlendSurfaces::No });
}

void Scene::testStretchBlt()
{
	auto extent = getExtent();

	CRect stretchRect(CPoint(10, extent.cy / 2 + 10), CSize(extent.cx/5, extent.cy/5 - 5));
	m_secondary->GetBlitter().StretchBlt(&stretchRect, *m_stretchSurface, nullptr, BltParams{});
}

void Scene::testTileBlt()
{
	auto extent = getExtent();

	m_tileSurface->GetBlitter().ColorBlt(nullptr, RGB(128, 0, 0));
	CRect stretchRect(CPoint(10, 150), CSize(10*10, 5*10));
	m_secondary->GetBlitter().TileBlt(stretchRect, *m_tileSurface,  nullptr, 0, 0, UseSrcChromaKey::No);
}

void Scene::Render()
{
	// must be first
	testColorBlt();
	testBitBlt();
	testStretchBlt();
	testTileBlt();

	m_primary->Flip();
}
