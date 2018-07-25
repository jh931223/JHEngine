#pragma once
class RWLock
{
public:
	RWLock();
	~RWLock();

	RWLock(const RWLock& rhs) = delete;
	RWLock& operator=(const RWLock& rhs) = delete;

	/// exclusive mode
	void EnterWriteLock();
	void LeaveWriteLock();

	/// shared mode
	void EnterReadLock();
	void LeaveReadLock();

	long GetLockFlag() const { return mLockFlag; }
private:
	enum LockFlag
	{
		LF_WRITE_MASK = 0x7FF00000,
		LF_WRITE_FLAG = 0x00100000,
		LF_READ_MASK = 0x000FFFFF ///< 하위 20비트를 readlock을 위한 플래그로 사용한다.
	};
	volatile long mLockFlag;
};



