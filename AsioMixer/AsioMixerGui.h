#pragma once
#include "AsioMixerTrayIcon.h"
#include "AsioMixer.h"


class AsioMixerGui
{
private:
	HWND m_hWnd;
	HINSTANCE m_Instance;
	AsioMixerTrayIcon m_Tray;
	AsioMixer m_Mixer;

	HICON m_Icon;
	HWND m_CtlDrvList;
	HWND m_CtlApplyBtn;
	HWND m_CtlCplBtn;
	HWND m_Group1[8];
	HWND m_Group2[8];
	HWND m_CtlSampleRate;
	bool m_UpdateState;

	std::wstring m_SettingPath;

public:
	AsioMixerGui();
	~AsioMixerGui();

	bool Create(HINSTANCE hInst, const std::wstring& title, int nCmdShow);
	int Loop();
	void RestartAudio();
	void RestoreWindow();

private:
	void OnCreate();
	void OnDestroy();
	void OnSaveSetting();
	void OnLoadSetting();
	void OnEndSession();
	void OnResize(WORD width, WORD height, bool minimized);
	void OnApplyClicked();
	void OnTrayInteraction(LPARAM lParam);
	void UpdateState();

	ATOM MyRegisterClass();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

