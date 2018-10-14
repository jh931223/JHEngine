#pragma once
#include<time.h>
class Timer
{
public:
	static void TimerUpdate();
	static float DeltaTime() { return deltaTime; }
	static clock_t deltaTimeCheck;
	static float deltaTime;
};

