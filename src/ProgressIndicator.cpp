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
    startTime = std::chrono::high_resolution_clock::now();
    value = 0;
}

void ProgressIndicator::Update(float newValue)
{
	if (100 * (newValue - value) < 0.5)
		return;

	value = newValue;
	size_t pos = static_cast<int>(barWidth * value);
	std::string s(pos > 0 ? pos - 1 : 0, '=');
	s += '>';

    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - startTime;
    std::cout << std::setprecision(1) << std::fixed << elapsed.count() << " sec ";
    std::cout << "[" << s << "] " << std::setprecision(1) << std::fixed << (value * 100.0) << "%\r";
	std::cout.flush();
}
