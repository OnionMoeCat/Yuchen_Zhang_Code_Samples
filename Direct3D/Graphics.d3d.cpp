// Header Files
//=============

#include "../Graphics.h"

#include <cassert>
#include <d3d9.h>
#include "../../UserOutput/UserOutput.h"
#include "../Renderable.h"
#include "../RenderableHelper.h"
#include "../RenderableManager.h"

// Static Data Initialization
//===========================
namespace
{
	HWND s_renderingWindow = NULL;
	IDirect3D9* s_direct3dInterface = NULL;
	IDirect3DDevice9* s_direct3dDevice = NULL;
}

// Helper Function Declarations
//=============================

namespace
{
	bool CreateContext();
	bool CreateDevice();
	bool CreateInterface();
	bool SetRenderState();
}

// Interface
//==========

bool eae6320::Graphics::Core::Initialize( const HWND i_renderingWindow )
{
	s_renderingWindow = i_renderingWindow;

	// Initialize the interface to the Direct3D9 library
	if ( !CreateInterface() )
	{
		return false;
	}
	// Create an interface to a Direct3D device
	if ( !CreateDevice() )
	{
		goto OnError;
	}
	if (!SetRenderState())
	{
		goto OnError;
	}
	if (!CreateContext())
	{
		goto OnError;
	}

	return true;

OnError:

	ShutDown();
	return false;
}

bool eae6320::Graphics::Core::ShutDown()
{
	bool wereThereErrors = false;

	if ( s_direct3dInterface )
	{
		if (s_direct3dDevice)
		{
			RenderableManager::Get().CleanUp();	
		
			s_direct3dDevice->SetVertexDeclaration(NULL);

			s_direct3dDevice->Release();
			s_direct3dDevice = NULL;
		}

		s_direct3dInterface->Release();
		s_direct3dInterface = NULL;
	}
	s_renderingWindow = NULL;

	return !wereThereErrors;
}

bool eae6320::Graphics::Core::Clear(eae6320::Graphics::sColor color, float depth, eae6320::Graphics::Context context)
{
	const D3DRECT* subRectanglesToClear = NULL;
	const DWORD subRectangleCount = 0;
	const DWORD clearTheRenderTargetAndZBuffer = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
	D3DCOLOR clearColor;
	{
		// Black is usually used:
		clearColor = D3DCOLOR_RGBA(eae6320::Graphics::ColorHelper::ColorFloatToUint8(color.r),
			eae6320::Graphics::ColorHelper::ColorFloatToUint8(color.g),
			eae6320::Graphics::ColorHelper::ColorFloatToUint8(color.b),
			eae6320::Graphics::ColorHelper::ColorFloatToUint8(color.a));
	}
	const float zBufferDepth = depth;
	const DWORD noStencilBuffer = 0;
	HRESULT result = context.device->Clear(subRectangleCount, subRectanglesToClear,
		clearTheRenderTargetAndZBuffer, clearColor, zBufferDepth, noStencilBuffer);
	return SUCCEEDED(result);
}

bool eae6320::Graphics::Core::OnSubmitRenderCommands_start(eae6320::Graphics::Context context)
{
	return SUCCEEDED(context.device->BeginScene());
}

bool eae6320::Graphics::Core::OnSubmitRenderCommands_end(eae6320::Graphics::Context context)
{
	return SUCCEEDED(context.device->EndScene());
}

bool eae6320::Graphics::Core::DisplayRenderedBuffer(eae6320::Graphics::Context context)
{
	const RECT* noSourceRectangle = NULL;
	const RECT* noDestinationRectangle = NULL;
	const HWND useDefaultWindow = NULL;
	const RGNDATA* noDirtyRegion = NULL;
	HRESULT result = s_direct3dDevice->Present(noSourceRectangle, noDestinationRectangle, useDefaultWindow, noDirtyRegion);
	return SUCCEEDED(result);
}

// Helper Function Definitions
//============================

namespace
{
	bool CreateContext()
	{
		eae6320::Graphics::Context::Get().device = s_direct3dDevice;
		return true;
	}

	bool CreateDevice()
	{
		const UINT useDefaultDevice = D3DADAPTER_DEFAULT;
		const D3DDEVTYPE useHardwareRendering = D3DDEVTYPE_HAL;
		const DWORD useHardwareVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		D3DPRESENT_PARAMETERS presentationParameters = { 0 };
		{
			{
				const unsigned int useRenderingWindowDimensions = 0;
				presentationParameters.BackBufferWidth = useRenderingWindowDimensions;
				presentationParameters.BackBufferHeight = useRenderingWindowDimensions;
			}
			presentationParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
			presentationParameters.BackBufferCount = 1;
			presentationParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
			presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentationParameters.hDeviceWindow = s_renderingWindow;
			presentationParameters.Windowed = TRUE;
			presentationParameters.EnableAutoDepthStencil = TRUE;
			presentationParameters.AutoDepthStencilFormat = D3DFMT_D16;
			presentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
		HRESULT result = s_direct3dInterface->CreateDevice( useDefaultDevice, useHardwareRendering,
			s_renderingWindow, useHardwareVertexProcessing, &presentationParameters, &s_direct3dDevice );
		if ( SUCCEEDED( result ) )
		{
			return true;
		}
		else
		{
			eae6320::UserOutput::Print( "Direct3D failed to create a Direct3D9 device" );
			return false;
		}
	}

	bool CreateInterface()
	{
		// D3D_SDK_VERSION is #defined by the Direct3D header files,
		// and is just a way to make sure that everything is up-to-date
		s_direct3dInterface = Direct3DCreate9( D3D_SDK_VERSION );
		if ( s_direct3dInterface )
		{
			return true;
		}
		else
		{
			eae6320::UserOutput::Print( "DirectX failed to create a Direct3D9 interface" );
			return false;
		}
	}

	bool SetRenderState()
	{
		bool wereThereErrors = false;

		HRESULT result = s_direct3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		if (FAILED(result))
		{
			wereThereErrors = true;
		}
		result = s_direct3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		if (FAILED(result))
		{
			wereThereErrors = true;
		}
		result = s_direct3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		if (FAILED(result))
		{
			wereThereErrors = true;
		}

		if (wereThereErrors)
		{
			eae6320::UserOutput::Print("Direct3D failed to set render state");
		}
		return !wereThereErrors;
	}
}
