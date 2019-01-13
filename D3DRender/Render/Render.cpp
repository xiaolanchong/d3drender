#include "stdafx.h"
#include "Render.h"
#include "Surface.h"
#include "PrimarySurface.h"
#include "SecondarySurface.h"

namespace d3drender
{
	namespace
	{
		class NullLogger : public ILogger
		{
		public:
			virtual void Log(Severity /*severity*/, const std::string& /*record*/,
				const char* /*fileName*/, int /*line*/) override
			{

			}
		};
	}

	namespace shader_texture
	{
	#include "shaders/Texture.h"
	}

	namespace shader_chroma_key
	{
	#include "shaders/ChromaKey.h"
	}

	Render::Render(HWND hWnd, bool windowMode, const ILoggerPtr& logger)
	: m_logger(logger)
	, m_windowMode(windowMode)
	{
		if (m_logger == nullptr)
		{
			m_logger = std::make_shared<NullLogger>();
		}

		m_d3d = ::Direct3DCreate9(D3D_SDK_VERSION);
		if (m_d3d == nullptr)
		{
			throw std::runtime_error("Cannot create Direct3D");
		}

		HRESULT hr = E_FAIL;
		// Set up the structure used to create the D3DDevice. Since we are now
		// using more complex geometry, we will create a device with a zbuffer.
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY; // D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.hDeviceWindow = hWnd;

		const DWORD creationFlags = /*D3DCREATE_MULTITHREADED| */D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		// Create the D3DDevice
		hr = m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			creationFlags,
			&d3dpp, &m_device);
		if (FAILED(hr))
		{
			m_logger->Log(Severity::Error, std::string("Cannot create Direct3DDevice, error=") + std::to_string(hr),
				__FILE__, __LINE__);
			throw std::runtime_error("Cannot create Direct3DDevice");
		}

		// Turn off culling
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		// Turn off D3D lighting
		m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

		// Turn off the zbuffer
		m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

		createShaders();
	}

	void Render::createShaders()
	{
		HRESULT hr = E_FAIL;

		static_assert(sizeof(shader_texture::g_ps20_main) % sizeof(DWORD) == 0, "Shader obj code must be DWORD aligned");
		hr = m_device->CreatePixelShader((const DWORD*)shader_texture::g_ps20_main, &m_shaderContext.m_textureShader);
		ATLVERIFY(hr == S_OK);

		static_assert(sizeof(shader_chroma_key::g_ps20_main) % sizeof(DWORD) == 0, "Shader obj code must be DWORD aligned");
		hr = m_device->CreatePixelShader((const DWORD*)shader_chroma_key::g_ps20_main, &m_shaderContext.m_chromaKeyShader);
		ATLVERIFY(hr == S_OK);
	}

	ISurfacePtr Render::CreateSurface(const SurfaceCreationParams& params)
	{
		return std::make_shared<Surface>(m_device, m_shaderContext, params, false);
	}

	ISurfacePtr Render::CreateTemporarySurface(const SurfaceCreationParams& params)
	{
		return std::make_shared<Surface>(m_device, m_shaderContext, params, true);
	}

	ISurfacePtr Render::CreatePrimarySurface(const SurfaceCreationParams& params)
	{
		return std::make_shared<PrimarySurface>(m_device, params);
	}

	ISurfacePtr Render::CreateSecondarySurface(const SurfaceCreationParams& params)
	{
		return std::make_shared<SecondarySurface>(m_device, m_shaderContext, params);
	}

}