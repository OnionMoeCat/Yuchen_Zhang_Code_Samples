// Header Files
//=============

#include "Graphics.h"

#include <cassert>
#include <gl/GL.h>
#include <string>
#include <sstream>
#include "../UserOutput/UserOutput.h"
#include "../Windows/Functions.h"
#include "Renderable.h"
#include "RenderableHelper.h"
#include "RenderableManager.h"
#include "MaterialHelper.h"

// Interface
//==========

void eae6320::Graphics::Core::Render()
{
	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	bool result;
	{
		//Color in format of RGBA
		sColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		float depth = 1.0f;
		result = Clear(clearColor, depth, Context::Get());
		assert(result);
	}

	// writing to the "back buffer" begins
	result = OnSubmitRenderCommands_start(Context::Get());
	assert(result);

	// The actual function calls that draw geometry
	{
		// Pass the mesh and material of renderables to the device
		{
			for (unsigned int i = 0; i < RenderableManager::Get().GetSize(); i++)
			{
				// for this renderable
				Renderable* renderable = RenderableManager::Get().GetRenderableAtIndex(i);
				// pass its render states to devices
				result = EffectHelper::Bind(*renderable->m_material->m_effect, Context::Get());
				assert(result);
				// pass uniforms of its material 
				result = MaterialHelper::SetMaterialUniforms(*renderable->m_material, Context::Get());
				assert(result);
				// pass textures of its material
				result = MaterialHelper::SetMaterialTextures(*renderable->m_material, Context::Get());
				assert(result);
				// pass project matrix as uniforms
				result = EffectHelper::SetDrawCallUniforms(*renderable->m_material->m_effect, Context::Get());
				assert(result);
				// pass mesh (vertices and indices) and draw it
				result = MeshHelper::DrawMesh(*renderable->m_mesh, Context::Get());
				assert(result);
			}
			// cleanup the renderable container after rendering
			RenderableManager::Get().CleanUp();
		}
	}

	// writing to the "back buffer" ends
	result = OnSubmitRenderCommands_end(Context::Get());
	assert(result);

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it, the contents of the back buffer must be swapped with the "front buffer"
	// (which is what the user sees)
	result = DisplayRenderedBuffer(Context::Get());
	assert(result);
}