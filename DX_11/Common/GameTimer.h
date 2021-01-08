//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const;  // 초 단위
	float DeltaTime()const; // 초 단위

	void Reset(); // 메시지 루프 돌기 전에 초기화
	void Start(); // 타이머 시작, 재시작에 호출
	void Stop();  // 타이머 정지에 호출
	void Tick();  // 프레임마다 호출

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};

#endif // GAMETIMER_H