#pragma once


namespace EditorFramework
{
	void SetDPIAware();

	float GetDPIScale();

	template<class T>
	inline T ScaleToDPI(const T& value)
	{
		return (T)(value * GetDPIScale());
	}
}