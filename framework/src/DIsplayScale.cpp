#include "DisplayScale.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace EditorFramework
{
	static float DPIScale = 1;

	void SetDPIAware()
	{
#if defined(_WIN32)
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		int dpi = GetDpiForSystem();
		DPIScale =  dpi / 96.0f;
#endif
	}

	float GetDPIScale()
	{
		return DPIScale;
	}
}