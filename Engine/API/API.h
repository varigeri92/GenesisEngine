#pragma once

#ifdef BUILD_ENGINE_LIB
#define GNS_API __declspec(dllexport)
#else
#define GNS_API __declspec(dllimport)
#endif