#include "pch.h"
#include "ProgressIndicator.h"

#include <iostream>
#include <iomanip>
#include <string>

ProgressIndicator::ProgressIndicator()
{
    Start();
}

void ProgressIndicator::Start()
{
    value = 0;
    startTime = std::chrono::high_resolution_clock::now();
}

void ProgressIndicator::PrintTimeElapsed()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - startTime;
    std::cout << std::setprecision(1) << std::fixed << elapsed.count() << " sec ";
}

void ProgressIndicator::PrintText(const std::wstring &text)
{
    PrintTimeElapsed();
    std::wcout << text << std::endl;
}

// Show indicator as x sec [====>] y%
void ProgressIndicator::Update(float newValue)
{
	if ((newValue != 0) && (100.0 * (newValue - value) < 0.1))
		return;

    PrintTimeElapsed();

	value = newValue;
	int pos = static_cast<int>(barWidth * value);
	std::string s(pos > 0 ? pos - 1 : 0, '=');
	s += '>';

    std::cout << "[" << s << "] " << std::setprecision(1) << std::fixed << (value * 100.0) << "%\r";
	std::cout.flush();
}
