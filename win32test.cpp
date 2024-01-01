// win32test.
#include "Win32Window.h"

class MyWindow : public win32::Window
{
public:
	MyWindow(const TCHAR* className, const TCHAR* windowTitle) 
		: win32::Window(className, windowTitle, WS_OVERLAPPEDWINDOW | WS_EX_COMPOSITED) {
		AddCombo();
		ShowWindow();
		DisableMaximizeButton(*this);
	}
protected:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& doDefault) override {
		// cout << "WindowProc in my derived class" << endl;
		printf("WindowProc in myWindow: msg %ld\n", (int)uMsg);
		switch (uMsg) {
		case WM_SIZE: {
			CenterControlHorizontally(*this, m_hWndCombo);
			break;
		}
		case WM_PARENTNOTIFY: {
			//auto x = LOWORD(wParam);
			//if (x == 1) // 2 is WM=_DESTROY coming

			break;
		}
		default: break;
		}
		return 0;
	}

	HWND m_hWndCombo = 0;

	void AddCombo() {
		int xpos = 1;            // Horizontal position of the window.
		int ypos = 1;            // Vertical position of the window.
		int nwidth = 200;          // Width of the window
		int nheight = 200;         // Height of the window
		HWND hwndParent = hWnd(); // Handle to the parent window

		HWND hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""),
			CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			xpos, ypos, nwidth, nheight, hwndParent, NULL, hInst(),
			NULL);

		m_hWndCombo = hWndComboBox;
		::SendMessage(hWndComboBox, WM_SETFONT, (WPARAM)font(), TRUE);
		CenterControlHorizontally(*this, m_hWndCombo);

	}
};


int main(int argc, char** argv) {

	try {
		MyWindow window(TEXT("MyWindowClass"), TEXT("Window Test"));
		BOOL bRet;
		MSG msg;
		InitCommonControls();

		while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				// handle the error and possibly exit
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (window.closed()) {
					break;
				}
			}
		}
		return 0;
	}
	catch (const win32::Exception& e) {
		puts("Fatal error: ");
		puts(e.what());
	}
	catch (const std::exception& se) {
		puts("Fatal error (but no context):");
		puts(se.what());
	}
}