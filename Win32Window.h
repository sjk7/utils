#pragma once
// Win32Window.h
// Helper functions and classes to help encapsulate making a window
// with minimal dependencies, using the Win32 API.

#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include <string>
#include <cassert>
#include <stdexcept>
#include <exception>
#include <system_error>
#include <CommCtrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifdef UNICODE
#define String std::wstring
#else
#define String std::string
#endif

// #define WIN32WINDOW_TEST 77
#pragma comment(lib, "Comctl32.lib")

namespace win32 {

void DisableCloseButton(HWND hwnd)
{
	EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE,
		MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}
void EnableCloseButton(HWND hwnd)
{
	EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE,
		MF_BYCOMMAND | MF_ENABLED);
}

void DisableMinimizeButton(HWND hwnd)
{
	SetWindowLong(hwnd, GWL_STYLE,
		GetWindowLong(hwnd, GWL_STYLE) & ~WS_MINIMIZEBOX);
}
void EnableMinimizeButton(HWND hwnd)
{
	SetWindowLong(hwnd, GWL_STYLE,
		GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX);
}
void DisableMaximizeButton(HWND hwnd)
{
	SetWindowLong(hwnd, GWL_STYLE,
		GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
}
void EnableMaximizeButton(HWND hwnd)
{
	SetWindowLong(hwnd, GWL_STYLE,
		GetWindowLong(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX);
}

// Get the position of the window relative to the entire screen
void GetRelativeCtrlRect(HWND hWnd, RECT& rc) {
	GetWindowRect(hWnd, &rc);

	// Now convert those with regards to the control's parent
	ScreenToClient(GetParent(hWnd), (LPPOINT) & ((LPPOINT)&rc)[0]);
	ScreenToClient(GetParent(hWnd), (LPPOINT) & ((LPPOINT)&rc)[1]);
}

RECT client_rect_in_screen_space(HWND const hWnd) {
	RECT rc{ 0 };
	if (!::GetClientRect(hWnd, &rc)) {
		auto const err_val{ ::GetLastError() };
		throw std::system_error(err_val, std::system_category());
	}

	::SetLastError(ERROR_SUCCESS);
	if (::MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rc), 2) == 0) {
		auto const err_val{ ::GetLastError() };
		if (err_val != ERROR_SUCCESS) {
			throw std::system_error(err_val, std::system_category());
		}
	}

	return rc;
}

HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}

void Move(int x, int y, HWND h)
{
	::SetWindowPos(h, 0, x, y, 0, 0,
		SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CenterControlHorizontally(HWND parent, HWND control) {
	RECT windowrect, cboRect;
	::GetClientRect(parent, &windowrect);
	::GetClientRect(control, &cboRect);

	int winWidth = windowrect.right - windowrect.left;
	int cboWidth = cboRect.right - cboRect.left;
	int newX = ((winWidth / 2) - (cboWidth / 2));

	Move(newX, cboRect.top, control);
}

class Exception : public std::exception {
public:
	Exception(const char* context, const char* error) : 
		std::exception(error)
		, m_ctx{context}{}

	char const* what() const override {
		m_tmp = m_ctx + std::exception::what();
		return m_tmp.c_str();
	}

private:
	std::string m_ctx;
	mutable std::string m_tmp;
};

class Window
{
	String m_className;
	HBRUSH m_BackgroundBrush = 0;
	BOOL AppClosed = FALSE;
	HWND m_hWnd = {};
	HINSTANCE m_hInstance = {};
	HFONT myFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		bool doDefault = true;
		LONG_PTR thisPtr = GetWindowLongPtr(hwnd, 0);
		if (thisPtr) {
			Window* pthis = (Window*)thisPtr;
			pthis->pWindowProc(hwnd, uMsg, wParam, lParam, doDefault);
		}
		if (doDefault)
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		else
			return 0;
	}

	LRESULT  pWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& doDefault) {

		LRESULT lr = WindowProc(hwnd, uMsg, wParam, lParam, doDefault);
		if (!doDefault) {
			return lr;
		}

		switch (uMsg)
		{

		case WM_DESTROY:
			PostQuitMessage(0);
			AppClosed = TRUE;
			return 0;

		case WM_PAINT:
		{
			if (m_BackgroundBrush == 0) {
				m_BackgroundBrush = ::CreateSolidBrush(RGB(20, 40, 20));
			}
			PAINTSTRUCT ps = {};
			HDC hdc = BeginPaint(hwnd, &ps);

			// All painting occurs here, between BeginPaint and EndPaint.
			FillRect(hdc, &ps.rcPaint, m_BackgroundBrush);
			EndPaint(hwnd, &ps);
		}
		return 0;
		}

		return 0;
	}

	HFONT myfont = {};
	void mySetFont() {
		// Get the DPI for which your window should scale to
		const UINT dpi{ GetDpiForWindow(m_hWnd) };

		// Obtain the recommended fonts, which are already correctly scaled for the current DPI
		NONCLIENTMETRICSW non_client_metrics = {};
		non_client_metrics.cbSize = sizeof(non_client_metrics);

		if (!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,
			sizeof(non_client_metrics), &non_client_metrics, 0, dpi))
		{
			// Error handling
			assert(0);
		}

		// Create an appropriate font(s)
		myfont = CreateFontIndirect(&non_client_metrics.lfMessageFont);
		assert(myfont);
		::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)myfont, TRUE);

	}

	void cleanup() {
		if (m_BackgroundBrush) {
			::DeleteObject(m_BackgroundBrush);
			m_BackgroundBrush = 0;
		}
	}

	int m_flags{ 0 };

protected:
	virtual LRESULT WindowProc(HWND, UINT, WPARAM, LPARAM, bool&) {

		return 0;
	}

public:
	HWND hWnd() const noexcept { return m_hWnd; }
	BOOL closed() const noexcept { return AppClosed; }
	HFONT font() const noexcept { return myFont; }
	HINSTANCE hInst() const noexcept { return m_hInstance; }
	operator HWND() { return(m_hWnd); }

	virtual ~Window() = default;
	Window(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(const Window& rhs) = delete;

	// Creata an instance of a Win32 window.
	Window(const TCHAR* className, const TCHAR* windowTitle, int flags = -1) : m_className{ className } {

		WNDCLASSEX wc = {};
		wc.hInstance = ::GetModuleHandle(NULL);
		wc.lpszClassName = className;
		wc.lpfnWndProc = &WindowProcStatic;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.cbWndExtra = sizeof(void*);
		
		m_hInstance = wc.hInstance;

		ATOM a = ::RegisterClassEx(&wc);
		
		if (!a) {
			auto const err_val{ ::GetLastError() };
			if (err_val != ERROR_SUCCESS) {
				auto se= std::system_error(err_val, std::system_category());
				Exception e("RegisterClassEx failed: ", se.code().message().c_str());
				throw e;
			}
		}
		assert(a);

		if (flags <= 0) {
			flags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		}
		m_flags = flags;
		HWND window = CreateWindowEx(
			0,
			className,
			windowTitle,
			flags,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			400,
			300,
			0,
			0,
			wc.hInstance,
			0);

		m_hWnd = window;
		if (!window) {
			auto const err_val{ ::GetLastError() };
			if (err_val != ERROR_SUCCESS) {
				auto se = std::system_error(err_val, std::system_category());
				Exception e("CreateWindowEx failed: ", se.code().message().c_str());
				throw e;
			}
		}
		SetWindowLongPtr(window, 0, (LONG_PTR)this);
		mySetFont();
	}


	void ShowWindow(int nCmdShow = SW_SHOW) {

		int x0 = GetSystemMetrics(SM_XVIRTUALSCREEN);
		int x1 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		HWND& hWnd = m_hWnd;

		RECT rect;
		GetWindowRect(hWnd, &rect);

		// resize and move off-screen
		SetWindowPos(hWnd, NULL, x1 - x0, 0, 0, 0, SWP_NOREDRAW);

		// show window
		::ShowWindow(hWnd, nCmdShow);

		// restore and redraw
		SetWindowPos(hWnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
	}

	// The main message loop. You don't have to call into me if you already have a
	// messagepump in your application.
	void MessagePump() const {

		BOOL bRet;
		MSG msg;

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
				if (closed()) {
					break;
				}
			}
		}
	}

}; // Win32Window class
} // namespace win32

#ifdef WIN32WINDOW_TEST
#include  "win32test.cpp"
#endif