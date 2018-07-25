#include "stdafx.h"
#include "RWLock.h"


RWLock::RWLock()
{
}


RWLock::~RWLock()
{
}

void RWLock::EnterWriteLock()
{
	while (true)
	{
		/// 다른놈이 writelock 풀어줄때까지 기다린다.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();
		if ((InterlockedAdd(&mLockFlag, LF_WRITE_FLAG) & LF_WRITE_MASK) == LF_WRITE_FLAG)
		{
			/// 다른놈이 readlock 풀어줄때까지 기다린다.
			while (mLockFlag & LF_READ_MASK)
				YieldProcessor();

			return;
		}
		InterlockedAdd(&mLockFlag, -LF_WRITE_FLAG);
	}
}

void RWLock::LeaveWriteLock()
{
	InterlockedAdd(&mLockFlag, -LF_WRITE_FLAG);
}

void RWLock::EnterReadLock()
{
	while (true)
	{
		/// 다른놈이 writelock 풀어줄때까지 기다린다.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		//WRITE LOCK 체크
		if ((InterlockedIncrement(&mLockFlag) & LF_WRITE_MASK) == 0)
			return;
		else
			InterlockedDecrement(&mLockFlag);
	}
}

void RWLock::LeaveReadLock()
{
	InterlockedDecrement(&mLockFlag);
}