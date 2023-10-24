#pragma once

#include "../Platform/Windows/Window.h"

namespace SplashScreen {

	void Open();
	void Close();
	void SetMessage(const WCHAR* message);

}

class SplashWindow : public Win32::Window {

public:

	SplashWindow();
	~SplashWindow();

	LRESULT MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;


private:

	WCHAR m_OutputMessage[MAX_NAME_STRING];

};