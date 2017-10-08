#pragma once

std::wstring ToWide(const std::string& str);
std::string ToUtf8(const std::wstring& str);
std::wstring GetAppPath();