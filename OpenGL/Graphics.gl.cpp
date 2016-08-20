// Header Files
//=============

#include "../Graphics.h"

#include <cassert>
#include <gl/GL.h>
#include <string>
#include <sstream>
#include "../../UserOutput/UserOutput.h"
#include "../../Windows/Functions.h"
#include "../../../External/OpenGlExtensions/OpenGlExtensions.h"
#include "../Renderable.h"
#include "../RenderableHelper.h"
#include "../RenderableManager.h"

// Static Data Initialization
//===========================

namespace
{
	HWND s_renderingWindow = NULL;
	HDC s_deviceContext = NULL;
	HGLRC s_openGlRenderingContext = NULL;
}

// Helper Function Declarations
//=============================

namespace
{
	bool CreateContext();
	bool CreateRenderingContext();
	bool SetRenderState();
}

// Interface
//==========

bool eae6320::Graphics::Core::Initialize( const HWND i_renderingWindow )
{
	s_renderingWindow = i_renderingWindow;

	// Create an OpenGL rendering context
	if ( !CreateRenderingContext() )
	{
		goto OnError;
	}

	// Load any required OpenGL extensions
	{
		std::string errorMessage;
		if ( !OpenGlExtensions::Load( &errorMessage ) )
		{
			UserOutput::Print( errorMessage );
			goto OnError;
		}
	}

	{
		glEnable(GL_CULL_FACE);
		GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			std::stringstream errorMessage;
			errorMessage << "OpenGL failed to enable culling: " <<
				reinterpret_cast<const char*>(gluErrorString(errorCode));
			eae6320::UserOutput::Print(errorMessage.str());
			goto OnError;
		}
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

	if ( s_openGlRenderingContext != NULL )
	{

		if ( wglMakeCurrent( s_deviceContext, NULL ) != FALSE )
		{
			if ( wglDeleteContext( s_openGlRenderingContext ) == FALSE )
			{
				std::stringstream errorMessage;
				errorMessage << "Windows failed to delete the OpenGL rendering context: " << GetLastWindowsError();
				UserOutput::Print( errorMessage.str() );
				wereThereErrors = true;
			}
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "Windows failed to unset the current OpenGL rendering context: " << GetLastWindowsError();
			UserOutput::Print( errorMessage.str() );
			wereThereErrors = true;
		}
		s_openGlRenderingContext = NULL;
	}

	if ( s_deviceContext != NULL )
	{
		// The documentation says that this call isn't necessary when CS_OWNDC is used
		ReleaseDC( s_renderingWindow, s_deviceContext );
		s_deviceContext = NULL;
	}

	s_renderingWindow = NULL;

	return !wereThereErrors;
}

bool eae6320::Graphics::Core::Clear(eae6320::Graphics::sColor color, float depth, eae6320::Graphics::Context context)
{
	bool wereThereErrors = false;
	// Black is usually used
	glClearColor(color.r, color.g, color.b, color.a);
	if (glGetError() != GL_NO_ERROR)
	{
		wereThereErrors = true;
	}

	glClearDepth(depth);
	if (glGetError() != GL_NO_ERROR)
	{
		wereThereErrors = true;
	}

	glDepthMask(GL_TRUE);
	if (glGetError() != GL_NO_ERROR)
	{
		wereThereErrors = true;
	}

	// In addition to the color, "depth" and "stencil" can also be cleared,
	const GLbitfield clearColorAndDepth = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	glClear(clearColorAndDepth);
	if (glGetError() != GL_NO_ERROR)
	{
		wereThereErrors = true;
	}
	return !wereThereErrors;
}

bool eae6320::Graphics::Core::OnSubmitRenderCommands_start(eae6320::Graphics::Context context)
{
	return true;
}

bool eae6320::Graphics::Core::OnSubmitRenderCommands_end(eae6320::Graphics::Context context)
{
	return true;
}

bool eae6320::Graphics::Core::DisplayRenderedBuffer(eae6320::Graphics::Context context)
{
	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it, the contents of the back buffer must be swapped with the "front buffer"
	// (which is what the user sees)

	BOOL result = SwapBuffers(context.device);
	return result == TRUE;
}

// Helper Function Declarations
//=============================

namespace
{
	bool CreateContext()
	{
		eae6320::Graphics::Context::Get().device = s_deviceContext;
		return true;
	}

	bool CreateRenderingContext()
	{
		// A "device context" can be thought of an abstraction that Windows uses
		// to represent the graphics adaptor used to display a given window
		s_deviceContext = GetDC( s_renderingWindow );
		if ( s_deviceContext == NULL )
		{
			eae6320::UserOutput::Print( "Windows failed to get the device context" );
			return false;
		}
		// Windows requires that an OpenGL "render context" is made for the window we want to use to render into
		{
			// Set the pixel format of the rendering window
			{
				PIXELFORMATDESCRIPTOR desiredPixelFormat = { 0 };
				{
					desiredPixelFormat.nSize = sizeof( PIXELFORMATDESCRIPTOR );
					desiredPixelFormat.nVersion = 1;

					desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
					desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
					desiredPixelFormat.cColorBits = 32;
					desiredPixelFormat.iLayerType = PFD_MAIN_PLANE ;
					desiredPixelFormat.cDepthBits = 16;
				}
				// Get the ID of the desired pixel format
				int pixelFormatId;
				{
					pixelFormatId = ChoosePixelFormat( s_deviceContext, &desiredPixelFormat );
					if ( pixelFormatId == 0 )
					{
						std::stringstream errorMessage;
						errorMessage << "Windows couldn't choose the closest pixel format: " << eae6320::GetLastWindowsError();
						eae6320::UserOutput::Print( errorMessage.str() );
						return false;
					}
				}
				// Set it
				if ( SetPixelFormat( s_deviceContext, pixelFormatId, &desiredPixelFormat ) == FALSE )
				{
					std::stringstream errorMessage;
					errorMessage << "Windows couldn't set the desired pixel format: " << eae6320::GetLastWindowsError();
					eae6320::UserOutput::Print( errorMessage.str() );
					return false;
				}
			}
			// Create the OpenGL rendering context
			s_openGlRenderingContext = wglCreateContext( s_deviceContext );
			if ( s_openGlRenderingContext == NULL )
			{
				std::stringstream errorMessage;
				errorMessage << "Windows failed to create an OpenGL rendering context: " << eae6320::GetLastWindowsError();
				eae6320::UserOutput::Print( errorMessage.str() );
				return false;
			}
			// Set it as the rendering context of this thread
			if ( wglMakeCurrent( s_deviceContext, s_openGlRenderingContext ) == FALSE )
			{
				std::stringstream errorMessage;
				errorMessage << "Windows failed to set the current OpenGL rendering context: " << eae6320::GetLastWindowsError();
				eae6320::UserOutput::Print( errorMessage.str() );
				return false;
			}
		}

		return true;
	}

	bool SetRenderState()
	{
		bool wereThereErrors = false;

		glEnable(GL_DEPTH_TEST);
		GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
		}
		glDepthMask(GL_TRUE);
		errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
		}
		glDepthFunc(GL_LEQUAL);
		errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
		}

		if (wereThereErrors)
		{
			eae6320::UserOutput::Print("OpenGL failed to set render state");
		}
		return !wereThereErrors;
	}
}
