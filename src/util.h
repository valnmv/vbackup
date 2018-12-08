#pragma once

#include <atlbase.h>
#include <comdef.h>
#include <codecvt>
#include <string>

std::string GetLastErrorText(DWORD errorCode);
std::wstring s2ws(const std::string& str);
std::string ws2s(const std::wstring& wstr);

inline void throw_if_fail(HRESULT hr)
{
    if (FAILED(hr))
    {
        _com_error err(hr);
        std::wstring s{ err.ErrorMessage() };
        throw std::exception(ws2s(s).c_str());
    }
}

inline void check_last_error(bool result)
{
    if (!result)
    {
        DWORD errorCode = GetLastError();
        throw std::runtime_error(GetLastErrorText(errorCode));
    }
}

inline bool ends_with(std::wstring const &value, std::wstring const &ending)
{
    if (ending.size() > value.size()) 
        return false;
    
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}