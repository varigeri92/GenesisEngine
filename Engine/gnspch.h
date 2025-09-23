#pragma once

#include <iostream>
#include <deque>
#include <functional>
#include <span>
#include <vector>
#include <map>
#include <unordered_map>

#include "Utils/Logger.h"

struct FrameDiagnosticsData
{
	static float FrameTime;
	static uint32_t DrawCalls;
	static uint32_t PipelinesBound;
	static uint32_t UniformsBound;
	static float RenderTime;
	static float FPS;
};