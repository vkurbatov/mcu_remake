#include "guard_lock.h"

namespace core
{

namespace media
{

GuardLock::GuardLock(const ISyncPoint& sync_point)
	: m_sync_point(sync_point)
	, m_lock_counter(0)
{
	Lock();
}

GuardLock::~GuardLock()
{
	Unlock();
}

void GuardLock::Lock() const
{
	// need use CAS function

	if (m_lock_counter++ == 0)
	{
		m_sync_point.Lock();
	}

}

void GuardLock::Unlock() const
{
	if (m_lock_counter > 0)
	{
		m_sync_point.Unlock();
		m_lock_counter--;
	}
}


} // media

} // core
