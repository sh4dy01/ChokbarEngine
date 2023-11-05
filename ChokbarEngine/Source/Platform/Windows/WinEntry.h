#pragma once

#include "IApplication.h"
#include "Core/DebugUtils.h"


extern Win32::IApplication* EntryApplication();

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{

#if defined(DEBUG) | defined(_DEBUG)
	// Enable run-time memory check for debug builds.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		PerGameSettings GameSettings;

		auto EntryApp = EntryApplication();

		EntryApp->SetupPerGameSettings();

		//Logger logger;

		EntryApp->PreInitialize();
		EntryApp->Initialize();

		EntryApp->Run();

		EntryApp->Shutdown();

		DELPTR(EntryApp)

	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // DEBUG

	return 0;
}