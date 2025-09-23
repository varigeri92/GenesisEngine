#include "gnspch.h"
#include "Window.h"
#include "../Input/InputBackend.h"
#include "../Utils/Logger.h"

gns::Window::Window() : _open(true){}

gns::Window::~Window()
{
	SDL_DestroyWindow(sdlWindow);
}

void gns::Window::CloseWindow()
{
	_open = false;
}

bool gns::Window::PollEvents()
{
	return InputBackend::ProcessInput(sdl_event, this);
}

void gns::Window::GetExtentions(uint32_t& count, const char** names)
{
	
	if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &count, nullptr))
	{
		LOG_ERROR("failed to get Extensions Count");
	}

	if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &count, names))
	{
		LOG_ERROR("failed to get Extensions names");
	}
}

void gns::Window::GetExtent(int& width, int& height)
{
		SDL_Vulkan_GetDrawableSize(sdlWindow, &width, &height);
}

void gns::Window::WindowEvent(const SDL_Event* event)
{
	if(event->window.windowID != SDL_GetWindowID(sdlWindow))
	{
		return;
	}

	if (event->type == SDL_WINDOWEVENT) {
		switch (event->window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			CloseWindow();
			LOG_INFO("Main Window CLOSE!");
			break;
		case SDL_WINDOWEVENT_SHOWN:
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			break;
		case SDL_WINDOWEVENT_MOVED:
			break;
		case SDL_WINDOWEVENT_RESIZED:
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			LOG_INFO("Window size change!");
			isMinimized = false;
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			LOG_INFO("Window minimized!");
			isMinimized = true;
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			LOG_INFO("Window maximized!");
			isMinimized = false;
			break;
		case SDL_WINDOWEVENT_RESTORED:
			LOG_INFO("Window restored!");
			isMinimized = false;
			break;
		case SDL_WINDOWEVENT_ENTER:
			break;
		case SDL_WINDOWEVENT_LEAVE:
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
		case SDL_WINDOWEVENT_TAKE_FOCUS:
			break;
		case SDL_WINDOWEVENT_HIT_TEST:
			break;
#endif
		default:

			break;
		}
	}
}

void gns::Window::InitWindow(std::string title, uint32_t width, uint32_t height)
{
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		}

	sdlWindow = SDL_CreateWindow(
		title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		static_cast<int>(width), static_cast<int>(height),
		SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!sdlWindow) {
		SDL_Log("Failed to create window: %s", SDL_GetError());
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdlWindow, &wmInfo);
	hwndHandle = wmInfo.info.win.window;
	hinstance = wmInfo.info.win.hinstance;

	int w = 0;
	int h = 0;
	SDL_Vulkan_GetDrawableSize(sdlWindow, &w, &h);
}