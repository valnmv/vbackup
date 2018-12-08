#pragma once

#include <chrono>
#include <string>

class ProgressIndicator
{
private:
	float value = 0;
	int barWidth = 50;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
public:
    void Start();
    void PrintTimeElapsed();
    void PrintText(const std::wstring &text = L"");
    void Update(float newValue);
    ProgressIndicator();
};

