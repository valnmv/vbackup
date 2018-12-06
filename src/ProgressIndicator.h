#pragma once

#include <chrono>

class ProgressIndicator
{
private:
	float value = 0;
	int barWidth = 50;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
public:
    void Start();
	void Update(float newValue);
    ProgressIndicator();
};

