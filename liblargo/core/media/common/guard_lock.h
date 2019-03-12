#ifndef GUARD_LOCK_H
#define GUARD_LOCK_H

#include "core/media/common/i_sync_point.h"

#include <atomic>

namespace core
{

namespace media
{

class GuardLock : public ISyncPoint
{
    mutable std::atomic_uint_fast32_t	m_lock_counter;
    const ISyncPoint&                   m_sync_point;

public:
	GuardLock(const ISyncPoint& sync_point);
    ~GuardLock() override;

	// ISyncPoint interface
public:
	void Lock() const override;
	void Unlock() const override;
};

} // media

} // core

#endif // GUARD_LOCK_H
