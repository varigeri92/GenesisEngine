#pragma once

class Screen
{
public:
	uint32_t width;
	uint32_t height;
	float scale;
	float aspectRatio;

	bool resized;
	bool updateRenderTargetTarget;

};
