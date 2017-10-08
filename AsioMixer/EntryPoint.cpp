/*
AsioMixer - A simple mixer for ASIO devices.

Copyright (C) 2017 Syahmi Azhar.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "AsioMixerGui.h"

#define ASIOMIXER_HELP_STRING L"Options:\n /hide"


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	AsioMixerGui gui;

	LPWSTR *szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	bool HelpRequest = false;

	if (szArglist) {
		for (int i = 0; i < nArgs; i++) {
			if (_wcsicmp(szArglist[i], L"/hide") == 0 ||
				_wcsicmp(szArglist[i], L"--hide") == 0)
			{
				nCmdShow = SW_HIDE;
			}
			else if (_wcsicmp(szArglist[i], L"--help") == 0 ||
				_wcsicmp(szArglist[i], L"-h") == 0 ||
				_wcsicmp(szArglist[i], L"/h") == 0 ||
				_wcsicmp(szArglist[i], L"/?") == 0)
			{
				HelpRequest = true;
			}
		}

		if (HelpRequest) {
			if (GetConsoleWindow() == NULL) {
				MessageBoxW(NULL, ASIOMIXER_HELP_STRING, L"AsioMixer help", MB_OK);
			}
			wprintf(ASIOMIXER_HELP_STRING);
			return 0;
		}
	}

	if (!gui.Create(hInstance, L"AsioMixer", nCmdShow)) {
		return -1;
	}

	return gui.Loop();
}