//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include <windows.h>
#include "GameTimer.h"

GameTimer::GameTimer()
: mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), mStopTime(0),
  mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec); // 초당 진동수, CPU 클럭
	mSecondsPerCount = 1.0 / (double)countsPerSec; // 진동 한번에 걸리는 시간, 주기
}

// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float GameTimer::TotalTime()const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// mStopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if( mStopped )
	{
		return (float)(((mStopTime - mPausedTime)-mBaseTime)*mSecondsPerCount);
	}

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime
	
	else
	{
		return (float)(((mCurrTime-mPausedTime)-mBaseTime)*mSecondsPerCount);
	}
}

float GameTimer::DeltaTime()const
{
	return (float)mDeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime; // 게임 시작 초창기 시간
	mPrevTime = currTime; // 초기화
	mStopTime = 0;
	mStopped  = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if( mStopped )
	{
		mPausedTime += (startTime - mStopTime);	// 멈췄던 시간들이 누적된다.

		mPrevTime = startTime;
		mStopTime = 0;
		mStopped  = false;
	}
}

void GameTimer::Stop()
{
	if( !mStopped )
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mStopTime = currTime;
		mStopped  = true;
	}
}

// 매 프레임마다 Run함수에서 호출
void GameTimer::Tick()
{
	if( mStopped )
	{
		mDeltaTime = 0.0;
		return;
	}

	// 이번 프레임의 시간을 얻는다.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // 진동수를 얻는 함수
	mCurrTime = currTime; // 얘는 진동수이고 시간이 아니다. 따라서 시간으로 만드려면 mSecondsPerCount 얘를 곱해주어야 한다.

	// 이 시간과 이전 프레임의 시간 차이를 구한다.
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	// 다음 프레임을 준비
	mPrevTime = mCurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	// mDeltaTime이 음수가 될 수 있기에 이를 방지
	if(mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}

