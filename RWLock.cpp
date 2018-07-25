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
		/// �ٸ����� writelock Ǯ���ٶ����� ��ٸ���.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();
		if ((InterlockedAdd(&mLockFlag, LF_WRITE_FLAG) & LF_WRITE_MASK) == LF_WRITE_FLAG)
		{
			/// �ٸ����� readlock Ǯ���ٶ����� ��ٸ���.
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
		/// �ٸ����� writelock Ǯ���ٶ����� ��ٸ���.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		//WRITE LOCK üũ
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