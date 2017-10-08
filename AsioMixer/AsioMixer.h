#pragma once

#include <asiosys.h>
#include <asio.h>
#include <asiodrivers.h>

class AsioMixer
{
private:
	static AsioMixer* _this;

	ASIODriverInfo m_DriverInfo;
	AsioDrivers m_Driver;
	std::string m_LastDriverName;
	std::vector<ASIOBufferInfo> m_Buffer;
	std::vector<ASIOChannelInfo> m_ChannelInfo;
	ASIOCallbacks m_Callbacks;
	ASIOSampleRate m_SampleRate;
	long m_BuffSize;
	bool m_RestartEngineRequest;

	std::vector<std::wstring> m_Drivers;

	std::vector<ASIOBufferInfo*> m_Input;
	std::vector<ASIOBufferInfo*> m_Output;
	std::vector<size_t> m_Group1;
	std::vector<size_t> m_Group2;

	std::function<void(long selector, long val)> m_Callback;

public:
	AsioMixer();
	~AsioMixer();

	void ProcessMessage();

	void EnumerateDrivers();
	std::vector<std::wstring>& GetDrivers();

	bool LoadDriver(const char* DriverName);
	bool LoadDriver(int DriverIndex);
	bool Init();
	void Shutdown();

	void ShowControlPanel();

	void SetSampleRate(ASIOSampleRate sampleRate);
	void Start();
	void Stop();
	void SwitchChannel(int group, int outputChannel, size_t inputChannel);
	size_t GetChannel(int group, int outputChannel);

	void SetCallback(std::function<void(long selector, long val)> callback);

private:
	static long CallbackMessage(long selector, long value, void* message, double* opt);
	static void CallbackBufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
	static ASIOTime* CallbackBufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	static void CallbackSampleRateChange(ASIOSampleRate rate);
};