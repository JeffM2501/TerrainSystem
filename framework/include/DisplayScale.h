#pragma once

#include "imgui.h"

namespace EditorFramework
{
	void SetDPIAware();

	float GetDPIScale();

	template<class T>
	inline T ScaleToDPI(const T& value)
	{
		return (T)(value * GetDPIScale());
	}

    inline ImVec2 ScaleToDPI(float x, float y)
    {
		return ImVec2{ x * GetDPIScale(), y * GetDPIScale() };
    }
}