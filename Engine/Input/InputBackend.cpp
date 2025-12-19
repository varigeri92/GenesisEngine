#include "gnspch.h"
#include "InputBackend.h"
#include "../Window/Window.h"
#include "API.h"
#include "imgui.h"
#include "../../imgui/backends/imgui_impl_sdl2.h"
#include "../../imgui/backends/imgui_impl_vulkan.h"

namespace gns
{
	FrameInput InputBackend::frameInput = {};
	FrameInput InputBackend::previousFrameInput = {};
	glm::vec2 InputBackend::mousePos = {};
	glm::vec2 InputBackend::p_mousePos = {};
	glm::vec2 InputBackend::mouseVelocity = {};


	bool InputBackend::GetKey(int keyCode)
	{
		return frameInput.keysHeld[keyCode];
	}
	bool InputBackend::GetKeyUp(int keyCode)
	{
		return frameInput.keysUp[keyCode];
	}

	bool InputBackend::GetKeyDown(int keyCode)
	{
		return frameInput.keysDown[keyCode];
	}

	bool InputBackend::ProcessInput(SDL_Event& event, gns::Window* window)
	{
		frameInput.keysDown.clear();
		frameInput.keysUp.clear();

		frameInput.mouseDown.clear();
		frameInput.mouseUp.clear();

		p_mousePos.x = mousePos.x;
		p_mousePos.y = mousePos.y;
		mouseVelocity = { 0,0 };

		while (SDL_PollEvent(&event) != 0)
		{
			window->WindowEvent(&event);
			switch (event.type)
			{
			case SDL_QUIT:
				window->CloseWindow();
				LOG_INFO("HELLO!");
				return true;
			case SDL_KEYDOWN:
				if (!frameInput.keysDown[event.key.keysym.sym])
				{
					frameInput.keysDown[event.key.keysym.sym] = true;
				}
				frameInput.keysHeld[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				frameInput.keysUp[event.key.keysym.sym] = true;
				frameInput.keysHeld[event.key.keysym.sym] = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				frameInput.mouseDown[event.button.button] = true;
				frameInput.mouseHeld[event.button.button] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				frameInput.mouseUp[event.button.button] = true;
				frameInput.mouseHeld[event.button.button] = false;
				break;
			case SDL_MOUSEMOTION:
				mousePos.x = static_cast<float>(event.motion.x);
				mousePos.y = static_cast<float>(event.motion.y);
				mouseVelocity.x = event.motion.xrel;
				mouseVelocity.y = event.motion.yrel;
				break;
			default:
				break;
			}

			ImGui_ImplSDL2_ProcessEvent(&event);
		}
		return false;
	}
	bool InputBackend::GetMouseButtonDown(int mouseButton)
	{
		return frameInput.mouseDown[mouseButton];
	}
	bool InputBackend::GetMouseButtonUp(int mouseButton)
	{
		return frameInput.mouseUp[mouseButton];
	}

	glm::vec2 InputBackend::GetMouseVelocity()
	{
		return mouseVelocity;
	}

	bool InputBackend::GetMouseButton(int mouseButton)
	{
		return frameInput.mouseHeld[mouseButton];
	}
}
