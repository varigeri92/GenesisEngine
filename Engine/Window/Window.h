#pragma once
#define SDL_MAIN_HANDLED

#ifndef __WIN32__
#define __WIN32__
#endif // __WIN32__
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "API.h"
#include "Screen.h"

namespace gns
{

	class Window
	{
		friend class Engine;
	public:
		uint32_t width;
		uint32_t height;

		Window();
		~Window();

		void CloseWindow();
		bool IsOpen() const { return _open; }
		SDL_Window* sdlWindow;
	private:
		SDL_Event sdl_event;
		bool _open;

	public:
		HWND hwndHandle;
		HINSTANCE hinstance;
		bool isMinimized;

		bool PollEvents();
		void GetExtentions(uint32_t& count, const char** names);
		void GetExtent(int& width, int& height);
		void WindowEvent(const SDL_Event* event);
		void InitWindow(std::string title, uint32_t width, uint32_t height);
		
	};
};
