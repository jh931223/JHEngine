#include "stdafx.h"
#include "Timer.h"


void Timer::TimerUpdate()
{
	clock_t newT = clock();
	deltaTime = float(newT - deltaTimeCheck) / 1000.0f;
	deltaTimeCheck = newT;
}
clock_t Timer::deltaTimeCheck = 0;
float Timer::deltaTime = 0;