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
#include "AsioMixerTrayIcon.h"

// Using GUID will causes an issue, see all the details here: https://github.com/electron/electron/pull/2882
static const GUID TRAY_GUID = { 0xf5b95301, 0xd87e, 0x45fd, {0x96, 0xfb, 0xae, 0x4f, 0xfd, 0x08, 0x1d, 0xec} };

AsioMixerTrayIcon::AsioMixerTrayIcon()
{
	memset(&m_Data, 0, sizeof(m_Data));
	m_Data.cbSize = sizeof(m_Data);
	m_Data.guidItem = TRAY_GUID;
}


AsioMixerTrayIcon::~AsioMixerTrayIcon()
{
	Destroy();
}

bool AsioMixerTrayIcon::CreateIcon(HWND hWnd, HICON hIcon, UINT callbackMsg)
{
	m_Data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP/* | NIF_GUID*/;
	m_Data.hWnd = hWnd;
	wcsncpy_s(m_Data.szTip, ARRAYSIZE(m_Data.szTip), m_Tooltip.c_str(), min(ARRAYSIZE(m_Data.szTip), m_Tooltip.length()));
	m_Data.hIcon = hIcon;
	m_Data.uID = 1;
	m_Data.uCallbackMessage = callbackMsg;

	return Shell_NotifyIcon(NIM_ADD, &m_Data) > 0;
}

bool AsioMixerTrayIcon::Destroy()
{
	return Shell_NotifyIcon(NIM_DELETE, &m_Data) > 0;
}

bool AsioMixerTrayIcon::SetTooltip(const std::wstring& tooltip)
{
	m_Tooltip = tooltip;
	wcsncpy_s(m_Data.szTip, ARRAYSIZE(m_Data.szTip), m_Tooltip.c_str(), min(ARRAYSIZE(m_Data.szTip), m_Tooltip.length()));

	return Shell_NotifyIcon(NIM_MODIFY, &m_Data) > 0;
}
