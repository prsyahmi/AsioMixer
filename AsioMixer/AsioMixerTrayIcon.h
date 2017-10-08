#pragma once

class AsioMixerTrayIcon
{
private:
	NOTIFYICONDATA m_Data;
	std::wstring m_Tooltip;

public:
	AsioMixerTrayIcon();
	~AsioMixerTrayIcon();

	bool CreateIcon(HWND hWnd, HICON hIcon, UINT callbackMsg);
	bool Destroy();

	bool SetTooltip(const std::wstring& tooltip);
};

