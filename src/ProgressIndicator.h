#pragma once
class ProgressIndicator
{
private:
	float value = 0;
	int barWidth = 50;
public:
	void Update(float newValue);
};

