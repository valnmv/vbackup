// Utility class, see https://blogs.msdn.microsoft.com/oldnewthing/20040520-00/?p=39243
//
#pragma once

#include <comdef.h>

// Automatic COM unitialization
class CCoInitialize
{
private:
    HRESULT hr_;
public:
    CCoInitialize() : hr_(CoInitialize(NULL)) {}
    ~CCoInitialize() { if (SUCCEEDED(hr_)) CoUninitialize(); }
};

