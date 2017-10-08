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
#include "AsioMixer.h"

AsioMixer* AsioMixer::_this = nullptr;

AsioMixer::AsioMixer()
	: m_RestartEngineRequest(false)
{
	if (_this) {
		throw std::exception("Only one instance of AsioMixer is allowed");
	}

	_this = this;
	
	m_Group1.resize(32);
	m_Group2.resize(32);
	
	for (long i = 0; i < 8; i++) {
		m_Group1[i] = i;
		m_Group2[i] = -1;
	}

	EnumerateDrivers();
}

AsioMixer::~AsioMixer()
{
	Stop();
	Shutdown();
}

void AsioMixer::ProcessMessage()
{
	if (m_RestartEngineRequest) {
		m_RestartEngineRequest = false;

		Stop();
		Shutdown();

		LoadDriver(m_LastDriverName.c_str());
		Init();
		Start();
	}
}

void AsioMixer::EnumerateDrivers()
{
	std::string sDriverName;
	sDriverName.resize(256);

	m_Drivers.clear();

	for (LONG i = 0; i < m_Driver.asioGetNumDev(); i++)
	{
		if (m_Driver.asioGetDriverName(i, &sDriverName[0], (int)sDriverName.size()) == 0)
		{
			m_Drivers.push_back(ToWide(sDriverName));
		}
		else
		{
			m_Drivers.push_back(L"Error retrieving driver information");
		}
	}
}

std::vector<std::wstring>& AsioMixer::GetDrivers()
{
	return m_Drivers;
}

bool AsioMixer::LoadDriver(const char* DriverName)
{
	m_Driver.removeCurrentDriver();
	m_LastDriverName = DriverName;
	return m_Driver.loadDriver((char*)DriverName);
}

bool AsioMixer::LoadDriver(int DriverIndex)
{
	m_Driver.removeCurrentDriver();
	m_LastDriverName = ToUtf8(m_Drivers[DriverIndex]);
	return m_Driver.loadDriver((char*)m_LastDriverName.c_str());
}

bool AsioMixer::Init()
{
	return ASIOInit(&m_DriverInfo) == ASE_OK;
}

void AsioMixer::Shutdown()
{
	m_Driver.removeCurrentDriver();
}

void AsioMixer::ShowControlPanel()
{
	ASIOControlPanel();
}

void AsioMixer::SetSampleRate(ASIOSampleRate sampleRate)
{
	m_SampleRate = sampleRate;
	ASIOSetSampleRate(sampleRate);
}

void AsioMixer::Start()
{
	m_Callbacks.asioMessage = CallbackMessage;
	m_Callbacks.bufferSwitch = CallbackBufferSwitch;
	m_Callbacks.bufferSwitchTimeInfo = CallbackBufferSwitchTimeInfo;
	m_Callbacks.sampleRateDidChange = CallbackSampleRateChange;

	long totalInput, totalOutput;
	long minSize, maxSize, prefSize, granularity;

	if (ASIOGetChannels(&totalInput, &totalOutput) != ASE_OK) {
		throw std::exception("Unable to get channels number");
	}

	ASIOSetSampleRate(m_SampleRate);

	m_Buffer.resize(totalInput + totalOutput);
	m_ChannelInfo.resize(totalInput + totalOutput);
	for (long i = 0; i < totalInput; i++) {
		m_Buffer[i].channelNum = i;
		m_Buffer[i].isInput = true;
	}
	for (long i = totalInput; i < totalInput + totalOutput; i++) {
		m_Buffer[i].channelNum = i - totalInput;
		m_Buffer[i].isInput = false;
	}

	for (size_t i = 0; i < m_ChannelInfo.size(); i++) {
		m_ChannelInfo[i].channel = m_Buffer[i].channelNum;
		m_ChannelInfo[i].isInput = m_Buffer[i].isInput;
		ASIOGetChannelInfo(&m_ChannelInfo[i]);
	}

	if (ASIOGetBufferSize(&minSize, &maxSize, &prefSize, &granularity) != ASE_OK) {
		throw std::exception("Unable to get buffer size");
	}

	bool populate = m_Group1.size() == 0;

	m_BuffSize = prefSize;
	m_Input.clear();
	m_Output.clear();
	m_Input.resize(totalInput);
	m_Output.resize(totalOutput);
	for (long i = 0; i < totalInput + totalOutput; i++) {
		if (i < totalInput) {
			m_Input[i] = &m_Buffer[i];
		} else {
			m_Output[i - totalInput] = &m_Buffer[i];
		}
	}

	if (ASIOCreateBuffers(&m_Buffer[0], (long)m_Buffer.size(), m_BuffSize, &m_Callbacks) != ASE_OK) {
		throw std::exception("Unable to create buffers");
	}

	if (ASIOStart() != ASE_OK) {
		throw std::exception("Unable to start streaming");
	}
}

void AsioMixer::Stop()
{
	ASIODisposeBuffers();
	ASIOStop();
}

void AsioMixer::SwitchChannel(int group, int outputIndex, size_t inputChannel)
{
	if (group == 1) {
		m_Group1[outputIndex] = inputChannel - 1;
	} else if (group == 2) {
		m_Group2[outputIndex] = inputChannel - 1;
	}
}

size_t AsioMixer::GetChannel(int group, int outputIndex)
{
	if (group == 1) {
		return m_Group1[outputIndex] + 1;
	} else if (group == 2) {
		return m_Group2[outputIndex] + 1;
	}

	return 0;
}

void AsioMixer::SetCallback(std::function<void(long selector, long val)> callback)
{
	m_Callback = callback;
}

long AsioMixer::CallbackMessage(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;

	switch (selector)
	{
		case kAsioSelectorSupported:
			if (value == kAsioResetRequest
				|| value == kAsioEngineVersion
				|| value == kAsioResyncRequest
				|| value == kAsioLatenciesChanged
				// the following three were added for ASIO 2.0, you don't necessarily have to support them
				|| value == kAsioSupportsTimeInfo
				|| value == kAsioSupportsTimeCode
				|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			_this->m_RestartEngineRequest = true;
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}

	if (_this->m_Callback) _this->m_Callback(selector, value);

	return ret;
}

void AsioMixer::CallbackBufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
{
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime timeInfo;
	memset(&timeInfo, 0, sizeof(timeInfo));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	CallbackBufferSwitchTimeInfo(&timeInfo, doubleBufferIndex, directProcess);
}

ASIOTime* AsioMixer::CallbackBufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
{
	// Only support int32lsb at the moment

	for (size_t i = 0; i < _this->m_Output.size(); ++i) {
		memset(_this->m_Output[i]->buffers[doubleBufferIndex], 0, _this->m_BuffSize * sizeof(int32_t));

		float mix = 0.f;
		int32_t* output = (int32_t*)_this->m_Output[i]->buffers[doubleBufferIndex];

		for (int x = 0; x < _this->m_BuffSize; x++) {
			if (_this->m_Group1[i] < _this->m_Input.size()) {
				size_t channelIndex = _this->m_Group1[i];
				if (channelIndex < _this->m_Input.size()) {
					int32_t* input = (int32_t*)_this->m_Input[channelIndex]->buffers[doubleBufferIndex];
					output[x] = input[x];
				}
			}

			if (_this->m_Group2[i] < _this->m_Input.size()) {
				size_t channelIndex = _this->m_Group2[i];
				if (channelIndex < _this->m_Input.size()) {
					int32_t* input = (int32_t*)_this->m_Input[channelIndex]->buffers[doubleBufferIndex];
					output[x] += input[x];
				}
			}
		}
	}
	/*int TotalBufferSize = 0;
	// OK do processing for the outputs only
	switch (_this->m_ChannelInfo[0].type)
	{
		case ASIOSTInt16LSB:
			TotalBufferSize = _this->m_BuffSize * 2;
			break;
		case ASIOSTInt24LSB:		// used for 20 bits as well
			TotalBufferSize = _this->m_BuffSize * 3;
			break;
		case ASIOSTInt32LSB:
			TotalBufferSize = _this->m_BuffSize * 4;
			break;
		case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
			TotalBufferSize = _this->m_BuffSize * 4;
			break;
		case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
			TotalBufferSize = _this->m_BuffSize * 8;
			break;

			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these
		case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
		case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
			TotalBufferSize = _this->m_BuffSize * 4;
			break;

		case ASIOSTInt16MSB:
			TotalBufferSize = _this->m_BuffSize * 2;
			break;
		case ASIOSTInt24MSB:		// used for 20 bits as well
			TotalBufferSize = _this->m_BuffSize * 3;
			break;
		case ASIOSTInt32MSB:
			TotalBufferSize = _this->m_BuffSize * 4;
			break;
		case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
			TotalBufferSize = _this->m_BuffSize * 4;
			break;
		case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
			TotalBufferSize = _this->m_BuffSize * 8;
			break;

			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these
		case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
		case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
			TotalBufferSize = _this->m_BuffSize * 4;
			break;
	}

	int32_t* input1 = (int32_t*)_this->m_Buffer[0].buffers[doubleBufferIndex];
	int32_t* input2 = (int32_t*)_this->m_Buffer[1].buffers[doubleBufferIndex];
	int32_t* input3 = (int32_t*)_this->m_Buffer[2].buffers[doubleBufferIndex];
	int32_t* input4 = (int32_t*)_this->m_Buffer[3].buffers[doubleBufferIndex];
	int32_t* output1 = (int32_t*)_this->m_Buffer[4].buffers[doubleBufferIndex];
	int32_t* output2 = (int32_t*)_this->m_Buffer[5].buffers[doubleBufferIndex];

	float vol = 1.f;
	for (int i = 0; i < _this->m_BuffSize; i++) {
		float n1 = ((float)input1[i] + 32767.0f) / 65536.0f;
		float n2 = ((float)input2[i] + 32767.0f) / 65536.0f;
		float n3 = ((float)input3[i] + 32767.0f) / 65536.0f;
		float n4 = ((float)input4[i] + 32767.0f) / 65536.0f;
		float left = n1 + n3;
		float right = n2 + n4;
		output1[i] = (int32_t)((left * 65536.0f - 32767.0f) * vol);
		output2[i] = (int32_t)((right * 65536.0f - 32767.0f) * vol);
	}

	//memcpy(_this->m_BuffInfo[1].buffers[doubleBufferIndex], _this->m_BuffInfo[0].buffers[doubleBufferIndex], TotalBufferSize);
	//ASIOOutputReady();*/

	return 0;
}

void AsioMixer::CallbackSampleRateChange(ASIOSampleRate rate)
{

}
