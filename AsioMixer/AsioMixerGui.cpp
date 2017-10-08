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
#include "resource.h"

#define ASIOMIXER_GUI_CLASS L"ASIOMIXERSZ_CLASS"
#define SWM_TRAYINTERACTION    WM_APP + 1

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int GSampleRate[] = {
	8000,
	11025,
	22050,
	44100,
	48000,
	64000,
	88200,
	96000,
	192000,
};


AsioMixerGui::AsioMixerGui()
{
	INITCOMMONCONTROLSEX c;
	c.dwSize = sizeof(c);
	c.dwICC = 0;

	InitCommonControlsEx(&c);

	m_SettingPath = GetAppPath() + L"\\AsioMixer.ini";
}


AsioMixerGui::~AsioMixerGui()
{
}

bool AsioMixerGui::Create(HINSTANCE hInst, const std::wstring& title, int nCmdShow)
{
	m_Instance = hInst;
	SetProcessDPIAware();

	MyRegisterClass();

	m_hWnd = CreateWindowW(ASIOMIXER_GUI_CLASS, title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 700, 320, nullptr, nullptr, m_Instance, this);

	if (!m_hWnd) return false;

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	m_Mixer.SetCallback([&](long selector, long val) {
		if (selector == kAsioResetRequest) {
			PostMessage(m_hWnd, WM_NULL, 0, 0);
		}
	});

	return m_hWnd != NULL;
}

int AsioMixerGui::Loop()
{
	MSG msg;
	BOOL ret;

	while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (ret == -1) {
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
		m_Mixer.ProcessMessage();
	}

	return (int)msg.wParam;
}

void AsioMixerGui::RestartAudio()
{
	int sr = ComboBox_GetCurSel(m_CtlSampleRate);
	if (sr < 0 || sr >= _countof(GSampleRate)) {
		sr = 3;
	}

	m_Mixer.Stop();
	m_Mixer.Shutdown();

	m_Mixer.LoadDriver(ComboBox_GetCurSel(m_CtlDrvList));
	m_Mixer.Init();
	m_Mixer.SetSampleRate((ASIOSampleRate)GSampleRate[sr]);
	m_Mixer.Start();
}

void AsioMixerGui::RestoreWindow()
{
	ShowWindow(m_hWnd, SW_RESTORE);
	SetForegroundWindow(m_hWnd);
}

void AsioMixerGui::OnCreate()
{
	SetWindowLongPtr(m_hWnd, GWL_STYLE, GetWindowLongPtr(m_hWnd, GWL_STYLE) & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX);

	m_Tray.CreateIcon(m_hWnd, m_Icon, SWM_TRAYINTERACTION);
	m_Tray.SetTooltip(L"AsioMixer");

	HFONT FontBold = CreateFontA(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
	HFONT FontNormal = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
	HWND hStatic;
	
	wchar_t szTemp[32];

	hStatic = CreateWindow(L"STATIC", L"Select Driver:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 13, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDrvList = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 10, 400, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDrvList, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlApplyBtn = CreateWindow(L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 560, 9, 100, 30, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlApplyBtn, WM_SETFONT, (WPARAM)FontBold, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Sample Rate:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 45, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlSampleRate = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 45, 200, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlSampleRate, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlCplBtn = CreateWindow(L"BUTTON", L"Control", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 560, 45, 100, 30, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlCplBtn, WM_SETFONT, (WPARAM)FontBold, TRUE);

	int left = 8 * 55 / 2;
	for (int i = 0; i < 8; i++) {
		swprintf_s(szTemp, L"%d", i + 1);

		hStatic = CreateWindow(L"STATIC", szTemp, WS_CHILD | WS_VISIBLE | SS_NOPREFIX | ES_CENTER, left + (i * 35), 150, 30, 20, m_hWnd, NULL, m_Instance, NULL);
		SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

		m_Group1[i] = CreateWindow(L"EDIT", szTemp, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER | ES_CENTER, left + (i * 35), 190, 30, 25, m_hWnd, NULL, m_Instance, NULL);
		SendMessage(m_Group1[i], WM_SETFONT, (WPARAM)FontNormal, TRUE);
		m_Group2[i] = CreateWindow(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER | ES_CENTER, left + (i * 35), 220, 30, 25, m_hWnd, NULL, m_Instance, NULL);
		SendMessage(m_Group2[i], WM_SETFONT, (WPARAM)FontNormal, TRUE);
	}

	int preferredAsioDriver = 0;
	int preferredSampleRate = 3;
	try
	{
		auto drivers = m_Mixer.GetDrivers();
		for (size_t i = 0; i < drivers.size(); i++) {
			ComboBox_AddString(m_CtlDrvList, drivers[i].c_str());
			if (drivers[i].find(L"ASIO4ALL") != std::wstring::npos) {
				preferredAsioDriver = (int)i;
			}
		}
	}
	catch (const std::exception& ex)
	{
		ComboBox_AddString(m_CtlDrvList, ToWide(ex.what()).c_str());
	}


	for (size_t i = 0; i < _countof(GSampleRate); i++) {
		swprintf_s(szTemp, L"%d", GSampleRate[i]);
		ComboBox_AddString(m_CtlSampleRate, szTemp);
	}

	ComboBox_SetCurSel(m_CtlDrvList, preferredAsioDriver);
	ComboBox_SetCurSel(m_CtlSampleRate, preferredSampleRate);

	OnLoadSetting();

	try
	{
		RestartAudio();
	}
	catch (const std::exception&)
	{
	}
}

void AsioMixerGui::OnDestroy()
{
	OnSaveSetting();
	PostQuitMessage(0);
}

void AsioMixerGui::OnSaveSetting()
{
	std::wstring sTemp;
	std::wstring sTemp2;
	sTemp.resize(128);
	sTemp2.resize(128);

	ComboBox_GetText(m_CtlDrvList, &sTemp[0], (int)sTemp.size() * sizeof(wchar_t));
	WritePrivateProfileString(L"Settings", L"Driver", sTemp.c_str(), m_SettingPath.c_str());

	ComboBox_GetText(m_CtlSampleRate, &sTemp[0], (int)sTemp.size() * sizeof(wchar_t));
	WritePrivateProfileString(L"Settings", L"SampleRate", sTemp.c_str(), m_SettingPath.c_str());

	for (int i = 0; i < 8; i++) {
		Edit_GetText(m_Group1[i], &sTemp[0], (int)sTemp.size() * sizeof(wchar_t));
		swprintf_s(&sTemp2[0], 128, L"G1_%d", i);
		WritePrivateProfileString(L"Settings", sTemp2.c_str(), sTemp.c_str(), m_SettingPath.c_str());

		Edit_GetText(m_Group2[i], &sTemp[0], (int)sTemp.size() * sizeof(wchar_t));
		swprintf_s(&sTemp2[0], 128, L"G2_%d", i);
		WritePrivateProfileString(L"Settings", sTemp2.c_str(), sTemp.c_str(), m_SettingPath.c_str());
	}
}

void AsioMixerGui::OnLoadSetting()
{
	auto readSetting = [this](const std::wstring& key, const std::wstring& defaultVal = L"") {
		static std::wstring val;
		val.resize(128);

		if (GetPrivateProfileString(L"Settings", key.c_str(), defaultVal.c_str(), &val[0], (DWORD)val.size(), m_SettingPath.c_str()) == 0) {
			return defaultVal;
		}

		return val;
	};

	try
	{
		std::wstring savedDriver = readSetting(L"Driver");
		std::wstring sampleRate = readSetting(L"SampleRate");

		auto drivers = m_Mixer.GetDrivers();
		for (size_t i = 0; i < drivers.size(); i++) {
			if (wcscmp(drivers[i].c_str(), savedDriver.c_str()) == 0) {
				ComboBox_SetCurSel(m_CtlDrvList, i);
				break;
			}
		}

		for (size_t i = 0; i < _countof(GSampleRate); i++) {
			if (_wtoi(sampleRate.c_str()) == GSampleRate[i]) {
				ComboBox_SetCurSel(m_CtlSampleRate, i);
				break;
			}
		}
	}
	catch (const std::exception&)
	{
	}

	std::wstring sKey;
	std::wstring sVal;
	std::wstring sValDef;
	sKey.resize(128);
	sVal.resize(128);
	sValDef.resize(32);
	for (int i = 0; i < 8; i++) {
		swprintf_s(&sValDef[0], 32, L"%d", i + 1);

		swprintf_s(&sKey[0], 128, L"G1_%d", i);
		sVal = readSetting(sKey, sValDef);
		Edit_SetText(m_Group1[i], sVal.c_str());

		swprintf_s(&sKey[0], 128, L"G2_%d", i);
		sVal = readSetting(sKey, L"0");
		Edit_SetText(m_Group2[i], sVal.c_str());
	}
}

void AsioMixerGui::OnEndSession()
{
}

void AsioMixerGui::OnResize(WORD width, WORD height, bool minimized)
{
	if (minimized) {
		ShowWindow(m_hWnd, SW_HIDE);
	}
}

void AsioMixerGui::OnApplyClicked()
{
	try
	{
		RestartAudio();
	}
	catch (const std::exception& ex)
	{
		MessageBox(m_hWnd, ToWide(ex.what()).c_str(), L"AsioMixer encountered an error", MB_OK);
	}
}

void AsioMixerGui::OnTrayInteraction(LPARAM lParam)
{
	switch (lParam)
	{
		case WM_LBUTTONUP:
			if (IsWindowVisible(m_hWnd) && !IsIconic(m_hWnd)) {
				ShowWindow(m_hWnd, SW_HIDE);
			} else {
				ShowWindow(m_hWnd, SW_RESTORE);
				SetForegroundWindow(m_hWnd);
			}
			break;
		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:
			break;
	}
}

void AsioMixerGui::UpdateState()
{
	if (!m_UpdateState) return;
}

ATOM AsioMixerGui::MyRegisterClass()
{
	WNDCLASSEXW wcex;

	m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_ASIOMIXER));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_Instance;
	wcex.hIcon = m_Icon;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = ASIOMIXER_GUI_CLASS;
	wcex.hIconSm = wcex.hIcon;

	return RegisterClassExW(&wcex);
}

LRESULT CALLBACK AsioMixerGui::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AsioMixerGui* _this = (AsioMixerGui*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCTW pCreateParam = (LPCREATESTRUCTW)lParam;
			if (pCreateParam) {
				_this = (AsioMixerGui*)pCreateParam->lpCreateParams;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreateParam->lpCreateParams);
			}

			if (_this) {
				_this->m_hWnd = hWnd;
				_this->OnCreate();
			}
			break;
		}

		case WM_ENDSESSION:
			if (_this) _this->OnEndSession();
			break;

		case WM_DESTROY:
			if (_this) _this->OnDestroy();
			break;

		case WM_SIZE:
			if (_this) _this->OnResize(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED);
			break;

		case WM_PAINT:
			_this->UpdateState();
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;

		case WM_COMMAND:
		{
			if (_this) {
				HWND hTarget = (HWND)lParam;
				if (hTarget == _this->m_CtlApplyBtn) {
					EnableWindow(_this->m_CtlApplyBtn, FALSE);
					_this->OnApplyClicked();
					EnableWindow(_this->m_CtlApplyBtn, TRUE);
				} else if (hTarget == _this->m_CtlCplBtn) {
					_this->m_Mixer.ShowControlPanel();
				} else if (hTarget == _this->m_CtlSampleRate && HIWORD(wParam) == CBN_SELCHANGE) {
					int sr = ComboBox_GetCurSel(_this->m_CtlSampleRate);
					if (sr < 0 || sr >= _countof(GSampleRate)) {
						sr = 3;
					}

					_this->m_Mixer.SetSampleRate(GSampleRate[sr]);
				}

				wchar_t szTemp[64] = { 0 };

				for (int i = 0; i < 8; i++) {
					if (hTarget == _this->m_Group1[i]) {
						Edit_GetText(hTarget, szTemp, sizeof(szTemp) / sizeof(wchar_t));
						_this->m_Mixer.SwitchChannel(1, i, _wtoi(szTemp));
					} else if (hTarget == _this->m_Group2[i]) {
						Edit_GetText(hTarget, szTemp, sizeof(szTemp) / sizeof(wchar_t));
						_this->m_Mixer.SwitchChannel(2, i, _wtoi(szTemp));
					}
				}
			}
			break;
		}

		case SWM_TRAYINTERACTION:
			if (_this) _this->OnTrayInteraction(lParam);
			break;

		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	return 0;
}
