/*
	This file contains the function declarations for graphics
	
	Graphics.h

	The definition of interfaces of Graphics module to the others parts of the engine
*/

#ifndef EAE6320_GRAPHICS_H
#define EAE6320_GRAPHICS_H

// Header Files
//=============

#include "../Windows/Includes.h"
#include "Mesh.h"
#include "MeshHelper.h"
#include "Includes.h"
#include "Effect.h"
#include "EffectHelper.h"
#include "Context.h"

// Interface
//==========

namespace eae6320
{
	namespace Graphics
	{
		struct Core
		{
		public:
			// set up graphic environment.
			bool static Initialize(const HWND i_renderingWindow);
			void static Render();
			// clean up graphic environment after use.
			bool static ShutDown();
		private:
			// clear the "back buffer", which will be swapped with "front buffer".
			// "front buffer" is what users see.
			bool static Clear(sColor color, float depth, Context context);
			// begin writing in "back buffer"
			bool static OnSubmitRenderCommands_start(Context context);
			// end writing in "back buffer"
			bool static OnSubmitRenderCommands_end(Context context);
			// display the render buffer
			bool static DisplayRenderedBuffer(Context context);
		};

	}
}

#endif	// EAE6320_GRAPHICS_H
