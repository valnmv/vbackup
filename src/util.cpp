#include "pch.h"

#include "util.h"

std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

std::string GetLastErrorText(DWORD errorCode)
{
    PCHAR pwszBuffer = NULL;
    DWORD dwRet = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pwszBuffer, 0, NULL);

    if (dwRet == 0)
        return std::string("<Unknown error code>");

    std::string errorText(pwszBuffer);
    LocalFree(pwszBuffer);
    return errorText;
}
