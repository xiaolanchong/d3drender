#pragma once

#include "../D3DRender/D3DRender.h"

class Scene
{
public:
	Scene(HWND hWnd);

	void Render();
private:

	CSize getExtent() const;

	void testColorBlt();
	void testBitBlt();
	void testStretchBlt();
	void testTileBlt();
private:
	
	d3drender::IRenderPtr m_render;
	d3drender::ISurfacePtr m_primary;
	d3drender::ISurfacePtr m_secondary;

	d3drender::ISurfacePtr m_bltSurface;
	d3drender::ISurfacePtr m_bltSurfaceWithMask;
	d3drender::ISurfacePtr m_stretchSurface;
	d3drender::ISurfacePtr m_tileSurface;

	HWND m_hWnd;
};

