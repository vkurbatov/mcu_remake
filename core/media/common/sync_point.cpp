#include "sync_point.h"

namespace core
{

namespace media
{

SyncPoint::SyncPoint(bool is_fake)
	: m_is_fake(is_fake)
{

}

void SyncPoint::Lock() const
{
	if (!m_is_fake)
	{
		m_mutex.lock();
	}
}

void SyncPoint::Unlock() const
{
	if (!m_is_fake)
	{
		m_mutex.unlock();
	}
}

bool SyncPoint::IsFake() const
{
	return m_is_fake;
}

} // media

} // core
